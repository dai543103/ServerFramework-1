#include <string.h>

#include "GameProtocol.hpp"
#include "ThreadLogManager.hpp"
#include "SeqConverter.hpp"
#include "StringUtility.hpp"
#include "RoleDBApp.hpp"
#include "AppDef.hpp"
#include "ConfigManager.hpp"
#include "ErrorNumDef.hpp"
#include "ProtoDataUtility.hpp"

#include "RoleDBMailHandler.hpp"

using namespace ServerLib;

CRoleDBMailHandler::CRoleDBMailHandler(DBClientWrapper* pDatabase, char** ppQueryBuff)
{
	m_pDatabase = pDatabase;
	m_ppQueryBuff = ppQueryBuff;
}

void CRoleDBMailHandler::OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult, TNetHead_V2* pstNetHead)
{
	m_pstRequestMsg = &stReqMsg;
	switch (m_pstRequestMsg->sthead().uimsgid())
	{
	case MSGID_WORLD_SENDMAIL_REQUEST:
	{
		OfflineMail(*pstHandleResult);
	}
	break;

	default:
	{

	}
	break;
	}
}

int CRoleDBMailHandler::OfflineMail(SHandleResult& stHandleResult)
{
	//获取请求
	const World_SendMail_Request& stReq = m_pstRequestMsg->mutable_stbody()->m_stworld_sendmail_request();

	//返回消息
	GenerateMsgHead(stHandleResult.stResponseMsg, 0, MSGID_WORLD_SENDMAIL_RESPONSE, stReq.uifromuin());
	World_SendMail_Response* pstResp = stHandleResult.stResponseMsg.mutable_stbody()->mutable_m_stworld_sendmail_response();
	pstResp->set_uifromuin(stReq.uifromuin());
	pstResp->set_uitouin(stReq.uitouin());
	pstResp->set_itrytimes(stReq.itrytimes()+1);
	pstResp->mutable_stmailinfo()->CopyFrom(stReq.stmailinfo());

	//读取ROLEDB相关的配置
	const OneDBInfo* pstDBConfig = ConfigManager::Instance()->GetRoleDBConfigByIndex(m_iThreadIdx);
	if (!pstDBConfig)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to get roledb config, invalid index %d\n", m_iThreadIdx);

		pstResp->set_iresult(T_ROLEDB_PARA_ERR);
		return T_ROLEDB_PARA_ERR;
	}

	//设置要访问的数据库信息
	m_pDatabase->SetMysqlDBInfo(pstDBConfig->szDBHost, pstDBConfig->szUserName, pstDBConfig->szUserPasswd, pstDBConfig->szDBName);

	//拉取离线数据
	RESERVED1DBINFO stOfflineInfo;
	int iRet = GetOfflineInfo(stReq.uitouin(), stOfflineInfo);
	if (iRet)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to get offline info, uin %u, ret %d\n", stReq.uitouin(), iRet);

		pstResp->set_iresult(iRet);
		return -2;
	}

	//增加离线邮件信息
	AddOfflineMailInfo(stReq.stmailinfo(), stOfflineInfo);

	//更新离线数据
	iRet = UpdateOfflineInfo(stReq.uitouin(), stOfflineInfo);
	if (iRet)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to update offline info, uin %u, ret %d\n", stReq.uitouin(), iRet);

		pstResp->set_iresult(iRet);
		return -3;
	}

	pstResp->set_iresult(T_SERVER_SUCCESS);
	return 0;
}

//拉取离线数据
int CRoleDBMailHandler::GetOfflineInfo(unsigned uin, RESERVED1DBINFO& stOfflineInfo)
{
	if (!m_pDatabase)
	{
		return -1;
	}

	//构造SQL语句
	char* pszQueryString = *m_ppQueryBuff;
	static int iQueryStringLen = MAX_QUERYBUFF_SIZE - 1;

	//拉取离线数据
	int iLength = SAFE_SPRINTF(pszQueryString, iQueryStringLen, "select reserved1 from %s where uin=%u\n", MYSQL_USERINFO_TABLE, uin);

	//执行
	int iRet = m_pDatabase->ExecuteRealQuery(pszQueryString, iLength, true);
	if (iRet)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to execute select sql query, uin %u\n", uin);
		return iRet;
	}

	//分析返回结果
	int iRowNum = m_pDatabase->GetNumberRows();
	if (iRowNum == 0)
	{
		//账号不存在
		return T_ROLEDB_INVALID_RECORD;
	}

	MYSQL_ROW pstResult = NULL;
	unsigned long* pLengths = NULL;
	unsigned int uFields = 0;

	iRet = m_pDatabase->FetchOneRow(pstResult, pLengths, uFields);
	if (iRet)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to fetch rows, uin %u, ret %d\n", uin, iRet);
		return iRet;
	}

	std::string strOffline(pstResult[0], pLengths[0]);
	if (!DecodeProtoData(strOffline, stOfflineInfo))
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to decode reserved1 data, uin %u\n", uin);
		return T_ROLEDB_INVALID_RECORD;
	}

	return T_SERVER_SUCCESS;
}

//增加离线邮件信息
void CRoleDBMailHandler::AddOfflineMailInfo(const OneMailInfo& stMailInfo, RESERVED1DBINFO& stOfflineInfo)
{
	MAILOFFLINEDBINFO* pstMailInfo = stOfflineInfo.mutable_stmailinfo();
	pstMailInfo->add_stmails()->CopyFrom(stMailInfo);;

	return;
}

//更新离线数据
int CRoleDBMailHandler::UpdateOfflineInfo(unsigned uin, RESERVED1DBINFO& stOfflineInfo)
{
	if (!m_pDatabase)
	{
		return -1;
	}

	//构造SQL语句
	char* pszQueryString = *m_ppQueryBuff;
	static int iQueryStringLen = MAX_QUERYBUFF_SIZE - 1;

	//拉取离线数据
	SAFE_SPRINTF(pszQueryString, iQueryStringLen, "update %s set ", MYSQL_USERINFO_TABLE);
	char* pEnd = pszQueryString + strlen(pszQueryString);

	//玩家离线数据
	std::string strOffline;
	if (!EncodeProtoData(stOfflineInfo, strOffline))
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to encode proto data, uin %u\n", uin);
		return T_ROLEDB_INVALID_RECORD;
	}

	if (strOffline.size() == 0)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to update offline info, invalid length, uin %u\n", uin);
		return T_ROLEDB_PARA_ERR;
	}

	pEnd += SAFE_SPRINTF(pEnd, iQueryStringLen, "reserved1=");

	//玩家离线数据
	*pEnd++ = '\'';
	pEnd += mysql_real_escape_string(&m_pDatabase->GetCurMysqlConn(), pEnd, strOffline.c_str(), strOffline.size());
	*pEnd++ = '\'';

	pEnd += SAFE_SPRINTF(pEnd, iQueryStringLen, " where uin = %u\n", uin);

	int iLength = pEnd - pszQueryString;

	//执行
	int iRet = m_pDatabase->ExecuteRealQuery(pszQueryString, iLength, false);
	if (iRet)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to execute select sql query, uin %u\n", uin);
		return iRet;
	}

	DEBUG_THREAD(m_iThreadIdx, "Success to update offline info, uin %u\n", uin);

	return T_SERVER_SUCCESS;
}
