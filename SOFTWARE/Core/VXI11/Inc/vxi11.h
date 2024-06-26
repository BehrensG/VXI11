/*
 * vxi11.h
 *
 *  Created on: May 26, 2024
 *      Author: grzegorz
 */

#ifndef VXI11_INC_VXI11_H_
#define VXI11_INC_VXI11_H_

#include "vxi11core.h"
#include "rpc.h"


#define VXI11_PORT	1024


typedef struct
{
	char* buffer;
	u32_t len;
}netconn_data_t;

typedef struct
{
  ip_addr_t* addr;
  u16_t port;
}vxi11_conn_info_t;

typedef enum
{
	VXI11_IDLE,
	VXI11_CONNECT,
	VXI11_RECV_ERR,
	VXI11_MSG,
	VXI11_ACCEPT,
	VXI11_RECV

}vxi11_state_t;

typedef struct {
    char data[sizeof(rpc_msg_reply_t) + sizeof(rpc_msg_call_t) + VXI11_MAX_RECV_SIZE]; // RPC package size + VXI11 data
    u16_t len;
    err_t err;
} vxi11_netbuf_t;


typedef struct {
	struct netconn* newconn;
	struct netconn* conn;
	u8_t accept;
}vxi11_netconn_t;

typedef struct {
	Create_LinkParms create_link_parms;
	Create_LinkResp create_link_resp;
	Device_GenericParms device_clear;
	Device_WriteResp device_write_resp;
	Device_WriteParms device_write_parms;
	Device_ReadResp device_read_resp;
	Device_ReadParms device_read_parms;
	vxi11_netconn_t netconn;
	vxi11_netbuf_t netbuf;
	u8_t connected;
}vxi11_core_t;

typedef struct {
	vxi11_netconn_t netconn;
}vxi11_abort_t;

typedef struct
{
	vxi11_state_t state;

	// New
	vxi11_core_t core;
	vxi11_abort_t abort;

}vxi11_instr_t;


void vxi11_server_start(void);
void vxi11_core_connect(vxi11_instr_t* vxi11_instr);

Create_LinkResp vxi11_create_link(vxi11_instr_t* vxi11_instr);
Device_WriteResp vxi11_device_write(vxi11_instr_t* vxi11_instr);
Device_ReadResp vxi11_device_read(vxi11_instr_t* vxi11_instr);
Device_Error vxi11_destroy_link(vxi11_instr_t* vxi11_instr);

#endif /* VXI11_INC_VXI11_H_ */
