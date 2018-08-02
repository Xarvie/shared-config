#include "xmConfig.h"
#include "xCore.h"
#include "xSlab.h"
#include <stddef.h>
#include <cstdio>
#include <stdlib.h>

#define xSLAB_PAGE_MASK   3
#define xSLAB_PAGE        0
#define xSLAB_BIG         1
#define xSLAB_EXACT       2
#define xSLAB_SMALL       3
#if (xPTR_SIZE == 4)
#define xSLAB_PAGE_FREE   0
#define xSLAB_PAGE_BUSY   0xffffffff
#define xSLAB_PAGE_START  0x80000000
#define xSLAB_SHIFT_MASK  0x0000000f
#define xSLAB_MAP_MASK    0xffff0000
#define xSLAB_MAP_SHIFT   16
#define xSLAB_BUSY        0xffffffff
#else 
#define xSLAB_PAGE_FREE   0
#define xSLAB_PAGE_BUSY   0xffffffffffffffff
#define xSLAB_PAGE_START  0x8000000000000000
#define xSLAB_SHIFT_MASK  0x000000000000000f
#define xSLAB_MAP_MASK    0xffffffff00000000
#define xSLAB_MAP_SHIFT   32
#define xSLAB_BUSY        0xffffffffffffffff
#endif
#define xslab_slots(pool)                                                  \
    (xslab_page_t *) ((u_char *) (pool) + sizeof(xslab_pool_t))
#define xslab_page_type(page)   ((page)->prev & xSLAB_PAGE_MASK)
#define xslab_page_prev(page)                                              \
    (xslab_page_t *) ((page)->prev & ~xSLAB_PAGE_MASK)
#define xslab_page_addr(pool, page)                                        \
    ((((page) - (pool)->pages) << xpagesize_shift)                         \
     + (uintptr_t) (pool)->start)
#if (xDEBUG_MALLOC)
#define xslab_junk(p, size)     xmemset(p, 0xA5, size)
#elif (xHAVE_DEBUG_MALLOC)
#define xslab_junk(p, size)                                                \
    if (xdebug_malloc)          xmemset(p, 0xA5, size)
#else
#define xslab_junk(p, size)
#endif
xuint_t xpagesize;
xuint_t xpagesize_shift;
xuint_t xcacheline_size;
#define xTIME_SLOTS   64
volatile xtime_t *xcached_time;
static xslab_page_t *xslab_alloc_pages(xslab_pool_t *pool,
		xuint_t pages);
static void xslab_free_pages(xslab_pool_t *pool, xslab_page_t *page,
		xuint_t pages);
