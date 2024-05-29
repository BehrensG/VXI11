/*
 * udp.c
 *
 *  Created on: May 24, 2024
 *      Author: grzegorz
 */


#include "string.h"
#include "stddef.h"
#include "cmsis_os.h"
#include "err.h"
#include "ip_addr.h"
#include "udp.h"
#include "api.h"
#include "def.h"
#include "socket.h"

#include "rpc.h"
#include "portmap.h"
#include "vxi11core.h"


TaskHandle_t pmap_udp_task_handler;
uint32_t pmap_udp_task_buffer[DEFAULT_THREAD_STACKSIZE/2];
StaticTask_t pmap_udp_task_control_block;

TaskHandle_t pmap_tcp_task_handler;
uint32_t pmap_tcp_task_buffer[DEFAULT_THREAD_STACKSIZE/2];
StaticTask_t pmap_tcp_task_control_block;

xQueueHandle pmap_udp_queue;
xQueueHandle pmap_tcp_queue;


typedef struct
{
  ip_addr_t* addr;
  u16_t port;
}portmap_conn_t;

static portmap_conn_t pmap_udp_conn;
static portmap_conn_t pmap_tcp_conn;

static struct netconn* pmap_udp_netconn;
static struct netconn* pmap_tcp_netconn;
static struct netconn* pmap_tcp_newconn;


typedef enum
{
	PMAP_NEW_UDP_DATA,
	PMAP_NEW_TCP_DATA,
	PMAP_CLOSE_TCP

}pmap_state_t;

#define IPPROTO_TCP     6
#define IPPROTO_UDP     17

static pmap_state_t pmap_udp_state;
static pmap_state_t pmap_tcp_state;

static err_t pmap_getport_proc(rpc_msg_call_t* call, void* data, uint16_t len, u32_t protocol);
static struct netconn*  pmap_netconn_bind( enum netconn_type type, netconn_callback callback, u16_t port);
static void pmap_udp_netconn_callback(struct netconn *conn, enum netconn_evt even, u16_t len);
static void pmap_tcp_netconn_callback(struct netconn *conn, enum netconn_evt even, u16_t len);


static err_t pmap_tcp_send(struct netconn* conn, rpc_msg_reply_t* reply, u32_t port)
{
	//struct netbuf *buf;

	char* payload;

	rpc_header_t header;

	err_t err;

	size_t rpc_sizes[] = {sizeof(rpc_header_t), sizeof(rpc_msg_reply_t), sizeof(u32_t)};

	size_t rpc_sum = rpc_sum_size(rpc_sizes, sizeof(rpc_sizes)/sizeof(rpc_sizes[0]));

	header.data = (rpc_sum - rpc_sizes[0]) | RPC_HEADER_LAST;
	header.data = htonl(header.data);

	void* rpc_sources[] = { &header, reply, &port};

	payload = malloc(rpc_sum);

	if(NULL != payload)
	{

		rpc_copy_memory(payload, rpc_sources, rpc_sizes, sizeof(rpc_sizes)/sizeof(rpc_sizes[0]));

		err = netconn_write(conn, payload, rpc_sum, NETCONN_NOFLAG);

		free(payload);

	}

	return err;

}

static err_t pmap_udp_send(struct netconn* conn, rpc_msg_reply_t* reply, u32_t port)
{
	struct netbuf *buf;
	void *payload;

	err_t err;

	size_t rpc_sizes[] = {sizeof(rpc_msg_reply_t), sizeof(u32_t)};
	void* rpc_data[] = {reply, &port};
	size_t rpc_sum = rpc_sum_size(rpc_sizes, sizeof(rpc_sizes)/sizeof(rpc_sizes[0]));

	buf = netbuf_new();

	if(NULL != buf)
	{
		payload = netbuf_alloc(buf, rpc_sum);

		rpc_copy_memory(payload, rpc_data, rpc_sizes, sizeof(rpc_sizes)/sizeof(rpc_sizes[0]));


		err = netconn_connect(conn, pmap_udp_conn.addr, pmap_udp_conn.port);

		err = netconn_send(conn, buf);

		netbuf_delete(buf);

		err = netconn_close(conn);
		err = netconn_delete(conn);

		pmap_udp_netconn = pmap_netconn_bind(NETCONN_UDP, pmap_udp_netconn_callback, PMAPPORT);
	}

	return err;

}


static void pmap_tcp_netconn_callback(struct netconn *conn, enum netconn_evt even, u16_t len)
{
	err_t err;

	if(NETCONN_EVT_RCVPLUS == even)
	{

		pmap_tcp_state = PMAP_NEW_TCP_DATA;
		xQueueSend(pmap_tcp_queue, &pmap_tcp_state, 1000);

	}
}

static void pmap_udp_netconn_callback(struct netconn *conn, enum netconn_evt even, u16_t len)
{

	if(NETCONN_EVT_RCVPLUS == even)
	{
		pmap_udp_state = PMAP_NEW_UDP_DATA;
		xQueueSend(pmap_udp_queue, &pmap_udp_state, 1000);
	}

}


err_t pmap_udp_recv(struct netconn *conn)
{
	struct netbuf *buf;
	void *data;
	err_t err;
	uint16_t len;
	rpc_msg_call_t rcp_msg;
#if LWIP_SO_RCVTIMEO == 1
	netconn_set_recvtimeout(conn, 1000);
#endif
	err = netconn_recv(conn, &buf);

	if (ERR_OK != err)
	{
		netbuf_delete(buf);
	}
	else
	{
		pmap_udp_conn.addr = netbuf_fromaddr(buf); // get the address of the client
		pmap_udp_conn.port = netbuf_fromport(buf); // get the Port of the client

		netbuf_data(buf, &data, &len);

		rpc_parser(data, len, NULL, &rcp_msg);

		if((CALL == rcp_msg.rm_direction))
		{
			if( PMAPPROC_GETPORT == rcp_msg.ru.RM_cmb.cb_proc)
			{
				err = pmap_getport_proc(&rcp_msg, data, len, IPPROTO_UDP);
			}
		}

		netbuf_delete(buf);

	}

	return err;

}

