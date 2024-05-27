/*
 * vxi11.c
 *
 *  Created on: May 24, 2024
 *      Author: grzegorz
 */

#include <stddef.h>

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
#include "vxi11.h"

TaskHandle_t vxi11_core_handler;
uint32_t vxi11_core_buffer[DEFAULT_THREAD_STACKSIZE];
StaticTask_t vxi11_core_control_block;

TaskHandle_t vxi11_abort_handler;
uint32_t vxi11_abort_buffer[DEFAULT_THREAD_STACKSIZE/2];
StaticTask_t vxi11_abort_control_block;

xQueueHandle vxi11_queue;


typedef struct
{
	char* buffer;
	u32_t len;
}netconn_data_t;

typedef struct
{
  ip_addr_t* addr;
  u16_t port;
}vxi11_conn_info_t;

typedef enum
{
	VXI11_IDLE,
	VXI11_ERROR,
	VXI11_CONNECT,
	VXI11_RECV_ERR,
	VXI11_MSG

}vxi11_state_t;


typedef struct
{
	struct netconn* conn;
	struct netconn* newconn;
}vxi11_netconn_t;


typedef struct
{
	vxi11_state_t state;
	vxi11_netconn_t core;
	vxi11_netconn_t abort;
	vxi11_conn_info_t conn_info;
	Create_LinkParms create_link_parms;
	Create_LinkResp create_link_resp;
	Device_GenericParms device_clear;

}vxi11_instr_t;


static vxi11_instr_t vxi11_instr;
static vxi11_state_t vxi11_state = VXI11_IDLE;
static Device_Link lid = VXI11_LINKID_DEFAULT;

Device_Error vxi11_device_clear(Device_GenericParms* device_generic_parms);


void vxi11_copy_memory(void **sources, size_t *sizes, u_int num_sources, char *destination);
u_int vx11_sum_size(u_int* sizes, u_int len);



void vxi11_connect(vxi11_instr_t* vxi11_instr);


static void vxi11_core_callback(struct netconn *conn, enum netconn_evt even, u16_t len)
{

	if(NETCONN_EVT_RCVPLUS == even)
	{
		if(NULL == vxi11_instr.core.newconn)
		{
			vxi11_state = VXI11_CONNECT;
		}
		else
		{
			vxi11_state = VXI11_MSG;
		}

		xQueueSend(vxi11_queue, &vxi11_state, 1000);

	}
}

static void vxi11_abort_callback(struct netconn *conn, enum netconn_evt even, u16_t len)
{

	if(NETCONN_EVT_RCVPLUS == even)
	{

	}
}

static struct netconn*  vxi11_bind( netconn_callback callback, u16_t port)
{

	struct netconn* conn;
	err_t err;

	conn = netconn_new_with_callback(NETCONN_TCP, callback);
	err = netconn_bind(conn, IP_ADDR_ANY, port);
	err = netconn_listen(conn);
#if LWIP_SO_RCVTIMEO == 1
	netconn_set_recvtimeout(conn, 20000);
#endif
	return conn;
}

static void vxi11_init(vxi11_instr_t* vxi11_instr)
{
	vxi11_instr->state = VXI11_IDLE;
	vxi11_instr->core.conn = NULL;
	vxi11_instr->core.newconn = NULL;
	vxi11_instr->abort.conn = NULL;
	vxi11_instr->abort.newconn = NULL;
	vxi11_instr->conn_info.port = 0;
	vxi11_instr->conn_info.addr = NULL;


}

static vxi11_state_t vx11_queue()
{

	vxi11_state_t vxi11_state;
	if(pdTRUE == xQueueReceive(vxi11_queue, &vxi11_state, 1000U))
	{
		vxi11_state = vxi11_state;
	}
	else
	{
		vxi11_state= VXI11_IDLE;
	}

	return vxi11_state;
}

// ------------------------------------------------------------------------------------------------------------------------

Create_LinkResp vxi11_create_link(Create_LinkParms* create_link_parms);
err_t vxi11_create_link_parser(void* data, u16_t len, Create_LinkParms* create_link_parms);

