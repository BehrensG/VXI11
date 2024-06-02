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

extern Device_Link lid;

Device_ReadResp device_read(Device_ReadParms* device_read_parms)
{
	Device_ReadResp device_read_resp;

	char test_string[] = "Agilent Technologies,34410A,MY47019757,2.35-2.35-0.09-46-09";
	size_t str_len = strlen(test_string);
	u8_t term_char = device_read_parms->termChar;

	if (device_read_parms->lid == lid)
	{
		device_read_resp.error = NO_ERR;

		if(!term_char)
		{
			term_char = 0xA;
		}


		memcpy(device_read_resp.data.data_val, test_string, str_len);
		memcpy(device_read_resp.data.data_val + str_len, &term_char, 1); // termination char

		device_read_resp.data.data_len = str_len + 1; // add termination char

		device_read_resp.reason = VXI11_READ_REASON_END;

	}
	else
	{
		device_read_resp.error = INVALID_LINK_IDENTIFIER;
		device_read_resp.data.data_len = 0;
		device_read_resp.reason = 0;
		memset(device_read_resp.data.data_val, 0, VXI11_MAX_RECV_SIZE);
	}


	device_read_resp.error =htonl(device_read_resp.error);
	device_read_resp.reason =htonl(device_read_resp.reason);
	device_read_resp.data.data_len = htonl(device_read_resp.data.data_len);

	return device_read_resp;

}

err_t vxi11_device_read_parser(void* data, u16_t len, Device_ReadParms* device_read_parms)
{

	size_t sizes[] = {sizeof(rpc_msg_call_t), sizeof(rpc_header_t)};

	u32_t rpc_msg_end = rpc_sum_size(sizes, sizeof(sizes)/sizeof(sizes[0]));
	u32_t offset = 0;

	for(u8_t i = 0; i < 6; i++)
	{
		memcpy(device_read_parms + offset, data + rpc_msg_end + offset, sizeof(u32_t));
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



Device_ReadResp vxi11_device_read(vxi11_instr_t* vxi11_instr)
{

	rpc_msg_call_t rpc_msg_call;
	rpc_header_t rpc_header;

	//Device_ReadParms device_read_parms;


	rpc_tcp_call_parser(vxi11_instr->core.netbuf.data, vxi11_instr->core.netbuf.len, &rpc_msg_call, &rpc_header);

	if(ERR_OK == vxi11_device_read_parser(vxi11_instr->core.netbuf.data, vxi11_instr->core.netbuf.len, &vxi11_instr->core.device_read_parms))
	{
		vxi11_instr->core.device_read_resp = device_read(&vxi11_instr->core.device_read_parms);


		rpc_msg_reply_t rpc_msg_reply = rpc_reply(rpc_msg_call.rm_xid, MSG_ACCEPTED);

		size_t sizes[] = {sizeof(rpc_header_t), sizeof(rpc_msg_reply_t), sizeof(Device_ReadResp)};
		void *sources[] = { &rpc_header, &rpc_msg_reply, &vxi11_instr->core.device_read_resp};

		u32_t data_len = ntohl(vxi11_instr->core.device_read_resp.data.data_len);
		vxi11_instr->core.netbuf.len = rpc_sum_size(sizes, sizeof(sizes)/sizeof(sizes[0])) - (VXI11_MAX_RECV_SIZE - data_len);

		rpc_header.data = (vxi11_instr->core.netbuf.len - sizes[0]) | RPC_HEADER_LAST;

		rpc_header.data = htonl(rpc_header.data);

		rpc_copy_memory(vxi11_instr->core.netbuf.data, sources, sizes, sizeof(sizes)/sizeof(sizes[0]));

		vTaskDelay(pdMS_TO_TICKS(1));

		netconn_write(vxi11_instr->core.netconn.newconn, vxi11_instr->core.netbuf.data, vxi11_instr->core.netbuf.len, NETCONN_NOFLAG);

	}

	return vxi11_instr->core.device_read_resp;
}
