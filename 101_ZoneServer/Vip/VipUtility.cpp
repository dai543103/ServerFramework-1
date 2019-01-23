
#include "GameProtocol.hpp"
#include "TimeUtility.hpp"
#include "ErrorNumDef.hpp"
#include "LogAdapter.hpp"
#include "Kernel/ZoneMsgHelper.hpp"

#include "VipUtility.h"

static GameProtocolMsg stMsg;

//更新救济金信息
void CVipUtility::UpdateAlmsInfo(CGameRoleObj& stRoleObj, int iAlmsNum, int iNextAlmsTime)
{
	//设置信息
	stRoleObj.SetAlmsNum(iAlmsNum);
	stRoleObj.SetNextAlmsTime(iNextAlmsTime);
	stRoleObj.SetLastAlmsUpdateTime(CTimeUtility::GetNowTime());

	//推送通知
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_UPDATEALMS_NOTIFY);
	Zone_UpdateAlms_Notify* pstNotify = stMsg.mutable_stbody()->mutable_m_stzone_updatealms_notify();
	pstNotify->set_igetnum(iAlmsNum);
	pstNotify->set_inextgettime(iNextAlmsTime);

	CZoneMsgHelper::SendZoneMsgToRole(stMsg, &stRoleObj);

	return;
}

//触发救济金更新
int CVipUtility::TriggerAlmsUpdate(CGameRoleObj& stRoleObj)
{
	int iTimeNow = CTimeUtility::GetNowTime();

	//是否隔天
	if (!CTimeUtility::IsInSameDay(iTimeNow, stRoleObj.GetLastAlmsUpdateTime()))
	{
		//隔天
		UpdateAlmsInfo(stRoleObj, 0, 0);
	}

	//是否可触发
	if (stRoleObj.GetResource(RESOURCE_TYPE_COIN) != 0 || stRoleObj.GetNextAlmsTime()!=0)
	{
		//不满足触发条件
		return T_SERVER_SUCCESS;
	}

	//获取配置
	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();
	const VipLevelConfig* pstVipConfig = stBaseCfgMgr.GetVipConfig(stRoleObj.GetVIPLevel());
	if (!pstVipConfig)
	{
		LOGERROR("Failed to get vip config, invalid level %d, uin %u\n", stRoleObj.GetVIPLevel(), stRoleObj.GetUin());
		return T_ZONE_INVALID_CFG;
	}

	const VipPriv* pstPrivConfig = NULL;
	for (unsigned i = 0; i < pstVipConfig->vPrivs.size(); ++i)
	{
		if (pstVipConfig->vPrivs[i].iPrivType == VIP_PRIV_GETALMS)
		{
			pstPrivConfig = &pstVipConfig->vPrivs[i];
			break;
		}
	}

	if (!pstPrivConfig)
	{
		//该VIP等级没有救济金
		return T_SERVER_SUCCESS;
	}

	if (stRoleObj.GetAlmsNum() >= pstPrivConfig->aiParams[1])
	{
		//已经没有次数
		return T_SERVER_SUCCESS;
	}

	if (stRoleObj.GetAlmsNum() == 0)
	{
		//首次触发
		UpdateAlmsInfo(stRoleObj, stRoleObj.GetAlmsNum(), iTimeNow);
	}
	else
	{
		//非首次触发,开始倒计时
		UpdateAlmsInfo(stRoleObj, stRoleObj.GetAlmsNum(), iTimeNow+ pstPrivConfig->aiParams[0]);
	}

	return T_SERVER_SUCCESS;
}
