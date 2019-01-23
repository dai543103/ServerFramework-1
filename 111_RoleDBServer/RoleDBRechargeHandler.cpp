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

#include "RoleDBRechargeHandler.hpp"

using namespace ServerLib;

CRoleDBRechargeHandler::CRoleDBRechargeHandler(DBClientWrapper* pDatabase, char** ppQueryBuff)
{    
    m_pDatabase = pDatabase;
	m_ppQueryBuff = ppQueryBuff;
}

void CRoleDBRechargeHandler::OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult, TNetHead_V2* pstNetHead)
{
	m_pstRequestMsg = &stReqMsg;
	switch (m_pstRequestMsg->sthead().uimsgid())
	{
	case MSGID_WORLD_USERRECHARGE_REQUEST:
	{
		OfflineRecharge(*pstHandleResult);
	}
	break;

	case MSGID_WORLD_GETUSERINFO_REQUEST:
	{
		OnGetUserInfoRequest(*pstHandleResult);
		break;
	}

	default:
	{

	}
	break;
	}
}

int CRoleDBRechargeHandler::OfflineRecharge(SHandleResult& stHandleResult)
{
	//获取请求
	const World_UserRecharge_Request& stReq = m_pstRequestMsg->mutable_stbody()->m_stworld_userrecharge_request();
	unsigned int uiUin = stReq.uin();

	//返回消息
	GenerateMsgHead(stHandleResult.stResponseMsg, m_pstRequestMsg->sthead().uisessionfd(), MSGID_WORLD_USERRECHARGE_RESPONSE, uiUin);
	World_UserRecharge_Response* pstResp = stHandleResult.stResponseMsg.mutable_stbody()->mutable_m_stworld_userrecharge_response();
	pstResp->set_uin(uiUin);
	pstResp->set_irechargeid(stReq.irechargeid());
	pstResp->set_strorderid(stReq.strorderid());
	pstResp->set_iplatform(stReq.iplatform());

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
	int iRet = GetOfflineInfo(stReq.uin(), stOfflineInfo);
	if (iRet)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to get offline info, uin %u, ret %d\n", stReq.uin(), iRet);

		pstResp->set_iresult(iRet);
		return -2;
	}

	//增加离线充值信息
	AddOfflineRechargeInfo(stReq.irechargeid(), stReq.itime(), stReq.strorderid(), stOfflineInfo);

	//更新离线数据
	iRet = UpdateOfflineInfo(stReq.uin(), stOfflineInfo);
	if (iRet)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to update offline info, uin %u, ret %d\n", stReq.uin(), iRet);

		pstResp->set_iresult(iRet);
		return -3;
	}

	pstResp->set_iresult(T_SERVER_SUCCESS);
	return 0;
}

//拉取充值玩家信息
int CRoleDBRechargeHandler::OnGetUserInfoRequest(SHandleResult& stHandleResult)
{
	//获取请求
	const World_GetUserInfo_Request& stReq = m_pstRequestMsg->stbody().m_stworld_getuserinfo_request();
	unsigned int uiUin = stReq.uin();

	//返回消息
	GenerateMsgHead(stHandleResult.stResponseMsg, m_pstRequestMsg->sthead().uisessionfd(), MSGID_WORLD_GETUSERINFO_RESPONSE, uiUin);
	World_GetUserInfo_Response* pstResp = stHandleResult.stResponseMsg.mutable_stbody()->mutable_m_stworld_getuserinfo_response();
	pstResp->set_uin(uiUin);
	pstResp->set_iplatformid(stReq.iplatformid());

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

	//拉取玩家基础数据
	BASEDBINFO stBaseInfo;
	int iRet = GetBaseInfo(stReq.uin(), stBaseInfo);
	if (iRet)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to get base info, uin %u, ret %d\n", stReq.uin(), iRet);

		pstResp->set_iresult(iRet);
		return -2;
	}

	pstResp->set_straccount(stBaseInfo.sznickname());
	pstResp->set_strnickname(stBaseInfo.sznickname());
	pstResp->set_iresult(T_SERVER_SUCCESS);

	return 0;
}

//拉取离线数据
int CRoleDBRechargeHandler::GetOfflineInfo(unsigned uin, RESERVED1DBINFO& stOfflineInfo)
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

//增加离线充值信息
void CRoleDBRechargeHandler::AddOfflineRechargeInfo(int iRechargeID, int iTime, const std::string& strOrderID, RESERVED1DBINFO& stOfflineInfo)
{
	PAYOFFLINEDBINFO* pstPayInfo = stOfflineInfo.mutable_stpayinfo();

	RechargeRecord* pstOneRecord = pstPayInfo->add_strecords();
	pstOneRecord->set_irechargeid(iRechargeID);
	pstOneRecord->set_itime(iTime);
	pstOneRecord->set_strorderid(strOrderID);

	return;
}

//更新离线数据
int CRoleDBRechargeHandler::UpdateOfflineInfo(unsigned uin, RESERVED1DBINFO& stOfflineInfo)
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

//拉取基础数据
int CRoleDBRechargeHandler::GetBaseInfo(unsigned uin, BASEDBINFO& stBaseInfo)
{
	if (!m_pDatabase)
	{
		return -1;
	}

	//构造SQL语句
	char* pszQueryString = *m_ppQueryBuff;
	static int iQueryStringLen = MAX_QUERYBUFF_SIZE - 1;

	//拉取离线数据
	int iLength = SAFE_SPRINTF(pszQueryString, iQueryStringLen, "select base_info from %s where uin=%u\n", MYSQL_USERINFO_TABLE, uin);

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
	if (!DecodeProtoData(strOffline, stBaseInfo))
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to decode base info data, uin %u\n", uin);
		return T_ROLEDB_INVALID_RECORD;
	}

	return T_SERVER_SUCCESS;
}