err_t pmap_tcp_recv(struct netconn *conn)
{
	struct netbuf *buf;
	void *data;
	err_t err;
	uint16_t len;
	rpc_msg_call_t rcp_msg;
	rpc_header_t header;


	err = netconn_recv(conn, &buf);

	if (ERR_OK != err)
	{
		netbuf_delete(buf);
	}
	else
	{
		pmap_tcp_conn.addr = netbuf_fromaddr(buf); // get the address of the client
		pmap_tcp_conn.port = netbuf_fromport(buf); // get the Port of the client

		netbuf_data(buf, &data, &len);

		rpc_parser(data, len, &header, &rcp_msg);

		if((CALL == rcp_msg.rm_direction))
		{
			if( PMAPPROC_GETPORT == rcp_msg.ru.RM_cmb.cb_proc)
			{
				err = pmap_getport_proc(&rcp_msg, data, len, IPPROTO_TCP);
			}
		}

		netbuf_delete(buf);

	}

	return err;

}


static err_t pmap_getport_proc(rpc_msg_call_t* call, void* data, uint16_t len, u32_t protocol)
{
	pmap_t pmap;
	err_t err;

	u32_t port = 1024;
	rpc_msg_reply_t reply;

	size_t offset = sizeof(struct call_body) + 2 * sizeof(u32_t);
	size_t size = 0;

	if(len > offset)
	{
		size = len - offset;
		memcpy(&pmap, &data[offset], size);

		pmap.pm_prog = ntohl(pmap.pm_prog);
		pmap.pm_vers = ntohl(pmap.pm_vers);
		pmap.pm_prot = ntohl(pmap.pm_prot);
		pmap.pm_port = ntohl(pmap.pm_port);

		port = htonl(port);

		//err = rpc_reply(&reply, call, MSG_ACCEPTED);
		reply = rpc_reply(&call->rm_xid, MSG_ACCEPTED);


		if(IPPROTO_UDP == protocol)
		{
			err = pmap_udp_send(pmap_udp_netconn, &reply, port);
		}
		else if(IPPROTO_TCP == protocol)
		{
			err = pmap_tcp_send(pmap_tcp_newconn, &reply, port);
		}

	}
		return err;
}


static struct netconn*  pmap_netconn_bind( enum netconn_type type, netconn_callback callback, u16_t port)
{
	err_t err;
	struct netconn* conn;

	conn = netconn_new_with_callback(type, callback);
	err = netconn_bind(conn, IP_ADDR_ANY, port);

	if( NETCONN_TCP == type)
	{
		netconn_listen(conn);
	#if LWIP_SO_RCVTIMEO == 1
		netconn_set_recvtimeout(conn, 1000);
	#endif
	}

	return conn;
}


static void pmap_udp_server_task(void const *argument)
{
	pmap_udp_netconn = pmap_netconn_bind(NETCONN_UDP, pmap_udp_netconn_callback, PMAPPORT);

	for (;;)
	{
		if(pdTRUE == xQueueReceive(pmap_udp_queue, &pmap_udp_state, 5U))
		{
			switch(pmap_udp_state)
			{
				case PMAP_NEW_UDP_DATA: pmap_udp_recv(pmap_udp_netconn); break;
				default : ; break;
			}

		}
		osDelay(pdMS_TO_TICKS(10));
	}
}


static void pmap_tcp_server_task(void const *argument)
{
	pmap_tcp_netconn = pmap_netconn_bind(NETCONN_TCP, pmap_tcp_netconn_callback, PMAPPORT);

	for (;;)
	{
		if(pdTRUE == xQueueReceive(pmap_tcp_queue, &pmap_tcp_state, 5U))
		{
			switch(pmap_tcp_state)
			{
				case PMAP_NEW_TCP_DATA:
				{
					netconn_accept(pmap_tcp_netconn, &pmap_tcp_newconn);
					pmap_tcp_recv(pmap_tcp_newconn);
				};break;
				case PMAP_CLOSE_TCP :
				{
					netconn_close(pmap_tcp_newconn);
					netconn_delete(pmap_tcp_newconn);
				};break;
				 default: osDelay(pdMS_TO_TICKS(10)); break;
			}

		}
		osDelay(pdMS_TO_TICKS(10));
	}
}

void pmap_udp_server_start(void)
{

	pmap_udp_task_handler = xTaskCreateStatic(pmap_udp_server_task,"pmap_udp_task",
			DEFAULT_THREAD_STACKSIZE/2, (void*)1, tskIDLE_PRIORITY,
			pmap_udp_task_buffer, &pmap_udp_task_control_block);

	pmap_udp_queue = xQueueCreate(1, sizeof(pmap_state_t));
}

void pmap_tcp_server_start(void)
{

	pmap_tcp_task_handler = xTaskCreateStatic(pmap_tcp_server_task,"pmap_tcp_task",
			DEFAULT_THREAD_STACKSIZE/2, (void*)1, tskIDLE_PRIORITY,
			pmap_tcp_task_buffer, &pmap_tcp_task_control_block);

	pmap_tcp_queue = xQueueCreate(1, sizeof(pmap_state_t));
}
