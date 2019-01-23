
#include <stdint.h>

#include "GameProtocol.hpp"
#include "NowTime.hpp"
#include "Kernel/ZoneObjectHelper.hpp"
#include "Kernel/ConfigManager.hpp"
#include "Kernel/GameRole.hpp"
#include "FishpondObj.h"

#include "FishUtility.h"

using namespace ServerLib;

//捕鱼辅助工具类

//桌子唯一ID
unsigned CFishUtility::uTableUniqID = 0;

//出鱼唯一ID
unsigned CFishUtility::uFishUniqID = 0;

//子弹唯一ID
unsigned CFishUtility::uBulletUniqID = 0;

//红包唯一ID
unsigned CFishUtility::uReturnID = 0;

//周期唯一ID
unsigned CFishUtility::uCycleUniqID = 0;

//获取桌子ID
unsigned CFishUtility::GetTableUniqID()
{
	return ++uTableUniqID;
}

//获取鱼唯一ID
unsigned CFishUtility::GetFishUniqID()
{
	return ++uFishUniqID;
}

//获取子弹唯一ID
unsigned CFishUtility::GetBulletUniqID()
{
	return ++uBulletUniqID;
}

//获取红包唯一ID
unsigned CFishUtility::GetReturnUniqID()
{
	return ++uReturnID;
}

//获取周期唯一ID
unsigned CFishUtility::GetCycleUniqID()
{
	return ++uCycleUniqID;
}

//获取鱼池对象
CFishpondObj* CFishUtility::GetFishpondByID(unsigned uTableID)
{
	return GameTypeK32<CFishpondObj>::GetByKey(uTableID);
}

//判断时间是否同一天
bool CFishUtility::IsSameDay(long lTime1, long lTime2)
{
	if (abs(lTime1 - lTime2) >= 24 * 60 * 60 * 1000)
	{
		//间隔超过24小时
		return false;
	}

	struct tm *tm1;
	struct tm *tm2;
	time_t time1 = lTime1 / 1000;
	time_t time2 = lTime2 / 1000;
	tm1 = localtime(&time1);
	tm2 = localtime(&time2);

	return (tm1->tm_mday == tm2->tm_mday);
}
