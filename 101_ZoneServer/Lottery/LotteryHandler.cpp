
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
#include "LasvegasManager.h"

#include "LotteryHandler.h"

using namespace ServerLib;

static GameProtocolMsg stMsg;

CLotteryHandler::~CLotteryHandler()
{

}

int CLotteryHandler::OnClientMsg()
{
	switch (m_pRequestMsg->sthead().uimsgid())
	{
	case MSGID_ZONE_LOTTERY_REQUEST:
	{
		//玩家海盗宝藏抽奖
		OnRequestLottery();
	}
	break;

	case MSGID_ZONE_LIMITLOTTERY_REQUEST:
	{
		//玩家限量抽奖
		OnRequestLimitLottery();
	}
	break;

	case MSGID_ZONE_LIMITLOTTERY_RESPONSE:
	{
		//玩家限量抽奖返回
		OnResponseLimitLottery();
	}
	break;

	case MSGID_ZONE_PAYLOTTERYRECORD_REQUEST:
	{
		//玩家拉取充值抽奖请求
		OnRequestPayLotteryRecord();
	}
	break;

	case MSGID_ZONE_PAYLOTTERYRECORD_RESPONSE:
	{
		//玩家拉取充值抽奖返回
		OnResponsePayLotteryRecord();
	}
	break;

	case MSGID_ZONE_ENTERLASVEGAS_REQUEST:
	{
		//玩家进出大转盘抽奖请求
		OnRequestEnterLasvegas();
	}
	break;

	case MSGID_ZONE_LASVEGASBET_REQUEST:
	{
		//玩家大转盘下注的请求
		OnRequestBetLasvegas();
	}
	break;

	case MSGID_ZONE_GETREWARDINFO_REQUEST:
	{
		//玩家拉取中奖纪录的请求
		OnRequestGetRewardInfo();
	}
	break;

	case MSGID_WORLD_UPDATELASVEGAS_NOTIFY:
	{
		//World更新大转盘信息
		OnNotifyUpdateLasvegas();
	}
	break;

	default:
		break;
	}

	return 0;
}

//玩家抽奖操作
int CLotteryHandler::OnRequestLottery()
{
	int iRet = SecurityCheck();
	if (iRet < 0)
	{
		LOGERROR("Security Check Failed: Uin = %u, iRet = %d\n", m_pRequestMsg->sthead().uin(), iRet);
		return -1;
	}

	//返回消息
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_LOTTERY_RESPONSE);
	Zone_Lottery_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_lottery_response();

	//获取配置
	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();

	//抽奖消耗
	const LotteryCostConfig* pstCostConfig = stBaseCfgMgr.GetLotteryCostConfig(m_pRoleObj->GetLotteryNum()+1);
	if (!pstCostConfig || pstCostConfig->iCost > m_pRoleObj->GetResource(RESOURCE_TYPE_COIN) || 
		pstCostConfig->iVIPLevel>m_pRoleObj->GetVIPLevel())
	{
		LOGERROR("Failed to get lottery cost config, uin %u, vip %d, lotterynum %d\n", m_pRoleObj->GetUin(), 
			m_pRoleObj->GetVIPLevel(), m_pRoleObj->GetLotteryNum());

		pstResp->set_iresult(T_ZONE_INVALID_CFG);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -3;
	}

	//是否达到最大次数
	const VipLevelConfig* pstVipConfig = stBaseCfgMgr.GetVipConfig(m_pRoleObj->GetVIPLevel());
	if (!pstVipConfig)
	{
		LOGERROR("Failed to get vip config, vip level %d, uin %u\n", m_pRoleObj->GetVIPLevel(), m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_INVALID_CFG);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -4;
	}

	for (unsigned i = 0; i < pstVipConfig->vPrivs.size(); ++i)
	{
		if (pstVipConfig->vPrivs[i].iPrivType != VIP_PRIV_LOTTERY)
		{
			continue;
		}

		if (m_pRoleObj->GetLotteryNum() >= pstVipConfig->vPrivs[i].aiParams[2])
		{
			//已达到最大次数
			LOGERROR("Failed to lottery, reach num max %d, uin %u\n", m_pRoleObj->GetLotteryNum(), m_pRoleObj->GetUin());

			pstResp->set_iresult(T_ZONE_PARA_ERROR);
			CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
			return -5;
		}

		break;
	}

	//扣钱和次数
	CResourceUtility::AddUserRes(*m_pRoleObj, RESOURCE_TYPE_COIN, -pstCostConfig->iCost);
	m_pRoleObj->SetLotteryNum(m_pRoleObj->GetLotteryNum()+1);

	//抽奖
	const LotteryItemConfig* pstItemConfig = stBaseCfgMgr.GetLotteryItemConfig();
	if (!pstItemConfig)
	{
		LOGERROR("Failed to get lottery game config, uin %u\n", m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_INVALID_CFG);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -6;
	}

	//玩家抽奖前对应的道具或资源的数量，运营日志使用
	long8 lOldNum = 0;
	switch (pstItemConfig->stReward.iType)
	{
	case REWARD_TYPE_RES:
	{
		//资源
		lOldNum = m_pRoleObj->GetResource(pstItemConfig->stReward.iRewardID);
	}
	break;

	case REWARD_TYPE_ITEM:
	{
		//道具
		lOldNum = m_pRoleObj->GetRepThingsManager().GetRepItemNum(pstItemConfig->stReward.iRewardID);
	}
	break;

	case REWARD_TYPE_ENTITY:
	{

	}
	break;

	default:
		break;
	}

	//增加奖励
	iRet = CRewardUtility::GetReward(*m_pRoleObj, 1, &pstItemConfig->stReward, 1, ITEM_CHANNEL_LOTTERY);
	if (iRet)
	{
		LOGERROR("Failed to add lottery reward, uin %u, ret %d\n", m_pRoleObj->GetUin(), iRet);

		pstResp->set_iresult(iRet);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -7;
	}

	//打印运营日志
	CZoneOssLog::TraceVipLottery(m_pRoleObj->GetUin(), m_pRoleObj->GetChannel(), m_pRoleObj->GetNickName(), pstCostConfig->iCost, pstItemConfig->stReward.iType,
		pstItemConfig->stReward.iRewardID, lOldNum, lOldNum + pstItemConfig->stReward.iRewardNum);

	//发送返回
	pstResp->set_iresult(0);
	pstResp->set_ilotteryitemid(pstItemConfig->iLotteryItemID);
	CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);

	//抽奖次数
	CGameEventManager::NotifyLottery(*m_pRoleObj, 1);

	return T_SERVER_SUCCESS;
}

