/*
 * 
tutorial_xjson.cxx
 *
 *  Created on: Jul 13, 2018
 *      Author: Xarvie
 */

#ifdef XCONFIG
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
#include <sys/time.h>
#include "xConfig/xConfig.h"
using namespace std;
using namespace xJson;
/*
{
    "200000016":
    {
    	"name": "还魂丹",
        "type": 1001.0
    },
    "100113413":
    {
        "name": "永生·斩魔战刃",
        "type": 1,
        "part": 11,
        "effects": {
            "path": "equip/ck_wp_70",
            "open": [7, 8, 9]
        },
        "lstore_score": 426
    }
}
*/

int turorial()
{
	return 0;

	xDocument& s = xCONFIG_INSTANCE->item_config_.item_.json();

	cout << "output:" << s["200000016"]["name"].asCString() << endl;
/*
output:还魂丹
*/

    for (xJson::xValue::const_iterator itr = s.begin(); itr !=s.end(); ++itr)
        printf("output: key:%s value: type: %d\n",  itr->name.asCString(), itr->value.GetType());
/*
Type:
    kNullType = 0,
    kFalseType = 1,
    kTrueType = 2,
    kObjectType = 3,
    kArrayType = 4,
    kStringType = 5,
    kNumberType = 6

output: key:"200000016" value:type: 3
output: key:"100113413" value:type: 3
(返回type 3(类型), 3 是json对象型 = kObjectType)
*/

    cout << "output: double强制转int "  << xCONFIG_INSTANCE->item(200000016)["type"].asInt() << endl;
/*
output: double强制转int 1001
*/
    return 0;


}


/*
以下称 共享内存创建者 Daemon             进程为 Sharer
以下称 内存访问者    Logic | Map | .... 进程为 Accesser

ChangeLog:
ALPHA 0.8.2947:
Add:新增xConfig目录和这份使用指南
Mod:Sharer增加xconfig初始化
Mod:在GameConifg.h内增加include xConfig

ALPHA 0.8.2949:
Del:去除实验性代码

ALPHA 0.8.2958:
Fixed:修复Accesser意外使用了xslab::malloc函数,导致内存段整体偏移的bug,
      去除Accesser的xslab, xslab为了提高内存分配速度，使用了空锁，所以写操作是非线程安全的(预留空锁，替换空锁即可支持线程安全)。
      本身Accesser禁止对共享内存进行写操作,后续操作xslab::malloc | xslab::free 都会导致Accesser崩溃.

ALPHA 1.0.2966:
Add:xConfig热更新支持,额外需要修改config/update_config.sh以解除对Daemon kill 35信号的禁止

*/
#endif
