/*
 * vxi11_destroy_link.c
 *
 *  Created on: May 28, 2024
 *      Author: BehrensG
 */


#include "string.h"
#include "err.h"
#include "api.h"
#include "def.h"

#include "rpc.h"
#include "portmap.h"
#include "vxi11.h"


Device_Error destroy_link(Device_Link* device_link);
err_t vxi11_destroy_link_parser(void* data, u16_t len, Device_Link* device_link);

Device_Error vxi11_destroy_link(vxi11_instr_t* vxi11_instr)
{

	Device_Link device_link;
	Device_Error device_error;

	rpc_msg_call_t rpc_msg_call;
	rpc_msg_reply_t rpc_msg_reply;
	rpc_header_t rpc_header;


	rpc_tcp_call_parser(vxi11_instr->core.netbuf.data, vxi11_instr->core.netbuf.len, &rpc_msg_call, &rpc_header);

	if(ERR_OK == vxi11_destroy_link_parser(vxi11_instr->core.netbuf.data, vxi11_instr->core.netbuf.len, &device_link))
	{

		device_error = destroy_link(&device_link);


		rpc_msg_reply_t rpc_msg_reply = rpc_reply(rpc_msg_call.rm_xid, MSG_ACCEPTED);

		size_t sizes[] = {sizeof(rpc_header_t), sizeof(rpc_msg_reply_t), sizeof(Device_Error)};
		void *sources[] = { &rpc_header, &rpc_msg_reply, &device_error};

		vxi11_instr->core.netbuf.len = rpc_sum_size(sizes, sizeof(sizes)/sizeof(sizes[0]));

		rpc_header.data = (vxi11_instr->core.netbuf.len - sizes[0]) | RPC_HEADER_LAST;

		rpc_header.data = htonl(rpc_header.data);


		rpc_copy_memory(vxi11_instr->core.netbuf.data, sources, sizes, sizeof(sizes)/sizeof(sizes[0]));

		netconn_write(vxi11_instr->core.netconn.newconn, vxi11_instr->core.netbuf.data, vxi11_instr->core.netbuf.len, NETCONN_NOFLAG);
		HAL_Delay(1);
		netconn_shutdown(vxi11_instr->core.netconn.newconn, 1,1);
		netconn_close(vxi11_instr->core.netconn.newconn);

		netconn_delete(vxi11_instr->core.netconn.newconn);

	}

	return device_error;
}


err_t vxi11_destroy_link_parser(void* data, u16_t len, Device_Link* device_link)
{
	size_t sizes[] = {sizeof(rpc_msg_call_t), sizeof(rpc_header_t)};

	u32_t rpc_msg_end = rpc_sum_size(sizes, sizeof(sizes)/sizeof(sizes[0]));

	memcpy(device_link, data + rpc_msg_end, sizeof(u32_t));

	device_link = ntohl(device_link);

	return ERR_OK;
}

Device_Error destroy_link(Device_Link* device_link)
{
	Device_Error device_error;

	device_error.error = 0;

	return device_error;
}
