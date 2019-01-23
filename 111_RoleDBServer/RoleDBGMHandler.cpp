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

#include "RoleDBGMHandler.hpp"

using namespace ServerLib;

CRoleDBGMHandler::CRoleDBGMHandler(DBClientWrapper* pDatabase, char** ppQueryBuff)
{    
    m_pDatabase = pDatabase;
	m_ppQueryBuff = ppQueryBuff;
}

void CRoleDBGMHandler::OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult, TNetHead_V2* pstNetHead)
{
	m_pstRequestMsg = &stReqMsg;
	switch (m_pstRequestMsg->sthead().uimsgid())
	{
	case MSGID_WORLD_GAMEMASTER_REQUEST:
	{
		GMOperaOffline(*pstHandleResult);
		break;
	}

	default:
	{
		break;
	}
	}
}

int CRoleDBGMHandler::GMOperaOffline(SHandleResult& stHandleResult)
{
	//获取请求
	const GameMaster_Request& stReq = m_pstRequestMsg->stbody().m_stworld_gamemaster_request();
	unsigned int uiUin = stReq.utouin();

    //获取返回消息,返回发送给操作者
    GenerateMsgHead(stHandleResult.stResponseMsg, 0, MSGID_WORLD_GAMEMASTER_RESPONSE, stReq.ufromuin());
	GameMaster_Response* pstResp = stHandleResult.stResponseMsg.mutable_stbody()->mutable_m_stworld_gamemaster_response();
	pstResp->set_ioperatype(stReq.ioperatype());
	pstResp->set_utouin(stReq.utouin());
	pstResp->set_ufromuin(stReq.ufromuin());

	//读取ROLEDB相关的配置
	const OneDBInfo* pstDBConfig = ConfigManager::Instance()->GetRoleDBConfigByIndex(m_iThreadIdx);
	if (!pstDBConfig)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to get roledb config, invalid index %d\n", m_iThreadIdx);

		pstResp->set_iresult(T_ROLEDB_PARA_ERR);
		return -1;
	}

	//设置要访问的数据库信息
	m_pDatabase->SetMysqlDBInfo(pstDBConfig->szDBHost, pstDBConfig->szUserName, pstDBConfig->szUserPasswd, pstDBConfig->szDBName);

	int iRet = T_SERVER_SUCCESS;
	switch (stReq.ioperatype())
	{
	case GM_OPERA_ADDRES:
	case GM_OPERA_ADDITEM:
	case GM_OPERA_RECHARGE:
	{
		//修改玩家数据
		iRet = GMModifyRoleData(stReq);
		if (iRet)
		{
			TRACE_THREAD(m_iThreadIdx, "Failed to modify role data, ret %d, to uin %u\n", iRet, uiUin);
			
			pstResp->set_iresult(iRet);
			return iRet;
		}
	}
	break;

	case GM_OPERA_GETBASEINFO:
	{
		//获取基础数据
		GameLoginInfo stUserInfo;
		iRet = GMGetRoleData(uiUin, stUserInfo, ROLEDB_DATA_BASEINFO);
		if (iRet)
		{
			TRACE_THREAD(m_iThreadIdx,"Failed to gm get baseinfo, ret %d, to uin %u\n", iRet, uiUin);

			pstResp->set_iresult(iRet);
			return iRet;
		}

		pstResp->mutable_stbaseinfo()->CopyFrom(stUserInfo.stbaseinfo());
	}
	break;

	case GM_OPERA_GETREPINFO:
	{
		//拉取玩家数据
		GameLoginInfo stUserInfo;
		iRet = GMGetRoleData(uiUin, stUserInfo, ROLEDB_DATA_ITEMINFO);
		if (iRet)
		{
			TRACE_THREAD(m_iThreadIdx, "Failed to gm get iteminfo, ret %d, to uin %u\n", iRet, uiUin);

			pstResp->set_iresult(iRet);
			return iRet;
		}

		pstResp->mutable_stiteminfo()->CopyFrom(stUserInfo.stiteminfo());
	}
	break;

	default:
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to run gm command, invalid type %d, gm user %u\n", stReq.ioperatype(), stReq.ufromuin());

		pstResp->set_iresult(T_ROLEDB_PARA_ERR);
		return T_ROLEDB_PARA_ERR;
	}
	break;
	}

	pstResp->set_iresult(T_SERVER_SUCCESS);
	return 0;
}

