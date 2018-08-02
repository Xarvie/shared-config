#include "xSharedMemory.h"
#include <stddef.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>
#include <iostream>
#include <errno.h>
#include <map>
#include <assert.h>
struct SharedMemory shm;


int SharedMemory::shm_init_begin(const char* path)
{
	string s(path);
	s += "/shmid";
	FILE * ffp = fopen(s.c_str(), "rb");
	if (ffp)
	{
		int old_id = 0;
		fread(&old_id, sizeof(int), 1, ffp);
		fclose(ffp);
		int rt = shmctl(old_id, IPC_RMID, 0);
		//assert(rt != -1);
		if(rt == -1)
		{
		    FILE *pfp;
		    char buff[512], cmd[128];
		    sprintf(cmd,"ipcrm -m %d", old_id);
		    if ((pfp=popen(cmd, "r")) != NULL)
		        if (fgets(buff, 512, pfp) != NULL)
		        	std::cout << buff << std::endl;
		    pclose(pfp);
		}
	}
	return 0;
}
int SharedMemory::shm_init_end(const char* path)
{
	string s(path);
	s += "/shmid";
	FILE * ffp = fopen(s.c_str(), "wb");
	if(ffp)
	{
		fwrite((void *)&this->shmid, sizeof(int), 1, ffp);
		fclose(ffp);
	}
	return 0;
}
xslab_pool_t* SharedMemory::shm_init(const char* path, unsigned char shm_id,
		size_t shm_size)
{
	shm_init_begin(path);
	xslab_pool_t* rt = NULL;
	shm_key = ftok(path, shm_id);
	assert(shm_key != -1);
	shmid = shmget(shm_key, shm_size, 0777 | IPC_CREAT);
	assert(shmid != -1);
	shm_ptr = shmat(shmid, 0, 0);
	assert(shm_ptr != (void * ) -1);
#if defined(mempool1)
	static_init(shm_ptr, shm_size);
#elif defined (mempool3)
	rt = static_init_3(shm_ptr, shm_size);
#endif
	shm_init_end(path);
	return rt;
}
int SharedMemory::shm_end()
{
	int rt = shmdt(shm_ptr);
	assert(rt != -1);
	rt = shmctl(shmid, IPC_RMID, 0);
	//assert(rt != -1);
	return 0;
}
xslab_pool_t* SharedMemory::shm_access_init(void * share_ptr, const char* path,
		unsigned char shm_id, size_t shm_size)
{
	xslab_pool_t* rt = NULL;
	shm_key = ftok(path, shm_id);
	assert(shm_key != -1);
	shmid = shmget(shm_key, shm_size, 0666 | IPC_CREAT);
	assert(shmid != -1);
	shm_ptr = shmat(shmid, share_ptr, 0);
	assert(shm_ptr != (void * ) -1);
#if defined(mempool1)
	static_init(shm_ptr, shm_size);
#elif defined (mempool3)
	rt = static_access_init_3(shm_ptr, shm_size);
#endif
	return rt;
}
int SharedMemory::shm_access_end()
{
	int rt = shmdt(shm_ptr);
	//assert(rt != -1);
	return rt;
}
void* smalloc(size_t size)
{
	if(size == 0)
		return NULL;
	return malloc_s(size);
}
void* scalloc(size_t s1, size_t s2)
{
	void * p = malloc_s(s1*s2);
	memset(p, 0, s1 * s2);
	return p;
}
void* srealloc(void* p, size_t size)
{
	sfree(p);
	return malloc_s(size);
}
void sfree(void * p)
{
	if (p == NULL)
		return;
	free_s(p);
}
