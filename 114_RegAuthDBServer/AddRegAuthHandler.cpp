#include <stdlib.h>
#include <string.h>

#include "GameProtocol.hpp"
#include "AppDef.hpp"
#include "ErrorNumDef.hpp"
#include "ThreadLogManager.hpp"
#include "UnixTime.hpp"
#include "NowTime.hpp"
#include "StringUtility.hpp"
#include "ConfigManager.hpp"

#include "AddRegAuthHandler.hpp"

using namespace ServerLib;

CAddRegAuthHandler::CAddRegAuthHandler(DBClientWrapper* pDatabase, char** ppQueryBuff)
{
    m_pDatabase = pDatabase;
	m_ppQueryBuff = ppQueryBuff;
}

void CAddRegAuthHandler::OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult, TNetHead_V2* pstNetHead)
{
	// 增加新帐号记录的请求
	m_pstRequestMsg = &stReqMsg;
	switch (m_pstRequestMsg->sthead().uimsgid())
	{
	case MSGID_REGAUTHDB_ADDACCOUNT_REQUEST:
	{
		OnAddRegAuthRequest(*pstHandleResult);
		break;
	}

	default:
	{
		break;
	}
	}

	return;
}

//进行必要的参数检查
int CAddRegAuthHandler::CheckParams(const RegAuthDB_AddAccount_Request& stReq)
{
    //检查请求的参数
    if(stReq.stinfo().straccount().size() == 0)
    {
        TRACE_THREAD(m_iThreadIdx, "Failed to add new RegAuth, invalid param!\n");
        return T_REGAUTHDB_PARA_ERROR;
    }

    return T_SERVER_SUCCESS;
}

void CAddRegAuthHandler::OnAddRegAuthRequest(SHandleResult& stHandleResult)
{
	//返回消息
	GenerateMsgHead(stHandleResult.stResponseMsg, m_pstRequestMsg->sthead().uisessionfd(), MSGID_REGAUTHDB_ADDACCOUNT_RESPONSE, 0);
	RegAuthDB_AddAccount_Response* pstResp = stHandleResult.stResponseMsg.mutable_stbody()->mutable_m_stregauthdb_addaccount_response();

	//获取请求
	const RegAuthDB_AddAccount_Request& stReq = m_pstRequestMsg->stbody().m_stregauthdb_addaccount_request();
	const std::string& strAccount = stReq.stinfo().straccount();

    int iRet = CheckParams(stReq);
    if(iRet)
    {
        TRACE_THREAD(m_iThreadIdx, "Failed to add account, param check failed, ret %d, account %s\n", iRet, strAccount.c_str());

		pstResp->set_iresult(iRet);
        return;
    }

    //检查帐号是否存在
    bool bIsExist = false;
    iRet = CheckAccountExist(stReq.stinfo(), bIsExist);
    if(iRet)
    {
        TRACE_THREAD(m_iThreadIdx, "Failed to check account exist, ret %d, account %s\n", iRet, strAccount.c_str());

		pstResp->set_iresult(iRet);
        return;
    }

    if(bIsExist)
    {
        //帐号已经存在
        TRACE_THREAD(m_iThreadIdx, "Failed to add account, already exist, ret %d, account %s\n", T_REGAUTHDB_ACCOUNT_EXISTS, strAccount.c_str());

		pstResp->set_iresult(T_REGAUTHDB_ACCOUNT_EXISTS);
        return;
    }

    //生成唯一uin
	unsigned uin = 0;
    iRet = GetAvaliableUin(uin);
    if(iRet)
    {
        TRACE_THREAD(m_iThreadIdx, "Failed to get avaliable uin, ret %d\n", iRet);
		
		pstResp->set_iresult(iRet);
        return;
    }

    //插入新的记录
    iRet = AddNewRecord(stReq.stinfo(), uin, stReq.iworldid());
    if(iRet)
    {
        TRACE_THREAD(m_iThreadIdx, "Failed to add new record, account %s, ret %d\n", strAccount.c_str(), iRet);

		pstResp->set_iresult(iRet);
        return;
    }

	pstResp->set_iresult(T_SERVER_SUCCESS);
    return;
}

