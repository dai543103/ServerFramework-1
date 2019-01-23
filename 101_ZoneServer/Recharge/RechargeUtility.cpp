
#include "GameProtocol.hpp"
#include "Kernel/GameRole.hpp"
#include "Kernel/ZoneMsgHelper.hpp"

#include "RechargeUtility.h"

static GameProtocolMsg stMsg;

//增加月卡时间, iAddTime是天数
void CRechargeUtility::AddMonthEndTime(CGameRoleObj& stRoleObj, int iAddTime)
{
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_UPDATEMONTHTIME_NOTIFY);
	Zone_UpdateMonthTime_Notify* pstNotify = stMsg.mutable_stbody()->mutable_m_stzone_updatemonthtime_notify();

	//增加月卡时间
	int iTimeNow = CTimeUtility::GetNowTime();
	int iMonthEndTime = 0;
	if (iTimeNow <= stRoleObj.GetMonthEndTime())
	{
		//直接加上去
		iMonthEndTime = stRoleObj.GetMonthEndTime() + iAddTime * 24 * 60 * 60;
	}
	else
	{
		//重新计算时间
		iMonthEndTime = CTimeUtility::GetTodayTime(iTimeNow) + iAddTime * 24 * 60 * 60;
	}

	stRoleObj.SetMonthEndTime(iMonthEndTime);
	stRoleObj.SetLastMonthTime(iTimeNow-24*60*60);	//设置上次礼包领取时间为昨天

	pstNotify->set_imonthendtime(stRoleObj.GetMonthEndTime());

	CZoneMsgHelper::SendZoneMsgToRole(stMsg, &stRoleObj);

	return;
}
