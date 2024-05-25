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
uint32_t pmap_udp_task_buffer[DEFAULT_THREAD_STACKSIZE];
StaticTask_t pmap_udp_task_control_block;

TaskHandle_t pmap_tcp_task_handler;
uint32_t pmap_tcp_task_buffer[DEFAULT_THREAD_STACKSIZE];
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



typedef enum
{
	PMAP_NEW_UDP_DATA,
	PMAP_NEW_TCP_DATA

}pmap_state_t;


static pmap_state_t pmap_udp_state;
static pmap_state_t pmap_tcp_state;

static err_t pmap_getport_proc(rpc_msg_t* rpc_msg, void* data, uint16_t len, u32_t protocol);
static struct netconn*  pmap_netconn_bind( enum netconn_type type, netconn_callback callback, u16_t port);
static void pmap_udp_netconn_callback(struct netconn *conn, enum netconn_evt even, u16_t len);
static void pmap_tcp_netconn_callback(struct netconn *conn, enum netconn_evt even, u16_t len);

#define pmap_udp_bind() pmap_netconn_bind(NETCONN_UDP, pmap_udp_netconn_callback, PMAPPORT)
#define pmap_tcp_bind() pmap_netconn_bind(NETCONN_TCP, pmap_tcp_netconn_callback, PMAPPORT)


static err_t pmap_udp_send(struct netconn* conn, rpc_msg_t* reply, u32_t port)
{
	struct netbuf *buf;
	void *payload;

	err_t err;

	size_t offset = sizeof(struct accepted_reply) + sizeof(u32_t);
	size_t size = offset + sizeof(u32_t);

	buf = netbuf_new();

	if(NULL != buf)
	{
		payload = netbuf_alloc(buf, size);

		memcpy(payload, reply, size);
		memcpy(payload + offset, &port, sizeof(u32_t));

		err = netconn_connect(conn, pmap_udp_conn.addr, pmap_udp_conn.port);

		err = netconn_send(conn, buf);

		netbuf_delete(buf);

		err = netconn_close(conn);
		err = netconn_delete(conn);
		pmap_udp_netconn = pmap_udp_bind();
	}

	return err;

}


static void pmap_tcp_netconn_callback(struct netconn *conn, enum netconn_evt even, u16_t len)
{
	if (NETCONN_EVT_RCVPLUS == even)
	{
		pmap_tcp_state = PMAP_NEW_TCP_DATA;
		 xQueueSend(pmap_udp_queue, &pmap_tcp_state, 1000);
	}
}

static void pmap_udp_netconn_callback(struct netconn *conn, enum netconn_evt even, u16_t len)
{

	if (NETCONN_EVT_RCVPLUS == even)
	{
		pmap_udp_state = PMAP_NEW_UDP_DATA;
		 xQueueSend(pmap_udp_queue, &pmap_udp_state, 1000);
	}

}


err_t pmap_netconn_recv(struct netconn *conn, u32_t protocol)
{
	struct netbuf *buf;
	void *data;
	err_t err;
	uint16_t len;
	rpc_msg_t rcp_msg;

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

		err = rpc_netconn_call(data, len, &rcp_msg);

		if((CALL == rcp_msg.rm_direction))
		{
			if( PMAPPROC_GETPORT == rcp_msg.ru.RM_cmb.cb_proc)
			{
				err = pmap_getport_proc(&rcp_msg, data, len, protocol);
			}
		}

		netbuf_delete(buf);

	}

	return err;

}
//static err_t pmap_getport_proc(rpc_msg_t* rpc_msg, )

static err_t pmap_getport_proc(rpc_msg_t* call, void* data, uint16_t len, u32_t protocol)
{
	pmap_t pmap;
	err_t err;

	rpc_msg_t reply;
	u32_t port = 1024;

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

		err = rpc_netconn_reply(call,&reply, 1);

		if(IPPROTO_UDP == protocol)
		{
			err = pmap_udp_send(pmap_udp_netconn, &reply, port);
		}
		else if(IPPROTO_TCP == protocol)
		{

		}

	}
		return err;
}


static struct netconn*  pmap_netconn_bind( enum netconn_type type, netconn_callback callback, u16_t port)
{
	err_t err;
	static struct netconn* conn;

	conn = netconn_new_with_callback(type, callback);
	err = netconn_bind(conn, IP_ADDR_ANY, port);

	if( NETCONN_TCP == type)
	{
		netconn_listen(conn);
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
				case PMAP_NEW_UDP_DATA: pmap_netconn_recv(pmap_udp_netconn, IPPROTO_UDP); break;
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
				case PMAP_NEW_TCP_DATA: pmap_netconn_recv(pmap_tcp_netconn, IPPROTO_TCP); break;
			}

		}
		osDelay(pdMS_TO_TICKS(10));
	}
}

void pmap_udp_server_start(void)
{

	pmap_udp_task_handler = xTaskCreateStatic(pmap_udp_server_task,"pmap_udp_task",
			DEFAULT_THREAD_STACKSIZE, (void*)1, tskIDLE_PRIORITY,
			pmap_udp_task_buffer, &pmap_udp_task_control_block);

	pmap_udp_queue = xQueueCreate(1, sizeof(pmap_state_t));
}

void pmap_tcp_server_start(void)
{

	pmap_tcp_task_handler = xTaskCreateStatic(pmap_tcp_server_task,"pmap_tcp_task",
			DEFAULT_THREAD_STACKSIZE, (void*)1, tskIDLE_PRIORITY,
			pmap_tcp_task_buffer, &pmap_tcp_task_control_block);

	pmap_tcp_queue = xQueueCreate(1, sizeof(pmap_state_t));
}
