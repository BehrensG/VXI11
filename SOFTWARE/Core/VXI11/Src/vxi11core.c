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

TaskHandle_t vxi11_task_handler;
uint32_t vxi11_task_buffer[DEFAULT_THREAD_STACKSIZE];
StaticTask_t vxi11_task_control_block;

xQueueHandle vxi11_queue;


typedef struct
{
  ip_addr_t* addr;
  u16_t port;
}vxi11_conn_t;

vxi11_conn_t vxi11_conn;

static struct netconn* vxi11_netconn;
static struct netconn* vxi11_newnetconn;

static void vxi11_callback(struct netconn *conn, enum netconn_evt even, u16_t len)
{

	struct netbuf *buf;
	err_t err;

	if(NETCONN_EVT_RCVPLUS == even)
	{
		netconn_set_recvtimeout(conn, 1000);
		err = netconn_recv(conn, &buf);

		vxi11_conn.addr = netbuf_fromaddr(buf); // get the address of the client
		vxi11_conn.port = netbuf_fromport(buf); // get the Port of the client
	}
}

static struct netconn*  vxi11_netconn_bind( netconn_callback callback, u16_t port)
{

	static struct netconn* conn;

	conn = netconn_new_with_callback(NETCONN_TCP, callback);
	netconn_bind(conn, IP_ADDR_ANY, port);
	netconn_listen(conn);

	return conn;
}


static void vxi11_server_task(void const *argument)
{
	err_t err;
	vxi11_netconn = netconn_new(NETCONN_TCP);
	err = netconn_bind(vxi11_netconn, IP_ADDR_ANY, 1024);
	netconn_listen(vxi11_netconn);

	for (;;)
	{

	}
}

void vxi11_server_start(void)
{

	vxi11_task_handler = xTaskCreateStatic(vxi11_server_task,"vxi11_task",
			DEFAULT_THREAD_STACKSIZE, (void*)1, tskIDLE_PRIORITY,
			vxi11_task_buffer, &vxi11_task_control_block);

	vxi11_queue = xQueueCreate(1, sizeof(u_int));
}

Create_LinkResp vxi11_create_link(Create_LinkParms* create_link_parms)
{

}

