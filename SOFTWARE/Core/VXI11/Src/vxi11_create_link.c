/*
 * vxi11_create_link.c
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

Device_Link lid = VXI11_LINKID_DEFAULT;
extern vxi11_instr_t vxi11_instr;


static Create_LinkResp create_link(Create_LinkParms* create_link_parms);
static err_t vxi11_create_link_parser(void* data, u16_t len, Create_LinkParms* create_link_parms);



Create_LinkResp vxi11_create_link(vxi11_instr_t* vxi11_instr)
{

	Create_LinkParms create_link_parms;
	Create_LinkResp create_link_resp;

	rpc_msg_call_t rpc_msg_call;
	rpc_header_t rpc_header;

	//vxi11_netbuf_t vxi11_netbuf_reply;

	rpc_tcp_call_parser(vxi11_instr->core.netbuf.data, vxi11_instr->core.netbuf.len, &rpc_msg_call, &rpc_header);

	if(ERR_OK == vxi11_create_link_parser(vxi11_instr->core.netbuf.data, vxi11_instr->core.netbuf.len, &create_link_parms))
	{

		create_link_resp = create_link(&create_link_parms);

		memcpy(&vxi11_instr->core.create_link_parms,&create_link_parms,sizeof(Create_LinkParms));
		memcpy(&vxi11_instr->core.create_link_resp,&create_link_resp,sizeof(Create_LinkResp));

		rpc_msg_reply_t rpc_msg_reply = rpc_reply(rpc_msg_call.rm_xid, MSG_ACCEPTED);

		size_t sizes[] = {sizeof(rpc_header_t), sizeof(rpc_msg_reply_t), sizeof(Create_LinkResp)};
		void *sources[] = { &rpc_header, &rpc_msg_reply, &create_link_resp};

		vxi11_instr->core.netbuf.len = rpc_sum_size(sizes, sizeof(sizes)/sizeof(sizes[0]));

		rpc_header.data = (vxi11_instr->core.netbuf.len - sizes[0]) | RPC_HEADER_LAST;

		rpc_header.data = htonl(rpc_header.data);


		rpc_copy_memory(vxi11_instr->core.netbuf.data, sources, sizes, sizeof(sizes)/sizeof(sizes[0]));

		netconn_write(vxi11_instr->core.netconn.newconn, vxi11_instr->core.netbuf.data, vxi11_instr->core.netbuf.len, NETCONN_NOFLAG);
		HAL_Delay(1);
	}

	return create_link_resp;
}

void vxi11_core_connect(vxi11_instr_t* vxi11_instr)
{
	struct netconn *newconn;
	struct netbuf* buf;

	netconn_data_t netconn_call;
	netconn_data_t netconn_reply;

	err_t err = ERR_OK;

	rpc_msg_call_t rpc_msg_call;
	rpc_header_t rpc_header;

	err = netconn_accept(vxi11_instr->core.netconn.conn, &newconn);

	if(ERR_OK == err)
	{
		vxi11_instr->core.netconn.newconn = newconn;


		err = netconn_recv(vxi11_instr->core.netconn.newconn, &buf);

		if(err != ERR_OK)
		{
			vxi11_instr->state = VXI11_RECV_ERR;
			netconn_close(vxi11_instr->core.netconn.newconn);

		}

		netbuf_data(buf, &netconn_call.buffer, &netconn_call.len);

		rpc_tcp_call_parser(netconn_call.buffer, netconn_call.len, &rpc_msg_call, &rpc_header);

		if(DEVICE_CORE == rpc_msg_call.ru.RM_cmb.cb_prog)
		{
			if(CREATE_LINK == rpc_msg_call.ru.RM_cmb.cb_proc)
			{
				if(ERR_OK == vxi11_create_link_parser(netconn_call.buffer, netconn_call.len, &vxi11_instr->core.create_link_parms))
				{
					vxi11_instr->core.create_link_resp = create_link(&vxi11_instr->core.create_link_parms);

					rpc_msg_reply_t rpc_msg_reply = rpc_reply(rpc_msg_call.rm_xid, MSG_ACCEPTED);


					size_t sizes[] = {sizeof(rpc_header_t), sizeof(rpc_msg_reply_t), sizeof(Create_LinkResp)};
					void *sources[] = { &rpc_header, &rpc_msg_reply, &vxi11_instr->core.create_link_resp};

					netconn_reply.len = rpc_sum_size(sizes, sizeof(sizes)/sizeof(sizes[0]));

					rpc_header.data = (netconn_reply.len - sizes[0]) | RPC_HEADER_LAST;
					rpc_header.data = htonl(rpc_header.data);

					netconn_reply.buffer = (char*)malloc(netconn_reply.len);

					rpc_copy_memory(netconn_reply.buffer, sources, sizes, sizeof(sizes)/sizeof(sizes[0]));

					netconn_write(vxi11_instr->core.netconn.newconn, netconn_reply.buffer, netconn_reply.len, NETCONN_NOFLAG);
					HAL_Delay(1);

					free(netconn_reply.buffer);
				}

			}
		}

		netbuf_delete(buf);

	}

}


static err_t vxi11_create_link_parser(void* data, u16_t len, Create_LinkParms* create_link_parms)
{
	size_t rpc_size = sizeof(rpc_msg_call_t) + sizeof(rpc_header_t);
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

static Create_LinkResp create_link(Create_LinkParms* create_link_parms)
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
