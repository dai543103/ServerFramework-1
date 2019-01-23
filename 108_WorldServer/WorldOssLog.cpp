#include <time.h>

#include "GameProtocol.hpp"
#include "WorldOssLog.hpp"

#include "StringUtility.hpp"
#include "WorldRoleStatus.hpp"
#include "TimeValue.hpp"
#include "ModuleHelper.hpp"


//玩家实时在线日志
void CWorldOssLog::TraceOnline(int iZoneID, int iOnlineNum, int iWorldOnlineNum)
{
	//| OSS_LOG_WORLD_ONLINE | time | uin | worldonlinenum | zoneid | zoneonlinenum 
	TRACEBILL("|%d|%d|%u|%d|%d|%d\n", OSS_LOG_WORLD_ONLINE, CTimeUtility::GetNowTime(), 0, iWorldOnlineNum, iZoneID, iOnlineNum);

	return;
}
