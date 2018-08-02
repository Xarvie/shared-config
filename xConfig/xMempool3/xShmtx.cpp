#include "xmConfig.h"
#include "xCore.h"
#include "xShmtx.h"
#if (xHAVE_ATOMIC_OPS)
xint_t xshmtx_create(xshmtx_t *mtx, xshmtx_sh_t *addr, u_char *name)
{
	mtx->lock = &addr->lock;
	if (mtx->spin == (xuint_t) -1)
	{
		return xOK;
	}
	mtx->spin = 2048;
#if (xHAVE_POSIX_SEM)
	mtx->wait = &addr->wait;
	if (sem_init(&mtx->sem, 1, 0) == -1)
	{
		xlog_error(xLOG_ALERT, xcycle->log, xerrno,
				"sem_init() failed");
	}
	else
	{
		mtx->semaphore = 1;
	}
#endif
	return xOK;
}
void xshmtx_destroy(xshmtx_t *mtx)
{
#if (xHAVE_POSIX_SEM)
	if (mtx->semaphore)
	{
		if (sem_destroy(&mtx->sem) == -1)
		{
			xlog_error(xLOG_ALERT, xcycle->log, xerrno,
					"sem_destroy() failed");
		}
	}
#endif
}
xuint_t xshmtx_trylock(xshmtx_t *mtx)
{
	return xuint_t();
}
void xshmtx_lock(xshmtx_t *mtx)
{
}
void xshmtx_unlock(xshmtx_t *mtx)
{
}
xuint_t xshmtx_force_unlock(xshmtx_t *mtx, xpid_t pid)
{
	return 0;
}
void xshmtx_wakeup(xshmtx_t *mtx)
{
#if (xHAVE_POSIX_SEM)
	xatomic_uint_t wait;
	if (!mtx->semaphore)
	{
		return;
	}
	for (;; )
	{
		wait = *mtx->wait;
		if ((xatomic_int_t) wait <= 0)
		{
			return;
		}
		if (xatomic_cmp_set(mtx->wait, wait, wait - 1))
		{
			break;
		}
	}
	xlog_debug1(xLOG_DEBUG_CORE, xcycle->log, 0,
			"shmtx wake %uA", wait);
	if (sem_post(&mtx->sem) == -1)
	{
		xlog_error(xLOG_ALERT, xcycle->log, xerrno,
				"sem_post() failed while wake shmtx");
	}
#endif
}
#else
xint_t
xshmtx_create(xshmtx_t *mtx, xshmtx_sh_t *addr, u_char *name)
{
	return xOK;
}
void
xshmtx_destroy(xshmtx_t *mtx)
{
}
xuint_t
xshmtx_trylock(xshmtx_t *mtx)
{
	return 0;
}
void
xshmtx_lock(xshmtx_t *mtx)
{
}
void
xshmtx_unlock(xshmtx_t *mtx)
{
}
xuint_t
xshmtx_force_unlock(xshmtx_t *mtx, xpid_t pid)
{
	return 0;
}
#endif