//检查帐号是否存在
int CAddRegAuthHandler::CheckAccountExist(const AccountInfo& stInfo, bool& bIsExist)
{
    //设置连接的DB
    const OneDBInfo* pstDBConfig = ConfigManager::Instance()->GetRegAuthDBConfigByIndex(m_iThreadIdx);
    if(!pstDBConfig)
    {
        TRACE_THREAD(m_iThreadIdx, "Failed to get RegAuth db config, index %d\n", m_iThreadIdx);
        return -1;
    }

    int iRet = m_pDatabase->SetMysqlDBInfo(pstDBConfig->szDBHost, pstDBConfig->szUserName, pstDBConfig->szUserPasswd, pstDBConfig->szDBName);
    if(iRet)
    {
        TRACE_THREAD(m_iThreadIdx, "Failed to set mysql db info, ret %d\n", iRet);
        return iRet;
    }

    char* pszQueryString = *m_ppQueryBuff;
    int iLength = SAFE_SPRINTF(pszQueryString, MAX_QUERYBUFF_SIZE-1, "select uin from %s where accountID='%s' and accountType=%d", 
                 MYSQL_ACCOUNTINFO_TABLE, stInfo.straccount().c_str(), stInfo.itype());

    iRet = m_pDatabase->ExecuteRealQuery(pszQueryString, iLength, true);
    if(iRet)
    {
        TRACE_THREAD(m_iThreadIdx, "Failed to execute sql query, ret %d\n", iRet);
        return iRet;
    }

    if(m_pDatabase->GetNumberRows() != 0)
    {
        //已经存在该帐号
        bIsExist = true;
    }
    else
    {
        bIsExist = false;
    }

    return T_SERVER_SUCCESS;
}

//拉取可用的帐号UIN
int CAddRegAuthHandler::GetAvaliableUin(unsigned int& uin)
{
    //拉取帐号数据库的配置
    const OneDBInfo* pstDBConfig = ConfigManager::Instance()->GetUniqUinDBConfigByIndex(m_iThreadIdx);
    if(!pstDBConfig)
    {
        TRACE_THREAD(m_iThreadIdx, "Failed to get UniqUinDB config!\n");
        return -1;
    }

    int iRet = m_pDatabase->SetMysqlDBInfo(pstDBConfig->szDBHost, pstDBConfig->szUserName, pstDBConfig->szUserPasswd, pstDBConfig->szDBName);
    if(iRet)
    {
        TRACE_THREAD(m_iThreadIdx, "Failed to set mysql db info, db name %s\n", pstDBConfig->szDBName);
        return iRet;
    }

    //生成SQL语句
    char* pszQueryString = *m_ppQueryBuff;
    int iLength = SAFE_SPRINTF(pszQueryString, MAX_QUERYBUFF_SIZE-1, "insert into %s set uin=NULL", MYSQL_UNIQUININFO_TABLE);

    iRet = m_pDatabase->ExecuteRealQuery(pszQueryString, iLength, false);
    if(iRet)
    {
        TRACE_THREAD(m_iThreadIdx, "Failed to execute sql query, ret %d\n", iRet);
        return iRet;
    }

    uin = m_pDatabase->GetLastInsertID();
    if(uin == 0)
    {
        TRACE_THREAD(m_iThreadIdx, "Failed to get uniq uin, should not be zero!\n");
        return -2;
    }

    TRACE_THREAD(m_iThreadIdx, "Success to get new register uin: %u\n", uin);

    return T_SERVER_SUCCESS;
}

//插入新的帐号记录
int CAddRegAuthHandler::AddNewRecord(const AccountInfo& stInfo, unsigned int uin, int iWorldID)
{
    //设置连接的DB
    const OneDBInfo* pstDBConfig = ConfigManager::Instance()->GetRegAuthDBConfigByIndex(m_iThreadIdx);
    if(!pstDBConfig)
    {
        TRACE_THREAD(m_iThreadIdx, "Failed to get RegAuth db config, index %d\n", m_iThreadIdx);
        return -1;
    }

    int iRet = m_pDatabase->SetMysqlDBInfo(pstDBConfig->szDBHost, pstDBConfig->szUserName, pstDBConfig->szUserPasswd, pstDBConfig->szDBName);
    if(iRet)
    {
        TRACE_THREAD(m_iThreadIdx, "Failed to set mysql db info, ret %d\n", iRet);
        return iRet;
    }

    char* pszQueryString = *m_ppQueryBuff;
    int iLength = SAFE_SPRINTF(pszQueryString, MAX_QUERYBUFF_SIZE-1, \
		"insert into %s(accountID,accountType,uin,password,worldID) "
		"values('%s',%d,%u,'%s',%d)", 
		MYSQL_ACCOUNTINFO_TABLE, 
		stInfo.straccount().c_str(), stInfo.itype(), uin, stInfo.strpassword().c_str(), iWorldID);

    iRet = m_pDatabase->ExecuteRealQuery(pszQueryString, iLength, false);
    if(iRet)
    {
        TRACE_THREAD(m_iThreadIdx, "Failed to execute sql query, ret %d\n", iRet);
        return iRet;
    }

    TRACE_THREAD(m_iThreadIdx, "Success to add new account: %s, uin %u\n", stInfo.straccount().c_str(), uin);

    return T_SERVER_SUCCESS;
}
