
#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "ErrorNumDef.hpp"
#include "TimeUtility.hpp"
#include "Kernel/ZoneMsgHelper.hpp"
#include "Kernel/ModuleHelper.hpp"
#include "Kernel/UnitUtility.hpp"
#include "Kernel/ZoneOssLog.hpp"
#include "Resource/ResourceUtility.h"

#include "LasvegasManager.h"

using namespace ServerLib;

static GameProtocolMsg stMsg;

CLasvegasManager* CLasvegasManager::Instance()
{
	static CLasvegasManager* pInstance = NULL;
	if (!pInstance)
	{
		pInstance = new CLasvegasManager;
	}

	return pInstance;
}

CLasvegasManager::~CLasvegasManager()
{

}

CLasvegasManager::CLasvegasManager()
{
	Reset();
}

//初始化
void CLasvegasManager::Init()
{
	Reset();
}

//玩家进出转盘
void CLasvegasManager::EnterLasvegas(CGameRoleObj& stRoleObj, bool bIsEnter)
{
	if (bIsEnter)
	{
		//进入
		UserBetData stUserData;
		stUserData.uin = stRoleObj.GetUin();
		if (std::find(m_vUserList.begin(), m_vUserList.end(), stUserData) != m_vUserList.end())
		{
			//已经进入
			return;
		}

		//玩家进入转盘
		m_vUserList.push_back(stUserData);

		//推送转盘信息
		SendLasvegasInfoNotify(&stRoleObj);
	}
	else
	{
		//离开
		UserBetData stUserData;
		stUserData.uin = stRoleObj.GetUin();
		std::vector<UserBetData>::iterator it = std::find(m_vUserList.begin(), m_vUserList.end(), stUserData);
		if (it != m_vUserList.end())
		{
			//退还玩家下注
			int iRetCoins = 0;
			for (unsigned i = 0; i < it->vBets.size(); ++i)
			{
				iRetCoins += it->vBets[i].iBetCoins;
			}

			CResourceUtility::AddUserRes(stRoleObj, RESOURCE_TYPE_COIN, iRetCoins);

			m_vUserList.erase(it);
		}
	}

	return;
}

//玩家下注
int CLasvegasManager::BetLasvegas(CGameRoleObj& stRoleObj, int iNumber, int iBetCoins)
{
	//是否处于下注阶段
	if (m_iStepType != LASVEGAS_STEP_BET)
	{
		//不能下注
		LOGERROR("Failed to bet lasvegas, invalid step %d, uin %u\n", m_iStepType, stRoleObj.GetUin());
		return T_ZONE_INVALID_LOTTERYSTEP;
	}

	//读取转盘配置
	const LasvegasConfig* pstConfig = CConfigManager::Instance()->GetBaseCfgMgr().GetLasvegasConfigByNumber(iNumber);
	if (!pstConfig || iBetCoins<=0)
	{
		LOGERROR("Failed to bet lasvegas, num %d, coins %d, uin %u\n", iNumber, iBetCoins, stRoleObj.GetUin());
		return T_ZONE_PARA_ERROR;
	}

	//获取玩家下注信息
	LasvegasBetData* pstBetData = NULL;
	for (unsigned i = 0; i < m_vUserList.size(); ++i)
	{
		if (m_vUserList[i].uin != stRoleObj.GetUin())
		{
			continue;
		}

		//查找玩家下注信息
		for (unsigned j = 0; j < m_vUserList[i].vBets.size(); ++j)
		{
			if (m_vUserList[i].vBets[j].iNumber == iNumber)
			{
				//找到下注信息
				pstBetData = &m_vUserList[i].vBets[j];
				break;
			}
		}

		if (!pstBetData)
		{
			//找不到下注信息，增加一个
			LasvegasBetData stOneData;
			stOneData.iNumber = iNumber;

			m_vUserList[i].vBets.push_back(stOneData);

			pstBetData = &(*m_vUserList[i].vBets.rbegin());
		}

		break;
	}

	if (!pstBetData)
	{
		//找不到信息
		LOGERROR("Failed to get user bet data, uin %u, number %d\n", stRoleObj.GetUin(), iNumber);
		return T_ZONE_PARA_ERROR;
	}

	//是否满足下注额要求
	if (iBetCoins<pstConfig->iMinBet || (pstBetData->iBetCoins + iBetCoins) >pstConfig->iMaxBet)
	{
		LOGERROR("Failed to bet number %d, current bet coins %d, bet coins %d, uin %u\n", iNumber, pstBetData->iBetCoins, iBetCoins, stRoleObj.GetUin());
		return T_ZONE_PARA_ERROR;
	}

	//扣除玩家金币
	if (!CResourceUtility::AddUserRes(stRoleObj, RESOURCE_TYPE_COIN, -iBetCoins))
	{
		//扣钱失败
		LOGERROR("Failed to bet lasvegas, uin %u, num %d, bet coins %d\n", stRoleObj.GetUin(), iNumber, iBetCoins);
		return T_ZONE_PARA_ERROR;
	}

	//玩家下注
	pstBetData->iBetCoins += iBetCoins;
	
	//打印运营日志 玩家下注
	CZoneOssLog::TraceLasvegasBet(stRoleObj.GetUin(), stRoleObj.GetChannel(), stRoleObj.GetNickName(), iNumber, iBetCoins);

	//更新World下注信息
	SendUpdateBetInfoToWorld(iNumber, iBetCoins);

	return T_SERVER_SUCCESS;
}

