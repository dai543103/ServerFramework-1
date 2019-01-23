#include <assert.h>
#include <arpa/inet.h>

#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "StringUtility.hpp"
#include "HashUtility.hpp"
#include "TimeUtility.hpp"
#include "ConfigManager.hpp"
#include "PasswordEncryptionUtility.hpp"
#include "SessionKeyUtility.hpp"
#include "ErrorNumDef.hpp"
#include "SessionManager.hpp"
#include "RegAuthMsgProcessor.hpp"
#include "RegAuthDBFetchHandler.hpp"
#include "RegAuthOssLog.hpp"

#include "AuthAccountHandler.hpp"

using namespace ServerLib;

static GameProtocolMsg stMsg;

CAuthAccountHandler::CAuthAccountHandler()
{

}

//第二个参数传空
void CAuthAccountHandler::OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult, TNetHead_V2* pstNetHead)
{
	m_pstNetHead = pstNetHead;
	m_pstRequestMsg = &stReqMsg;

	switch (m_pstRequestMsg->sthead().uimsgid())
	{
	case MSGID_REGAUTH_AUTHACCOUNT_REQUEST:
	{
		//认证帐号的请求
		OnRequestAuthAccount();
	}
	break;

	case MSGID_REGAUTH_PLATFORMAUTH_RESPONSE:
	{
		//平台认证的返回
		OnResponsePlatformAuth();
	}
	break;

	case MSGID_REGAUTH_AUTHACCOUNT_RESPONSE:
	{
		//RegAuthDB认证的返回
		OnResponseRegDBAuth();
	}
	break;

	default:
	{

	}
	break;
	}

	return;
}

int CAuthAccountHandler::OnRequestAuthAccount()
{
	ASSERT_AND_LOG_RTN_INT(m_pstNetHead);

	//获取请求
	const RegAuth_AuthAccount_Request& stReq = m_pstRequestMsg->stbody().m_stregauth_authaccount_request();
	const std::string& strAccount = stReq.stinfo().straccount();
	int iType = stReq.stinfo().itype();

	//返回消息
	GenerateMsgHead(stMsg, 0, MSGID_REGAUTH_AUTHACCOUNT_RESPONSE, 0);
	RegAuth_AuthAccount_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stregauth_authaccount_response();

	//参数检查
	if (strAccount.size() == 0 || iType <= LOGIN_PLATFORM_INVALID || iType >= LOGIN_PLATFORM_MAX)
	{
		LOGERROR("Failed to auth account %s, type %d, invalid param.\n", strAccount.c_str(), iType);
		CRegAuthOssLog::PlayerRegAuth(stReq.stinfo(), 0, 0, 0);

		pstResp->set_iresult(T_REGAUTH_PARA_ERROR);
		CRegAuthMsgProcessor::Instance()->SendRegAuthToLotus(m_pstNetHead, stMsg);
		return -1;
	}

	// 检查session是否已经存在，session个数是否到达上限
	int iRet = CSessionManager::Instance()->CheckSession(*m_pstNetHead);
	if (iRet)
	{
		LOGERROR("Failed to create auth session, ret %d, account %s, type %d\n", iRet, strAccount.c_str(), iType);
		CRegAuthOssLog::PlayerRegAuth(stReq.stinfo(), 0, 0, 0);

		pstResp->set_iresult(iRet);
		CRegAuthMsgProcessor::Instance()->SendRegAuthToLotus(m_pstNetHead, stMsg);
		return -4;
	}

	//缓存NetHead，即session
	CSessionObj* pSessionObj = CSessionManager::Instance()->CreateSession(*m_pstNetHead);
	ASSERT_AND_LOG_RTN_INT(pSessionObj);

	pSessionObj->SetCreatedTime(CTimeUtility::GetNowTime());

	m_uiSessionFD = pSessionObj->GetSessionFD();
	m_unValue = pSessionObj->GetValue();
	LOGDEBUG("sockfd: %u, value: %d\n", m_uiSessionFD, m_unValue);

	char szTime[64] = {0};
	pSessionObj->GetCreatedTime(szTime);
	LOGDEBUG("session cache created time: %s\n", szTime);

	//进行平台认证
	iRet = PlatformAuthAccount(stReq.stinfo());
	if (iRet)
	{
		LOGERROR("Failed to auth platfrom, ret %d, account %s, type %d\n", iRet, strAccount.c_str(), iType);
		CRegAuthOssLog::PlayerRegAuth(stReq.stinfo(), 0, 0, 0);

		pstResp->set_iresult(iRet);
		CRegAuthMsgProcessor::Instance()->SendRegAuthToLotus(m_pstNetHead, stMsg);
		return -5;
	}

	return T_SERVER_SUCCESS;
}

