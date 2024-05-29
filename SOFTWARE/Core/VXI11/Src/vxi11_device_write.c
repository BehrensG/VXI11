/*
 * vxi11_device_write.c
 *
 *  Created on: May 28, 2024
 *      Author: BehrensG
 */


#include <stddef.h>

#include "string.h"
#include "err.h"
#include "api.h"
#include "def.h"

#include "rpc.h"
#include "portmap.h"
#include "vxi11.h"

extern Device_Link lid;

err_t vxi11_device_write_parser(void* data, u16_t len, Device_WriteParms* device_write_parms)
{

	size_t sizes[] = {sizeof(rpc_msg_call_t), sizeof(rpc_header_t)};

	u32_t rpc_msg_end = rpc_sum_size(sizes, sizeof(sizes)/sizeof(sizes[0]));
	u32_t offset = 0;

	for(u8_t i = 0; i < 5; i++)
	{
		memcpy(device_write_parms, data + rpc_msg_end + offset, sizeof(u32_t));
		offset += sizeof(u32_t);
	}

	device_write_parms->lid = ntohl(device_write_parms->lid);
	device_write_parms->io_timeout = ntohl(device_write_parms->io_timeout);
	device_write_parms->lock_timeout = ntohl(device_write_parms->lock_timeout);
	device_write_parms->flags = ntohl(device_write_parms->flags);
	device_write_parms->data.data_len = ntohl(device_write_parms->data.data_len);

	// Don't have to swap bytes
	memset(device_write_parms->data.data_val, 0, VXI11_MAX_RECV_SIZE);
	memcpy(device_write_parms->data.data_val, data + rpc_msg_end + offset, device_write_parms->data.data_len);

	return ERR_OK;

}


Device_WriteResp device_write(Device_WriteParms* device_write_parms)
{
	Device_WriteResp device_write_resp;

	if (device_write_parms->lid == lid)
	{
		device_write_resp.error = 0;
		device_write_resp.size = device_write_parms->data.data_len;
	}

	return device_write_resp;
}

Device_WriteResp vxi11_device_write(vxi11_instr_t* vxi11_instr, vxi11_netconn_t* vxi11_netconn, vxi11_netbuf_t* vxi11_netbuf_call)
{

	rpc_msg_call_t rpc_msg_call;
	rpc_msg_reply_t rpc_msg_reply;
	rpc_header_t rpc_header;

	Device_WriteResp device_write_resp;
	Device_WriteParms device_write_parms;

	vxi11_netbuf_t vxi11_netbuf_reply;

	rpc_tcp_call_parser(vxi11_netbuf_call->data, vxi11_netbuf_call->len, &rpc_msg_call, &rpc_header);

	if(ERR_OK == vxi11_device_write_parser(vxi11_netbuf_call->data, vxi11_netbuf_call->len, &device_write_parms))
	{
		device_write_resp = device_write(&device_write_parms);

		memcpy(&vxi11_instr->device_write_parms, &device_write_parms, sizeof(Device_WriteParms));
		memcpy(&vxi11_instr->device_write_resp, &device_write_resp, sizeof(Device_WriteResp));


		rpc_reply(&rpc_msg_reply, &rpc_msg_call, MSG_ACCEPTED);

		size_t sizes[] = {sizeof(rpc_header_t), sizeof(rpc_msg_reply_t), sizeof(Device_WriteResp)};
		void *sources[] = { &rpc_header, &rpc_msg_reply, &device_write_resp};

		vxi11_netbuf_reply.len = rpc_sum_size(sizes, sizeof(sizes)/sizeof(sizes[0]));

		rpc_header.data = (vxi11_netbuf_reply.len - sizes[0]) | RPC_HEADER_LAST;

		rpc_header.data = htonl(rpc_header.data);


		rpc_copy_memory(vxi11_netbuf_reply.data, sources, sizes, sizeof(sizes)/sizeof(sizes[0]));

		netconn_write(vxi11_netconn->newconn, vxi11_netbuf_reply.data, vxi11_netbuf_reply.len, NETCONN_NOFLAG);

	}

	return device_write_resp;
}