//更新转盘信息
void CLasvegasManager::UpdateLasvegasInfo(const World_UpdateLasvegas_Notify& stNotify)
{
	int iOldStepType = m_iStepType;

	m_iStepType = stNotify.stinfo().isteptype();
	m_iStepEndTime = stNotify.stinfo().istependtime();

	//开奖记录
	m_vLotteryIDs.clear();
	for (int i = 0; i < stNotify.stinfo().ilotteryids_size(); ++i)
	{
		m_vLotteryIDs.push_back(stNotify.stinfo().ilotteryids(i));
	}

	//中奖信息
	m_vPrizeDatas.clear();
	LotteryPrizeData stOnePrize;
	for (int i = 0; i < stNotify.stinfo().stprizes_size(); ++i)
	{
		stOnePrize.strNickName = stNotify.stinfo().stprizes(i).strname();
		stOnePrize.iNumber = stNotify.stinfo().stprizes(i).inumber();
		stOnePrize.iRewardCoins = stNotify.stinfo().stprizes(i).irewardcoins();

		m_vPrizeDatas.push_back(stOnePrize);
	}

	//下注信息
	m_vBetDatas.clear();
	LasvegasBetData stOneBet;
	for (int i = 0; i < stNotify.stinfo().stbets_size(); ++i)
	{
		stOneBet.iNumber = stNotify.stinfo().stbets(i).inumber();
		stOneBet.iBetCoins = stNotify.stinfo().stbets(i).lbetcoins();

		m_vBetDatas.push_back(stOneBet);
	}

	if (iOldStepType != m_iStepType)
	{
		//切换阶段,强制推送转盘信息
		SendLasvegasInfoNotify(NULL);

		if (m_iStepType == LASVEGAS_STEP_LOTTERY)
		{
			//进入开奖阶段

			//清除下注信息
			m_vBetDatas.clear();
			
			//设置开奖倒计时
			BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();
			m_iLotteryRewardTime = CTimeUtility::GetNowTime() + stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_LASVEGAS_REWARDTIME);

			m_bNeedSendReward = true;
			m_iLotteryID = *(m_vLotteryIDs.rbegin());
		}

		return;
	}

	//需要推送更新给玩家
	m_bSendUserNotify = true;

	return;
}

