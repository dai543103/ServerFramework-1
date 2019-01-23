
#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "ErrorNumDef.hpp"
#include "Kernel/ModuleHelper.hpp"
#include "Kernel/ZoneMsgHelper.hpp"
#include "Kernel/UnitUtility.hpp"
#include "Kernel/ZoneOssLog.hpp"
#include "Mail/MailUtility.h"

#include "ExchangeHandler.h"

using namespace ServerLib;

static GameProtocolMsg stMsg;

CExchangeHandler::~CExchangeHandler()
{

}

int CExchangeHandler::OnClientMsg()
{
	switch (m_pRequestMsg->sthead().uimsgid())
	{
	case MSGID_ZONE_SETEXCHANGE_REQUEST:
	{
		//修改用户信息
		OnRequestChangeUserInfo();
	}
	break;

	case MSGID_ZONE_EXCHANGEITEM_REQUEST:
	{
		//玩家兑换
		OnRequestExchange();
	}
	break;

	case MSGID_ZONE_GETLIMITNUM_REQUEST:
	{
		//玩家拉取限量
		OnRequestGetLimitInfo();
	}
	break;

	case MSGID_ZONE_GETLIMITNUM_RESPONSE:
	{
		//World拉取限量返回
		OnResponseGetLimitInfo();
	}
	break;

	case MSGID_WORLD_ADDLIMITNUM_RESPONSE:
	{
		//World修改限量的返回
		OnResponseUpdateLimit();
	}
	break;

	case MSGID_WORLD_GETCARDNO_RESPONSE:
	{
		//World拉取卡密信息的返回
		OnResponseGetCardNo();
	}
	break;

	case MSGID_ZONE_GETEXCHANGEREC_REQUEST:
	{
		//玩家拉取兑换记录的请求
		OnRequestGetExchangeRec();
	}
	break;

	case MSGID_ZONE_GETEXCHANGEREC_RESPONSE:
	{
		//World拉取兑换记录的返回
		OnResponseGetExchangeRec();
	}
	break;

	default:
		break;
	}

	return 0;
}

//玩家修改信息的请求
int CExchangeHandler::OnRequestChangeUserInfo()
{
	int iRet = SecurityCheck();
	if (iRet < 0)
	{
		LOGERROR("Security Check Failed: Uin = %u, iRet = %d\n", m_pRequestMsg->sthead().uin(), iRet);
		return -1;
	}

	//返回消息
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_SETEXCHANGE_RESPONSE);
	Zone_SetExchange_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_setexchange_response();

	//处理请求
	const Zone_SetExchange_Request& stRequest = m_pRequestMsg->stbody().m_stzone_setexchange_request();
	iRet = m_pRoleObj->GetExchangeManager().SetUserExchangeInfo(stRequest.stuserinfo(), *pstResp);
	if (iRet)
	{
		LOGERROR("Failed to set user exchange info, uin %u, ret %d\n", m_pRoleObj->GetUin(), iRet);

		pstResp->set_iresult(iRet);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -2;
	}

	//发送返回
	pstResp->set_iresult(T_SERVER_SUCCESS);
	CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);

	return T_SERVER_SUCCESS;
}

