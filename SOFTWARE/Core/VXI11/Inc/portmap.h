/*
 * pmap_prot.h
 *
 *  Created on: May 24, 2024
 *      Author: grzegorz
 */

#ifndef VXI11_INC_PORTMAP_H_
#define VXI11_INC_PORTMAP_H_

#define PMAPPORT		((u_short)111)
#define PMAPPROG		((u32_t)100000)
#define PMAPVERS		((u32_t)2)
#define PMAPVERS_PROTO		((u32_t)2)
#define PMAPVERS_ORIG		((u32_t)1)
#define PMAPPROC_NULL		((u32_t)0)
#define PMAPPROC_SET		((u32_t)1)
#define PMAPPROC_UNSET		((u32_t)2)
#define PMAPPROC_GETPORT	((u32_t)3)
#define PMAPPROC_DUMP		((u32_t)4)
#define PMAPPROC_CALLIT		((u32_t)5)

#pragma pack(push, 1)

struct pmap {
	u32_t pm_prog;
	u32_t pm_vers;
	u32_t pm_prot;
	u32_t pm_port;
};

typedef struct pmap pmap_t;

#pragma pack(pop)

void pmap_server_start(void);

#endif /* VXI11_INC_PORTMAP_H_ */