//拉取转盘中奖信息
int CLasvegasManager::GetRewardInfo(CGameRoleObj& stRoleObj, Zone_GetRewardInfo_Response& stResp, int iFrom, int iNum)
{
	//检查参数
	if (iFrom <= 0 || iNum <= 0)
	{
		LOGERROR("Failed to get lasvegas reward info, uin %u, from %d, num %d\n", stRoleObj.GetUin(), iFrom, iNum);
		return T_ZONE_PARA_ERROR;
	}

	for (int i = 0; i < iNum; ++i)
	{
		if ((iFrom + i) > (int)m_vPrizeDatas.size())
		{
			//已经没有数据了
			break;
		}

		//封装数据
		PrizeInfo* pstOneInfo = stResp.add_stinfos();
		pstOneInfo->set_strname(m_vPrizeDatas[iFrom+i-1].strNickName);
		pstOneInfo->set_inumber(m_vPrizeDatas[iFrom + i - 1].iNumber);
		pstOneInfo->set_irewardcoins(m_vPrizeDatas[iFrom + i - 1].iRewardCoins);
		pstOneInfo->set_iindex(iFrom + i);
	}

	return T_SERVER_SUCCESS;
}

//定时器
void CLasvegasManager::OnTick()
{
	int iTimeNow = CTimeUtility::GetNowTime();

	//推送转盘信息给玩家,2s推送1次
	if (m_bSendUserNotify && (m_iLastSendNotifyTime + 2) < iTimeNow)
	{
		//推送转盘信息
		SendLasvegasInfoNotify(NULL);
	}

	//是否发放奖励
	if (m_bNeedSendReward && m_iLotteryRewardTime <= iTimeNow)
	{
		//发放奖励
		SendLasvegasReward();
	}

	return;
}

//发放奖励
void CLasvegasManager::SendLasvegasReward()
{
	//读取开奖数字
	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();
	const LasvegasConfig* pstConfig = stBaseCfgMgr.GetLasvegasConfig(m_iLotteryID);
	if (!pstConfig)
	{
		LOGERROR("Failed to send lasvegas reward, invalid lottery id %d\n", m_iLotteryID);
		
		m_bNeedSendReward = false;
		m_iLotteryID = 0;
		return;
	}

	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_WORLD_UPDATEPRIZEINFO_REQUEST);
	World_UpdatePrizeInfo_Request* pstReq = stMsg.mutable_stbody()->mutable_m_stworld_updateprizeinfo_request();

	for (unsigned i=0; i<m_vUserList.size(); ++i)
	{
		for (unsigned j = 0; j < m_vUserList[i].vBets.size(); ++j)
		{
			if (m_vUserList[i].vBets[j].iNumber != pstConfig->iNumber)
			{
				continue;
			}

			//找到对应下注,增加奖励
			CGameRoleObj* pstRoleObj = CUnitUtility::GetRoleByUin(m_vUserList[i].uin);
			if (pstRoleObj)
			{
				int iRewardCoins = m_vUserList[i].vBets[j].iBetCoins * (pstConfig->iNumber + 1);
				if (!CResourceUtility::AddUserRes(*pstRoleObj, RESOURCE_TYPE_COIN, iRewardCoins))
				{
					LOGERROR("Failed to add lasvegas reward, uin %u, coins %d\n", pstRoleObj->GetUin(), iRewardCoins);
					continue;
				}

				LasvegasBetData stBetDate = m_vUserList[i].vBets[j];

				//获取当前金币数，运营日志使用
				long8 lNewCoinNum = pstRoleObj->GetResource(RESOURCE_TYPE_COIN);

				//打印运营日志  拉斯维加斯转盘
				CZoneOssLog::TraceLasvegas(pstRoleObj->GetUin(), pstRoleObj->GetChannel(), pstRoleObj->GetNickName(), stBetDate.iNumber, stBetDate.iBetCoins,
					pstConfig->iNumber, iRewardCoins, lNewCoinNum - iRewardCoins, lNewCoinNum);

				//是否能上榜
				if (iRewardCoins >= stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_LASVEGAS_REWARDNUM))
				{
					//发送上榜请求
					PrizeInfo* pstOneInfo = pstReq->add_stprizes();
					pstOneInfo->set_strname(pstRoleObj->GetNickName());
					pstOneInfo->set_inumber(pstConfig->iNumber);
					pstOneInfo->set_irewardcoins(iRewardCoins);
				}
			}

			break;
		}

		//清理玩家下注信息
		m_vUserList[i].vBets.clear();
	}

	CZoneMsgHelper::SendZoneMsgToWorld(stMsg);

	//设置为已发放
	m_bNeedSendReward = false;
	m_iLotteryID = 0;

	return;
}

