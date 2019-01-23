
#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "ErrorNumDef.hpp"
#include "TimeUtility.hpp"
#include "Kernel/ZoneObjectHelper.hpp"
#include "Kernel/ZoneMsgHelper.hpp"
#include "Kernel/ConfigManager.hpp"
#include "Kernel/ZoneOssLog.hpp"
#include "Resource/ResourceUtility.h"
#include "RepThings/RepThingsUtility.hpp"
#include "Reward/RewardUtility.h"
#include "Chat/ChatUtility.h"

#include "ExchangeManager.h"

using namespace ServerLib;

CExchangeManager::CExchangeManager()
{
	Initialize();
}

CExchangeManager::~CExchangeManager()
{

}

//初始化
int CExchangeManager::Initialize()
{
	m_uiUin = 0;
	
	m_stExchangeData.Reset();
	m_vOrderData.clear();

	return T_SERVER_SUCCESS;
}

void CExchangeManager::SetOwner(unsigned int uin)
{
	m_uiUin = uin;
}

CGameRoleObj* CExchangeManager::GetOwner()
{
	return GameTypeK32<CGameRoleObj>::GetByKey(m_uiUin);
}

//设置兑换信息
int CExchangeManager::SetUserExchangeInfo(const ExchangeUser& stUserInfo, Zone_SetExchange_Response& stResp)
{
	int iTimeNow = CTimeUtility::GetNowTime();

	//判断是否能设置
	if (CTimeUtility::IsInSameWeek(m_stExchangeData.iLastSetTime, iTimeNow))
	{
		//不能设置
		LOGERROR("Failed to set exchange info, uin %u, last:now %d:%d\n", m_uiUin, m_stExchangeData.iLastSetTime, iTimeNow);
		return T_ZONE_PARA_ERROR;
	}

	m_stExchangeData.iLastSetTime = iTimeNow;
	m_stExchangeData.strName = stUserInfo.strname();
	m_stExchangeData.strPhone = stUserInfo.strphone();
	m_stExchangeData.strMailNum = stUserInfo.strmailnum();
	m_stExchangeData.strAddress = stUserInfo.straddress();
	m_stExchangeData.strRemarks = stUserInfo.strremarks();
	m_stExchangeData.strQQNum = stUserInfo.strqqnum();

	stResp.mutable_stuserinfo()->CopyFrom(stUserInfo);
	stResp.mutable_stuserinfo()->set_ilastsettime(iTimeNow);

	return T_SERVER_SUCCESS;
}

