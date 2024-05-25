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

static void rcp_ntoh(rpc_msg_t* rpc_msg)
{
	rpc_msg->ru.RM_cmb.cb_rpcvers = ntohl(rpc_msg->ru.RM_cmb.cb_rpcvers);
	rpc_msg->ru.RM_cmb.cb_proc = ntohl(rpc_msg->ru.RM_cmb.cb_proc);
	rpc_msg->ru.RM_cmb.cb_vers = ntohl(rpc_msg->ru.RM_cmb.cb_vers);
	rpc_msg->ru.RM_cmb.cb_proc = ntohl(rpc_msg->ru.RM_cmb.cb_proc);
	rpc_msg->ru.RM_cmb.cb_proc = ntohl(rpc_msg->ru.RM_cmb.cb_proc);
	rpc_msg->ru.RM_cmb.cb_cred.oa_flavor = ntohl(rpc_msg->ru.RM_cmb.cb_cred.oa_flavor);
	rpc_msg->ru.RM_cmb.cb_cred.oa_length = ntohl(rpc_msg->ru.RM_cmb.cb_cred.oa_length);
}

static void rcp_hton(rpc_msg_t* rpc_msg)
{
	rpc_msg->ru.RM_cmb.cb_rpcvers = htonl(rpc_msg->ru.RM_cmb.cb_rpcvers);
	rpc_msg->ru.RM_cmb.cb_proc = htonl(rpc_msg->ru.RM_cmb.cb_proc);
	rpc_msg->ru.RM_cmb.cb_vers = htonl(rpc_msg->ru.RM_cmb.cb_vers);
	rpc_msg->ru.RM_cmb.cb_proc = htonl(rpc_msg->ru.RM_cmb.cb_proc);
	rpc_msg->ru.RM_cmb.cb_proc = htonl(rpc_msg->ru.RM_cmb.cb_proc);
	rpc_msg->ru.RM_cmb.cb_cred.oa_flavor = htonl(rpc_msg->ru.RM_cmb.cb_cred.oa_flavor);
	rpc_msg->ru.RM_cmb.cb_cred.oa_length = htonl(rpc_msg->ru.RM_cmb.cb_cred.oa_length);
}

err_t rpc_udp_call(void* data, u16_t len, rpc_msg_t* call)
{
	err_t err = ERR_OK;
	size_t rpc_msg_size = sizeof(rpc_msg_t);


	if(len >= rpc_msg_size)
	{
		memcpy(call, data, rpc_msg_size);
		rcp_ntoh(call);
	}

	return err;

}

err_t rpc_tcp_call(void* data, u16_t len, rpc_msg_t* call, rpc_header_t* header)
{
	err_t err = ERR_OK;
	size_t rpc_msg_size = sizeof(rpc_msg_t);


	if(len >= rpc_msg_size)
	{
		memcpy(header, data, RPC_HEADER_SIZE);
		header->data = ntohl(header->data);
		memcpy(call, data + RPC_HEADER_SIZE, rpc_msg_size);
		rcp_ntoh(call);
	}

	return err;

}



err_t rpc_reply(rpc_msg_t* call, rpc_msg_t* replay, u_char accepted)
{

	err_t err = ERR_OK;
	memset(replay, 0, sizeof(rpc_msg_t));

	if(accepted)
	{
	replay->rm_xid = call->rm_xid;
	replay->rm_direction = REPLY;
	replay->rm_direction = htonl(replay->rm_direction);
	replay->ru.RM_rmb.rp_stat = MSG_ACCEPTED;

	replay->ru.RM_rmb.ru.RP_ar.ar_stat = _SUCCESS;
	replay->ru.RM_rmb.ru.RP_ar.ar_verf.oa_flavor = 0;
	replay->ru.RM_rmb.ru.RP_ar.ru.AR_results.proc = 0;
	replay->ru.RM_rmb.ru.RP_ar.ru.AR_versions.high = 0;
	replay->ru.RM_rmb.ru.RP_ar.ru.AR_versions.low = 0;

	rcp_hton(replay);
	}
	else
	{
		// TBD - call was not accepted
		// Currently see no need to implement it
	}
	return err;

}

