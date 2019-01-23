#include "ModuleHelper.hpp"
#include "WorldRoleStatus.hpp"
#include "WorldObjectHelperW_K32.hpp"
#include "WorldMsgHelper.hpp"
#include "LogoutHandler.hpp"
#include "WorldOssLog.hpp"

#include "ZoneTick.hpp"

int CZoneTick::Initialize(bool bResumeMode)
{
    time_t tNow = time(NULL);

    for (int iZoneID = 1; iZoneID < MAX_ZONE_PER_WORLD; iZoneID++)
	{
		TZoneStatus& rstZoneStatus = m_astZoneStatus[iZoneID];
		rstZoneStatus.m_iZoneID = iZoneID;
		rstZoneStatus.m_iLastActiveTime = tNow;
		rstZoneStatus.m_iOnlineNumber = 0;
		rstZoneStatus.m_ucActive = 0;
		rstZoneStatus.m_ucState = SERVER_STATE_IDLE;
	}

    m_iLastTickTime = tNow;
	m_iActiveZoneNum = 0;

    CModuleHelper::RegisterZoneTick(this);

    return 0;
}

int CZoneTick::OnTick()
{
    time_t tNow = time(NULL);
    if (tNow < (m_iLastTickTime + ZONE_TICK_INTERVAL))
    {
        return 0;
    }
	
	m_iActiveZoneNum = 0;
	m_iWorldOnlineNum = 0;

    for (int iZoneID = 1; iZoneID < MAX_ZONE_PER_WORLD; iZoneID++)
    {
        TZoneStatus& rstZoneStatus = m_astZoneStatus[iZoneID];
        if (!rstZoneStatus.m_ucActive)
        {
            continue;
        }
		
        if (tNow > (rstZoneStatus.m_iLastActiveTime + UNACTIVE_ZONE_TIMEOUT))
        {
			// Zone 超时死亡
            rstZoneStatus.m_ucActive = 0;
			rstZoneStatus.m_iOnlineNumber = 0;

			//TNM2_REPORT_WARNING_EX(SS_MSG_SEND_FAILED, "Zone Dead: %d", iZoneID);
			TRACESVR("Zone Dead: %d\n", iZoneID);
        }
		else
		{
			m_iActiveZoneNum++;
		}

		m_iWorldOnlineNum += rstZoneStatus.m_iOnlineNumber;

		//打印在线日志
		CWorldOssLog::TraceOnline(iZoneID, rstZoneStatus.m_iOnlineNumber, m_iWorldOnlineNum);
    }

    m_iLastTickTime = tNow;

    return 0;
}

void CZoneTick::ActiveRefresh(int iZoneID, int iOnlineNumber)
{   
    if (iZoneID < 1 || iZoneID >= MAX_ZONE_PER_WORLD)
    {
        return;
    }

	if (iOnlineNumber == -1)
	{
		// Zone 恢复，剔除所有本线的残留缓存
		OnZoneDead(iZoneID);

		iOnlineNumber = 0;
	}

    TZoneStatus& rstZoneStatus = m_astZoneStatus[iZoneID];
    rstZoneStatus.m_iLastActiveTime = time(NULL);
	if (rstZoneStatus.m_ucActive != 1)
	{
		m_iActiveZoneNum++;
    	rstZoneStatus.m_ucActive = 1;

		TRACESVR("Zone Alive: %d, Online %d\n", iZoneID, iOnlineNumber);
	}

    if (iOnlineNumber >= 0)
    {
        rstZoneStatus.m_iOnlineNumber = iOnlineNumber;
    }
	
	rstZoneStatus.m_ucState = SERVER_STATE_IDLE;

    return;
}

const TZoneStatus& CZoneTick::GetZoneStatus(int iZoneID)
{
    return m_astZoneStatus[iZoneID];
}

bool CZoneTick::IsZoneActive(int iZoneID)
{
    if (iZoneID < 1 || iZoneID >= MAX_ZONE_PER_WORLD)
    {
        return false;
    } 

    TZoneStatus& rstZoneStatus = m_astZoneStatus[iZoneID];
    return rstZoneStatus.m_ucActive; 
}

void CZoneTick::OnZoneDead(int iZoneID)
{ 
    TRACESVR("Kick Dead Zone roles: %d\n", iZoneID);

    // 将所有该Zone上的玩家都踢下线
    CWorldRoleStatusWObj* pRoleObj = WorldTypeK32<CWorldRoleStatusWObj>::GetFirstUin();
    CWorldRoleStatusWObj* pNextRoleObj = pRoleObj;
    while(1)
    {
        pRoleObj = pNextRoleObj;
        if (!pRoleObj)
        {
            break;
        }

        pNextRoleObj = WorldTypeK32<CWorldRoleStatusWObj>::GetNextUin(pRoleObj->GetObjectID());

        if (pRoleObj->GetZoneID() != iZoneID)
        {
            continue;
        }

        TRACESVR("Kick Unactive Role: %u\n", pRoleObj->GetObjectID());
        CLogoutHandler::LogoutNotify(pRoleObj);
    }
}

int CZoneTick::GetLowestZoneID()
{
	int iZoneID = 0;
	int iZoneNum = -1;
	for (int i = 0; i < MAX_ZONE_PER_WORLD; i++)
	{
		if (m_astZoneStatus[i].m_ucActive)
		{
			if (iZoneNum < 0 || m_astZoneStatus[i].m_iOnlineNumber <= iZoneNum)
			{
				iZoneID = i;
				iZoneNum = m_astZoneStatus[i].m_iOnlineNumber;
			}
		}
	}

	return iZoneID;
}