//玩家兑换
int CExchangeManager::ExchangeItem(const ExchangeConfig& stConfig, ExchangeOrder& stOrderInfo)
{
	//获取玩家信息
	CGameRoleObj* pstRoleObj = GetOwner();
	if (!pstRoleObj)
	{
		return T_ZONE_GAMEROLE_NOT_EXIST;
	}

	//增加兑换获得
	int iRet = CRewardUtility::GetReward(*pstRoleObj, 1, &stConfig.stReward, 1, ITEM_CHANNEL_EXCHANGE);
	if (iRet)
	{
		LOGERROR("Failed to get exchange reward, ret %d, uin %u, exchange id %d\n", iRet, m_uiUin, stConfig.iID);
		return iRet;
	}

	int iTimeNow = CTimeUtility::GetNowTime();

	//增加玩家订单记录
	OrderData stOrder;
	stOrder.iExchangeID = stConfig.iID;
	stOrder.strOrderID = GetOrderID(iTimeNow);
	stOrder.strPhone = m_stExchangeData.strPhone;
	stOrder.iExchangeTime = iTimeNow;
	stOrder.iStat = 0;

	m_vOrderData.insert(m_vOrderData.begin(), stOrder);

	//设置返回订单信息
	stOrderInfo.set_iexchangeid(stOrder.iExchangeID);
	stOrderInfo.set_strorderid(stOrder.strOrderID);
	stOrderInfo.set_strphone(stOrder.strPhone);
	stOrderInfo.set_iexchangetime(stOrder.iExchangeTime);
	stOrderInfo.set_istat(stOrder.iStat);

	if (stConfig.stReward.iType == REWARD_TYPE_ENTITY || stConfig.stReward.iType == REWARD_TYPE_CARDNO)
	{
		//实物兑换，记录兑换信息
		CZoneOssLog::TraceEntityExchange(m_uiUin, stOrder.iExchangeID, stOrder.strOrderID, stOrder.iExchangeTime, m_stExchangeData.strName, m_stExchangeData.strPhone,
			m_stExchangeData.strMailNum, m_stExchangeData.strAddress, m_stExchangeData.strRemarks, m_stExchangeData.strQQNum);
	}
	else if (stConfig.stReward.iType == REWARD_TYPE_RES || stConfig.stReward.iType == REWARD_TYPE_ITEM)
	{
		//获取当前鱼票数量 运营日志使用
		long8 lNewTicketNum = GetOwner()->GetResource(RESOURCE_TYPE_TICKET);

		//获取奖励道具的数量 运营日志使用
		long8 lNewRewardNum = 0;
		switch (stConfig.stReward.iType)
		{
		case REWARD_TYPE_RES:
		{
			lNewRewardNum = GetOwner()->GetResource(stConfig.stReward.iRewardID);
		}
		break;

		case REWARD_TYPE_ITEM:
		{
			lNewRewardNum = GetOwner()->GetRepThingsManager().GetRepItemNum(stConfig.stReward.iRewardID);
		}
		break;

		default:
			break;
		}

		//打印运营日志 兑换
		CZoneOssLog::TraceExchange(GetOwner()->GetUin(), GetOwner()->GetChannel(), GetOwner()->GetNickName(), stConfig.iID, stConfig.lCostNum,
			lNewTicketNum + stConfig.lCostNum, lNewTicketNum, stConfig.stReward.iType, stConfig.stReward.iRewardID, lNewRewardNum - stConfig.stReward.iRewardNum,
			lNewRewardNum);

	}

	//处理个人兑换限量
	if (stConfig.stReward.iRewardID == EXCHANGE_FIVEBILL_ID)
	{
		SetPersonLimit(PERSONLIMIT_TYPE_FIVEBILL, GetPersonLimitInfo(PERSONLIMIT_TYPE_FIVEBILL) + 1);
	}
	else if (stConfig.stReward.iRewardID == EXCHANGE_TENBILL_ID)
	{
		SetPersonLimit(PERSONLIMIT_TYPE_TENBILL, GetPersonLimitInfo(PERSONLIMIT_TYPE_TENBILL) + 1);
	}

	if (stConfig.bIsNotice)
	{
		//播放兑换走马灯
		HorseLampData stOneData;
		stOneData.iLampID = HORSELAMP_TYPE_EXCHANGE;

		char szParam[64] = {0};
		SAFE_SPRINTF(szParam, sizeof(szParam)-1, "%s", pstRoleObj->GetNickName());
		stOneData.vParams.push_back(szParam);

		SAFE_SPRINTF(szParam, sizeof(szParam)-1, "%d", stConfig.iID);
		stOneData.vParams.push_back(szParam);

		std::vector<HorseLampData> vLampData;
		vLampData.push_back(stOneData);

		CChatUtility::SendHorseLamp(vLampData);
	}

	//增加服务器兑换记录
	static GameProtocolMsg stMsg;
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_WORLD_ADDEXCREC_REQUEST);

	World_AddExcRec_Request* pstReq = stMsg.mutable_stbody()->mutable_m_stworld_addexcrec_request();
	pstReq->mutable_strecord()->set_itime(iTimeNow);
	pstReq->mutable_strecord()->set_iexchangeid(stConfig.iID);
	pstReq->mutable_strecord()->set_strname(pstRoleObj->GetNickName());

	CZoneMsgHelper::SendZoneMsgToWorld(stMsg);

	return T_SERVER_SUCCESS;
}

