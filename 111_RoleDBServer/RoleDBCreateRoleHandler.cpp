#include <string.h>

#include "GameProtocol.hpp"
#include "ThreadLogManager.hpp"
#include "NowTime.hpp"
#include "UnixTime.hpp"
#include "SeqConverter.hpp"
#include "StringUtility.hpp"
#include "RoleDBApp.hpp"
#include "AppDef.hpp"
#include "ConfigManager.hpp"
#include "ErrorNumDef.hpp"

#include "RoleDBCreateRoleHandler.hpp"

using namespace ServerLib;

CRoleDBCreateRoleHandler::CRoleDBCreateRoleHandler(DBClientWrapper* pDatabase, char** ppQueryBuff)
{
    m_pDatabase = pDatabase;
	m_ppQueryBuff = ppQueryBuff;
}

void CRoleDBCreateRoleHandler::OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult, TNetHead_V2* pstNetHead)
{
    m_pstRequestMsg = &stReqMsg;

	//请求头
	const GameCSMsgHead& stHead = m_pstRequestMsg->sthead();
    
	//返回消息
	GenerateMsgHead(pstHandleResult->stResponseMsg, stHead.uisessionfd(), MSGID_WORLD_CREATEROLE_RESPONSE, stHead.uin());
	World_CreateRole_Response* pstResp = pstHandleResult->stResponseMsg.mutable_stbody()->mutable_m_stworld_createrole_response();

	//请求消息
	const World_CreateRole_Request& stReq = m_pstRequestMsg->stbody().m_stworld_createrole_request();

    //获取角色创建的时间
    unsigned short usCreateTime = 0;
    GetRoleCreateTime(usCreateTime);

	unsigned int uiDBSeq = 0;

    // 响应消息体
    pstResp->mutable_stroleid()->set_uin(stReq.uin());
    pstResp->mutable_stroleid()->set_uiseq(uiDBSeq);
	pstResp->set_irealnamestat(stReq.irealnamestat());
	pstResp->mutable_stkicker()->CopyFrom(stReq.stkicker());

    // 将所有的用户数据写入数据表DBRoleInfo
    int iRet = InsertNewRoleRecord(stReq, uiDBSeq);
    if (iRet)
    {
		TRACE_THREAD(m_iThreadIdx, "Failed to insert new record, ret %d, uin %u\n", iRet, stReq.uin());

		pstResp->set_iresult(iRet);
		return;
    }

    DEBUG_THREAD(m_iThreadIdx, "create new role success, uin: %u\n", pstResp->stroleid().uin());

	pstResp->set_iresult(T_SERVER_SUCCESS);
    return;
}

int CRoleDBCreateRoleHandler::InsertNewRoleRecord(const World_CreateRole_Request& stReq, unsigned int uiSeq)
{
    //从XML文件中读取配置
    const OneDBInfo* pstDBInfoConfig = ConfigManager::Instance()->GetRoleDBConfigByIndex(m_iThreadIdx);
    if(!pstDBInfoConfig)
    {
        TRACE_THREAD(m_iThreadIdx, "Failed to insert new role record, invalid cfg, thread idx %d\n", m_iThreadIdx);
        return T_ROLEDB_PARA_ERR;
    }

    //设置要操作的数据库相关信息
    m_pDatabase->SetMysqlDBInfo(pstDBInfoConfig->szDBHost, pstDBInfoConfig->szUserName, pstDBInfoConfig->szUserPasswd, pstDBInfoConfig->szDBName);

    //初始化SQL语句
    int iLength = 0;
    int iRet = GenerateQueryString(stReq, uiSeq, *m_ppQueryBuff, MAX_QUERYBUFF_SIZE-1, iLength);
    if(iRet)
    {
        TRACE_THREAD(m_iThreadIdx, "Fail to generate insert query sql string, ret %d\n", iRet);
        return iRet;
    }

    //执行SQL语句
    iRet = m_pDatabase->ExecuteRealQuery(*m_ppQueryBuff, iLength, false);
    if(iRet)
    {
        TRACE_THREAD(m_iThreadIdx, "Fail to insert user data, uin %u, ret %d\n", stReq.uin(), iRet);
        return iRet;
    }

    DEBUG_THREAD(m_iThreadIdx, "The number of affected rows is %d\n", m_pDatabase->GetAffectedRows());
    return 0;
}