//修改玩家数据
int CRoleDBGMHandler::GMModifyRoleData(const GameMaster_Request& stReq)
{
	unsigned uiUin = stReq.utouin();

	//拉取离线数据
	GameLoginInfo stUserInfo;
	int iRet = GMGetRoleData(uiUin, stUserInfo, ROLEDB_DATA_OFFLINE);
	if (iRet)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to get offline data, ret %d, uin %u\n", iRet, uiUin);
		return iRet;
	}

	RESERVED1DBINFO* pstOfflineInfo = stUserInfo.mutable_streserved1();
	switch (stReq.ioperatype())
	{
	case GM_OPERA_ADDRES:
	{
		//增加资源
		if (stReq.strparams_size() != 2)
		{
			TRACE_THREAD(m_iThreadIdx, "Failed to gm add resource, invalid param, gm user %u\n", stReq.ufromuin());
			return T_ROLEDB_PARA_ERR;
		}

		AddResInfo* pstOneInfo = pstOfflineInfo->mutable_stresinfo()->add_staddres();
		pstOneInfo->set_irestype(atoi(stReq.strparams(0).c_str()));
		pstOneInfo->set_iaddnum(atoi(stReq.strparams(1).c_str()));
	}
	break;

	case GM_OPERA_ADDITEM:
	{
		//增加道具
		if (stReq.strparams_size() != 2)
		{
			TRACE_THREAD(m_iThreadIdx, "Failed to gm add item, invalid param, gm user %u\n", stReq.ufromuin());
			return T_ROLEDB_PARA_ERR;
		}

		OneSlotInfo* pstOneInfo = pstOfflineInfo->mutable_stiteminfo()->add_stslots();
		pstOneInfo->set_iitemid(atoi(stReq.strparams(0).c_str()));
		pstOneInfo->set_iitemnum(atoi(stReq.strparams(1).c_str()));
	}
	break;

	case GM_OPERA_RECHARGE:
	{
		//GM充值
		if (stReq.strparams_size() != 2)
		{
			TRACE_THREAD(m_iThreadIdx, "Failed to gm recahrge, invalid param, gm user %u, to user %u\n", stReq.ufromuin(), uiUin);
			return T_ROLEDB_PARA_ERR;
		}

		RechargeRecord* pstOne = pstOfflineInfo->mutable_stpayinfo()->add_strecords();
		pstOne->set_irechargeid(atoi(stReq.strparams(0).c_str()));
		pstOne->set_itime(atoi(stReq.strparams(1).c_str()));
		pstOne->set_strorderid("GM");
	}
	break;

	default:
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to run gm, invalid opera type %d, gm user %u\n", stReq.ioperatype(), stReq.ufromuin());
		return T_ROLEDB_PARA_ERR;
	}
	break;
	}

	//更新到DB
	iRet = GMUpdateRoleData(uiUin, stUserInfo, ROLEDB_DATA_OFFLINE);
	if (iRet)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to gm update role, gm user %u, to user %u\n", stReq.ufromuin(), uiUin);
		return iRet;
	}

	return T_SERVER_SUCCESS;
}

