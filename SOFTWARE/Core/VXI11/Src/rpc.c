/*
 * rpc.c
 *
 *  Created on: May 25, 2024
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

#include "rpc.h"

static void rcp_call_swap(rpc_msg_call_t* rpc_msg)
{
	rpc_msg->rm_direction = htonl(rpc_msg->rm_direction);
	rpc_msg->rm_xid = htonl(rpc_msg->rm_xid);
	rpc_msg->ru.RM_cmb.cb_cred.oa_flavor = htonl(rpc_msg->ru.RM_cmb.cb_cred.oa_flavor);
	rpc_msg->ru.RM_cmb.cb_cred.oa_length = htonl(rpc_msg->ru.RM_cmb.cb_cred.oa_length);
	rpc_msg->ru.RM_cmb.cb_proc = htonl(rpc_msg->ru.RM_cmb.cb_proc);
	rpc_msg->ru.RM_cmb.cb_prog = htonl(rpc_msg->ru.RM_cmb.cb_prog);
	rpc_msg->ru.RM_cmb.cb_rpcvers = htonl(rpc_msg->ru.RM_cmb.cb_rpcvers);
	rpc_msg->ru.RM_cmb.cb_verf.oa_flavor = htonl(rpc_msg->ru.RM_cmb.cb_verf.oa_flavor);
	rpc_msg->ru.RM_cmb.cb_verf.oa_length = htonl(rpc_msg->ru.RM_cmb.cb_verf.oa_length);
	rpc_msg->ru.RM_cmb.cb_vers = htonl(rpc_msg->ru.RM_cmb.cb_vers);
}

static void rcp_reply_swap(rpc_msg_reply_t* rpc_msg)
{
	rpc_msg->rm_direction = htonl(rpc_msg->rm_direction);
	rpc_msg->rm_xid = htonl(rpc_msg->rm_xid);
	rpc_msg->ru.RM_rmb.rp_stat = htonl(rpc_msg->ru.RM_rmb.rp_stat);
	rpc_msg->ru.RM_rmb.ru.RP_ar.ar_verf.oa_flavor = htonl(rpc_msg->ru.RM_rmb.ru.RP_ar.ar_verf.oa_flavor);
	rpc_msg->ru.RM_rmb.ru.RP_ar.ar_verf.oa_length = htonl(rpc_msg->ru.RM_rmb.ru.RP_ar.ar_verf.oa_length);
	rpc_msg->ru.RM_rmb.ac_stat = htonl(rpc_msg->ru.RM_rmb.ac_stat);
}

err_t rpc_udp_call(void* data, u16_t len, rpc_msg_call_t* call)
{
	err_t err = ERR_OK;
	size_t rpc_msg_size = sizeof(rpc_msg_call_t);


	if(len >= rpc_msg_size)
	{
		memcpy(call, data, rpc_msg_size);
		rcp_call_swap(call);
	}

	return err;

}

err_t rpc_tcp_call_parser(void* data, u16_t len, rpc_msg_call_t* call, rpc_header_t* header)
{
	err_t err = ERR_OK;
	size_t rpc_msg_size = sizeof(rpc_msg_call_t);
	size_t rpc_header_size = sizeof(rpc_header_t);

	if(len >= rpc_msg_size)
	{
		memcpy(header, data, RPC_HEADER_SIZE);
		header->data = ntohl(header->data);
		memcpy(call, data + RPC_HEADER_SIZE, rpc_msg_size);
		rcp_call_swap(call);
	}

	return err;

}

err_t rpc_reply(rpc_msg_reply_t* replay, rpc_msg_call_t* call, enum reply_stat accepted)
{

	err_t err = ERR_OK;
	memset(replay, 0, sizeof(rpc_msg_reply_t));

	if(MSG_ACCEPTED == accepted)
	{
		replay->rm_xid = call->rm_xid;
		replay->rm_direction = REPLY;
		replay->ru.RM_rmb.rp_stat = MSG_ACCEPTED;

		replay->ru.RM_rmb.ru.RP_ar.ar_verf.oa_flavor = 0;
		replay->ru.RM_rmb.ac_stat = 0;


		rcp_reply_swap(replay);
	}
	else
	{
		// TBD - call was not accepted
		// Currently see no need to implement it
	}
	return err;

}

u32_t rpc_sum_size(u32_t* sizes, size_t len)
{
	u32_t sum = 0;

	for(u_char i = 0; i < len; i++)
	{
		sum += sizes[i];
	}

	return sum;
}

void rpc_copy_memory(char* destination, void** sources, size_t* sizes, u32_t num_sources)
{
    size_t offset = 0;
    for (u32_t i = 0; i < num_sources; i++)
    {
        memcpy(destination + offset, sources[i], sizes[i]);
        offset += sizes[i];
    }
}