//扣除兑换消耗
int CExchangeManager::DoExchangeCost(const ExchangeConfig& stConfig, bool bReturn)
{
	CGameRoleObj* pstRoleObj = GetOwner();
	long8 lCostNum = bReturn ? stConfig.lCostNum : -stConfig.lCostNum;

	//扣除消耗
	switch (stConfig.iCostType)
	{
	case EXCHANGE_COST_TICKET:
	{
		//扣除鱼票
		if (!CResourceUtility::AddUserRes(*pstRoleObj, RESOURCE_TYPE_TICKET, lCostNum))
		{
			return T_ZONE_PARA_ERROR;
		}
	}
	break;

	case EXCHANGE_COST_WARHEAD:
	{
		//弹头兑换，检测权限
		if (!pstRoleObj->HasVIPPriv(VIP_PRIV_EXCHANGE))
		{
			//没有开放弹头兑换
			LOGERROR("Failed to exchange warhead, no priv, uin %u\n", pstRoleObj->GetUin());
			return T_ZONE_PARA_ERROR;
		}

		//扣除弹头
		int iRet = CRepThingsUtility::AddItemNum(*pstRoleObj, EXCHANGE_WARHEAD_ID, lCostNum, ITEM_CHANNEL_EXCHANGE);
		if (iRet)
		{
			return iRet;
		}
	}
	break;

	default:
	{
		return T_ZONE_INVALID_CFG;
	}
	break;
	}

	return T_SERVER_SUCCESS;
}

//用户信息是否已设置
bool CExchangeManager::IsUserInfoSet()
{
	return (m_stExchangeData.iLastSetTime != 0);
}

//发送拉取卡密的请求
int CExchangeManager::SendGetCardNoRequest(unsigned uin, const ExchangeConfig& stConfig)
{
	static GameProtocolMsg stMsg;

	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_WORLD_GETCARDNO_REQUEST);
	
	World_GetCardNo_Request* pstReq = stMsg.mutable_stbody()->mutable_m_stworld_getcardno_request();
	pstReq->set_uin(uin);
	pstReq->set_ifromzoneid(CModuleHelper::GetZoneID());
	pstReq->set_iexchangeid(stConfig.iID);
	pstReq->set_icardid(stConfig.stReward.iRewardID);

	return CZoneMsgHelper::SendZoneMsgToWorld(stMsg);
}

//玩家兑换数据库操作函数
void CExchangeManager::UpdateExchangeToDB(EXCHANGEDBINFO& stExchangeInfo)
{
	//玩家兑换信息
	ExchangeUser* pstUserInfo = stExchangeInfo.mutable_stuserinfo();
	pstUserInfo->set_strname(m_stExchangeData.strName);
	pstUserInfo->set_strphone(m_stExchangeData.strPhone);
	pstUserInfo->set_strmailnum(m_stExchangeData.strMailNum);
	pstUserInfo->set_straddress(m_stExchangeData.strAddress);
	pstUserInfo->set_strremarks(m_stExchangeData.strRemarks);
	pstUserInfo->set_strqqnum(m_stExchangeData.strQQNum);
	pstUserInfo->set_ilastsettime(m_stExchangeData.iLastSetTime);

	//设置10，5话费兑换限量信息
	for (unsigned i = 0; i < sizeof(m_stExchangeData.auPersonLimit)/sizeof(unsigned); ++i)
	{
		pstUserInfo->add_uilimitinfo(m_stExchangeData.auPersonLimit[i]);
	}

	//兑换订单信息
	for (unsigned i = 0; i < m_vOrderData.size() && i < (unsigned)MAX_EXCHANGE_ORDER_NUM; ++i)
	{
		ExchangeOrder* pstOneOrder = stExchangeInfo.add_storders();
		pstOneOrder->set_iexchangeid(m_vOrderData[i].iExchangeID);
		pstOneOrder->set_strorderid(m_vOrderData[i].strOrderID);
		pstOneOrder->set_strphone(m_vOrderData[i].strPhone);
		pstOneOrder->set_iexchangetime(m_vOrderData[i].iExchangeTime);
		pstOneOrder->set_istat(m_vOrderData[i].iStat);
	}

	return;
}

