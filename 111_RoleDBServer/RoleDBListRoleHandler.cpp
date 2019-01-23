#include <string.h>

#include "GameProtocol.hpp"
#include "ThreadLogManager.hpp"
#include "SeqConverter.hpp"
#include "StringUtility.hpp"
#include "RoleDBApp.hpp"
#include "AppDef.hpp"
#include "ConfigManager.hpp"
#include "ErrorNumDef.hpp"

#include "RoleDBListRoleHandler.hpp"

CRoleDBListRoleHandler::CRoleDBListRoleHandler(DBClientWrapper* pDatabase, char** ppQueryBuff)
{    
    m_pDatabase = pDatabase;
	m_ppQueryBuff = ppQueryBuff;
}

void CRoleDBListRoleHandler::OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult, TNetHead_V2* pstNetHead)
{
	m_pstRequestMsg = &stReqMsg;
	switch (m_pstRequestMsg->sthead().uimsgid())
	{
	case MSGID_ACCOUNT_LISTROLE_REQUEST:
	{
		AccountListRole(*pstHandleResult);
	}
	break;

	default:
	{

	}
	break;
	}
}

int CRoleDBListRoleHandler::AccountListRole(SHandleResult& stHandleResult)
{
	//获取请求
	const Account_ListRole_Request& stReq = m_pstRequestMsg->stbody().m_staccountlistrolerequest();
	unsigned int uiUin = stReq.uin();
	unsigned short usWorldID = stReq.world();
    short usNewWorldID = 0;

	//返回消息
	GenerateMsgHead(stHandleResult.stResponseMsg, m_pstRequestMsg->sthead().uisessionfd(), MSGID_ACCOUNT_LISTROLE_RESPONSE, uiUin);
	Account_ListRole_Response* pstResp = stHandleResult.stResponseMsg.mutable_stbody()->mutable_m_staccountlistroleresponse();

    // 根据uin在DBRoleInfo表中检索出seq，解析其中的world信息，返回指定world上的role列表
	int iErrnoNum = T_ROLEDB_SQL_EXECUTE_FAILED;
	int iRet = QueryAndParseRole(uiUin, usWorldID, usNewWorldID, *pstResp, iErrnoNum);

	TRACE_THREAD(m_iThreadIdx, "Info of ListRoleResponse: result: %u, uin: %u, world: %u:%u, role exist: %d\n",
		iRet, uiUin, usWorldID, usNewWorldID, pstResp->bexist());

    if (iRet != 0)
    {
		pstResp->set_iresult(iErrnoNum);
        return -1;
    }

	pstResp->set_iresult(T_SERVER_SUCCESS);
	return 0;
}

int CRoleDBListRoleHandler::QueryAndParseRole(const unsigned int uiUin, short nWorldID, short nNewWorldID, 
	Account_ListRole_Response& stResp, int& iErrnoNum)
{
	//查询DB中是否已经存在玩家帐号

	//读取ROLEDB相关的配置
	const OneDBInfo* pstDBConfig = ConfigManager::Instance()->GetRoleDBConfigByIndex(m_iThreadIdx);
	if(!pstDBConfig)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to get roledb config, invalid index %d\n", m_iThreadIdx);
		return -2;
	}

	//设置要访问的数据库信息
	m_pDatabase->SetMysqlDBInfo(pstDBConfig->szDBHost, pstDBConfig->szUserName, pstDBConfig->szUserPasswd, pstDBConfig->szDBName);

	//构造SQL语句
	char* pszQueryString = *m_ppQueryBuff;
	static int iQueryStringLen = MAX_QUERYBUFF_SIZE-1;
	int iLength = 0;

	//1服用作开发服，2服用作预发布服,这两个服可能会从外网导入数据
	if(nNewWorldID==1 || nNewWorldID==2)
	{
		//开发服和预发布服，会从外网导数据进来
		iLength = SAFE_SPRINTF(pszQueryString, iQueryStringLen, "select uin,seq from %s where uin=%u\n", MYSQL_USERINFO_TABLE, uiUin);
	}
	else
	{
		//外网正式服，考虑合服的情况
		iLength = SAFE_SPRINTF(pszQueryString, iQueryStringLen, 
							   "select uin,seq from %s where uin = %u and floor(seq%%10000/10)=%d\n", 
							   MYSQL_USERINFO_TABLE, uiUin, nWorldID);
	}

	//执行
	int iRet = m_pDatabase->ExecuteRealQuery(pszQueryString, iLength, true);
	if(iRet)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to execute select sql query, uin %u\n", uiUin);
		return iRet;
	}

	//分析返回结果
	int iRowNum = m_pDatabase->GetNumberRows();
	if(iRowNum == 0)
	{
		//不存在玩家记录，直接返回成功
		stResp.set_bexist(false);
		return T_SERVER_SUCCESS;
	}
	
	TRACE_THREAD(m_iThreadIdx, "List Role Num %d, uin %u, world id %d\n", iRowNum, uiUin, nWorldID);

	MYSQL_ROW pstResult = NULL;
	unsigned long* pLengths = NULL;
	unsigned int uFields = 0;

	iRet = m_pDatabase->FetchOneRow(pstResult, pLengths, uFields);
	if (iRet)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to fetch rows, uin %u, ret %d\n", uiUin, iRet);
		return iRet;
	}

	stResp.set_bexist(true);
	stResp.set_world(nWorldID);
	stResp.mutable_stroleid()->set_uin(uiUin);
	stResp.mutable_stroleid()->set_uiseq(atoi(pstResult[1]));

    return T_SERVER_SUCCESS;
}

