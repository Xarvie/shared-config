#pragma once
#ifndef _xSHMTX_H_INCLUDED_
#define _xSHMTX_H_INCLUDED_
#include "xmConfig.h"
#include "xCore.h"
#include "xAtomic.h"
typedef struct
{
	xatomic_t lock;
#if (xHAVE_POSIX_SEM)
	xatomic_t wait;
#endif
} xshmtx_sh_t;
typedef struct
{
#if (xHAVE_ATOMIC_OPS)
	xatomic_t *lock;
#if (xHAVE_POSIX_SEM)
	xatomic_t *wait;
	xuint_t semaphore;
	sem_t sem;
#endif
#else
	u_char *name;
#endif
	xuint_t spin;
} xshmtx_t;
typedef pid_t xpid_t;
xint_t xshmtx_create(xshmtx_t *mtx, xshmtx_sh_t *addr, u_char *name);
void xshmtx_destroy(xshmtx_t *mtx);
xuint_t xshmtx_trylock(xshmtx_t *mtx);
void xshmtx_lock(xshmtx_t *mtx);
void xshmtx_unlock(xshmtx_t *mtx);
xuint_t xshmtx_force_unlock(xshmtx_t *mtx, xpid_t pid);
#endif 