//平台认证返回
int CAuthAccountHandler::OnResponsePlatformAuth()
{
	m_uiSessionFD = m_pstRequestMsg->sthead().uisessionfd();

	//获取请求
	const RegAuth_PlatformAuth_Response& stResp = m_pstRequestMsg->stbody().m_stregauth_platformauth_response();
	const std::string& strAccount = stResp.stinfo().straccount();
	int iType = stResp.stinfo().itype();

	//返回消息
	GenerateMsgHead(stMsg, 0, MSGID_REGAUTH_AUTHACCOUNT_RESPONSE, 0);
	RegAuth_AuthAccount_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stregauth_authaccount_response();

	if (stResp.iresult() != 0)
	{
		//平台认证失败
		LOGERROR("Failed to platform auth, ret %d, account %s, type %d\n", stResp.iresult(), strAccount.c_str(), iType);
		CRegAuthOssLog::PlayerRegAuth(stResp.stinfo(), 0, 0, 0);

		pstResp->set_iresult(stResp.iresult());
		CRegAuthMsgProcessor::Instance()->SendRegAuthToLotus(m_uiSessionFD, stMsg);
		return -1;
	}

	//发送认证请求到RegAuthDB
	int iRet = SendAuthToRegAuthDB(stResp.stinfo());
	if (iRet)
	{
		LOGERROR("Failed to send auth to db, ret %d, account %s, type %d\n", iRet, strAccount.c_str(), iType);
		CRegAuthOssLog::PlayerRegAuth(stResp.stinfo(), 0, 0, 0);

		pstResp->set_iresult(iRet);
		CRegAuthMsgProcessor::Instance()->SendRegAuthToLotus(m_uiSessionFD, stMsg);
		return -2;
	}

	return T_SERVER_SUCCESS;
}

//RegAuthDB认证返回
int CAuthAccountHandler::OnResponseRegDBAuth()
{
	m_uiSessionFD = m_pstRequestMsg->sthead().uisessionfd();

	//获取请求
	RegAuth_AuthAccount_Response* pstResp = m_pstRequestMsg->mutable_stbody()->mutable_m_stregauth_authaccount_response();
	if (pstResp->iresult() != 0)
	{
		//认证失败,直接转发给客户端
		CRegAuthOssLog::PlayerRegAuth(pstResp->stinfo(), 0, pstResp->bisregister(), 0);

		CRegAuthMsgProcessor::Instance()->SendRegAuthToLotus(m_uiSessionFD, *m_pstRequestMsg);
		return -1;
	}

	//注册认证成功
	CRegAuthOssLog::PlayerRegAuth(pstResp->stinfo(), pstResp->uin(), pstResp->bisregister(), 1);

	//实名状态
	int iRealNameStat = pstResp->stinfo().irealnamestat();
	const std::string strPicID = pstResp->stinfo().strpicid();

	//填充其他数据
	pstResp->mutable_stinfo()->Clear();

	//获取所在服务器组的配置
	const ServerInfoConfig* pstServerConfig = CConfigManager::Instance()->GetServerInfo(pstResp->iworldid());
	if (pstServerConfig)
	{
		pstResp->set_strhostinfo(pstServerConfig->szServerHostInfo);
	}

	//生成session key
	//session key: uin|time|WorldID|realname_stat|pic_url
	static char szOriginSessionKey[256] = {0};
	SAFE_SPRINTF(szOriginSessionKey, sizeof(szOriginSessionKey) - 1, "%u|%u|%d|%d|%s", pstResp->uin(), (unsigned)CTimeUtility::GetNowTime(), pstResp->iworldid(), iRealNameStat, strPicID.c_str());

	static char szSessionKey[256];
	int iSessionKeyLen = sizeof(szSessionKey);
	CSessionKeyUtility::GenerateSessionKey(szOriginSessionKey, strlen(szOriginSessionKey) + 1, szSessionKey, iSessionKeyLen);

	pstResp->set_strsessionkey(szSessionKey, iSessionKeyLen);

	//发送给客户端
	int iRet = CRegAuthMsgProcessor::Instance()->SendRegAuthToLotus(m_uiSessionFD, *m_pstRequestMsg);
	if (iRet)
	{
		TRACESVR("Failed to send auth account response to lotus server\n");
		return -2;
	}

	return T_SERVER_SUCCESS;
}

//发送平台认证
int CAuthAccountHandler::PlatformAuthAccount(const AccountInfo& stInfo)
{
	//请求消息
	GenerateMsgHead(stMsg, m_uiSessionFD, MSGID_REGAUTH_PLATFORMAUTH_REQUEST, 0);
	RegAuth_PlatformAuth_Request* pstReq = stMsg.mutable_stbody()->mutable_m_stregauth_platformauth_request();
	pstReq->mutable_stinfo()->CopyFrom(stInfo);

	//发送平台认证线程
	return CRegAuthMsgProcessor::Instance()->SendRegAuthToPlatform(stMsg);
}

//发送认证请求到RegAuthDB
int CAuthAccountHandler::SendAuthToRegAuthDB(const AccountInfo& stInfo)
{
	unsigned uiHashNum = CHashUtility::BKDRHash(stInfo.straccount().c_str(), stInfo.straccount().size());

	GenerateMsgHead(stMsg, m_uiSessionFD, MSGID_REGAUTH_AUTHACCOUNT_REQUEST, uiHashNum);
	RegAuth_AuthAccount_Request* pstReq = stMsg.mutable_stbody()->mutable_m_stregauth_authaccount_request();
	pstReq->mutable_stinfo()->CopyFrom(stInfo);

	return CRegAuthMsgProcessor::Instance()->SendRegAuthToDB(stMsg);
}