static xuint_t xslab_max_size;
static xuint_t xslab_exact_size;
static xuint_t xslab_exact_shift;
void xslab_sizes_init(void)
{
	xuint_t n;
	xslab_max_size = xpagesize / 2;
	xslab_exact_size = xpagesize / (8 * sizeof(uintptr_t));
	for (n = xslab_exact_size; n >>= 1; xslab_exact_shift++)
	{
	}
}
void xslab_init(xslab_pool_t *pool)
{
	u_char *p;
	size_t size;
	xint_t m;
	xuint_t i, n, pages;
	xslab_page_t *slots, *page;
	pool->min_size = (size_t) 1 << pool->min_shift;
	slots = xslab_slots(pool);
	p = (u_char *) slots;
	size = pool->end - p;
	xslab_junk(p, size);
	n = xpagesize_shift - pool->min_shift;
	for (i = 0; i < n; i++)
	{
		slots[i].slab = 0;
		slots[i].next = &slots[i];
		slots[i].prev = 0;
	}
	p += n * sizeof(xslab_page_t);
	pool->stats = (xslab_stat_t *) p;
	memset(pool->stats, 0, n * sizeof(xslab_stat_t));
	p += n * sizeof(xslab_stat_t);
	size -= n * (sizeof(xslab_page_t) + sizeof(xslab_stat_t));
	pages = (xuint_t) (size / (xpagesize + sizeof(xslab_page_t)));
	pool->pages = (xslab_page_t *) p;
	memset(pool->pages, 0, pages * sizeof(xslab_page_t));
	page = pool->pages;
	pool->free.slab = 0;
	pool->free.next = page;
	pool->free.prev = 0;
	page->slab = pages;
	page->next = &pool->free;
	page->prev = (uintptr_t) &pool->free;
	pool->start = xalign_ptr(p + pages * sizeof(xslab_page_t),
			xpagesize);
	m = pages - (pool->end - pool->start) / xpagesize;
	if (m > 0)
	{
		pages -= m;
		page->slab = pages;
	}
	pool->last = pool->pages + pages;
	pool->pfree = pages;
	pool->log_nomem = 1;
	pool->log_ctx = &pool->zero;
	pool->zero = '\0';
}
#include <iostream>
namespace global
{
int num1 = -3;
int num2 = 0;
int dis_malloc = 0;
}
void * xmalloc(xslab_pool_t *pool, size_t size)
{
	if(global::dis_malloc)
		abort();
	void *p;
	global::num1++;
	xshmtx_lock(&pool->mutex);
	p = xslab_alloc_locked(pool, size);
	xshmtx_unlock(&pool->mutex);
	return p;
}
void *
xslab_alloc_locked(xslab_pool_t *pool, size_t size)
{
	size_t s;
	uintptr_t p, m, mask, *bitmap;
	xuint_t i, n, slot, shift, map;
	xslab_page_t *page, *prev, *slots;
	if (size > xslab_max_size)
	{
		page = xslab_alloc_pages(pool,
				(size >> xpagesize_shift) + ((size % xpagesize) ? 1 : 0));
		if (page)
		{
			p = xslab_page_addr(pool, page);
		}
		else
		{
			p = 0;
			printf("xslab_error, xLOG_CRIT, xslab_alloc() failed: no memory \n");
			abort();
		}
		goto done;
	}
	if (size > pool->min_size)
	{
		shift = 1;
		for (s = size - 1; s >>= 1; shift++)
		{
		}
		slot = shift - pool->min_shift;
	}
	else
	{
		shift = pool->min_shift;
		slot = 0;
	}
	pool->stats[slot].reqs++;
	slots = xslab_slots(pool);
	page = slots[slot].next;
	if (page->next != page)
	{
		if (shift < xslab_exact_shift)
		{
			bitmap = (uintptr_t *) xslab_page_addr(pool, page);
			map = (xpagesize >> shift) / (8 * sizeof(uintptr_t));
			for (n = 0; n < map; n++)
			{
				if (bitmap[n] != xSLAB_BUSY)
				{
					for (m = 1, i = 0; m; m <<= 1, i++)
					{
						if (bitmap[n] & m)
						{
							continue;
						}
						bitmap[n] |= m;
						i = (n * 8 * sizeof(uintptr_t) + i) << shift;
						p = (uintptr_t) bitmap + i;
						pool->stats[slot].used++;
						if (bitmap[n] == xSLAB_BUSY)
						{
							for (n = n + 1; n < map; n++)
							{
								if (bitmap[n] != xSLAB_BUSY)
								{
									goto done;
								}
							}
							prev = xslab_page_prev(page);
							prev->next = page->next;
							page->next->prev = page->prev;
							page->next = NULL;
							page->prev = xSLAB_SMALL;
						}
						goto done;
					}
				}
			}
		}
		else if (shift == xslab_exact_shift)
		{
			for (m = 1, i = 0; m; m <<= 1, i++)
			{
				if (page->slab & m)
				{
					continue;
				}
				page->slab |= m;
				if (page->slab == xSLAB_BUSY)
				{
					prev = xslab_page_prev(page);
					prev->next = page->next;
					page->next->prev = page->prev;
					page->next = NULL;
					page->prev = xSLAB_EXACT;
				}
				p = xslab_page_addr(pool, page) + (i << shift);
				pool->stats[slot].used++;
				goto done;
			}
		}
		else
		{
			mask = ((uintptr_t) 1 << (xpagesize >> shift)) - 1;
			mask <<= xSLAB_MAP_SHIFT;
			for (m = (uintptr_t) 1 << xSLAB_MAP_SHIFT, i = 0; m & mask; m <<=
					1, i++)
			{
				if (page->slab & m)
				{
					continue;
				}
				page->slab |= m;
				if ((page->slab & xSLAB_MAP_MASK) == mask)
				{
					prev = xslab_page_prev(page);
					prev->next = page->next;
					page->next->prev = page->prev;
					page->next = NULL;
					page->prev = xSLAB_BIG;
				}
				p = xslab_page_addr(pool, page) + (i << shift);
				pool->stats[slot].used++;
				goto done;
			}
		}
		printf("ALERT_xslab_alloc(): page is busy");
	}
	page = xslab_alloc_pages(pool, 1);
	if (page)
	{
		if (shift < xslab_exact_shift)
		{
			bitmap = (uintptr_t *) xslab_page_addr(pool, page);
			n = (xpagesize >> shift) / ((1 << shift) * 8);
			if (n == 0)
			{
				n = 1;
			}
			for (i = 0; i < (n + 1) / (8 * sizeof(uintptr_t)); i++)
			{
				bitmap[i] = xSLAB_BUSY;
			}
			m = ((uintptr_t) 1 << ((n + 1) % (8 * sizeof(uintptr_t)))) - 1;
			bitmap[i] = m;
			map = (xpagesize >> shift) / (8 * sizeof(uintptr_t));
			for (i = i + 1; i < map; i++)
			{
				bitmap[i] = 0;
			}
			page->slab = shift;
			page->next = &slots[slot];
			page->prev = (uintptr_t) &slots[slot] | xSLAB_SMALL;
			slots[slot].next = page;
			pool->stats[slot].total += (xpagesize >> shift) - n;
			p = xslab_page_addr(pool, page) + (n << shift);
			pool->stats[slot].used++;
			goto done;
		}
		else if (shift == xslab_exact_shift)
		{
			page->slab = 1;
			page->next = &slots[slot];
			page->prev = (uintptr_t) &slots[slot] | xSLAB_EXACT;
			slots[slot].next = page;
			pool->stats[slot].total += 8 * sizeof(uintptr_t);
			p = xslab_page_addr(pool, page);
			pool->stats[slot].used++;
			goto done;
		}
		else
		{
			page->slab = ((uintptr_t) 1 << xSLAB_MAP_SHIFT) | shift;
			page->next = &slots[slot];
			page->prev = (uintptr_t) &slots[slot] | xSLAB_BIG;
			slots[slot].next = page;
			pool->stats[slot].total += xpagesize >> shift;
			p = xslab_page_addr(pool, page);
			pool->stats[slot].used++;
			goto done;
		}
	}
	p = 0;
	pool->stats[slot].fails++;
	done: return (void *) p;
}
void *
xslab_calloc(xslab_pool_t *pool, size_t size)
{
	void *p;
	xshmtx_lock(&pool->mutex);
	p = xslab_calloc_locked(pool, size);
	xshmtx_unlock(&pool->mutex);
	return p;
}
void *
xslab_calloc_locked(xslab_pool_t *pool, size_t size)
{
	void *p;
	p = xslab_alloc_locked(pool, size);
	if (p)
	{
		memset(p, 0, size);
	}
	return p;
}
void xfree(xslab_pool_t *pool, void *p)
{
	global::num2++;
	xshmtx_lock(&pool->mutex);
	xslab_free_locked(pool, p);
	xshmtx_unlock(&pool->mutex);
}
void xslab_free_locked(xslab_pool_t *pool, void *p)
{
	size_t size;
	uintptr_t slab, m, *bitmap;
	xuint_t i, n, type, slot, shift, map;
	xslab_page_t *slots, *page;
	if ((u_char *) p < pool->start || (u_char *) p > pool->end)
	{
		printf("ALERT,xslab_free(): outside of pool");
		goto fail;
	}
	n = ((u_char *) p - pool->start) >> xpagesize_shift;
	page = &pool->pages[n];
	slab = page->slab;
	type = xslab_page_type(page);
	switch (type)
	{
	case xSLAB_SMALL:
		shift = slab & xSLAB_SHIFT_MASK;
		size = (size_t) 1 << shift;
		if ((uintptr_t) p & (size - 1))
		{
			goto wrong_chunk;
		}
		n = ((uintptr_t) p & (xpagesize - 1)) >> shift;
		m = (uintptr_t) 1 << (n % (8 * sizeof(uintptr_t)));
		n /= 8 * sizeof(uintptr_t);
		bitmap = (uintptr_t *) ((uintptr_t) p & ~((uintptr_t) xpagesize - 1));
		if (bitmap[n] & m)
		{
			slot = shift - pool->min_shift;
			if (page->next == NULL)
			{
				slots = xslab_slots(pool);
				page->next = slots[slot].next;
				slots[slot].next = page;
				page->prev = (uintptr_t) &slots[slot] | xSLAB_SMALL;
				page->next->prev = (uintptr_t) page | xSLAB_SMALL;
			}
			bitmap[n] &= ~m;
			n = (xpagesize >> shift) / ((1 << shift) * 8);
			if (n == 0)
			{
				n = 1;
			}
			i = n / (8 * sizeof(uintptr_t));
			m = ((uintptr_t) 1 << (n % (8 * sizeof(uintptr_t)))) - 1;
			if (bitmap[i] & ~m)
			{
				goto done;
			}
			map = (xpagesize >> shift) / (8 * sizeof(uintptr_t));
			for (i = i + 1; i < map; i++)
			{
				if (bitmap[i])
				{
					goto done;
				}
			}
			xslab_free_pages(pool, page, 1);
			pool->stats[slot].total -= (xpagesize >> shift) - n;
			goto done;
		}
		goto chunk_already_free;
	case xSLAB_EXACT:
		m = (uintptr_t) 1
				<< (((uintptr_t) p & (xpagesize - 1)) >> xslab_exact_shift);
		size = xslab_exact_size;
		if ((uintptr_t) p & (size - 1))
		{
			goto wrong_chunk;
		}
		if (slab & m)
		{
			slot = xslab_exact_shift - pool->min_shift;
			if (slab == xSLAB_BUSY)
			{
				slots = xslab_slots(pool);
				page->next = slots[slot].next;
				slots[slot].next = page;
				page->prev = (uintptr_t) &slots[slot] | xSLAB_EXACT;
				page->next->prev = (uintptr_t) page | xSLAB_EXACT;
			}
			page->slab &= ~m;
			if (page->slab)
			{
				goto done;
			}
			xslab_free_pages(pool, page, 1);
			pool->stats[slot].total -= 8 * sizeof(uintptr_t);
			goto done;
		}
		goto chunk_already_free;
	case xSLAB_BIG:
		shift = slab & xSLAB_SHIFT_MASK;
		size = (size_t) 1 << shift;
		if ((uintptr_t) p & (size - 1))
		{
			goto wrong_chunk;
		}
		m = (uintptr_t) 1
				<< ((((uintptr_t) p & (xpagesize - 1)) >> shift)
						+ xSLAB_MAP_SHIFT);
		if (slab & m)
		{
			slot = shift - pool->min_shift;
			if (page->next == NULL)
			{
				slots = xslab_slots(pool);
				page->next = slots[slot].next;
				slots[slot].next = page;
				page->prev = (uintptr_t) &slots[slot] | xSLAB_BIG;
				page->next->prev = (uintptr_t) page | xSLAB_BIG;
			}
			page->slab &= ~m;
			if (page->slab & xSLAB_MAP_MASK)
			{
				goto done;
			}
			xslab_free_pages(pool, page, 1);
			pool->stats[slot].total -= xpagesize >> shift;
			goto done;
		}
		goto chunk_already_free;
	case xSLAB_PAGE:
		if ((uintptr_t) p & (xpagesize - 1))
		{
			goto wrong_chunk;
		}
		if (!(slab & xSLAB_PAGE_START))
		{
			printf("error ALERT xslab_free(): page is already free");
			goto fail;
		}
		if (slab == xSLAB_PAGE_BUSY)
		{
			printf("error ALERT xslab_free(): pointer to wrong page");
			goto fail;
		}
		n = ((u_char *) p - pool->start) >> xpagesize_shift;
		size = slab & ~xSLAB_PAGE_START;
		xslab_free_pages(pool, &pool->pages[n], size);
		xslab_junk(p, size << xpagesize_shift);
		return;
	}
	return;
	done: pool->stats[slot].used--;
	xslab_junk(p, size);
	return;
	wrong_chunk: printf(
			"error xLOG_ALERT,xslab_free(): pointer to wrong chunk");
	goto fail;
	chunk_already_free: printf(
			"error(xLOG_ALERT, xslab_free(): chunk is already free");
	fail: return;
}
static xslab_page_t *
xslab_alloc_pages(xslab_pool_t *pool, xuint_t pages)
{
	xslab_page_t *page, *p;
	for (page = pool->free.next; page != &pool->free; page = page->next)
	{
		if (page->slab >= pages)
		{
			if (page->slab > pages)
			{
				page[page->slab - 1].prev = (uintptr_t) &page[pages];
				page[pages].slab = page->slab - pages;
				page[pages].next = page->next;
				page[pages].prev = page->prev;
				p = (xslab_page_t *) page->prev;
				p->next = &page[pages];
				page->next->prev = (uintptr_t) &page[pages];
			}
			else
			{
				p = (xslab_page_t *) page->prev;
				p->next = page->next;
				page->next->prev = page->prev;
			}
			page->slab = pages | xSLAB_PAGE_START;
			page->next = NULL;
			page->prev = xSLAB_PAGE;
			pool->pfree -= pages;
			if (--pages == 0)
			{
				return page;
			}
			for (p = page + 1; pages; pages--)
			{
				p->slab = xSLAB_PAGE_BUSY;
				p->next = NULL;
				p->prev = xSLAB_PAGE;
				p++;
			}
			return page;
		}
	}
	if (pool->log_nomem)
	{
		printf("xslab_error, xLOG_CRIT, xslab_alloc() failed: no memory \n");
	}
	return NULL;
}
static void xslab_free_pages(xslab_pool_t *pool, xslab_page_t *page,
		xuint_t pages)
{
	xslab_page_t *prev, *join;
	pool->pfree += pages;
	page->slab = pages--;
	if (pages)
	{
		memset(&page[1], 0, pages * sizeof(xslab_page_t));
	}
	if (page->next)
	{
		prev = xslab_page_prev(page);
		prev->next = page->next;
		page->next->prev = page->prev;
	}
	join = page + page->slab;
	if (join < pool->last)
	{
		if (xslab_page_type(join) == xSLAB_PAGE)
		{
			if (join->next != NULL)
			{
				pages += join->slab;
				page->slab += join->slab;
				prev = xslab_page_prev(join);
				prev->next = join->next;
				join->next->prev = join->prev;
				join->slab = xSLAB_PAGE_FREE;
				join->next = NULL;
				join->prev = xSLAB_PAGE;
			}
		}
	}
	if (page > pool->pages)
	{
		join = page - 1;
		if (xslab_page_type(join) == xSLAB_PAGE)
		{
			if (join->slab == xSLAB_PAGE_FREE)
			{
				join = xslab_page_prev(join);
			}
			if (join->next != NULL)
			{
				pages += join->slab;
				join->slab += page->slab;
				prev = xslab_page_prev(join);
				prev->next = join->next;
				join->next->prev = join->prev;
				page->slab = xSLAB_PAGE_FREE;
				page->next = NULL;
				page->prev = xSLAB_PAGE;
				page = join;
			}
		}
	}
	if (pages)
	{
		page[pages].prev = (uintptr_t) page;
	}
	page->prev = (uintptr_t) &pool->free;
	page->next = pool->free.next;
	page->next->prev = (uintptr_t) page;
	pool->free.next = page;
}
xslab_pool_t * static_init_3(void * begin_ptr, size_t size)
{
	xslab_pool_t *sp;
	sp = (xslab_pool_t*) begin_ptr;
	sp->min_shift = 3;
	sp->end = (u_char *)begin_ptr + size;
	xuint_t x;
	xpagesize = getpagesize();
	for (x = xpagesize; x >>= 1; xpagesize_shift++);
	xslab_sizes_init();
	xslab_init(sp);
	return sp;
}

xslab_pool_t * static_access_init_3(void * begin_ptr, size_t size)
{
	xslab_pool_t *sp;
	sp = (xslab_pool_t*) begin_ptr;
	return sp;
}
