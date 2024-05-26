/*
 * vxi11core.h
 *
 *  Created on: May 24, 2024
 *      Author: grzegorz
 */

#ifndef VXI11_INC_VXI11CORE_H_
#define VXI11_INC_VXI11CORE_H_

#include <stdbool.h>


typedef int Device_Link;

enum Device_AddrFamily {
	DEVICE_TCP = 0,
	DEVICE_UDP = 1,
};
typedef enum Device_AddrFamily Device_AddrFamily;



typedef int Device_Flags;

typedef int Device_ErrorCode;

#pragma pack(push, 1)

struct Device_Error {
	Device_ErrorCode error;
};
typedef struct Device_Error Device_Error;

typedef struct {
	u_long lenght;
	char* contents;
}device_t;

struct Create_LinkParms {
	int clientId;
	u_int lockDevice;
	u_int lock_timeout;
	device_t device;
};
typedef struct Create_LinkParms Create_LinkParms;

struct Create_LinkResp {
	Device_ErrorCode error;
	Device_Link lid;
	u_short abortPort;
	u_int maxRecvSize;
};
typedef struct Create_LinkResp Create_LinkResp;

struct Device_WriteParms {
	Device_Link lid;
	u_int io_timeout;
	u_int lock_timeout;
	Device_Flags flags;
	struct {
		u_int data_len;
		char *data_val;
	} data;
};
typedef struct Device_WriteParms Device_WriteParms;

struct Device_WriteResp {
	Device_ErrorCode error;
	u_int size;
};
typedef struct Device_WriteResp Device_WriteResp;

struct Device_ReadParms {
	Device_Link lid;
	u_int requestSize;
	u_int io_timeout;
	u_int lock_timeout;
	Device_Flags flags;
	char termChar;
};
typedef struct Device_ReadParms Device_ReadParms;

struct Device_ReadResp {
	Device_ErrorCode error;
	int reason;
	struct {
		u_int data_len;
		char *data_val;
	} data;
};
typedef struct Device_ReadResp Device_ReadResp;

struct Device_ReadStbResp {
	Device_ErrorCode error;
	u_char stb;
};
typedef struct Device_ReadStbResp Device_ReadStbResp;

struct Device_GenericParms {
	Device_Link lid;
	Device_Flags flags;
	u_int lock_timeout;
	u_int io_timeout;
};
typedef struct Device_GenericParms Device_GenericParms;

struct Device_RemoteFunc {
	u_int hostAddr;
	u_int hostPort;
	u_int progNum;
	u_int progVers;
	Device_AddrFamily progFamily;
};
typedef struct Device_RemoteFunc Device_RemoteFunc;

struct Device_EnableSrqParms {
	Device_Link lid;
	bool enable;
	struct {
		u_int handle_len;
		char *handle_val;
	} handle;
};
typedef struct Device_EnableSrqParms Device_EnableSrqParms;

struct Device_LockParms {
	Device_Link lid;
	Device_Flags flags;
	u_int lock_timeout;
};
typedef struct Device_LockParms Device_LockParms;

struct Device_DocmdParms {
	Device_Link lid;
	Device_Flags flags;
	u_int io_timeout;
	u_int lock_timeout;
	int cmd;
	bool network_order;
	int datasize;
	struct {
		u_int data_in_len;
		char *data_in_val;
	} data_in;
};
typedef struct Device_DocmdParms Device_DocmdParms;

struct Device_DocmdResp {
	Device_ErrorCode error;
	struct {
		u_int data_out_len;
		char *data_out_val;
	} data_out;
};
typedef struct Device_DocmdResp Device_DocmdResp;


#pragma pack(pop)

#define DEVICE_ASYNC 0x0607B0
#define DEVICE_ASYNC_VERSION 1

#define DEVICE_CORE 0x0607AF
#define DEVICE_CORE_VERSION 1

#define CREATE_LINK			10
#define DEVICE_WRITE		11
#define DEVICE_READ			12
#define DEVICE_READ_STB		13
#define DEVICE_TRIGGER		14
#define DEVICE_CLEAR		15

Create_LinkResp vxi11_create_link(Create_LinkParms* create_link_parms);
Device_WriteResp vxi11_device_write(Device_WriteParms* device_write_parms);
Device_ReadResp vxi11_device_read(Device_ReadParms* device_read_parms);
Device_ReadStbResp vxi11_device_readstb (Device_GenericParms* device_generic_parms);
Device_Error vxi11_device_trigger(Device_GenericParms* device_generic_parms);
Device_Error vxi11_device_clear (Device_GenericParms* device_generic_parms);
Device_Error vxi11_device_remote(Device_GenericParms* device_generic_parms);
Device_Error vxi11_device_local(Device_GenericParms* device_generic_parms);
Device_Error vxi11_device_lock(Device_LockParms* device_lock_parms);
Device_Error vxi11_device_unlock(Device_Link* device_link);
Device_Error vxi11_device_enable_srq(Device_EnableSrqParms* device_enable_srq_param);
Device_DocmdResp vxi11_device_docmd(Device_DocmdParms* device_docm_parms);
Device_Error vxi11_destroy_link(Device_Link* device_link);
Device_Error vxi11_create_intr_chan(Device_RemoteFunc* device_remote_func);
Device_Error vxi11_destroy_intr_chan(void);

#endif /* VXI11_INC_VXI11CORE_H_ */
