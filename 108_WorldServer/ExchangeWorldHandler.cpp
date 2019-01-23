#include <assert.h>
#include <string.h>

#include "GameProtocol.hpp"
#include "ModuleHelper.hpp"
#include "WorldMsgHelper.hpp"
#include "LogAdapter.hpp"
#include "ErrorNumDef.hpp"
#include "WorldExchangeLimitManager.h"

#include "ExchangeWorldHandler.h"

static GameProtocolMsg stMsg;

CExchangeWorldHandler::~CExchangeWorldHandler()
{

}

int CExchangeWorldHandler::OnClientMsg(GameProtocolMsg* pMsg)
{
	ASSERT_AND_LOG_RTN_INT(pMsg);

	m_pRequestMsg = (GameProtocolMsg*)pMsg;

	switch (m_pRequestMsg->sthead().uimsgid())
	{
	case MSGID_ZONE_GETLIMITNUM_REQUEST:
	{
		//拉取限量兑换信息
		OnRequestGetExchange();
	}
	break;

	case MSGID_WORLD_ADDLIMITNUM_REQUEST:
	{
		//修改限量兑换信息
		OnRequestUpdateExchange();
	}
	break;

	case MSGID_WORLD_ONLINEEXCHANGE_REQUEST:
	{
		//在线兑换请求
		OnRequestOnlineExchange();
	}
	break;

	case MSGID_WORLD_ONLINEEXCHANGE_RESPONSE:
	{
		//在线兑换返回
		OnResponseOnlineExchange();
	}
	break;

	case MSGID_WORLD_ADDEXCREC_REQUEST:
	{
		//增加兑换记录请求
		OnRequestAddExchangeRec();
	}
	break;

	case MSGID_ZONE_GETEXCHANGEREC_REQUEST:
	{
		//拉取兑换记录请求
		OnRequestGetExchangeRec();
	}
	break;

	default:
		break;
	}

	return 0;
}

//拉取限量兑换信息
int CExchangeWorldHandler::OnRequestGetExchange()
{
	LOGDEBUG("OnRequestGetExchange\n");

	const Zone_GetLimitNum_Request& stReq = m_pRequestMsg->stbody().m_stzone_getlimitnum_request();
	
	CWorldMsgHelper::GenerateMsgHead(stMsg, 0, MSGID_ZONE_GETLIMITNUM_RESPONSE, stReq.uin());
	Zone_GetLimitNum_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_getlimitnum_response();

	CWorldExchangeLimitManager::Instance()->GetExchangeLimit(*pstResp);
	pstResp->set_iresult(T_SERVER_SUCCESS);

	//发送返回
	CWorldMsgHelper::SendWorldMsgToWGS(stMsg, stReq.izoneid());

	return T_SERVER_SUCCESS;
}

//修改限量兑换信息
int CExchangeWorldHandler::OnRequestUpdateExchange()
{
	LOGDEBUG("OnRequestUpdateExchange\n");

	//获取请求
	const World_AddLimitNum_Request& stReq = m_pRequestMsg->stbody().m_stworld_addlimitnum_request();

	//返回消息
	CWorldMsgHelper::GenerateMsgHead(stMsg, 0, MSGID_WORLD_ADDLIMITNUM_RESPONSE, stReq.uin());
	World_AddLimitNum_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stworld_addlimitnum_response();
	pstResp->set_iexchangeid(stReq.iexchangeid());
	pstResp->set_uin(stReq.uin());
	pstResp->set_ireason(stReq.ireason());

	//修改限量兑换
	int iRet = CWorldExchangeLimitManager::Instance()->UpdateExchangeLimit(stReq.iexchangeid(), stReq.iaddnum());
	if (iRet)
	{
		LOGERROR("Failed to update exchange, ret %d, exchange id %d, add num %d\n", iRet, stReq.iexchangeid(), stReq.iaddnum());

		pstResp->set_iresult(iRet);
		CWorldMsgHelper::SendWorldMsgToWGS(stMsg, stReq.izoneid());
		return iRet;
	}

	//发送成功的回复
	CWorldMsgHelper::SendWorldMsgToWGS(stMsg, stReq.izoneid());

	return T_SERVER_SUCCESS;;
}

//在线兑换请求
int CExchangeWorldHandler::OnRequestOnlineExchange()
{
	LOGDEBUG("OnRequestOnlineExchange\n");

	//直接转发给ExchangeServer
	CWorldMsgHelper::SendWorldMsgToExchange(*m_pRequestMsg);

	return T_SERVER_SUCCESS;;
}

//在线兑换返回
int CExchangeWorldHandler::OnResponseOnlineExchange()
{
	const World_OnlineExchange_Response& stResp = m_pRequestMsg->stbody().m_stworld_onlineexchange_response();

	//直接转发给玩家
	CWorldMsgHelper::SendWorldMsgToWGS(*m_pRequestMsg, stResp.ifromzoneid());

	return T_SERVER_SUCCESS;
}

//增加兑换记录请求
int CExchangeWorldHandler::OnRequestAddExchangeRec()
{
	LOGDEBUG("OnRequestAddExchangeRec\n");

	//获取请求
	const World_AddExcRec_Request& stReq = m_pRequestMsg->stbody().m_stworld_addexcrec_request();

	//不需要发送返回

	//增加兑换记录
	CWorldExchangeLimitManager::Instance()->AddExchangeRec(stReq.strecord());

	return T_SERVER_SUCCESS;
}

//拉取兑换记录请求
int CExchangeWorldHandler::OnRequestGetExchangeRec()
{
	LOGDEBUG("OnRequestGetExchangeRec\n");

	//获取请求
	const Zone_GetExchangeRec_Request& stReq = m_pRequestMsg->stbody().m_stzone_getexchangerec_request();

	//返回消息
	CWorldMsgHelper::GenerateMsgHead(stMsg, 0, MSGID_ZONE_GETEXCHANGEREC_RESPONSE, 0);
	Zone_GetExchangeRec_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_getexchangerec_response();
	pstResp->set_ifromindex(stReq.ifromindex());
	pstResp->set_inum(stReq.inum());
	pstResp->set_uin(stReq.uin());

	//拉取兑换记录
	int iRet = CWorldExchangeLimitManager::Instance()->GetExchangeRec(stReq.ifromindex(), stReq.inum(), *pstResp);
	if (iRet)
	{
		LOGERROR("Failed to get exchange record, ret %d\n", iRet);
		CWorldMsgHelper::SendWorldMsgToWGS(stMsg, stReq.izoneid());
		return -1;
	}

	//拉取成功
	CWorldMsgHelper::SendWorldMsgToWGS(stMsg, stReq.izoneid());

	return T_SERVER_SUCCESS;
}
