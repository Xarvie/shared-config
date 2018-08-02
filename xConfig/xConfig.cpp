#include "xConfig.h"
#include <cstdio>
#include <cstring>
using namespace xJson;
ShGameConfig * xCONFIG_INSTANCE;
ShGameConfig * xSharer::init(const char* path, unsigned char shm_id, size_t shm_size)
{
	xslab_pool_t* rt = shm.shm_init(path, shm_id, shm_size);
	ShGameConfig * xCONFIG_INSTANCE  = new_s(ShGameConfig);
	rt->user_data = (void*)xCONFIG_INSTANCE;
    return xCONFIG_INSTANCE;
}
int xSharer::finish()
{
	shm.shm_end();
	return 0;
}
ShGameConfig * xAccesser::init(void * share_ptr, const char* path, unsigned char shm_id, size_t shm_size)
{
	xslab_pool_t* rt = shm.shm_access_init(share_ptr, path, shm_id, shm_size);
	ShGameConfig * xCONFIG_INSTANCE = (ShGameConfig *)rt->user_data;
	return xCONFIG_INSTANCE;
}
int xAccesser::finish()
{
	shm.shm_access_end();
	return 0;
}