//获取玩家角色创建的时间,是从2014-01-01开始起的天数
void CRoleDBCreateRoleHandler::GetRoleCreateTime(unsigned short& usCreateTime)
{
    CUnixTime stNow(time(NULL));
    stNow.GetDaysAfterYear(GAME_START_YEAR, usCreateTime);
}

//生成插入的SQL Query语句
int CRoleDBCreateRoleHandler::GenerateQueryString(const World_CreateRole_Request& stReq, unsigned int uiSeq, char* pszBuff, int iBuffLen, int& iLength)
{
    if(!pszBuff)
    {
        TRACE_THREAD(m_iThreadIdx, "Fail to generate query string, invlaid buff pointer!\n");
        return T_ROLEDB_PARA_ERR;
    }

    unsigned int uiUin = stReq.uin();

    SAFE_SPRINTF(pszBuff, iBuffLen-1, "insert into %s(uin,seq,base_info,quest_info,item_info,friend_info,mail_info,reserved1,reserved2) values(%u,%u,", MYSQL_USERINFO_TABLE, uiUin, uiSeq);
    char* pEnd = pszBuff + strlen(pszBuff);

    MYSQL& stDBConn = m_pDatabase->GetCurMysqlConn();

    const GameUserInfo& rstUserInfo = stReq.stbirthdata();

    //1.玩家基本信息 base_info
    *pEnd++ = '\'';
    pEnd += mysql_real_escape_string(&stDBConn, pEnd, rstUserInfo.strbaseinfo().c_str(), rstUserInfo.strbaseinfo().size());
    *pEnd++ = '\'';
    *pEnd++ = ',';

    //2.玩家的任务信息 quest_info
    *pEnd++ = '\'';
    pEnd += mysql_real_escape_string(&stDBConn, pEnd, rstUserInfo.strquestinfo().c_str(), rstUserInfo.strquestinfo().size());
    *pEnd++ = '\'';
    *pEnd++ = ',';

    //3.玩家的物品信息 item_info
    *pEnd++ = '\'';
    pEnd += mysql_real_escape_string(&stDBConn, pEnd, rstUserInfo.striteminfo().c_str(), rstUserInfo.striteminfo().size());
    *pEnd++ = '\'';
    *pEnd++ = ',';

    //4.玩家的好友信息
    *pEnd++ = '\'';
    pEnd += mysql_real_escape_string(&stDBConn, pEnd, rstUserInfo.strfriendinfo().c_str(), rstUserInfo.strfriendinfo().size());
    *pEnd++ = '\'';
    *pEnd++ = ',';

	//5.玩家的邮件信息
	*pEnd++ = '\'';
	pEnd += mysql_real_escape_string(&stDBConn, pEnd, rstUserInfo.strmailinfo().c_str(), rstUserInfo.strmailinfo().size());
	*pEnd++ = '\'';
	*pEnd++ = ',';

    //6.玩家的保留字段1
    *pEnd++ = '\'';
    pEnd += mysql_real_escape_string(&stDBConn, pEnd, rstUserInfo.strreserved1().c_str(), rstUserInfo.strreserved1().size());
    *pEnd++ = '\'';
    *pEnd++ = ',';

    //7.玩家的保留字段2
    *pEnd++ = '\'';
    pEnd += mysql_real_escape_string(&stDBConn, pEnd, rstUserInfo.strreserved2().c_str(), rstUserInfo.strreserved2().size());
    *pEnd++ = '\'';
    *pEnd++ = ')';

    iLength = pEnd - pszBuff;

    return T_SERVER_SUCCESS;
}