//玩家兑换的请求
int CExchangeHandler::OnRequestExchange()
{
	int iRet = SecurityCheck();
	if (iRet < 0)
	{
		LOGERROR("Security Check Failed: Uin = %u, iRet = %d\n", m_pRequestMsg->sthead().uin(), iRet);
		return -1;
	}

	//返回消息
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_EXCHANGEITEM_RESPONSE);
	Zone_ExchangeItem_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_exchangeitem_response();

	//处理请求
	const Zone_ExchangeItem_Request& stRequest = m_pRequestMsg->stbody().m_stzone_exchangeitem_request();
	pstResp->mutable_storder()->set_iexchangeid(stRequest.iexchangeid());

	//读取配置
	const ExchangeConfig* pstConfig = CConfigManager::Instance()->GetBaseCfgMgr().GetExchangeConfig(stRequest.iexchangeid());
	if (!pstConfig)
	{
		LOGERROR("Failed to exchange item, invalid exchange id %d, uin %u\n", stRequest.iexchangeid(), m_pRoleObj->GetUin());

		pstResp->set_iresult(iRet);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -2;
	}

	//VIP等级是否满足
	if (m_pRoleObj->GetVIPLevel() < (pstConfig->iVIPLv + 1))
	{
		//VIP等级不满足
		LOGERROR("Failed to exchange item, vip real:need %d:%d, uin %u\n", m_pRoleObj->GetVIPLevel(), (pstConfig->iVIPLv + 1), m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -10;
	}

	//获取兑换管理器
	CExchangeManager& stExchangeMgr = m_pRoleObj->GetExchangeManager();

	if (pstConfig->stReward.iType == REWARD_TYPE_ENTITY && !stExchangeMgr.IsUserInfoSet())
	{
		//实物兑换并且用户信息未设置
		LOGERROR("Failed to exchange item, user info not set, exchange id %d, uin %u\n", stRequest.iexchangeid(), m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -10;
	}

	//检查玩家个人兑换限制
	if ((pstConfig->stReward.iRewardID == EXCHANGE_FIVEBILL_ID && !stExchangeMgr.CheckIsLimit(PERSONLIMIT_TYPE_FIVEBILL)) ||
		(pstConfig->stReward.iRewardID == EXCHANGE_TENBILL_ID && !stExchangeMgr.CheckIsLimit(PERSONLIMIT_TYPE_TENBILL)))
	{
		//玩家个人限量不足
		LOGERROR("Failed to exchange item, person limit check failed, exchange id %d, uin %u\n", stRequest.iexchangeid(), m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_PERSON_LIMIT);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -10;
	}

	//先扣消耗
	//限量如果发现不能购买，需要退还消耗
	iRet = stExchangeMgr.DoExchangeCost(*pstConfig);
	if (iRet)
	{
		//不能扣除消耗
		LOGERROR("Failed to do exchange cost, ret %d, uin %u, exchange id %d\n", iRet, m_pRoleObj->GetUin(), stRequest.iexchangeid());

		pstResp->set_iresult(iRet);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -3;
	}

	//处理限量
	if (pstConfig->iIsLimit)
	{
		//限量兑换
		CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_WORLD_ADDLIMITNUM_REQUEST);
		World_AddLimitNum_Request* pstReq = stMsg.mutable_stbody()->mutable_m_stworld_addlimitnum_request();
		pstReq->set_ireason(LIMIT_REASON_EXCHANGE);
		pstReq->set_iexchangeid(stRequest.iexchangeid());
		pstReq->set_iaddnum(-1);	//扣除1个
		pstReq->set_uin(m_pRoleObj->GetUin());
		pstReq->set_izoneid(CModuleHelper::GetZoneID());

		CZoneMsgHelper::SendZoneMsgToWorld(stMsg);

		return 0;
	}

	//是否兑换卡密
	if (pstConfig->stReward.iType == REWARD_TYPE_CARDNO)
	{
		//发送获取卡密请求
		iRet = CExchangeManager::SendGetCardNoRequest(m_pRoleObj->GetUin(), *pstConfig);
		if (iRet)
		{
			//返还消耗
			stExchangeMgr.DoExchangeCost(*pstConfig, true);

			pstResp->set_iresult(iRet);
			CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
			return -20;
		}

		return 0;
	}

	iRet = stExchangeMgr.ExchangeItem(*pstConfig, *pstResp->mutable_storder());
	if (iRet)
	{
		LOGERROR("Failed to exchange item, uin %u, ret %d\n", m_pRoleObj->GetUin(), iRet);

		pstResp->set_iresult(iRet);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -4;
	}

	//发送返回
	pstResp->set_iresult(T_SERVER_SUCCESS);
	CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);

	return T_SERVER_SUCCESS;
}

//玩家拉取限限量的请求
int CExchangeHandler::OnRequestGetLimitInfo()
{
	int iRet = SecurityCheck();
	if (iRet < 0)
	{
		LOGERROR("Security Check Failed: Uin = %u, iRet = %d\n", m_pRequestMsg->sthead().uin(), iRet);
		return -1;
	}

	//处理请求
	Zone_GetLimitNum_Request* pstReq = m_pRequestMsg->mutable_stbody()->mutable_m_stzone_getlimitnum_request();

	//封装参数
	pstReq->set_uin(m_pRoleObj->GetUin());
	pstReq->set_izoneid(CModuleHelper::GetZoneID());

	//转发请求到World
	CZoneMsgHelper::SendZoneMsgToWorld(*m_pRequestMsg);

	//等World返回后继续处理流程

	return T_SERVER_SUCCESS;
}

