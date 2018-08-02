#pragma once
#ifndef _xSLAB_H_INCLUDED_
#define _xSLAB_H_INCLUDED_
#include "xmConfig.h"
#include "xCore.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct xslab_page_s xslab_page_t;
typedef struct
{
	time_t sec;
	xuint_t msec;
	xint_t gmtoff;
} xtime_t;
struct xslab_page_s
{
	uintptr_t slab;
	xslab_page_t *next;
	uintptr_t prev;
};
typedef struct
{
	xuint_t total;
	xuint_t used;
	xuint_t reqs;
	xuint_t fails;
} xslab_stat_t;
typedef struct
{
	xshmtx_sh_t lock;
	size_t min_size;
	size_t min_shift;
	xslab_page_t *pages;
	xslab_page_t *last;
	xslab_page_t free;
	xslab_stat_t *stats;
	xuint_t pfree;
	u_char *start;
	u_char *end;
	xshmtx_t mutex;
	u_char *log_ctx;
	u_char zero;
	unsigned log_nomem :1;
	void *user_data;
} xslab_pool_t;
#ifdef __cplusplus
}
#endif
extern void xslab_sizes_init(void);
extern void xslab_init(xslab_pool_t *pool);
extern void *xmalloc(xslab_pool_t *pool, size_t size);
extern void *xslab_alloc_locked(xslab_pool_t *pool, size_t size);
extern void *xslab_calloc(xslab_pool_t *pool, size_t size);
extern void *xslab_calloc_locked(xslab_pool_t *pool, size_t size);
extern void xfree(xslab_pool_t *pool, void *p);
extern void xslab_free_locked(xslab_pool_t *pool, void *p);
extern xslab_pool_t * static_init_3(void * begin_ptr, size_t size);
extern xslab_pool_t * static_access_init_3(void * begin_ptr, size_t size);
namespace global
{
extern int num1;
extern int num2;
extern int dis_malloc;
}
#endif 
