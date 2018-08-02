#pragma once
#include "xJson/rapidjson/document.h"
#include "xJson/rapidjson/prettywriter.h"
#include "xShare/xSharedMemory.h"
#include "ShGameConfig.h"
namespace xJson
{
class ShGameConfig;
}
extern xJson::ShGameConfig * xCONFIG_INSTANCE;
class xSharer
{
public:
	static xJson::ShGameConfig* init(const char* path, unsigned char shm_id, size_t shm_size = 209715200);
	static int finish();
};
class xAccesser
{
public:
	static xJson::ShGameConfig* init(void * share_ptr, const char* path, unsigned char shm_id, size_t shm_size = 209715200);
	static int finish();
};