//推送转盘信息
void CLasvegasManager::SendLasvegasInfoNotify(CGameRoleObj* pstRoleObj)
{
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_UPDATELASVEGAS_NOTIFY);
	
	Zone_UpdateLasvegas_Notify* pstNotify = stMsg.mutable_stbody()->mutable_m_stzone_updatelasvegas_notify();
	PackLasvegasInfo(*pstNotify->mutable_stinfo());

	if (!pstRoleObj)
	{
		//定时推送的
		TRoleObjList stRoleList;
		stRoleList.m_iRoleNumber = m_vUserList.size();
		for (unsigned i = 0; i < m_vUserList.size(); ++i)
		{
			stRoleList.m_apstRoleObj[i] = CUnitUtility::GetRoleByUin(m_vUserList[i].uin);
		}

		CZoneMsgHelper::SendZoneMsgToRoleList(stMsg, stRoleList);

		m_bSendUserNotify = false;
		m_iLastSendNotifyTime = CTimeUtility::GetNowTime();
	}
	else
	{
		//推送给指定玩家
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, pstRoleObj);
	}

	return;
}

//更新World下注信息
void CLasvegasManager::SendUpdateBetInfoToWorld(int iNumber, int iBetCoins)
{
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_WORLD_UPDATEBETINFO_REQUEST);

	World_UpdateBetInfo_Request* pstReq = stMsg.mutable_stbody()->mutable_m_stworld_updatebetinfo_request();
	NumberBetInfo* pstOneInfo = pstReq->add_stbetinfos();
	pstOneInfo->set_inumber(iNumber);
	pstOneInfo->set_lbetcoins(iBetCoins);

	CZoneMsgHelper::SendZoneMsgToWorld(stMsg);

	return;
}

//打包转盘信息,不包括中奖纪录
void CLasvegasManager::PackLasvegasInfo(LasvegasInfo& stInfo)
{
	stInfo.set_isteptype(m_iStepType);
	stInfo.set_istependtime(m_iStepEndTime);

	//开奖记录
	for (unsigned i = 0; i < m_vLotteryIDs.size(); ++i)
	{
		stInfo.add_ilotteryids(m_vLotteryIDs[i]);
	}

	//下注信息
	for (unsigned i = 0; i < m_vBetDatas.size(); ++i)
	{
		NumberBetInfo* stOneInfo = stInfo.add_stbets();
		stOneInfo->set_inumber(m_vBetDatas[i].iNumber);
		stOneInfo->set_lbetcoins(m_vBetDatas[i].iBetCoins);
	}

	return;
}

void CLasvegasManager::Reset()
{
	//当前阶段
	m_iStepType = 0;

	//当前阶段结束时间
	m_iStepEndTime = 0;

	//是否推送更新给玩家
	m_bSendUserNotify = false;

	//最近推送更新给玩家的时间
	m_iLastSendNotifyTime = 0;

	//是否需要发放奖励
	m_bNeedSendReward = false;

	//开奖发放奖励时间
	m_iLotteryRewardTime = 0;

	//本次开奖信息
	m_iLotteryID = 0;

	//最近开奖信息
	m_vLotteryIDs.clear();

	//最近中奖纪录
	m_vPrizeDatas.clear();

	//当前下注信息
	m_vBetDatas.clear();

	//转盘玩家列表
	m_vUserList.clear();
}