void vxi11_core_connect(vxi11_instr_t* vxi11_instr)
{
	struct netconn *newconn;
	struct netbuf* buf;

	netconn_data_t netconn_call;
	netconn_data_t netconn_reply;

	err_t err = ERR_OK;

	rpc_msg_t rpc_msg_call;
	rpc_msg_t rpc_msg_reply;
	rpc_header_t rpc_header;



	err = netconn_accept(vxi11_instr->core.conn, &newconn);

	if(ERR_OK == err)
	{
		vxi11_instr->core.newconn = newconn;


		err = netconn_recv(vxi11_instr->core.newconn, &buf);

		if(err != ERR_OK)
		{
			vxi11_instr->state = VXI11_RECV_ERR;
			netconn_close(vxi11_instr->core.newconn);

		}

		netbuf_data(buf, &netconn_call.buffer, &netconn_call.len);

		rpc_tcp_call_parser(netconn_call.buffer, netconn_call.len, &rpc_msg_call, &rpc_header);

		if(DEVICE_CORE == rpc_msg_call.ru.RM_cmb.cb_prog)
		{
			if(CREATE_LINK == rpc_msg_call.ru.RM_cmb.cb_proc)
			{
				if(ERR_OK == vxi11_create_link_parser(netconn_call.buffer, netconn_call.len, &vxi11_instr->create_link_parms))
				{
					vxi11_instr->create_link_resp = vxi11_create_link(&vxi11_instr->create_link_parms);
					rpc_reply(&rpc_msg_call, &rpc_msg_reply,MSG_ACCEPTED);


					size_t sizes[] = {sizeof(rpc_header_t), sizeof(struct accepted_reply) + sizeof(u32_t), sizeof(Create_LinkResp)};
					void *sources[] = { &rpc_header, &rpc_msg_reply, &vxi11_instr->create_link_resp};

					netconn_reply.len = vx11_sum_size(sizes, sizeof(sizes)/sizeof(sizes[0]));

					rpc_header.data = netconn_reply.len  - sizes[0] | RPC_HEADER_LAST;
					rpc_header.data = htonl(rpc_header.data);
					netconn_reply.buffer = (char*)malloc(netconn_reply.len);

					vxi11_copy_memory(sources, sizes, sizeof(sizes)/sizeof(sizes[0]), netconn_reply.buffer);

					netconn_write(vxi11_instr->core.newconn, netconn_reply.buffer, netconn_reply.len, NETCONN_NOFLAG);

					free(netconn_reply.buffer);
				}

			}
		}

		netbuf_delete(buf);

	}

}

err_t vxi11_create_link_parser(void* data, u16_t len, Create_LinkParms* create_link_parms)
{
	size_t rpc_size = sizeof(rpc_msg_t) + sizeof(rpc_header_t);
	size_t u_int_size = sizeof(u_int);

	if(rpc_size >= len)
	{
		return ERR_BUF;
	}

	memcpy(&create_link_parms->clientId, data + rpc_size, u_int_size);
	memcpy(&create_link_parms->lockDevice, data + rpc_size + u_int_size, u_int_size);
	memcpy(&create_link_parms->lock_timeout, data + rpc_size + 2*u_int_size, u_int_size);
	memcpy(&create_link_parms->device.lenght, data + rpc_size + 3*u_int_size, u_int_size);

	create_link_parms->device.lenght = ntohl(create_link_parms->device.lenght);
	create_link_parms->clientId = ntohl(create_link_parms->clientId);
	create_link_parms->lock_timeout = ntohl(create_link_parms->lock_timeout);

	// Don't have to swap bytes
	memset(create_link_parms->device.contents, 0, VXI11_MAX_DEVICE_NAME);
	memcpy(create_link_parms->device.contents, data + rpc_size + 4*u_int_size, create_link_parms->device.lenght);

	return ERR_OK;

}

Create_LinkResp vxi11_create_link(Create_LinkParms* create_link_parms)
{
	Create_LinkResp create_link_resp;

	lid+=1;

	create_link_resp.abortPort = VXI11_ABORT_PORT;
	create_link_resp.error = NO_ERR;
	create_link_resp.maxRecvSize = VXI11_MAX_RECV_SIZE;
	create_link_resp.lid = lid;

	create_link_resp.abortPort = htonl(create_link_resp.abortPort);
	create_link_resp.error = htonl(create_link_resp.error);
	create_link_resp.maxRecvSize = htonl(create_link_resp.maxRecvSize);
	create_link_resp.lid = htonl(create_link_resp.lid);


	return create_link_resp;
}

// ------------------------------------------------------------------------------------------------------------------------


void vxi11_core_write(vxi11_instr_t* vxi11_instr)
{
	struct netconn *newconn;
	struct netbuf* buf;

	netconn_data_t netconn_call;
	netconn_data_t netconn_reply;

	err_t err = ERR_OK;

	rpc_msg_t rpc_msg_call;
	rpc_msg_t rpc_msg_reply;
	rpc_header_t rpc_header;




	if(ERR_OK == err)
	{

		err = netconn_recv(vxi11_instr->core.newconn, &buf);

		if(err != ERR_OK)
		{
			vxi11_instr->state = VXI11_RECV_ERR;
			netconn_close(vxi11_instr->core.newconn);

		}

		netbuf_data(buf, &netconn_call.buffer, &netconn_call.len);

		rpc_tcp_call_parser(netconn_call.buffer, netconn_call.len, &rpc_msg_call, &rpc_header);

		if(DEVICE_CORE == rpc_msg_call.ru.RM_cmb.cb_prog)
		{
			if(CREATE_LINK == rpc_msg_call.ru.RM_cmb.cb_proc)
			{
				if(ERR_OK == vxi11_create_link_parser(netconn_call.buffer, netconn_call.len, &vxi11_instr->create_link_parms))
				{
					vxi11_instr->create_link_resp = vxi11_create_link(&vxi11_instr->create_link_parms);
					rpc_reply(&rpc_msg_call, &rpc_msg_reply,MSG_ACCEPTED);


					size_t sizes[] = {sizeof(rpc_header_t), sizeof(struct accepted_reply), sizeof(Create_LinkResp)};
					void *sources[] = { &rpc_header, &rpc_msg_reply, &vxi11_instr->create_link_resp};

					for(u_char x = 0; x < sizeof(sizes)/sizeof(sizes[0]); x++)
					{
						netconn_reply.len += sizes[x];
					}

					rpc_header.data = netconn_reply.len - sizes[2] | RPC_HEADER_LAST;
					rpc_header.data = htonl(rpc_header.data);
					netconn_reply.buffer = (char*)malloc(netconn_reply.len);

					vxi11_copy_memory(sources, sizes, sizeof(sizes)/sizeof(sizes[0]), netconn_reply.buffer);

					netconn_write(vxi11_instr->core.newconn, netconn_reply.buffer, netconn_reply.len, NETCONN_NOFLAG);

					free(netconn_reply.buffer);
				}

			}
		}

		netbuf_delete(buf);

	}

}


