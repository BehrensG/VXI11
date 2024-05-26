/*
 * vxi11.c
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
#include "vxi11.h"

TaskHandle_t vxi11_task_handler;
uint32_t vxi11_task_buffer[DEFAULT_THREAD_STACKSIZE];
StaticTask_t vxi11_task_control_block;

xQueueHandle vxi11_queue;


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
	vxi11_state_t state;
	struct netconn* conn;
	struct netconn* newconn;
	vxi11_conn_info_t conn_info;

}vxi11_instr_t;

static vxi11_instr_t vxi11_instr;
static vxi11_state_t vxi11_state = VXI11_IDLE;

Create_LinkResp vxi11_create_link(Create_LinkParms* create_link_parms);
void vxi11_connect(vxi11_instr_t* vxi11_instr);

void vxi11_parser_create_link(void* data, u16_t len, Create_LinkParms* create_link_parms);

static void vxi11_callback(struct netconn *conn, enum netconn_evt even, u16_t len)
{

	if(NETCONN_EVT_RCVPLUS == even)
	{
		if(vxi11_instr.newconn)
		{
			vxi11_state = VXI11_MSG;
		}
		else
		{
			vxi11_state = VXI11_CONNECT;
		}

		xQueueSend(vxi11_queue, &vxi11_state, 1000);

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
	vxi11_instr->conn = NULL;
	vxi11_instr->newconn = NULL;
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


static void vxi11_server_task(void const *argument)
{
	struct netconn* conn;
	vxi11_state_t vxi11_state;
	vxi11_init(&vxi11_instr);
	vxi11_instr.conn = vxi11_bind(vxi11_callback, VXI11_PORT);

	for (;;)
	{
		vxi11_state = vx11_queue();

		switch(vxi11_state)
		{
			case VXI11_CONNECT : vxi11_connect(&vxi11_instr); break;
		}

	}
}

void vxi11_server_start(void)
{

	vxi11_task_handler = xTaskCreateStatic(vxi11_server_task,"vxi11_task",
			DEFAULT_THREAD_STACKSIZE, (void*)1, tskIDLE_PRIORITY,
			vxi11_task_buffer, &vxi11_task_control_block);

	vxi11_queue = xQueueCreate(1, sizeof(u_int));
}


void vxi11_connect(vxi11_instr_t* vxi11_instr)
{
	struct netconn *newconn;
	struct netbuf* buf;

	char* input_data;
	char* resp_data;

	u16_t len = 0;
	err_t err = ERR_OK;

	rpc_msg_t rpc_call;
	rpc_header_t rpc_header;

	Create_LinkParms create_link_parms;
	Create_LinkResp create_link_resp;

	err = netconn_accept(vxi11_instr->conn, &newconn);

	if(ERR_OK == err)
	{
		vxi11_instr->newconn = newconn;


		err = netconn_recv(vxi11_instr->newconn, &buf);

		if(err != ERR_OK)
		{
			vxi11_instr->state = VXI11_RECV_ERR;
			netconn_close(vxi11_instr->newconn);

		}

		netbuf_data(buf, &input_data, &len);

		vxi11_instr->conn_info.port = netbuf_fromport(buf);
		vxi11_instr->conn_info.addr = netbuf_fromaddr(buf);

		rpc_tcp_call(input_data, len, &rpc_call, &rpc_header);

		if(DEVICE_CORE == rpc_call.ru.RM_cmb.cb_prog)
		{
			if(CREATE_LINK == rpc_call.ru.RM_cmb.cb_proc)
			{
				vxi11_parser_create_link(input_data, len, &create_link_parms);
				create_link_resp = vxi11_create_link(&create_link_parms);

				//resp_data = malloc(sizeof(Create_LinkParms));
				//memcpy(resp_data, &create_link_resp, sizeof(Create_LinkParms));

			//	netconn_write(vxi11_instr->newconn, resp_data, sizeof(Create_LinkResp), NETCONN_NOFLAG);

			}
		}

		netbuf_delete(buf);
		free(resp_data);
	}

}

void vxi11_parser_create_link(void* data, u16_t len, Create_LinkParms* create_link_parms)
{
	size_t rpc_size = sizeof(rpc_msg_t) + sizeof(rpc_header_t);
	size_t vxi11_size = len - rpc_size;

	size_t u_int_size = sizeof(u_int);

	//memcpy(create_link_parms, data + rpc_size, vxi11_size - vxi11_device_name_size);
	memcpy(&create_link_parms->clientId, data + rpc_size, u_int_size);
	memcpy(&create_link_parms->lockDevice, data + rpc_size + u_int_size, u_int_size);
	memcpy(&create_link_parms->lock_timeout, data + rpc_size + 2*u_int_size, u_int_size);
	memcpy(&create_link_parms->device.lenght, data + rpc_size + 3*u_int_size, u_int_size);

	create_link_parms->device.lenght = ntohl(create_link_parms->device.lenght);

	create_link_parms->device.contents = (char*) malloc(create_link_parms->device.lenght);

	memcpy(create_link_parms->device.contents, &data[rpc_size + 4*u_int_size], u_int_size);

	create_link_parms->clientId = ntohl(create_link_parms->clientId);
	create_link_parms->lock_timeout = ntohl(create_link_parms->lock_timeout);

}

Create_LinkResp vxi11_create_link(Create_LinkParms* create_link_parms)
{
	Create_LinkResp link_resp;

	return link_resp;
}