//World拉取限量的返回
int CExchangeHandler::OnResponseGetLimitInfo()
{
	unsigned uin = m_pRequestMsg->sthead().uin();

	//直接转发给客户端
	CZoneMsgHelper::SendZoneMsgToRole(*m_pRequestMsg, CUnitUtility::GetRoleByUin(uin));

	return T_SERVER_SUCCESS;
}

//World更新限量的返回
int CExchangeHandler::OnResponseUpdateLimit()
{
	//获取返回
	const World_AddLimitNum_Response& stResp = m_pRequestMsg->stbody().m_stworld_addlimitnum_response();

	int iRet = T_SERVER_SUCCESS;
	switch (stResp.ireason())
	{
	case LIMIT_REASON_EXCHANGE:
	{
		//玩家兑换
		CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_EXCHANGEITEM_RESPONSE);
		Zone_ExchangeItem_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_exchangeitem_response();

		CGameRoleObj* pstRoleObj = CUnitUtility::GetRoleByUin(stResp.uin());
		if (!pstRoleObj)
		{
			return -1;
		}

		//获取兑换配置
		const ExchangeConfig* pstConfig = CConfigManager::Instance()->GetBaseCfgMgr().GetExchangeConfig(stResp.iexchangeid());
		if (!pstConfig)
		{
			LOGERROR("Failed to get exchange config, invalid id %d, uin %u\n", stResp.iexchangeid(), stResp.uin());

			pstResp->set_iresult(T_ZONE_INVALID_CFG);
			CZoneMsgHelper::SendZoneMsgToRole(stMsg, CUnitUtility::GetRoleByUin(stResp.uin()));
			return -2;
		}

		CExchangeManager& stExchangeMgr = pstRoleObj->GetExchangeManager();
		if (stResp.iresult() != 0)
		{
			//兑换失败,返还消耗
			iRet = stExchangeMgr.DoExchangeCost(*pstConfig, true);
			if (iRet)
			{
				LOGERROR("Failed to return exchange cost, ret %d, uin %u, exchange id %d\n", iRet, stResp.uin(), stResp.iexchangeid());
			}

			pstResp->set_iresult(stResp.iresult());
			CZoneMsgHelper::SendZoneMsgToRole(stMsg, pstRoleObj);
			return -3;
		}
		else
		{
			//处理兑换

			//是否兑换卡密
			if (pstConfig->stReward.iType == REWARD_TYPE_CARDNO)
			{
				//发送获取卡密请求
				iRet = CExchangeManager::SendGetCardNoRequest(pstRoleObj->GetUin(), *pstConfig);
				if (iRet)
				{
					//返还消耗
					stExchangeMgr.DoExchangeCost(*pstConfig, true);

					pstResp->set_iresult(iRet);
					CZoneMsgHelper::SendZoneMsgToRole(stMsg, pstRoleObj);
					return -20;
				}

				return 0;
			}

			iRet = stExchangeMgr.ExchangeItem(*pstConfig, *pstResp->mutable_storder());
			if (iRet)
			{
				LOGERROR("Failed to exchange item, uin %u, ret %d\n", pstRoleObj->GetUin(), iRet);

				pstResp->set_iresult(iRet);
				CZoneMsgHelper::SendZoneMsgToRole(stMsg, pstRoleObj);
				return -4;
			}

			//发送返回
			pstResp->set_iresult(T_SERVER_SUCCESS);
			CZoneMsgHelper::SendZoneMsgToRole(stMsg, pstRoleObj);
		}
	}
	break;

	case LIMIT_REASON_GM:
	{
		//GM管理员修改

	}
	break;

	default:
		break;
	}

	return T_SERVER_SUCCESS;
}