err_t vxi11_write_parser(netconn_data_t netconn_data, Device_WriteParms* device_write_parms)
{
	size_t sizes[] = {sizeof(rpc_header_t), sizeof(rpc_msg_t)};
	size_t rpc_msg_size = vx11_sum_size(sizes, sizeof(sizes)/sizeof(sizes[0]));




	return ERR_OK;

}
// ------------------------------------------------------------------------------------------------------------------------

err_t vxi11_device_clear_parser(void* data, u16_t len, Device_GenericParms* device_generic_parms);
Device_Error vxi11_device_clear(Device_GenericParms* device_generic_parms);


void vx11_core_clear(vxi11_instr_t* vxi11_instr)
{


}

err_t vxi11_device_clear_parser(void* data, u16_t len, Device_GenericParms* device_generic_parms)
{
	size_t rpc_size = sizeof(rpc_msg_t) + sizeof(rpc_header_t);
	size_t u_int_size = sizeof(u_int);

	if(rpc_size >= len)
	{
		return ERR_BUF;
	}

	memcpy(&device_generic_parms->lid, data + rpc_size, u_int_size);
	memcpy(&device_generic_parms->flags, data + rpc_size + u_int_size, u_int_size);
	memcpy(&device_generic_parms->io_timeout, data + rpc_size + 2*u_int_size, u_int_size);
	memcpy(&device_generic_parms->lock_timeout, data + rpc_size + 3*u_int_size, u_int_size);

	device_generic_parms->lid = ntohl(device_generic_parms->lid);
	device_generic_parms->flags = ntohl(device_generic_parms->flags);
	device_generic_parms->io_timeout = ntohl(device_generic_parms->io_timeout);
	device_generic_parms->lock_timeout = ntohl(device_generic_parms->lock_timeout);

	return ERR_OK;
}

Device_Error vxi11_device_clear(Device_GenericParms* device_generic_parms)
{
	Device_Error error;
	error.error = 0;

	return error;
}


static void vxi11_core_task(void const *argument)
{
	vxi11_state_t vxi11_state;
	vxi11_instr.core.conn = vxi11_bind(vxi11_core_callback, VXI11_PORT);

	for (;;)
	{
		vxi11_state = vx11_queue();

		switch(vxi11_state)
		{
			case VXI11_CONNECT : vxi11_core_connect(&vxi11_instr); break;
			default : osDelay(pdMS_TO_TICKS(10)); break;
		}

	}
}



static void vxi11_abort_task(void const *argument)
{

	vxi11_instr.abort.conn = vxi11_bind(vxi11_abort_callback, VXI11_ABORT_PORT);

	for (;;)
	{
		osDelay(pdMS_TO_TICKS(50));
	}
}

void vxi11_server_start(void)
{
	vxi11_init(&vxi11_instr);

	vxi11_core_handler = xTaskCreateStatic(vxi11_core_task,"vxi11_core_task",
			DEFAULT_THREAD_STACKSIZE, (void*)1, tskIDLE_PRIORITY,
			vxi11_core_buffer, &vxi11_core_control_block);

	vxi11_abort_handler = xTaskCreateStatic(vxi11_abort_task,"vxi11_abort_task",
			DEFAULT_THREAD_STACKSIZE/2, (void*)1, tskIDLE_PRIORITY,
			vxi11_abort_buffer, &vxi11_abort_control_block);

	vxi11_queue = xQueueCreate(1, sizeof(u_int));
}


//---------------------------------------------------------------------------------------------------------------
// Utils

u_int vx11_sum_size(u_int* sizes, u_int len)
{
	u_int sum = 0;

	for(u_char i = 0; i < len; i++)
	{
		sum += sizes[i];
	}

	return sum;
}

void vxi11_copy_memory(void** sources, size_t* sizes, u_int num_sources, char* destination)
{
    size_t offset = 0;
    for (u_int i = 0; i < num_sources; i++)
    {
        memcpy(destination + offset, sources[i], sizes[i]);
        offset += sizes[i];
    }
}
