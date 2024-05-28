/*
 * rpc.h
 *
 *  Created on: May 24, 2024
 *      Author: grzegorz
 */

#ifndef VXI11_INC_RPC_H_
#define VXI11_INC_RPC_H_

#define RPC_MSG_VERSION		((u32_t) 2)
#define RPC_SERVICE_PORT	((u16_t) 2048)


/*
 * Bottom up definition of an rpc message.
 * NOTE: call and reply use the same overall struct but
 * different parts of unions within it.
 */


enum msg_type {
	CALL=0,
	REPLY=1
};

enum reply_stat {
	MSG_ACCEPTED=0,
	MSG_DENIED=1
};

enum accept_stat {
	_SUCCESS=0,
	PROG_UNAVAIL=1,
	PROG_MISMATCH=2,
	PROC_UNAVAIL=3,
	GARBAGE_ARGS=4,
	SYSTEM_ERR=5
};

enum reject_stat {
	RPC_MISMATCH=0,
	AUTH_ERROR=1
};


typedef int xdrproc_t;
typedef int enum_t;

#pragma pack(push, 1)

struct opaque_auth {
        enum_t        oa_flavor;                /* flavor of auth */
   //     caddr_t        oa_base;                /* address of more auth stuff */
        u_int        oa_length;                /* not to exceed MAX_AUTH_BYTES */
};

/*
 * Reply part of an rpc exchange
 */

/*
 * Reply to an rpc request that was accepted by the server.
 * Note: there could be an error even though the request was
 * accepted.
 */
struct accepted_reply {
	struct opaque_auth	ar_verf;

};

enum auth_stat {
     AUTH_BADCRED      = 1, /* bad credentials      */
     AUTH_REJECTEDCRED = 2, /* begin new session    */
     AUTH_BADVERF      = 3, /* bad verifier         */
     AUTH_REJECTEDVERF = 4, /* expired or replayed  */
     AUTH_TOOWEAK      = 5, /* rejected for security*/
};

/*
 * Reply to an rpc request that was rejected by the server.
 */
struct rejected_reply {
	u32_t rj_stat;
	union {
		struct {
			u32_t low;
			u32_t high;
		} RJ_versions;
		u32_t RJ_why;  /* why authentication did not work */
	} ru;
#define	rj_vers	ru.RJ_versions
#define	rj_why	ru.RJ_why
};

/*
 * Body of a reply to an rpc request.
 */
struct reply_body {
	 u32_t rp_stat;
	union {
		struct accepted_reply RP_ar;
	//	struct rejected_reply RP_dr;
	} ru;
	u32_t ac_stat;
#define	rp_acpt	ru.RP_ar
#define	rp_rjct	ru.RP_dr
};

/*
 * Body of an rpc request call.
 */
struct call_body {
	u32_t cb_rpcvers;	/* must be equal to two */
	u32_t cb_prog;
	u32_t cb_vers;
	u32_t cb_proc;
	struct opaque_auth cb_cred;
	struct opaque_auth cb_verf; /* protocol specific - provided by client */
};

/*
 * The rpc message
 */
/*
struct rpc_msg {
	u32_t			rm_xid;
	u32_t		rm_direction;
	union {
		struct call_body RM_cmb;
		struct reply_body RM_rmb;
	} ru;
#define	rm_call		ru.RM_cmb
#define	rm_reply	ru.RM_rmb
};

typedef struct rpc_msg rpc_msg_t;
*/

struct rpc_msg_call {
	u32_t			rm_xid;
	u32_t		rm_direction;
	union {
		struct call_body RM_cmb;
	} ru;
#define	rm_call		ru.RM_cmb
#define	rm_reply	ru.RM_rmb
};

typedef struct rpc_msg_call rpc_msg_call_t;

struct rpc_msg_reply {
	u32_t			rm_xid;
	u32_t		rm_direction;
	union {
		struct reply_body RM_rmb;
	} ru;
#define	rm_call		ru.RM_cmb
#define	rm_reply	ru.RM_rmb
};

typedef struct rpc_msg_reply rpc_msg_reply_t;

struct rpc_header {
	u32_t data;
};

typedef struct rpc_header rpc_header_t;

#pragma pack(pop)

#define	acpted_rply	ru.RM_rmb.ru.RP_ar
#define	rjcted_rply	ru.RM_rmb.ru.RP_dr

#define RPC_HEADER_SIZE		4UL
#define RPC_HEADER_LAST		0x80000000


err_t rpc_udp_call_parser(void* data, u16_t len, rpc_msg_call_t* rcp_msg);
err_t rpc_tcp_call_parser(void* data, u16_t len, rpc_msg_call_t* call, rpc_header_t* header);
err_t rpc_reply(rpc_msg_reply_t* replay, rpc_msg_call_t* call, enum reply_stat accepted);

void rpc_copy_memory(char* destination, void** sources, size_t* sizes, u_int num_sources);
u_int rpc_sum_size(u_int* sizes, u_int len);

#endif /* VXI11_INC_RPC_H_ */
