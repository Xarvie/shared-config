#pragma once
#ifndef _xCONFIG_H_INCLUDED_
#define _xCONFIG_H_INCLUDED_
#include <stddef.h>
#include <stdint.h>
typedef intptr_t xint_t;
typedef uintptr_t xuint_t;
typedef intptr_t xflag_t;
#define xalign(d, a)     (((d) + (a - 1)) & ~(a - 1))
#define xalign_ptr(p, a)                                                   \
    (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))
extern xuint_t xpagesize;
extern xuint_t xpagesize_shift;
#endif 
