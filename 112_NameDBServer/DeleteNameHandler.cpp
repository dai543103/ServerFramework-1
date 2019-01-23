#include <string.h>

#include "GameProtocol.hpp"
#include "AppDef.hpp"
#include "ThreadLogManager.hpp"
#include "StringUtility.hpp"
#include "ConfigManager.hpp"
#include "NameDBApp.hpp"

#include "DeleteNameHandler.hpp"

using namespace ServerLib;

CDeleteNameHandler::CDeleteNameHandler(DBClientWrapper* pDatabase, char** ppQueryBuff)
{
    m_pDatabase = pDatabase;
	m_ppQueryBuff = ppQueryBuff;
}

void CDeleteNameHandler::OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult, TNetHead_V2* pstNetHead)
{
    m_pstRequestMsg = &stReqMsg;

	//获取请求
    const DeleteName_Request& stReq = m_pstRequestMsg->stbody().m_stdeletename_request();

    DEBUG_THREAD(m_iThreadIdx, "Handling DeleteNameRequest, name: %s, type %d\n", stReq.strname().c_str(), stReq.itype());

    //返回消息
	GenerateMsgHead(pstHandleResult->stResponseMsg, m_pstRequestMsg->sthead().uisessionfd(), MSGID_DELETENAME_RESPONSE, 0);
	DeleteName_Response* pstResp = pstHandleResult->stResponseMsg.mutable_stbody()->mutable_m_stdeletename_response();

    //根据NameID和NameType删除记录
    int iRet = DeleteName(stReq.strname(), stReq.itype());
    if (iRet)
    {
		pstResp->set_iresult(iRet);
        return;
    }

	pstResp->set_iresult(T_SERVER_SUCCESS);
    return;
}

//删除Name
int CDeleteNameHandler::DeleteName(const std::string& strName, int iType)
{
    //拉取NAMEDB数据库相关配置
    const OneDBInfo* pstDBConfig = ConfigManager::Instance()->GetNameDBConfigByIndex(m_iThreadIdx);
    if(!pstDBConfig)
    {
        TRACE_THREAD(m_iThreadIdx, "Failed to get name db config, thread index %d\n", m_iThreadIdx);
        return -1;
    }

    //设置要操作的数据库相关信息
    m_pDatabase->SetMysqlDBInfo(pstDBConfig->szDBHost, pstDBConfig->szUserName, pstDBConfig->szUserPasswd, pstDBConfig->szDBName);

    char* pszQueryString = *m_ppQueryBuff;

    //生成删除的SQL语句
    int iLength = SAFE_SPRINTF(pszQueryString, MAX_QUERYBUFF_SIZE -1, "delete from %s where nickname= '%s' and nametype=%d",
                 MYSQL_NAMEINFO_TABLE, strName.c_str(), iType);

    int iRet = m_pDatabase->ExecuteRealQuery(pszQueryString, iLength, false);
    if(iRet)
    {
        TRACE_THREAD(m_iThreadIdx, "Failed to execute delete sql query, name %s, type %d, ret %d\n", strName.c_str(), iType, iRet);
        return iRet;
    }

    DEBUG_THREAD(m_iThreadIdx, "The number of affected rows is %d\n", m_pDatabase->GetAffectedRows());

    return 0;
}
