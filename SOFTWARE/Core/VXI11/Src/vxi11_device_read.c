/*
 * vxi11_device_read.c
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



Device_ReadResp device_read(Device_ReadParms* device_read_parms)
{
	Device_ReadResp device_read_resp;

	char test_string[] = "Test device,ABC123,1.2 5-1,35-0.12343";
	size_t str_len = strlen(test_string);


	memcpy(device_read_resp.data.data_val, test_string, str_len);
	memcpy(device_read_resp.data.data_val + str_len, &device_read_parms->termChar, 1); // termination char

	device_read_resp.data.data_len = str_len + 1; // add termination char

	device_read_resp.error = 0;
	device_read_resp.reason = VXI11_READ_REASON_END;


	return device_read_resp;

}

err_t vxi11_device_read_parser(void* data, u16_t len, Device_ReadParms* device_read_parms)
{

	size_t sizes[] = {sizeof(rpc_msg_call_t), sizeof(rpc_header_t)};

	u32_t rpc_msg_end = rpc_sum_size(sizes, sizeof(sizes)/sizeof(sizes[0]));
	u32_t offset = 0;

	for(u8_t i = 0; i < 6; i++)
	{
		memcpy(device_read_parms, data + rpc_msg_end + offset, sizeof(u32_t));
		offset += sizeof(u32_t);
	}

	device_read_parms->lid = ntohl(device_read_parms->lid);
	device_read_parms->requestSize = ntohl(device_read_parms->requestSize);
	device_read_parms->io_timeout = ntohl(device_read_parms->io_timeout);
	device_read_parms->lock_timeout = ntohl(device_read_parms->lock_timeout);
	device_read_parms->flags = ntohl(device_read_parms->flags);
	device_read_parms->termChar = ntohl(device_read_parms->termChar);

	return ERR_OK;

}



Device_ReadResp vxi11_device_read(vxi11_instr_t* vxi11_instr, vxi11_netconn_t* vxi11_netconn, vxi11_netbuf_t* vxi11_netbuf_call)
{

	rpc_msg_call_t rpc_msg_call;
	rpc_header_t rpc_header;

	Device_ReadResp device_read_resp;
	Device_ReadParms device_read_parms;

	vxi11_netbuf_t vxi11_netbuf_reply;

	rpc_tcp_call_parser(vxi11_netbuf_call->data, vxi11_netbuf_call->len, &rpc_msg_call, &rpc_header);

	if(ERR_OK == vxi11_device_read_parser(vxi11_netbuf_call->data, vxi11_netbuf_call->len, &device_read_parms))
	{
		device_read_resp = device_read(&device_read_parms);

		memcpy(&vxi11_instr->device_read_parms, &device_read_parms, sizeof(Device_ReadParms));
		memcpy(&vxi11_instr->device_read_resp, &device_read_resp, sizeof(Device_ReadResp));


		rpc_msg_reply_t rpc_msg_reply = rpc_reply(rpc_msg_call.rm_xid, MSG_ACCEPTED);

		size_t sizes[] = {sizeof(rpc_header_t), sizeof(rpc_msg_reply_t), sizeof(Device_ReadResp)};
		void *sources[] = { &rpc_header, &rpc_msg_reply, &device_read_resp};

		vxi11_netbuf_reply.len = rpc_sum_size(sizes, sizeof(sizes)/sizeof(sizes[0]));

		rpc_header.data = (vxi11_netbuf_reply.len - sizes[0]) | RPC_HEADER_LAST;

		rpc_header.data = htonl(rpc_header.data);


		rpc_copy_memory(vxi11_netbuf_reply.data, sources, sizes, sizeof(sizes)/sizeof(sizes[0]));

		netconn_write(vxi11_netconn->newconn, vxi11_netbuf_reply.data, vxi11_netbuf_reply.len, NETCONN_NOFLAG);

	}

	return device_read_resp;
}
