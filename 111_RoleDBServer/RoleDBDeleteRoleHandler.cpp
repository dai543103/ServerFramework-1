#include <string.h>

#include "GameProtocol.hpp"
#include "AppDef.hpp"
#include "ThreadLogManager.hpp"
#include "SeqConverter.hpp"
#include "StringUtility.hpp"
#include "RoleDBApp.hpp"
#include "ConfigManager.hpp"
#include "ErrorNumDef.hpp"

#include "RoleDBDeleteRoleHandler.hpp"

using namespace ServerLib;

CRoleDBDeleteRoleHandler::CRoleDBDeleteRoleHandler(DBClientWrapper* pDatabase, char** ppQueryBuff)
{
    m_pDatabase = pDatabase;
	m_ppQueryBuff = ppQueryBuff;
}

void CRoleDBDeleteRoleHandler::OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult, TNetHead_V2* pstNetHead)
{
    m_pstRequestMsg = &stReqMsg;
 
	//返回消息
	GenerateMsgHead(pstHandleResult->stResponseMsg, 0, MSGID_ACCOUNT_DELETEROLE_RESPONSE, m_pstRequestMsg->sthead().uin());
	DeleteRole_Account_Response* pstResp = pstHandleResult->stResponseMsg.mutable_stbody()->mutable_m_staccountdeleteroleresponse();

	//获取请求
	const DeleteRole_Account_Request& stReq = m_pstRequestMsg->stbody().m_staccountdeleterolerequest();
    unsigned int uiUin = stReq.stroleid().uin();

    DEBUG_THREAD(m_iThreadIdx, "Handling DeleteRoleRequest, uin: %u\n", uiUin);

    // 根据uin和worldid读取数据库中的nickname，存入m_szNickname中
    int iRet = QueryRoleInfo(stReq.stroleid());
    if (iRet)
    {
        // 没有找到要删除的角色, 直接返回成功
		pstResp->set_iresult(T_SERVER_SUCCESS);
        return;
    }

    // 根据RoleID删除表DBRoleInfo中的相应记录
    iRet = DeleteRole(stReq.stroleid());
    if (iRet)
    {
		pstResp->set_iresult(iRet);
        return;
    }

	pstResp->set_iresult(T_SERVER_SUCCESS);
	return;
}

int CRoleDBDeleteRoleHandler::DeleteRole(const RoleID& stRoleID)
{
    //拉取ROLEDB数据库相关配置
    const OneDBInfo* pstDBConfig = ConfigManager::Instance()->GetRoleDBConfigByIndex(m_iThreadIdx);
    if(!pstDBConfig)
    {
        TRACE_THREAD(m_iThreadIdx, "Failed to get roledb config, thread index %d\n", m_iThreadIdx);
        return T_ROLEDB_PARA_ERR;
    }

    //设置要操作的数据库相关信息
    m_pDatabase->SetMysqlDBInfo(pstDBConfig->szDBHost, pstDBConfig->szUserName, pstDBConfig->szUserPasswd, pstDBConfig->szDBName);

    char* pszQueryString = *m_ppQueryBuff;
    int iQueryBuffLen = MAX_QUERYBUFF_SIZE - 1;
    int iLength = SAFE_SPRINTF(pszQueryString, iQueryBuffLen, "delete from %s where uin = %u and seq=%u\n", MYSQL_USERINFO_TABLE, stRoleID.uin(), stRoleID.uiseq());

    int iRet = m_pDatabase->ExecuteRealQuery(pszQueryString, iLength, false);
    if(iRet)
    {
        TRACE_THREAD(m_iThreadIdx, "Failed to execute delete sql query, uin %u, ret %d\n", stRoleID.uin(), iRet);
        return iRet;
    }

    DEBUG_THREAD(m_iThreadIdx, "The number of affected rows is %d\n", m_pDatabase->GetAffectedRows());
    return 0;
}

int CRoleDBDeleteRoleHandler::QueryRoleInfo(const RoleID& stRoleID)
{
    //从XML文件中读取数据库配置信息
	const OneDBInfo* pstDBConfig = ConfigManager::Instance()->GetRoleDBConfigByIndex(m_iThreadIdx);
    if(!pstDBConfig)
    {
        TRACE_THREAD(m_iThreadIdx, "Failed to query role info, invalid DB cfg, thread idx %d\n", m_iThreadIdx);
        return T_ROLEDB_PARA_ERR;
    }

    //设置要操作的数据库相关信息
    m_pDatabase->SetMysqlDBInfo(pstDBConfig->szDBHost, pstDBConfig->szUserName, pstDBConfig->szUserPasswd, pstDBConfig->szDBName);

    //初始化查询的SQL语句
    char* pszQueryString = *m_ppQueryBuff;
    int iQueryBuffLen = MAX_QUERYBUFF_SIZE - 1;
    int iLength = SAFE_SPRINTF(pszQueryString, iQueryBuffLen, "select uin from %s where uin=%u and seq=%u\n", MYSQL_USERINFO_TABLE, stRoleID.uin(), stRoleID.uiseq());

    int iRet = m_pDatabase->ExecuteRealQuery(pszQueryString, iLength, true);
    if(iRet)
    {
        TRACE_THREAD(m_iThreadIdx, "Fail to execute sql query, uin %u, ret %d\n", stRoleID.uin(), iRet);
        return T_ROLEDB_SQL_EXECUTE_FAILED;
    }

    //判断是否已经存在要删除的记录
    int iNum = m_pDatabase->GetNumberRows();
    if(iNum != 1)
    {
        TRACE_THREAD(m_iThreadIdx, "Wrong select result, uin %u\n", stRoleID.uin());
        return T_ROLEDB_INVALID_RECORD;
    }

    return 0;
}
