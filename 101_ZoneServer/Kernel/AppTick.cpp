#include "AppTick.hpp"
#include "LogAdapter.hpp"
#include "Kernel/ZoneOssLog.hpp"
#include "TimeUtility.hpp"
#include "ObjStatistic.hpp"
#include "PerformanceStatistic.hpp"
#include "Kernel/ModuleHelper.hpp"
#include "Kernel/ZoneObjectHelper.hpp"
#include "Kernel/AppLoop.hpp"

CAppTick::CAppTick()
{
}

int CAppTick::Initialize(bool bResume)
{
    m_OnlineStatTick.Initialize(bResume);

	if (bResume)
	{
		return 0;
	}

    m_iLastStaticsTick = time(NULL);

	return 0;
}

static const char * szObjectTypeName[] = {"CGameRoleObj"}; 

const int iObjectTypeNum = sizeof(szObjectTypeName)/sizeof(szObjectTypeName[0]);
int CAppTick::CountObjStat()
{
	int i = 0;

	ObjectStatisticSingleton::Instance()->SetObjectStatName(szObjectTypeName, iObjectTypeNum);

	ObjectStatisticSingleton::Instance()->AddObjectStat(i++, GameTypeK32<CGameRoleObj>::GetUsedObjNumber());

	//Çå0
	CTimeValue stZero(0, 0);
	CAppLoop::m_stLotusMsgMaxProsessTime = stZero;
	CAppLoop::m_stWorldMsgMaxProcessTime = stZero;
	CAppLoop::m_stTickMaxProcessTime = stZero;

	return 0;
}

int CAppTick::OnTick()
{
	CGameEventManager::NotifyTick();

	m_OnlineStatTick.OnTick();

	// Ò»·ÖÖÓTick
	int iSlapTime = CTimeUtility::m_uiTimeTick - m_iLastStaticsTick;
	if (iSlapTime >= 60)
    {
		CountObjStat();		
		ObjectStatisticSingleton::Instance()->Print();
        ObjectStatisticSingleton::Instance()->Reset();				
		
        MsgStatisticSingleton::Instance()->Print();
        MsgStatisticSingleton::Instance()->Reset();

		PerformanceStatisticSingleton::Instance()->Print();
		PerformanceStatisticSingleton::Instance()->Reset();

        m_iLastStaticsTick = CTimeUtility::m_uiTimeTick;
    }
    
	return 0;
}
