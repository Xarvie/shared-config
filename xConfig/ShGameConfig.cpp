#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <zlib.h>
#include <dirent.h>
#include <netdb.h>
#include "ShGameConfig.h"
#include "xShare/xSharedMemory.h"


namespace xJson {
xValue value_null(kNullType);
ShGameConfig::ShGameConfig() {
}
ShGameConfig::~ShGameConfig() {
}
ShGameConfig::xBasicConfig::xBasicConfig(void) :
		__cur_version(0)
{
	for(int i = 0; i< MAX_VERSION_CNT; i++)
	{
		this->__version_no[i] = shString();
		this->__map.push_back(ValueMap());
		//this->__json[i].;
	//	this->__map[i].clear();
	}
}
xDocument &ShGameConfig::xBasicConfig::json(void)
{
	return this->__json[this->__cur_version];
}
xDocument &ShGameConfig::xBasicConfig::revert_json(void)
{
	return this->__json[((this->__cur_version + 1) % MAX_VERSION_CNT)];
}
ValueMap &ShGameConfig::xBasicConfig::map(void)
{
    return this->__map[this->__cur_version];
}
ValueMap &ShGameConfig::xBasicConfig::revert_map(void)
{
	return this->__map[((this->__cur_version + 1) % MAX_VERSION_CNT)];
}
int ShGameConfig::xBasicConfig::current_version(void)
{
    return this->__cur_version;
}
int ShGameConfig::xBasicConfig::prev_version(void)
{
    return ((this->__cur_version + 1) % MAX_VERSION_CNT);
}
void ShGameConfig::xBasicConfig::update_version(void)
{
    this->__cur_version = (this->__cur_version + 1) % MAX_VERSION_CNT;
    time_t now ;
    struct tm *tm_now ;
    time(&now) ;
    tm_now = localtime(&now) ;
    long y = tm_now->tm_year+1900, m = tm_now->tm_mon+1, d = tm_now->tm_mday, h = tm_now->tm_hour, min = tm_now->tm_min, s = tm_now->tm_sec;
    char version_no[64 + 1];
    ::snprintf(version_no, 64, "%04ld%02ld%02ld%02ld%02ld%02ld",
    		y, m, d, h, min, s);
    version_no[64] = '\0';
    this->__version_no[this->__cur_version] = version_no;
}
void ShGameConfig::xBasicConfig::revert_version(void)
{
    this->__cur_version = (this->__cur_version + MAX_VERSION_CNT - 1) % MAX_VERSION_CNT;
}
inline const unsigned char *xskipBOM(const unsigned char *str, int *size)
{
	const char *p = "\xEF\xBB\xBF";
	const unsigned char *c = (const unsigned char *) str;
	int i = 0;
	do
	{
		if (i > *size)
			break;
		if (*c != *(const unsigned char *) p++)
		{
			*size -= i;
			return c;
		}
		++i;
		++c;
	} while (*p != '\0');
	*size -= i;
	return c;
}
int ShGameConfig::load_json_config(const char *doc, xDocument &conf)
{
	int file = ::open(doc, O_RDONLY);
	if (file < 0)
		return -1;
	struct stat statbuf;
	if (::fstat(file, &statbuf) < 0)
	{
		return -1;
	}
	void *src = 0;
	if ((src = ::mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, file, 0))
			== MAP_FAILED)
	{
		return -1;
	}
	int skip_src_size = statbuf.st_size;
	const unsigned char *skip_src = xskipBOM((const unsigned char *) src,
			&skip_src_size);
	//error_out(".");
	xDocument tmp;
	if(tmp.Parse((const char *) skip_src).HasParseError())
		return -1;
	conf.Swap(tmp);
	//error_out(".");
	if (::munmap(src, statbuf.st_size) < 0)
	{
		return -1;
	}
	::close(file);
	return 0;
}
const xJson::xDocument &ShGameConfig::activity()
{
	return this->activity_config_.json();
}
int ShGameConfig::init(const std::string& server_name)
{
	if(server_name == "daemon" || server_name == "debug")
	{
	    this->load_item_config();
	    this->load_boss_drop_reward();
	    this->load_monster_drop_reward();
	    return 0;
	}

    return 0;
}
int ShGameConfig::load_item_config()
{
    {
        if (this->load_json_config("config/item/item.json",
                this->item_config_.item_.revert_json()) == 0)
        {
        	this->item_config_.item_.convert_json_to_map();
            this->item_config_.item_.update_version();
        }
    }
    return 0;
}
int ShGameConfig::load_boss_drop_reward()
{
    if (this->load_json_config("config/boss/boss_drop_reward.json",
            this->boss_drop_reward_.revert_json()) == 0)
    {
    	this->boss_drop_reward_.convert_json_to_map();
        this->boss_drop_reward_.update_version();
    }
    return 0;
}
int ShGameConfig::load_monster_drop_reward()
{
    if (this->load_json_config("config/monster/monster_drop_group_reward.json",
            this->monster_drop_reward_.revert_json()) == 0)
    {
    	this->monster_drop_reward_.convert_json_to_map();
        this->monster_drop_reward_.update_version();
    }
    return 0;
}
const xJson::xValue &ShGameConfig::item(int item_id)
{
    const ValueMap& tmp_map = this->item_config_.item_.map();

    ValueMap::const_iterator it =tmp_map.find(item_id);
    if(it == tmp_map.end())
    		return xJson::value_null;
    if(it->second == NULL)
    	return xJson::value_null;
    return *it->second;
}
void ShGameConfig::xBasicConfig::convert_json_to_map(int debug_flag)
{
	this->revert_map().clear();
    for (xJson::xValue::const_iterator iter = this->revert_json().begin();
            iter != this->revert_json().end(); ++iter)
    {
    	const char * key = iter->name.GetString();
    	int i =::atoi(key);
    	const xJson::xValue *json = &iter->value;
        this->revert_map()[i] = json;
    }
}
int ShGameConfig::update_config(const char * str)
{
	//error_out("--");
	std::string folder(str);
	if(folder == "item")
	{
		this->load_item_config();
	}
	if(folder == "boss")
	{
		this->load_boss_drop_reward();
	}
    if (folder == "monster")
    {
        this->load_monster_drop_reward();
    }
	if (folder == "all")
	{
		this->load_item_config();
		this->load_boss_drop_reward();
        this->load_monster_drop_reward();
	}
	return 0;
}

const xJson::xDocument& ShGameConfig::boss_drop_reward()
{
	return this->boss_drop_reward_.json();
}

const xJson::xValue &ShGameConfig::boss_drop_reward(int id)
{
    const ValueMap& tmp_map = this->boss_drop_reward_.map();
    ValueMap::const_iterator it =tmp_map.find(id);
    if(it == tmp_map.end())
    	return xJson::value_null;
    if(it->second == NULL)
    	return xJson::value_null;
    return *it->second;
}

const xJson::xValue &ShGameConfig::monster_drop_group_reward()
{
	return this->monster_drop_reward_.json();
}


const xJson::xValue &ShGameConfig::monster_drop_group_reward(long long id)
{
    const ValueMap& tmp_map = this->monster_drop_reward_.map();
    ValueMap::const_iterator it =tmp_map.find(id);
    if(it == tmp_map.end())
    	return xJson::value_null;
    if(it->second == NULL)
    	return xJson::value_null;
    return *it->second;
}

}