void CExchangeManager::InitExchangeFromDB(const EXCHANGEDBINFO& stExchangeInfo)
{
	//玩家兑换信息
	m_stExchangeData.strName = stExchangeInfo.stuserinfo().strname();
	m_stExchangeData.strPhone = stExchangeInfo.stuserinfo().strphone();
	m_stExchangeData.strMailNum = stExchangeInfo.stuserinfo().strmailnum();
	m_stExchangeData.strAddress = stExchangeInfo.stuserinfo().straddress();
	m_stExchangeData.strRemarks = stExchangeInfo.stuserinfo().strremarks();
	m_stExchangeData.strQQNum = stExchangeInfo.stuserinfo().strqqnum();
	m_stExchangeData.iLastSetTime = stExchangeInfo.stuserinfo().ilastsettime();

	//10，5元话费兑换限制信息
	for (int i = 0; i < stExchangeInfo.stuserinfo().uilimitinfo_size(); ++i)
	{
		m_stExchangeData.auPersonLimit[i] = stExchangeInfo.stuserinfo().uilimitinfo(i);
	}

	//兑换订单信息
	m_vOrderData.clear();
	
	OrderData stOrder;
	for (int i = 0; i < stExchangeInfo.storders_size() && i<MAX_EXCHANGE_ORDER_NUM; ++i)
	{
		if (stExchangeInfo.storders(i).iexchangetime() <= 1514885705)
		{
			continue;
		}

		stOrder.iExchangeID = stExchangeInfo.storders(i).iexchangeid();
		stOrder.strOrderID = stExchangeInfo.storders(i).strorderid();
		stOrder.strPhone = stExchangeInfo.storders(i).strphone();
		stOrder.iExchangeTime = stExchangeInfo.storders(i).iexchangetime();
		stOrder.iStat = stExchangeInfo.storders(i).istat();

		m_vOrderData.push_back(stOrder);
	}

	return;
}

//获取订单号
std::string CExchangeManager::GetOrderID(int iTimeNow)
{
	static int iIndex = 0;

	char szPram[32] = { 0 };
	SAFE_SPRINTF(szPram, sizeof(szPram)-1, "%d%05d", iTimeNow, iIndex++%100000);

	return szPram;
}

//获取个人限制信息
unsigned CExchangeManager::GetPersonLimitInfo(int iLimitType)
{
	switch (iLimitType)
	{
	case PERSONLIMIT_TYPE_FIVEBILL:
	{
		return m_stExchangeData.auPersonLimit[0];
	}
	break;

	case PERSONLIMIT_TYPE_TENBILL:
	{
		return m_stExchangeData.auPersonLimit[1];
	}
	break;

	default:
		break;
	}

	return 0;
}

//是否允许个人限量兑换
bool CExchangeManager::CheckIsLimit(int iLimitType)
{
	switch (iLimitType)
	{
	case PERSONLIMIT_TYPE_FIVEBILL:
	{
		return (m_stExchangeData.auPersonLimit[0] == 0);
	}
	break;

	case PERSONLIMIT_TYPE_TENBILL:
	{
		unsigned int iInitTenBillNum = CConfigManager::Instance()->GetBaseCfgMgr().GetGlobalConfig(GLOBAL_TYPE_TENBILL_LIMIT);
		return (iInitTenBillNum > m_stExchangeData.auPersonLimit[1]);
	}
	break;

	default:
		break;
	}

	return false;
}

//处理个人兑换限量
void CExchangeManager::SetPersonLimit(int iLimitType, unsigned iNum)
{
	switch (iLimitType)
	{
	case PERSONLIMIT_TYPE_FIVEBILL:
	{
		//处理5元话费兑换限量
		m_stExchangeData.auPersonLimit[0] = iNum;
	}
	break;

	case PERSONLIMIT_TYPE_TENBILL:
	{
		//处理10元话费兑换限量
		m_stExchangeData.auPersonLimit[1] = iNum;
	}
	break;

	default:
		break;
	}

	return;
}
