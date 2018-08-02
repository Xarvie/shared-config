#ifndef SHGAMECONFIG_H_
#define SHGAMECONFIG_H_
#include "xConfig.h"


namespace xJson
{

//typedef std::map< int, const xValue *, std::less<int>, shared_stl_allocator < std::pair < const int, const xValue * > > > ValueMap;
typedef std::map< int, const xValue *, std::less<int>, shared_stl_allocator < std::pair < const int, const xValue * > > > ValueMap;
typedef std::vector< ValueMap, shared_stl_allocator<ValueMap> > VMapVec;
class ShGameConfig
{
public:
	enum
	{
		MAX_VERSION_CNT = 2
	};
	struct xBasicConfig
	{
	public:
		xBasicConfig(void);
		xDocument &json(void);
		xDocument &revert_json(void);

		ValueMap &map(void);
		ValueMap &revert_map(void);

		int current_version(void);
		int prev_version(void);
		void update_version(void);
		void revert_version(void);
		void convert_json_to_map(int debug_flag = false);
		int __cur_version;
	private:
		xDocument __json[MAX_VERSION_CNT];
		shString __version_no[MAX_VERSION_CNT];
		VMapVec __map;
		//ValueMap __map[MAX_VERSION_CNT];
	};
public:
    struct ItemConfig
    {
    	xBasicConfig item_;
    };
public:
    ShGameConfig();
	virtual ~ShGameConfig();
	int init(const std::string& server_name);
	int load_json_config(const char *doc, xDocument &conf);
	const xDocument &activity();
public:
	int update_config(const char * str);
	int load_item_config();
	const xValue& item(int item_id);
	int load_boss_drop_reward();
	int load_monster_drop_reward();
	const xDocument& boss_drop_reward();
	const xValue& boss_drop_reward(int id);
	const xValue& monster_drop_group_reward();
	const xValue& monster_drop_group_reward(long long id);
public:
	xBasicConfig activity_config_;
	xBasicConfig boss_drop_reward_;
	xBasicConfig monster_drop_reward_;
	ItemConfig item_config_;
};
extern xValue value_null;
}
#endif 
