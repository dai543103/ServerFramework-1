#include <assert.h>

#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "SessionObj.hpp"
#include "StringUtility.hpp"
#include "ConfigManager.hpp"
#include "PasswordEncryptionUtility.hpp"
#include "SessionKeyUtility.hpp"
#include "TimeUtility.hpp"
#include "ErrorNumDef.hpp"
#include "RegAuthMsgProcessor.hpp"

#include "RegAuthDBFetchHandler.hpp"

using namespace ServerLib;

static GameProtocolMsg stMsg;

CRegAuthDBFetchHandler::CRegAuthDBFetchHandler()
{
}

void CRegAuthDBFetchHandler::OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult, TNetHead_V2* pstNetHead)
{
	ASSERT_AND_LOG_RTN_VOID(pstNetHead);

	//暂不支持该协议
	return;

	m_pstNetHead = pstNetHead;
	m_pstRequestMsg = &stReqMsg;

	switch (m_pstRequestMsg->sthead().uimsgid())
	{
	case MSGID_REGAUTHDB_FETCH_RESPONSE:
	{
		//拉取帐号信息的返回
		OnResponseRegAuthDBFetch();
	}
	break;

	default:
	{

	}
	break;
	}

	return;
}

int CRegAuthDBFetchHandler::OnResponseRegAuthDBFetch()
{
	//获取发送消息的FD
	unsigned int uiSessionFd = m_pstRequestMsg->sthead().uisessionfd();

	//获取请求
	const RegAuthDB_GetAccount_Response& stResp = m_pstRequestMsg->stbody().m_stregauthdb_fetch_response();
	
	//返回消息
	GenerateMsgHead(stMsg, uiSessionFd, MSGID_REGAUTH_AUTHACCOUNT_RESPONSE, 0);
	RegAuth_AuthAccount_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stregauth_authaccount_response();

	if (stResp.iresult() != 0)
	{
		//认证查询帐号失败，返回认证失败给LotusServer
		pstResp->set_iresult(stResp.iresult());
		CRegAuthMsgProcessor::Instance()->SendRegAuthToLotus(uiSessionFd, stMsg);
		return -1;
	}

	//认证登录成功，返回认证成功给LotusServer
	SendAuthSuccessResponseToLotus(uiSessionFd, stResp.uin(), stResp.iworldid());

	LOGDEBUG("Success to do auth account, account: %s, uin %u, world:%d\n", stResp.stinfo().straccount().c_str(), stResp.uin(), stResp.iworldid());

	return T_SERVER_SUCCESS;
}

//发送认证成功回复给LotusServer
void CRegAuthDBFetchHandler::SendAuthSuccessResponseToLotus(unsigned int uiSessionFd, unsigned int uin, int iWorldID)
{
	GenerateMsgHead(stMsg, uiSessionFd, MSGID_REGAUTH_AUTHACCOUNT_RESPONSE, 0);

	//生成响应消息体
	RegAuth_AuthAccount_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stregauth_authaccount_response();
	pstResp->set_iresult(T_SERVER_SUCCESS);
	pstResp->set_uin(uin);
	pstResp->set_iworldid(iWorldID);

	//获取所在服务器组的配置
	const ServerInfoConfig* pstServerConfig = CConfigManager::Instance()->GetServerInfo(iWorldID);
	if (pstServerConfig)
	{
		pstResp->set_strhostinfo(pstServerConfig->szServerHostInfo);
	}

	//生成并返回sessionkey
	//session key: uin|time|WorldID
	static char szOriginSessionKey[256];
	SAFE_SPRINTF(szOriginSessionKey, sizeof(szOriginSessionKey) - 1, "%u|%u|%d", uin, (unsigned)CTimeUtility::GetNowTime(), iWorldID);

	static char szSessionKey[256];
	int iSessionKeyLen = sizeof(szSessionKey);
	CSessionKeyUtility::GenerateSessionKey(szOriginSessionKey, strlen(szOriginSessionKey) + 1, szSessionKey, iSessionKeyLen);

	pstResp->set_strsessionkey(szSessionKey, iSessionKeyLen);

	int iRet = CRegAuthMsgProcessor::Instance()->SendRegAuthToLotus(uiSessionFd, stMsg);
	if (iRet)
	{
		TRACESVR("Failed to send authaccount response to lotus server\n");
		return;
	}

	LOGDEBUG("Send authaccount response to lotus server, result: %d\n", T_SERVER_SUCCESS);

	return;
}