//World拉取卡密信息的返回
int CExchangeHandler::OnResponseGetCardNo()
{
	//获取返回
	const World_GetCardNo_Response& stResp = m_pRequestMsg->stbody().m_stworld_getcardno_response();

	CGameRoleObj* pstRoleObj = CUnitUtility::GetRoleByUin(stResp.uin());
	if (!pstRoleObj)
	{
		return -1;
	}

	//返回给玩家的消息
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_EXCHANGEITEM_RESPONSE);
	Zone_ExchangeItem_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_exchangeitem_response();

	//获取兑换配置
	const ExchangeConfig* pstConfig = CConfigManager::Instance()->GetBaseCfgMgr().GetExchangeConfig(stResp.iexchangeid());
	if (!pstConfig)
	{
		LOGERROR("Failed to get exchange config, invalid id %d, uin %u\n", stResp.iexchangeid(), stResp.uin());

		pstResp->set_iresult(T_ZONE_INVALID_CFG);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, pstRoleObj);
		return -3;
	}

	int iRet = T_SERVER_SUCCESS;
	CExchangeManager& stExchangeMgr = pstRoleObj->GetExchangeManager();
	if (stResp.iresult() != 0)
	{
		//拉取卡密失败，返回兑换消耗
		iRet = stExchangeMgr.DoExchangeCost(*pstConfig, true);
		if (iRet)
		{
			LOGERROR("Failed to return exchange cost, ret %d, uin %u, exchange id %d\n", iRet, stResp.uin(), stResp.iexchangeid());
		}

		pstResp->set_iresult(stResp.iresult());
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, pstRoleObj);
		return -5;
	}

	//拉取卡密成功

	//发送卡密邮件
	iRet = CMailUtility::SendCardNoMail(*pstRoleObj, pstConfig->iID, stResp.strcardno(), stResp.strcardpwd());
	if (iRet)
	{
		LOGERROR("Failed to send card no mail, exchange id %d, ret %d, uin %u\n", pstConfig->iID, iRet, stResp.uin());

		pstResp->set_iresult(stResp.iresult());
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, pstRoleObj);
		return -6;
	}

	//完成兑换订单
	iRet = stExchangeMgr.ExchangeItem(*pstConfig, *pstResp->mutable_storder());
	if (iRet)
	{
		LOGERROR("Failed to exchange item, uin %u, ret %d\n", stResp.uin(), iRet);

		pstResp->set_iresult(iRet);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, pstRoleObj);
		return -7;
	}

	//发送返回消息
	pstResp->set_iresult(T_SERVER_SUCCESS);
	CZoneMsgHelper::SendZoneMsgToRole(stMsg, pstRoleObj);

	return 0;
}

//玩家拉取兑换记录的请求
int CExchangeHandler::OnRequestGetExchangeRec()
{
	int iRet = SecurityCheck();
	if (iRet < 0)
	{
		LOGERROR("Security Check Failed: Uin = %u, iRet = %d\n", m_pRequestMsg->sthead().uin(), iRet);
		return -1;
	}

	//处理请求
	Zone_GetExchangeRec_Request* pstReq = m_pRequestMsg->mutable_stbody()->mutable_m_stzone_getexchangerec_request();
	
	//检查参数
	if (pstReq->ifromindex() <= 0 || pstReq->inum() <= 0)
	{
		LOGERROR("Failed to get exchange record, invalid param, uin %u\n", m_pRoleObj->GetUin());
		return -2;
	}

	//封装参数
	pstReq->set_uin(m_pRoleObj->GetUin());
	pstReq->set_izoneid(CModuleHelper::GetZoneID());

	//转发请求到World
	CZoneMsgHelper::SendZoneMsgToWorld(*m_pRequestMsg);

	//等World返回后继续处理流程

	return T_SERVER_SUCCESS;
}

//World拉取兑换记录的返回
int CExchangeHandler::OnResponseGetExchangeRec()
{
	//处理请求
	const Zone_GetExchangeRec_Response& stResp = m_pRequestMsg->stbody().m_stzone_getexchangerec_response();

	//直接发送给客户端
	CZoneMsgHelper::SendZoneMsgToRole(*m_pRequestMsg, CUnitUtility::GetRoleByUin(stResp.uin()));

	return T_SERVER_SUCCESS;
}

