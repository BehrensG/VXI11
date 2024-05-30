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
uint32_t vxi11_core_buffer[DEFAULT_THREAD_STACKSIZE*2];
StaticTask_t vxi11_core_control_block;

TaskHandle_t vxi11_abort_handler;
uint32_t vxi11_abort_buffer[DEFAULT_THREAD_STACKSIZE/2];
StaticTask_t vxi11_abort_control_block;

typedef enum
{
	VXI11_ERROR = 0,
	VXI11_NONE = 1,
	VXI11_CREATE_LINK =  CREATE_LINK,
	VXI11_DEVICE_WRITE =  DEVICE_WRITE,
	VXI11_DEVICE_READ =  DEVICE_READ,
	VXI11_DEVICE_READ_STB =  DEVICE_READ_STB,
	VXI11_DEVICE_TRIGGER = DEVICE_TRIGGER,
	VXI11_DEVICE_CLEAR = DEVICE_CLEAR,
	VXI11_DESTROY_LINK = DESTROY_LINK

}vx11_procedure_t;



xQueueHandle vxi11_queue;
xQueueHandle netbuf_call_queue;

//vxi11_netbuf_t vxi11_netbuf_call;
//vxi11_netconn_t	vxi11_netconn;

vxi11_instr_t vxi11_instr;

static vxi11_state_t vxi11_state = VXI11_IDLE;
static struct netconn*  vxi11_bind( netconn_callback callback, u16_t port);
static vxi11_state_t vx11_queue();




// ------------------------------------------------------------------------------------------------------------

// New code

vx11_procedure_t vx11_netconn_parser(vxi11_instr_t* vxi11_instr)
{

	vx11_procedure_t vxi11_procedure;

	rpc_msg_call_t call;
	rpc_header_t header;


	if(ERR_OK == vxi11_instr->core.netbuf.err)
	{
		rpc_tcp_call_parser(vxi11_instr->core.netbuf.data, vxi11_instr->core.netbuf.len, &call, &header);

		if(DEVICE_CORE == call.ru.RM_cmb.cb_prog)
		{
			return (vx11_procedure_t)call.ru.RM_cmb.cb_proc;
		}
		else
		{
			vxi11_procedure = VXI11_NONE;
		}

	}
	else
	{
		vxi11_procedure = VXI11_ERROR;
	}

	return vxi11_procedure;
}


static void vxi11_core_callback_v2(struct netconn *conn, enum netconn_evt even, u16_t len)
{

	if(NETCONN_EVT_RCVPLUS == even)
	{
		if(NULL == vxi11_instr.core.netconn.newconn)
		{
			struct netconn *newconn;
			netconn_accept(vxi11_instr.core.netconn.conn, &newconn);
			vxi11_instr.core.netconn.newconn = newconn;
		}
		else
		{
			vxi11_state = VXI11_RECV;
			xQueueSend(vxi11_queue, &vxi11_state, 1000);
		}

	}
}


vx11_procedure_t vxi11_netconn_recv(vxi11_instr_t* vxi11_instr)
{
	vx11_procedure_t vx11_procedure;

	err_t err;
	struct netbuf* buf;
	void* data;
	u16_t len = 0;
	u16_t offset = 0;

	vxi11_instr->core.netbuf.len = 0;

	netconn_set_recvtimeout(vxi11_instr->core.netconn.newconn, 1000);
	err = netconn_recv(vxi11_instr->core.netconn.newconn, &buf);

	if(ERR_OK == err)
	{
		do
		{
			netbuf_data(buf, &data, &len);
			vxi11_instr->core.netbuf.len +=len;

			if(vxi11_instr->core.netbuf.len <= sizeof(vxi11_instr->core.netbuf.data))
			{
				memcpy(vxi11_instr->core.netbuf.data + offset, data, len);

			}
			else
			{
				vxi11_instr->core.netbuf.err = ERR_MEM;
			}
			offset += len;


		}
		while(netbuf_next(buf) >= 0);

		netbuf_delete(buf);

		vx11_procedure = vx11_netconn_parser(vxi11_instr);
	}
	else
	{
		netconn_close(vxi11_instr->core.netconn.newconn);
		netconn_delete(vxi11_instr->core.netconn.newconn);
		vx11_procedure = VXI11_ERROR;
	}

	return vx11_procedure;

}