//玩家限量抽奖请求
int CLotteryHandler::OnRequestLimitLottery()
{
	int iRet = SecurityCheck();
	if (iRet < 0)
	{
		LOGERROR("Security Check Failed: Uin = %u, iRet = %d\n", m_pRequestMsg->sthead().uin(), iRet);
		return -1;
	}

	//获取请求
	Zone_LimitLottery_Request* pstReq = m_pRequestMsg->mutable_stbody()->mutable_m_stzone_limitlottery_request();

	//返回消息
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_LIMITLOTTERY_RESPONSE);
	Zone_LimitLottery_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_limitlottery_response();
	pstResp->set_ilotterytype(pstReq->ilotterytype());

	//扣除限量抽奖消耗
	int iCostType = RESOURCE_TYPE_INVALID;
	int iCostNum = 0;
	switch (pstReq->ilotterytype())
	{
	case LOTTERY_TYPE_RECHARGE:
	{
		//充值抽奖
		iCostType = RESOURCE_TYPE_PAYSCORE;
		iCostNum = pstReq->bistentimes() ? RECHARGE_LOTTERY_COSTNUM * 10 : RECHARGE_LOTTERY_COSTNUM;
	}
	break;

	case LOTTERY_TYPE_TICKET:
	{
		//鱼票抽奖
		iCostType = RESOURCE_TYPE_TICKET;
		iCostNum = CConfigManager::Instance()->GetBaseCfgMgr().GetGlobalConfig(GLOBAL_TYPE_TICKETLOTTERYCOST);
	}
	break;

	default:
		break;
	}

	if (!CResourceUtility::AddUserRes(*m_pRoleObj, iCostType, -iCostNum))
	{
		LOGERROR("Failed to do limit lottery, cost type:num %d:%d, uin %u\n", iCostType, iCostNum, m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -2;
	}

	//直接转发给World处理
	pstReq->set_uin(m_pRoleObj->GetUin());
	pstReq->set_izoneid(CModuleHelper::GetZoneID());
	pstReq->set_strnickname(m_pRoleObj->GetNickName());
	iRet = CZoneMsgHelper::SendZoneMsgToWorld(*m_pRequestMsg);
	if (iRet)
	{
		LOGERROR("Failed to send recharge lottery to world, ret %d, uin %u\n", iRet, m_pRoleObj->GetUin());

		pstResp->set_iresult(iRet);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -3;
	}

	return 0;
}

