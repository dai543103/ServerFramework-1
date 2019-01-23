#include <stdlib.h>
#include <string.h>

#include "GameProtocol.hpp"
#include "AppDef.hpp"
#include "ThreadLogManager.hpp"
#include "UnixTime.hpp"
#include "NowTime.hpp"
#include "StringUtility.hpp"
#include "RoleDBApp.hpp"
#include "ConfigManager.hpp"
#include "ErrorNumDef.hpp"

#include "RoleDBFetchRoleHandler.hpp"

using namespace ServerLib;

CRoleDBFetchRoleHandler::CRoleDBFetchRoleHandler(DBClientWrapper* pDatabase, char** ppQueryBuff)
{
	m_pDatabase = pDatabase;
	m_ppQueryBuff = ppQueryBuff;
}

void CRoleDBFetchRoleHandler::OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult, TNetHead_V2* pstNetHead)
{
	m_pstRequestMsg = &stReqMsg;
	switch (m_pstRequestMsg->sthead().uimsgid())
	{
	case MSGID_WORLD_FETCHROLE_REQUEST:
	{
		OnFetchRoleRequest(*pstHandleResult);
		break;
	}

	default:
	{
		break;
	}
	}
}

void CRoleDBFetchRoleHandler::OnFetchRoleRequest(SHandleResult& stHandleResult)
{
	//返回消息
	GenerateMsgHead(stHandleResult.stResponseMsg, 0, MSGID_WORLD_FETCHROLE_RESPONSE, m_pstRequestMsg->sthead().uin());
	World_FetchRole_Response* pstResp = stHandleResult.stResponseMsg.mutable_stbody()->mutable_m_stworld_fetchrole_response();

	//获取请求
	const World_FetchRole_Request& stReq = m_pstRequestMsg->stbody().m_stworld_fetchrole_request();

	unsigned int uiUin = stReq.stroleid().uin();
	TRACE_THREAD(m_iThreadIdx, "Handling FetchRoleRequest, uin: %u\n", uiUin);

	// 响应消息体
	pstResp->mutable_stroleid()->CopyFrom(stReq.stroleid());
	pstResp->set_bislogin(stReq.bislogin());
	pstResp->mutable_stkicker()->CopyFrom(stReq.stkicker());

	// 根据uin查询玩家角色信息
	int iRet = QueryRole(stReq.stroleid(), *pstResp);
	if (iRet != 0)
	{
		TRACE_THREAD(m_iThreadIdx, "Query Role info failed, uin:%u iRes:%d\n", uiUin, iRet);
		
		pstResp->set_iresult(iRet);
		return;
	}

	//todo jasonxiong 封帐号功能暂时不支持，等后续开发需要时再增加
	/*
	time_t tNow = time(NULL);
	if (tNow < pstFetchRoleResponse->m_stDBRoleInfo.fForbidTime)
	{
	TRACE_THREAD(m_iThreadIdx, "uin:%u is forbid login to time:%d\n", pstRoleID->m_uiUin, pstFetchRoleResponse->m_stDBRoleInfo.fForbidTime);
	return;
	}
	*/

	// 填充成功回复
	pstResp->set_iresult(T_SERVER_SUCCESS);
	return;
}

int CRoleDBFetchRoleHandler::QueryRole(const RoleID& stRoleID, World_FetchRole_Response& stResp)
{
	//读取ROLEDB数据库的配置
	const OneDBInfo* pstDBConfig = ConfigManager::Instance()->GetRoleDBConfigByIndex(m_iThreadIdx);
	if (!pstDBConfig)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to get roledb config, thread index %d\n", m_iThreadIdx);
		return T_ROLEDB_PARA_ERR;
	}

	//设置要操作的数据库相关信息
	m_pDatabase->SetMysqlDBInfo(pstDBConfig->szDBHost, pstDBConfig->szUserName, pstDBConfig->szUserPasswd, pstDBConfig->szDBName);

	//构造SQL语句
	char* pszQueryString = *m_ppQueryBuff;
	int iLength = SAFE_SPRINTF(pszQueryString, MAX_QUERYBUFF_SIZE - 1,
		"select uin,seq,base_info,quest_info,item_info,friend_info,mail_info,"
		"reserved1,reserved2 from %s where uin=%u and seq=%u ",
		MYSQL_USERINFO_TABLE, stRoleID.uin(), stRoleID.uiseq());

	//执行
	int iRet = m_pDatabase->ExecuteRealQuery(pszQueryString, iLength, true);
	if (iRet)
	{
		TRACE_THREAD(m_iThreadIdx, "Fail to execute select sql query, uin %u\n", stRoleID.uin());
		return iRet;
	}

	//分析结果
	int iRowNum = m_pDatabase->GetNumberRows();
	if (iRowNum != 1)
	{
		//查询的角色不存在
		stResp.set_bexist(false);
		return 0;
	}

	MYSQL_ROW pstResult = NULL;
	unsigned long* pLengths = NULL;
	unsigned int uFields = 0;

	iRet = m_pDatabase->FetchOneRow(pstResult, pLengths, uFields);
	if (iRet)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to fetch rows, uin %u, ret %d\n", stRoleID.uin(), iRet);
		return iRet;
	}

	//判断uFields是否相符
	if (uFields != MYSQL_USERINFO_FIELDS)
	{
		TRACE_THREAD(m_iThreadIdx, "Wrong result, real fields %u, needed %u\n", uFields, MYSQL_USERINFO_FIELDS);
		return T_ROLEDB_INVALID_RECORD;
	}

	stResp.set_bexist(true);

	GameUserInfo* pstUserInfo = stResp.mutable_stuserinfo();
	pstUserInfo->set_uin(stRoleID.uin());
	pstUserInfo->set_uiseq(stRoleID.uiseq());

	//从结果中解析需要的字段

	//字段1是uin, 字段2是seq, 都跳过

	//字段3是base_info
	pstUserInfo->set_strbaseinfo(pstResult[2], pLengths[2]);

	//字段4是quest_info
	pstUserInfo->set_strquestinfo(pstResult[3], pLengths[3]);

	//字段5是item_info
	pstUserInfo->set_striteminfo(pstResult[4], pLengths[4]);

	//字段6是friend_info
	pstUserInfo->set_strfriendinfo(pstResult[5], pLengths[5]);

	//字段7是mail_info
	pstUserInfo->set_strmailinfo(pstResult[6], pLengths[6]);

	//字段8是保留字段reserved1
	pstUserInfo->set_strreserved1(pstResult[7], pLengths[7]);

	//字段9是保留字段reserved2
	pstUserInfo->set_strreserved2(pstResult[8], pLengths[8]);

	return 0;
}