static void vxi11_core_task_v2(void const *argument)
{
	//struct netconn *newconn;

	vxi11_instr.core.netconn.conn = vxi11_bind(vxi11_core_callback_v2, VXI11_PORT);

	for (;;)
	{
		if(VXI11_RECV == vx11_queue())
		{
			switch(vxi11_netconn_recv(&vxi11_instr))
			{
				case VXI11_CREATE_LINK : vxi11_create_link(&vxi11_instr); break;
				case VXI11_DEVICE_WRITE : vxi11_device_write(&vxi11_instr); break;
				case VXI11_DEVICE_READ : vxi11_device_read(&vxi11_instr); break;
				case VXI11_DESTROY_LINK : vxi11_destroy_link(&vxi11_instr); break;
				default :/* DO NOTHING HERE, DON'T ADD WAITS ! */ ; break;
			}
		}
		else
		{

		}
	}
}

// ------------------------------------------------------------------------------------------------------------


static void vxi11_core_callback(struct netconn *conn, enum netconn_evt even, u16_t len)
{

	if(NETCONN_EVT_RCVPLUS == even)
	{
		if(NULL == vxi11_instr.core.netconn.newconn)
		{
			vxi11_state = VXI11_CONNECT;
		}
		else
		{
			vxi11_state = VXI11_MSG;
		}

		xQueueSend(vxi11_queue, &vxi11_state, 10);

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
	//netconn_set_recvtimeout(conn, 1000);
#endif
	return conn;
}

static void vxi11_init(vxi11_instr_t* vxi11_instr)
{
	vxi11_instr->state = VXI11_IDLE;


}

static vxi11_state_t vx11_queue()
{

	vxi11_state_t vxi11_state;
	if(pdTRUE == xQueueReceive(vxi11_queue, &vxi11_state, 10000U))
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

Device_Error vxi11_device_clear(Device_GenericParms* device_generic_parms)
{
	Device_Error error;
	error.error = 0;

	return error;
}


static void vxi11_core_task(void const *argument)
{
	vxi11_state_t vxi11_state;
	vxi11_instr.core.netconn.conn = vxi11_bind(vxi11_core_callback, VXI11_PORT);

	for (;;)
	{
		vxi11_state = vx11_queue();

		switch(vxi11_state)
		{
			case VXI11_CONNECT : vxi11_core_connect(&vxi11_instr); break;
			default : osDelay(pdMS_TO_TICKS(1)); break;
		}

	}
}



static void vxi11_abort_task(void const *argument)
{

	vxi11_instr.abort.netconn.conn = vxi11_bind(vxi11_abort_callback, VXI11_ABORT_PORT);

	for (;;)
	{
		osDelay(pdMS_TO_TICKS(5));
	}
}

void vxi11_server_start(void)
{
	vxi11_init(&vxi11_instr);

	vxi11_core_handler = xTaskCreateStatic(vxi11_core_task_v2,"vxi11_core_task",
			DEFAULT_THREAD_STACKSIZE, (void*)1, tskIDLE_PRIORITY,
			vxi11_core_buffer, &vxi11_core_control_block);
/*
	vxi11_abort_handler = xTaskCreateStatic(vxi11_abort_task,"vxi11_abort_task",
			DEFAULT_THREAD_STACKSIZE/2, (void*)1, tskIDLE_PRIORITY,
			vxi11_abort_buffer, &vxi11_abort_control_block);
*/
	vxi11_queue = xQueueCreate(1, sizeof(u_int));
	netbuf_call_queue = xQueueCreate(1, sizeof(vxi11_netbuf_t));
}


// ---------------------------------------------------------------------------------------------------------------