//更新玩家数据
int CRoleDBGMHandler::GMUpdateRoleData(unsigned uiUin, const GameLoginInfo& stUserInfo, int iDataType)
{
	if (!m_pDatabase || iDataType <= ROLEDB_DATA_INVALID || iDataType >= ROLEDB_DATA_MAX)
	{
		return -1;
	}

	//构造SQL语句
	char* pszQueryString = *m_ppQueryBuff;
	static int iQueryStringLen = MAX_QUERYBUFF_SIZE - 1;
	static const char* szDataName[ROLEDB_DATA_MAX] = { "","base_info", "item_info", "quest_info", "mail_info", "reserved1", "reserved2" };

	//更新数据
	SAFE_SPRINTF(pszQueryString, iQueryStringLen, "update %s set ", MYSQL_USERINFO_TABLE);
	char* pEnd = pszQueryString + strlen(pszQueryString);

	//玩家离线数据
	std::string strUpdateInfo;
	bool bEncodeRet = false;
	switch (iDataType)
	{
	case ROLEDB_DATA_BASEINFO:
	{
		bEncodeRet = EncodeProtoData(stUserInfo.stbaseinfo(), strUpdateInfo);
	}
	break;

	case ROLEDB_DATA_ITEMINFO:
	{
		bEncodeRet = EncodeProtoData(stUserInfo.stiteminfo(), strUpdateInfo);
	}
	break;

	case ROLEDB_DATA_QUESTINFO:
	{
		bEncodeRet = EncodeProtoData(stUserInfo.stquestinfo(), strUpdateInfo);
	}
	break;

	case ROLEDB_DATA_MAILINFO:
	{
		bEncodeRet = EncodeProtoData(stUserInfo.stmailinfo(), strUpdateInfo);
	}
	break;

	case ROLEDB_DATA_OFFLINE:
	{
		bEncodeRet = EncodeProtoData(stUserInfo.streserved1(), strUpdateInfo);
	}
	break;

	case ROLEDB_DATA_RESERVED2:
	{
		bEncodeRet = EncodeProtoData(stUserInfo.streserved2(), strUpdateInfo);
	}
	break;

	default:
		break;
	}

	if (!bEncodeRet || strUpdateInfo.size() == 0)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to update offline info, invalid length, uin %u\n", uiUin);
		return T_ROLEDB_PARA_ERR;
	}

	pEnd += SAFE_SPRINTF(pEnd, iQueryStringLen, "%s=", szDataName[iDataType]);

	//玩家离线数据
	*pEnd++ = '\'';
	pEnd += mysql_real_escape_string(&m_pDatabase->GetCurMysqlConn(), pEnd, strUpdateInfo.c_str(), strUpdateInfo.size());
	*pEnd++ = '\'';

	pEnd += SAFE_SPRINTF(pEnd, iQueryStringLen, " where uin = %u\n", uiUin);

	int iLength = pEnd - pszQueryString;

	//执行
	int iRet = m_pDatabase->ExecuteRealQuery(pszQueryString, iLength, false);
	if (iRet)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to execute select sql query, uin %u\n", uiUin);
		return iRet;
	}

	DEBUG_THREAD(m_iThreadIdx, "Success to gm update role info uin %u\n", uiUin);

	return T_SERVER_SUCCESS;
}

//拉取玩家数据
int CRoleDBGMHandler::GMGetRoleData(unsigned uiUin, GameLoginInfo& stUserInfo, int iDataType)
{
	if (!m_pDatabase || iDataType<=ROLEDB_DATA_INVALID || iDataType>=ROLEDB_DATA_MAX)
	{
		return -1;
	}

	//构造SQL语句
	char* pszQueryString = *m_ppQueryBuff;
	static int iQueryStringLen = MAX_QUERYBUFF_SIZE - 1;

	static const char* szDataName[ROLEDB_DATA_MAX] = {"","base_info", "item_info", "quest_info", "mail_info", "reserved1", "reserved2"};

	//拉取离线数据
	int iLength = SAFE_SPRINTF(pszQueryString, iQueryStringLen, "select %s from %s where uin=%u\n", szDataName[iDataType], MYSQL_USERINFO_TABLE, uiUin);

	//执行
	int iRet = m_pDatabase->ExecuteRealQuery(pszQueryString, iLength, true);
	if (iRet)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to execute select sql query, uin %u\n", uiUin);
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
		TRACE_THREAD(m_iThreadIdx, "Failed to fetch rows, uin %u, ret %d\n", uiUin, iRet);
		return iRet;
	}

	bool bDecodeRet = false;
	std::string strData(pstResult[0], pLengths[0]);
	switch (iDataType)
	{
	case ROLEDB_DATA_BASEINFO:
	{
		bDecodeRet = DecodeProtoData(strData, *stUserInfo.mutable_stbaseinfo());
	}
	break;

	case ROLEDB_DATA_ITEMINFO:
	{
		bDecodeRet = DecodeProtoData(strData, *stUserInfo.mutable_stiteminfo());
	}
	break;

	case ROLEDB_DATA_QUESTINFO:
	{
		bDecodeRet = DecodeProtoData(strData, *stUserInfo.mutable_stquestinfo());
	}
	break;

	case ROLEDB_DATA_MAILINFO:
	{
		bDecodeRet = DecodeProtoData(strData, *stUserInfo.mutable_stmailinfo());
	}
	break;

	case ROLEDB_DATA_OFFLINE:
	{
		bDecodeRet = DecodeProtoData(strData, *stUserInfo.mutable_streserved1());
	}
	break;

	case ROLEDB_DATA_RESERVED2:
	{
		bDecodeRet = DecodeProtoData(strData, *stUserInfo.mutable_streserved2());
	}
	break;

	default:
		break;
	}

	if (!bDecodeRet)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to decode user data, uin %u\n", uiUin);
		return T_ROLEDB_INVALID_RECORD;
	}

	return T_SERVER_SUCCESS;
}
