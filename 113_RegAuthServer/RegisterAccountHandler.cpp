#include <assert.h>
#include <arpa/inet.h>

#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "SessionObj.hpp"
#include "StringUtility.hpp"
#include "HashUtility.hpp"
#include "TimeUtility.hpp"
#include "ConfigManager.hpp"
#include "PasswordEncryptionUtility.hpp"
#include "ErrorNumDef.hpp"
#include "SessionManager.hpp"
#include "RegAuthMsgProcessor.hpp"
#include "RegAuthDBAddHandler.hpp"

#include "RegisterAccountHandler.hpp"

using namespace ServerLib;

static GameProtocolMsg stMsg;

CRegisterAccountHandler::CRegisterAccountHandler()
{

}

int CRegisterAccountHandler::CheckParam()
{
	return 0;
}

void CRegisterAccountHandler::OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult, TNetHead_V2* pstNetHead)
{
	ASSERT_AND_LOG_RTN_VOID(pstNetHead);

	//暂不支持该协议，注释掉
	return;

	m_pstNetHead = pstNetHead;
	m_pstRequestMsg = &stReqMsg;

	m_uiSessionFD = m_pstRequestMsg->sthead().uisessionfd();

	switch (m_pstRequestMsg->sthead().uimsgid())
	{
	case MSGID_REGAUTH_REGACCOUNT_REQUEST:
	{
		//注册平台帐号的请求
		OnRequestRegAccount();
		return;
	}
	break;

	default:
	{

	}
	break;
	}

	return;
}

int CRegisterAccountHandler::OnRequestRegAccount()
{
	//返回消息
	GenerateMsgHead(stMsg, m_uiSessionFD, MSGID_REGAUTH_REGACCOUNT_RESPONSE, 0);
	RegAuth_RegAccount_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stregauth_regaccount_response();

	// 检查请求消息中是否存在非法字段
	if (CheckParam() != 0)
	{
		TRACESVR("Invalid parameter found in the request\n");

		pstResp->set_iresult(T_REGAUTH_PARA_ERROR);
		CRegAuthMsgProcessor::Instance()->SendRegAuthToLotus(m_uiSessionFD, stMsg);
		return -1;
	}

	//获取请求
	const RegAuth_RegAccount_Request& stReq = m_pstRequestMsg->stbody().m_stregauth_regaccount_request();
	
	TRACESVR("Handling RegAccountRequest from lotus server, account: %s, type: %d\n",
		stReq.stinfo().straccount().c_str(), stReq.stinfo().itype());

	// 检查session是否已经存在，session个数是否到达上限
	int iRet = CSessionManager::Instance()->CheckSession(*m_pstNetHead);
	if (iRet)
	{
		pstResp->set_iresult(iRet);
		CRegAuthMsgProcessor::Instance()->SendRegAuthToLotus(m_uiSessionFD, stMsg);
		return -4;
	}
	
	// 缓存NetHead，即session
	CSessionObj* pSessionObj = CSessionManager::Instance()->CreateSession(*m_pstNetHead);
	ASSERT_AND_LOG_RTN_INT(pSessionObj);
	pSessionObj->SetCreatedTime(CTimeUtility::GetNowTime());

	m_uiSessionFD = pSessionObj->GetSessionFD();
	m_unValue = pSessionObj->GetValue();
	TRACESVR("sockfd: %u, value: %d\n", m_uiSessionFD, m_unValue);

	unsigned int uiSessionFd = m_uiSessionFD;

	//直接转发注册请求给RegAuthDB
	AddAccount(uiSessionFd, stReq.stinfo());

	return T_SERVER_SUCCESS;
}

//新增加帐号
void CRegisterAccountHandler::AddAccount(unsigned uiSessionFd, const AccountInfo& stInfo)
{
	unsigned uiHashNum = CHashUtility::BKDRHash(stInfo.straccount().c_str(), stInfo.straccount().size());

	//生成消息头
	GenerateMsgHead(stMsg, uiSessionFd, MSGID_REGAUTHDB_ADDACCOUNT_REQUEST, uiHashNum);

	//RegAuthDB插入新帐号的请求
	RegAuthDB_AddAccount_Request* pstReq = stMsg.mutable_stbody()->mutable_m_stregauthdb_addaccount_request();
	pstReq->mutable_stinfo()->CopyFrom(stInfo);

	pstReq->set_iworldid(1);

	//加密密码
	char szEncryptPasswd[256] = { 0 };
	int iEncryptBuffLen = sizeof(szEncryptPasswd);

	int iRet = CPasswordEncryptionUtility::DoPasswordEncryption(stInfo.strpassword().c_str(), stInfo.strpassword().size(), szEncryptPasswd, iEncryptBuffLen);
	if (iRet)
	{
		TRACESVR("Failed to encrypt account password, account: %s, password: %s\n", stInfo.straccount().c_str(), stInfo.strpassword().c_str());
		return;
	}

	//设置密码为加密后的密码
	pstReq->mutable_stinfo()->set_strpassword(szEncryptPasswd, strlen(szEncryptPasswd));

	//转发消息给RegAuthDBServer
	iRet = CRegAuthMsgProcessor::Instance()->SendRegAuthToDB(stMsg);
	{
		TRACESVR("Failed to send add account request to Account DB server\n");
		return;
	}

	LOGDEBUG("Send add account request to Account DB server\n");

	return;
}
