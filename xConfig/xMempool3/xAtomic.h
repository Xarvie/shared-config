#pragma once
#ifndef _xATOMIC_H_INCLUDED_
#define _xATOMIC_H_INCLUDED_
#include "xmConfig.h"
#include "xCore.h"
#if (xHAVE_LIBATOMIC)
#define AO_REQUIRE_CAS
#include <atomic_ops.h>
#define xHAVE_ATOMIC_OPS  1
typedef long xatomic_int_t;
typedef AO_t xatomic_uint_t;
typedef volatile xatomic_uint_t xatomic_t;
#if (xPTR_SIZE == 8)
#define xATOMIC_T_LEN            (sizeof("-9223372036854775808") - 1)
#else
#define xATOMIC_T_LEN            (sizeof("-2147483648") - 1)
#endif
#define xatomic_cmp_set(lock, old, new)                                    \
    AO_compare_and_swap(lock, old, new)
#define xatomic_fetch_add(value, add)                                      \
    AO_fetch_and_add(value, add)
#define xmemory_barrier()        AO_nop()
#define xcpu_pause()
#elif (xDARWIN_ATOMIC)
#include <libkern/OSAtomic.h>
#if 0
#undef bool
#endif
#define xHAVE_ATOMIC_OPS  1
#if (xPTR_SIZE == 8)
typedef int64_t xatomic_int_t;
typedef uint64_t xatomic_uint_t;
#define xATOMIC_T_LEN            (sizeof("-9223372036854775808") - 1)
#define xatomic_cmp_set(lock, old, new)                                    \
    OSAtomicCompareAndSwap64Barrier(old, new, (int64_t *) lock)
#define xatomic_fetch_add(value, add)                                      \
    (OSAtomicAdd64(add, (int64_t *) value) - add)
#else
typedef int32_t xatomic_int_t;
typedef uint32_t xatomic_uint_t;
#define xATOMIC_T_LEN            (sizeof("-2147483648") - 1)
#define xatomic_cmp_set(lock, old, new)                                    \
    OSAtomicCompareAndSwap32Barrier(old, new, (int32_t *) lock)
#define xatomic_fetch_add(value, add)                                      \
    (OSAtomicAdd32(add, (int32_t *) value) - add)
#endif
#define xmemory_barrier()        OSMemoryBarrier()
#define xcpu_pause()
typedef volatile xatomic_uint_t xatomic_t;
#elif (xHAVE_GCC_ATOMIC)
#define xHAVE_ATOMIC_OPS  1
typedef long xatomic_int_t;
typedef unsigned long xatomic_uint_t;
#if (xPTR_SIZE == 8)
#define xATOMIC_T_LEN            (sizeof("-9223372036854775808") - 1)
#else
#define xATOMIC_T_LEN            (sizeof("-2147483648") - 1)
#endif
typedef volatile xatomic_uint_t xatomic_t;
#define xatomic_cmp_set(lock, old, set)                                    \
    __sync_bool_compare_and_swap(lock, old, set)
#define xatomic_fetch_add(value, add)                                      \
    __sync_fetch_and_add(value, add)
#define xmemory_barrier()        __sync_synchronize()
#if ( __i386__ || __i386 || __amd64__ || __amd64 )
#define xcpu_pause()             __asm__ ("pause")
#else
#define xcpu_pause()
#endif
#elif ( __i386__ || __i386 )
typedef int32_t xatomic_int_t;
typedef uint32_t xatomic_uint_t;
typedef volatile xatomic_uint_t xatomic_t;
#define xATOMIC_T_LEN            (sizeof("-2147483648") - 1)
#if ( __SUNPRO_C )
#define xHAVE_ATOMIC_OPS  1
xatomic_uint_t
xatomic_cmp_set(xatomic_t *lock, xatomic_uint_t old,
		xatomic_uint_t set);
xatomic_int_t
xatomic_fetch_add(xatomic_t *value, xatomic_int_t add);
void
xcpu_pause(void);
#define xmemory_barrier()        __asm (".volatile"); __asm (".nonvolatile")
#else
#define xHAVE_ATOMIC_OPS  1
#include "xgcc_atomic_x86.h"
#endif
#elif ( __amd64__ || __amd64 )
typedef int64_t xatomic_int_t;
typedef uint64_t xatomic_uint_t;
typedef volatile xatomic_uint_t xatomic_t;
#define xATOMIC_T_LEN            (sizeof("-9223372036854775808") - 1)
#if ( __SUNPRO_C )
#define xHAVE_ATOMIC_OPS  1
xatomic_uint_t
xatomic_cmp_set(xatomic_t *lock, xatomic_uint_t old,
		xatomic_uint_t set);
xatomic_int_t
xatomic_fetch_add(xatomic_t *value, xatomic_int_t add);
void
xcpu_pause(void);
#define xmemory_barrier()        __asm (".volatile"); __asm (".nonvolatile")
#else 
#define xHAVE_ATOMIC_OPS  1
#endif
#elif ( __sparc__ || __sparc || __sparcv9 )
#if (xPTR_SIZE == 8)
typedef int64_t xatomic_int_t;
typedef uint64_t xatomic_uint_t;
#define xATOMIC_T_LEN            (sizeof("-9223372036854775808") - 1)
#else
typedef int32_t xatomic_int_t;
typedef uint32_t xatomic_uint_t;
#define xATOMIC_T_LEN            (sizeof("-2147483648") - 1)
#endif
typedef volatile xatomic_uint_t xatomic_t;
#if ( __SUNPRO_C )
#define xHAVE_ATOMIC_OPS  1
#include "xsunpro_atomic_sparc64.h"
#else 
#define xHAVE_ATOMIC_OPS  1
#include "xgcc_atomic_sparc64.h"
#endif
#elif ( __powerpc__ || __POWERPC__ )
#define xHAVE_ATOMIC_OPS  1
#if (xPTR_SIZE == 8)
typedef int64_t xatomic_int_t;
typedef uint64_t xatomic_uint_t;
#define xATOMIC_T_LEN            (sizeof("-9223372036854775808") - 1)
#else
typedef int32_t xatomic_int_t;
typedef uint32_t xatomic_uint_t;
#define xATOMIC_T_LEN            (sizeof("-2147483648") - 1)
#endif
typedef volatile xatomic_uint_t xatomic_t;
#include "xgcc_atomic_ppc.h"
#endif
#if !(xHAVE_ATOMIC_OPS)
#define xHAVE_ATOMIC_OPS  0
typedef int32_t xatomic_int_t;
typedef uint32_t xatomic_uint_t;
typedef volatile xatomic_uint_t xatomic_t;
#define xATOMIC_T_LEN            (sizeof("-2147483648") - 1)
static xinline xatomic_uint_t
xatomic_cmp_set(xatomic_t *lock, xatomic_uint_t old,
		xatomic_uint_t set)
{
	if (*lock == old)
	{
		*lock = set;
		return 1;
	}
	return 0;
}
static xinline xatomic_int_t
xatomic_fetch_add(xatomic_t *value, xatomic_int_t add)
{
	xatomic_int_t old;
	old = *value;
	*value += add;
	return old;
}
#define xmemory_barrier()
#define xcpu_pause()
#endif
#endif
