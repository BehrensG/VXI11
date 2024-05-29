/*
 * vxi11core.h
 *
 *  Created on: May 24, 2024
 *      Author: grzegorz
 */

#ifndef VXI11_INC_VXI11CORE_H_
#define VXI11_INC_VXI11CORE_H_


#define VXI11_MAX_RECV_SIZE		1024
#define VXI11_MAX_DEVICE_NAME	256
#define VXI11_ABORT_PORT		(u_short)889
#define VXI11_LINKID_DEFAULT	123

#define VXI11_WRITE_WAIT_UNTIL_LOCKED	0x00000001
#define VXI11_WRITE_SET_EOI				0x00000008
#define VXI11_WRITE_TERM_CHAR_SET		0x00000080

#define VXI11_READ_REASON_REQCNT		0x00000001
#define VXI11_READ_REASON_CHR			0x00000002
#define VXI11_READ_REASON_END			0x00000004

enum create_link_error
{
	NO_ERR,
	SYNTAX_ERR = 1,
	DEVICE_NOT_ACCESSIBLE = 3,
	OUT_OF_RESOURCES = 9,
	DEVICE_LOCKED_BY_LINK = 11,
	INVALID_ADDRESS = 21
};

typedef u32_t Device_Link;


enum Device_AddrFamily {
	DEVICE_TCP = 0,
	DEVICE_UDP = 1,
};
typedef enum Device_AddrFamily Device_AddrFamily;



typedef s32_t Device_Flags;

typedef s32_t Device_ErrorCode;

#pragma pack(push, 1)

typedef struct  {
	Device_ErrorCode error;
}Device_Error;


typedef struct {
	u32_t lenght;
	char contents [VXI11_MAX_DEVICE_NAME];
}device_t;

struct Create_LinkParms {
	int clientId;
	u32_t lockDevice;
	u32_t lock_timeout;
	device_t device;
};
typedef struct Create_LinkParms Create_LinkParms;

struct Create_LinkResp {
	Device_ErrorCode error;
	Device_Link lid;
	u32_t abortPort;
	u32_t maxRecvSize;
};
typedef struct Create_LinkResp Create_LinkResp;

struct Device_WriteParms {
	Device_Link lid;
	u32_t io_timeout;
	u32_t lock_timeout;
	Device_Flags flags;
	struct {
		u32_t data_len;
		char data_val[VXI11_MAX_RECV_SIZE];
	} data;
};
typedef struct Device_WriteParms Device_WriteParms;

struct Device_WriteResp {
	Device_ErrorCode error;
	u32_t size;
};
typedef struct Device_WriteResp Device_WriteResp;

struct Device_ReadParms {
	Device_Link lid;
	u32_t requestSize;
	u32_t io_timeout;
	u32_t lock_timeout;
	Device_Flags flags;
	u32_t termChar;
};
typedef struct Device_ReadParms Device_ReadParms;

struct Device_ReadResp {
	Device_ErrorCode error;
	int reason;
	struct {
		u32_t data_len;
		char data_val[VXI11_MAX_RECV_SIZE];
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
	u32_t lock_timeout;
	u32_t io_timeout;
};
typedef struct Device_GenericParms Device_GenericParms;

struct Device_RemoteFunc {
	u32_t hostAddr;
	u32_t hostPort;
	u32_t progNum;
	u32_t progVers;
	Device_AddrFamily progFamily;
};
typedef struct Device_RemoteFunc Device_RemoteFunc;

struct Device_EnableSrqParms {
	Device_Link lid;
	u_char enable;
	struct {
		u32_t handle_len;
		char *handle_val;
	} handle;
};
typedef struct Device_EnableSrqParms Device_EnableSrqParms;

struct Device_LockParms {
	Device_Link lid;
	Device_Flags flags;
	u32_t lock_timeout;
};
typedef struct Device_LockParms Device_LockParms;

struct Device_DocmdParms {
	Device_Link lid;
	Device_Flags flags;
	u32_t io_timeout;
	u32_t lock_timeout;
	int cmd;
	u_char network_order;
	int datasize;
	struct {
		u32_t data_in_len;
		char *data_in_val;
	} data_in;
};
typedef struct Device_DocmdParms Device_DocmdParms;

struct Device_DocmdResp {
	Device_ErrorCode error;
	struct {
		u32_t data_out_len;
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
#define DESTROY_LINK		23


//Device_ReadResp vxi11_device_read(Device_ReadParms* device_read_parms);
Device_ReadStbResp vxi11_device_readstb (Device_GenericParms* device_generic_parms);
Device_Error vxi11_device_trigger(Device_GenericParms* device_generic_parms);
Device_Error vxi11_device_clear (Device_GenericParms* device_generic_parms);
Device_Error vxi11_device_remote(Device_GenericParms* device_generic_parms);
Device_Error vxi11_device_local(Device_GenericParms* device_generic_parms);
Device_Error vxi11_device_lock(Device_LockParms* device_lock_parms);
Device_Error vxi11_device_unlock(Device_Link* device_link);
Device_Error vxi11_device_enable_srq(Device_EnableSrqParms* device_enable_srq_param);
Device_DocmdResp vxi11_device_docmd(Device_DocmdParms* device_docm_parms);
//Device_Error vxi11_destroy_link(Device_Link* device_link);
Device_Error vxi11_create_intr_chan(Device_RemoteFunc* device_remote_func);
Device_Error vxi11_destroy_intr_chan(void);

#endif /* VXI11_INC_VXI11CORE_H_ */