//玩家限量抽奖返回
int CLotteryHandler::OnResponseLimitLottery()
{
	//获取请求
	const Zone_LimitLottery_Response& stResp = m_pRequestMsg->stbody().m_stzone_limitlottery_response();

	//获取玩家
	m_pRoleObj = CUnitUtility::GetRoleByUin(stResp.uin());
	if (!m_pRoleObj)
	{
		LOGERROR("Failed to do recharge lottery, uin %u\n", stResp.uin());
		return -1;
	}
	
	if (stResp.iresult() != 0)
	{
		//失败直接返回
		CZoneMsgHelper::SendZoneMsgToRole(*m_pRequestMsg, m_pRoleObj);
		return -2;
	}

	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();

	//抽奖成功，增加奖励
	for (int i = 0; i < stResp.ilotteryids_size(); ++i)
	{
		if (stResp.ilotteryids(i) == 0)
		{
			continue;
		}

		const LimitLotteryConfig* pstConfig = stBaseCfgMgr.GetLimitLotteryConfig(stResp.ilotterytype(), stResp.ilotteryids(i));
		if (!pstConfig)
		{
			LOGERROR("Failed to get recharge lottery config, invalid id %d\n", stResp.ilotteryids(i));
			continue;
		}

		int iRet = CRewardUtility::GetReward(*m_pRoleObj, 1, &pstConfig->stReward, 1, ITEM_CHANNEL_LOTTERY);
		if (iRet)
		{
			LOGERROR("Failed to add recharge lottery reward, uin %u, lottery id %d\n", m_pRoleObj->GetUin(), stResp.ilotteryids(i));
			continue;
		}

		//玩家充值奖励获取之前 对应奖项的数量 运营日志使用
		long8 lNewRewardNum = 0;

		//获取奖项配置
		const RewardConfig& stRewardConfig = pstConfig->stReward;

		//获取奖励对应奖项的数量
		switch (stRewardConfig.iType)
		{
		case REWARD_TYPE_RES:
		{
			lNewRewardNum = m_pRoleObj->GetResource(stRewardConfig.iRewardID);
		}
		break;

		case REWARD_TYPE_ITEM:
		{
			lNewRewardNum = m_pRoleObj->GetRepThingsManager().GetRepItemNum(stRewardConfig.iRewardID);
		}
		break;

		default:
			break;
		}

		//打印运营日志 玩家充值抽奖 根据奖项分条记录
		CZoneOssLog::TraceLimitLottery(m_pRoleObj->GetUin(), m_pRoleObj->GetChannel(), m_pRoleObj->GetNickName(), stResp.ilotterytype(), stResp.ilotteryids(i), stRewardConfig.iType,
			stRewardConfig.iRewardID, lNewRewardNum - stRewardConfig.iRewardNum, lNewRewardNum);

	}

	//发送成功的回复
	CZoneMsgHelper::SendZoneMsgToRole(*m_pRequestMsg, m_pRoleObj);

	return 0;
}

//玩家拉取充值抽奖记录
int CLotteryHandler::OnRequestPayLotteryRecord()
{
	int iRet = SecurityCheck();
	if (iRet < 0)
	{
		LOGERROR("Security Check Failed: Uin = %u, iRet = %d\n", m_pRequestMsg->sthead().uin(), iRet);
		return -1;
	}

	//返回消息
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_PAYLOTTERYRECORD_RESPONSE);
	Zone_PayLotteryRecord_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_paylotteryrecord_response();

	//获取请求
	Zone_PayLotteryRecord_Request* pstReq = m_pRequestMsg->mutable_stbody()->mutable_m_stzone_paylotteryrecord_request();
	if (pstReq->ifrom() <= 0 || pstReq->ifrom() > MAX_RECHARGE_LOTTERY_RECORD || pstReq->inum()<=0)
	{
		LOGERROR("Failed to get recharge lottery record, from %d, num %d, uin %u\n", pstReq->ifrom(), pstReq->inum(), m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -2;
	}

	//设置来源
	pstReq->set_izoneid(CModuleHelper::GetZoneID());

	//转发给World处理
	iRet = CZoneMsgHelper::SendZoneMsgToWorld(*m_pRequestMsg);
	if (iRet)
	{
		LOGERROR("Failed to send get lottery record to world, ret %d, uin %u\n", iRet, m_pRoleObj->GetUin());

		pstResp->set_iresult(iRet);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -3;
	}

	return 0;
}

