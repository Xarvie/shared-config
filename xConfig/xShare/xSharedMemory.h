#pragma once
#include <iostream>
#include <map>
#include <vector>
#include <sys/types.h>
#include <sys/ipc.h>
#define mempool3
#if defined(mempool1)
#include "../mempool1/xslab.h"
#define malloc_s(s) xmalloc((xslab_pool_t*)shm.shm, s)
#define free_s(s) xfree((xslab_pool_t*)shm.shm, s)
#elif defined(mempool2)
#include "../mempool2/shmalloc.h"
#define malloc_s(s) malloc_(sh.shmid, s, shm.shm, shm_size, __FILE__, __LINE__)
#define free_s(s) free_(sh.shm, shm_size, s,  __FILE__, __LINE__)
#elif defined(mempool3)
#include "../xMempool3/xSlab.h"
#define malloc_s(s) xmalloc((xslab_pool_t*)shm.shm_ptr, s)
#define free_s(s) xfree((xslab_pool_t*)shm.shm_ptr, s)
#elif defined(syspool)
#include <cstdlib>
#define malloc_s(s) malloc(s)
#define free_s(x) free(x)
#endif
#define new_s(x) new (malloc_s(sizeof (x))) x
extern void* smalloc(size_t size);
extern void* scalloc(size_t s1, size_t s2);
extern void* srealloc(void* p, size_t size);
extern void sfree(void * p);
#include "../xJson/rapidjson/document.h"     
#include "../xJson/rapidjson/prettywriter.h" 
using namespace rapidjson;
using namespace std;
class ShAllocator {
public:
    static const bool kNeedFree = true;
    void* Malloc(size_t size) {
        if (size) 
            return smalloc(size);
        else
            return NULL; 
    }
    void* Realloc(void* originalPtr, size_t originalSize, size_t newSize) {
        (void)originalSize;
        if (newSize == 0) {
            sfree(originalPtr);
            return NULL;
        }
        sfree(originalPtr);
        return smalloc(newSize);
    }
    static void Free(void *ptr) { sfree(ptr); }
};
namespace xJson
{
typedef GenericDocument<UTF8<> , MemoryPoolAllocator<ShAllocator> > xDocument;
typedef GenericValue<UTF8<>, MemoryPoolAllocator<ShAllocator> > xValue;
}
struct SharedMemory
{
	void* shm_ptr;
	int shmid;
	key_t shm_key;
	int shm_init_begin(const char* path);
	int shm_init_end(const char* path);
	xslab_pool_t* shm_init(const char* path, unsigned char shm_id, size_t shm_size);
	xslab_pool_t* shm_access_init(void * share_ptr, const char* path, unsigned char shm_id, size_t shm_size);
	int shm_end();
	int shm_access_end();
};
extern short int shm_id;
extern struct SharedMemory shm;
#include "../xShare/xShareAllocator.hpp"
typedef std::basic_string<char, struct std::char_traits<char>,  shared_stl_allocator< std::basic_string<char> > > shString;
typedef std::basic_ostringstream<char, struct std::char_traits<char>,  shared_stl_allocator< std::basic_string<char> > > shOstringstream;
#define error_out(FMT, ...) do {\
		FILE * fid = fopen("txt_out","a+");\
		char tmp_str[256];\
		sprintf(tmp_str, "in %s:%d %s: "#FMT"\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__);\
		fputs(tmp_str,fid);\
		fclose(fid);\
}while(0)
