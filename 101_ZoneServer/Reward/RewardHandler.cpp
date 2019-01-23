
#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "ErrorNumDef.hpp"
#include "Kernel/ModuleHelper.hpp"
#include "Kernel/ZoneMsgHelper.hpp"
#include "Kernel/UnitUtility.hpp"
#include "Kernel/ZoneOssLog.hpp"
#include "RepThings/RepThingsUtility.hpp"
#include "Resource/ResourceUtility.h"
#include "Reward/RewardUtility.h"

#include "RewardHandler.h"

using namespace ServerLib;

static GameProtocolMsg stMsg;

CRewardHandler::~CRewardHandler()
{

}

int CRewardHandler::OnClientMsg()
{
	switch (m_pRequestMsg->sthead().uimsgid())
	{
	case MSGID_ZONE_GETLOGINREWARD_REQUEST:
	{
		//玩家领取登录奖励
		OnRequestGetLoginReward();
	}
	break;

	default:
		break;
	}

	return 0;
}

//玩家领取登录奖励
int CRewardHandler::OnRequestGetLoginReward()
{
	int iRet = SecurityCheck();
	if (iRet < 0)
	{
		LOGERROR("Security Check Failed: Uin = %u, iRet = %d\n", m_pRequestMsg->sthead().uin(), iRet);
		return -1;
	}

	//返回消息
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_GETLOGINREWARD_RESPONSE);
	Zone_GetLoginReward_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_getloginreward_response();

	//获取请求
	const Zone_GetLoginReward_Request& stReq = m_pRequestMsg->stbody().m_stzone_getloginreward_request();
	pstResp->set_irewardid(stReq.irewardid());

	//是否满足登录天数
	if (m_pRoleObj->GetLoginDays() < stReq.irewardid())
	{
		//该天数不能领取
		LOGERROR("Failed to get login reward, login days need:real %d:%d, uin %u\n", stReq.irewardid(), m_pRoleObj->GetLoginDays(), m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -6;
	}

	//读取配置
	const LoginRewardConfig* pstConfig = CConfigManager::Instance()->GetBaseCfgMgr().GetLoginRewardConfig(stReq.irewardid());
	if (!pstConfig)
	{
		LOGERROR("Failed to get login reward, uin %u, reward id %d\n", m_pRoleObj->GetUin(), stReq.irewardid());
		
		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -2;
	}

	//是否已领取
	if (m_pRoleObj->HasGetLoginReward(stReq.irewardid()))
	{
		//已经领取过
		LOGERROR("Failed to get login reward, already get it, uin %u, id %d\n", m_pRoleObj->GetUin(), stReq.irewardid());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -3;
	}

	//领取奖励
	iRet = CRewardUtility::GetReward(*m_pRoleObj, 1, &pstConfig->stReward, 1, ITEM_CHANNEL_LOGINADD);
	if (iRet)
	{
		LOGERROR("Failed to get login reward, uin %u, id %d, ret %d\n", m_pRoleObj->GetUin(), stReq.irewardid(), iRet);

		pstResp->set_iresult(iRet);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -4;
	}

	//设置为已领取
	m_pRoleObj->SetLoginRewardStat(stReq.irewardid());

	//获取玩家获得奖励对应数量
	long8 lNewRewardNum = 0;
	switch (pstConfig->stReward.iType)
	{
	case REWARD_TYPE_RES:
	{
		lNewRewardNum = m_pRoleObj->GetResource(pstConfig->stReward.iRewardID);
	}
	break;

	case REWARD_TYPE_ITEM:
	{
		lNewRewardNum = m_pRoleObj->GetRepThingsManager().GetRepItemNum(pstConfig->stReward.iRewardID);
	}
	default:
		break;
	}

	//打印运营日志
	CZoneOssLog::TraceLoginReward(m_pRoleObj->GetUin(), m_pRoleObj->GetChannel(), m_pRoleObj->GetNickName(), pstConfig->stReward.iRewardID, stReq.irewardid(),
		lNewRewardNum - pstConfig->stReward.iRewardNum, lNewRewardNum);

	//发送成功的回复
	pstResp->set_iresult(T_SERVER_SUCCESS);
	pstResp->set_irewardid(stReq.irewardid());
	CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);

	return 0;
}