//玩家拉取充值抽奖返回
int CLotteryHandler::OnResponsePayLotteryRecord()
{
	//直接转发给客户端
	m_pRoleObj = CUnitUtility::GetRoleByUin(m_pRequestMsg->sthead().uin());
	CZoneMsgHelper::SendZoneMsgToRole(*m_pRequestMsg, m_pRoleObj);

	return 0;
}

//玩家进出大转盘抽奖请求
int CLotteryHandler::OnRequestEnterLasvegas()
{
	int iRet = SecurityCheck();
	if (iRet < 0)
	{
		LOGERROR("Security Check Failed: Uin = %u, iRet = %d\n", m_pRequestMsg->sthead().uin(), iRet);
		return -1;
	}

	//返回消息
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_ENTERLASVEGAS_RESPONSE);
	Zone_EnterLasvegas_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_enterlasvegas_response();

	//获取请求
	const Zone_EnterLasvegas_Request& stReq = m_pRequestMsg->stbody().m_stzone_enterlasvegas_request();

	pstResp->set_iresult(T_SERVER_SUCCESS);
	pstResp->set_bisenter(stReq.bisenter());
	CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);

	//处理进出逻辑
	CLasvegasManager::Instance()->EnterLasvegas(*m_pRoleObj, stReq.bisenter());

	return 0;
}

//玩家大转盘下注的请求
int CLotteryHandler::OnRequestBetLasvegas()
{
	int iRet = SecurityCheck();
	if (iRet < 0)
	{
		LOGERROR("Security Check Failed: Uin = %u, iRet = %d\n", m_pRequestMsg->sthead().uin(), iRet);
		return -1;
	}

	//返回消息
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_LASVEGASBET_RESPONSE);
	Zone_LasvegasBet_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_lasvegasbet_response();

	//获取请求
	const Zone_LasvegasBet_Request& stReq = m_pRequestMsg->stbody().m_stzone_lasvegasbet_request();

	//处理下注逻辑
	iRet = CLasvegasManager::Instance()->BetLasvegas(*m_pRoleObj, stReq.stbet().inumber(), stReq.stbet().lbetcoins());
	if (iRet)
	{
		LOGERROR("Failed to bet for lasvegas, ret %d, uin %u, num %d, coins %lld\n", iRet, m_pRoleObj->GetUin(), stReq.stbet().inumber(), (long8)stReq.stbet().lbetcoins());
		
		pstResp->set_iresult(iRet);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return iRet;
	}

	//下注成功
	pstResp->mutable_stbet()->CopyFrom(stReq.stbet());
	pstResp->set_iresult(T_SERVER_SUCCESS);
	CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);

	return 0;
}

//玩家拉取中奖纪录的请求
int CLotteryHandler::OnRequestGetRewardInfo()
{
	int iRet = SecurityCheck();
	if (iRet < 0)
	{
		LOGERROR("Security Check Failed: Uin = %u, iRet = %d\n", m_pRequestMsg->sthead().uin(), iRet);
		return -1;
	}

	//返回消息
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_GETREWARDINFO_RESPONSE);
	Zone_GetRewardInfo_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_getrewardinfo_response();

	//获取请求
	const Zone_GetRewardInfo_Request& stReq = m_pRequestMsg->stbody().m_stzone_getrewardinfo_request();

	//处理拉取中奖纪录逻辑
	iRet = CLasvegasManager::Instance()->GetRewardInfo(*m_pRoleObj, *pstResp, stReq.ifromindex(), stReq.inum());
	if (iRet)
	{
		LOGERROR("Failed to get reward info, ret %d, uin %u, from %d, num %d\n", iRet, m_pRoleObj->GetUin(), stReq.ifromindex(), stReq.inum());

		pstResp->set_iresult(iRet);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return iRet;
	}

	//拉取成功
	pstResp->set_iresult(T_SERVER_SUCCESS);
	CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);

	return 0;
}

//World更新大转盘信息
int CLotteryHandler::OnNotifyUpdateLasvegas()
{
	//获取请求
	const World_UpdateLasvegas_Notify& stNotify = m_pRequestMsg->stbody().m_stworld_updatelasvegas_notify();

	//更新大转盘信息
	CLasvegasManager::Instance()->UpdateLasvegasInfo(stNotify);

	return 0;
}
