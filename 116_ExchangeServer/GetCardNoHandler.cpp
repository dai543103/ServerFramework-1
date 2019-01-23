#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "StringUtility.hpp"
#include "HashUtility.hpp"
#include "ConfigManager.hpp"
#include "ErrorNumDef.hpp"
#include "TimeUtility.hpp"
#include "ExchangeMsgProcessor.hpp"
#include "ThreadLogManager.hpp"

#include "GetCardNoHandler.hpp"

using namespace ServerLib;

CGetCardNoHandler::CGetCardNoHandler(DBClientWrapper* pDatabase, char** ppQueryBuff)
{
	m_pDatabase = pDatabase;
	m_ppQueryBuff = ppQueryBuff;
}

//第三个参数传空
void CGetCardNoHandler::OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult, TNetHead_V2* pstNetHead)
{
	ASSERT_AND_LOG_RTN_VOID(pstHandleResult);

	m_pstRequestMsg = &stReqMsg;
	switch (m_pstRequestMsg->sthead().uimsgid())
	{
	case MSGID_WORLD_GETCARDNO_REQUEST:
	{
		//拉取卡密
		OnRequestGetCardNo(*pstHandleResult);
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

//设置线程index
void CGetCardNoHandler::SetThreadIdx(int iThreadIndex)
{
	m_iThreadIndex = iThreadIndex;
}

//拉取卡密
int CGetCardNoHandler::OnRequestGetCardNo(SHandleResult& stHandleResult)
{
	//获取请求
	const World_GetCardNo_Request& stReq = m_pstRequestMsg->stbody().m_stworld_getcardno_request();

	//返回消息
	GenerateMsgHead(stHandleResult.stResponseMsg, 0, MSGID_WORLD_GETCARDNO_RESPONSE, 0);
	World_GetCardNo_Response* pstResp = stHandleResult.stResponseMsg.mutable_stbody()->mutable_m_stworld_getcardno_response();
	pstResp->set_ifromzoneid(stReq.ifromzoneid());
	pstResp->set_uin(stReq.uin());
	pstResp->set_icardid(stReq.icardid());
	pstResp->set_iexchangeid(stReq.iexchangeid());

	//设置连接的DB
	const OneDBInfo* pstDBConfig = CConfigManager::Instance()->GetCardNoDBConfigByIndex(m_iThreadIndex);
	if (!pstDBConfig)
	{
		TRACE_THREAD(m_iThreadIndex, "Failed to get cardno db config, index %d\n", m_iThreadIndex);

		pstResp->set_iresult(T_EXCHANGE_PARA_ERROR);
		return -1;
	}

	//设置DB信息
	int iRet = m_pDatabase->SetMysqlDBInfo(pstDBConfig->szDBHost, pstDBConfig->szUserName, pstDBConfig->szUserPasswd, pstDBConfig->szDBName);
	if (iRet)
	{
		TRACE_THREAD(m_iThreadIndex, "Failed to set mysql db info, ret %d\n", iRet);

		pstResp->set_iresult(iRet);
		return -2;
	}

	char* pQueryBuff = *m_ppQueryBuff;
	int iLength = 0;

	//生成SQL语句
	iLength = SAFE_SPRINTF(pQueryBuff, MAX_QUERYBUFF_SIZE-1, \
		"select cardno,cardpwd from %s where cardid=%d and uin is null limit 1", MYSQL_CARDNO_TABLE, stReq.icardid());

	//执行拉取操作
	iRet = m_pDatabase->ExecuteRealQuery(pQueryBuff, iLength, true);
	if (iRet)
	{
		TRACE_THREAD(m_iThreadIndex, "Failed to execute sql query, ret %d\n", iRet);

		pstResp->set_iresult(T_EXCHANGE_PARA_ERROR);
		return -4;
	}

	//获取DB返回结果
	int iRowNum = m_pDatabase->GetNumberRows();
	if (iRowNum != 1)
	{
		//没有可用卡密
		TRACE_THREAD(m_iThreadIndex, "Failed to get cardno, invalid record num %d, cardid %d\n", iRowNum, stReq.icardid());

		pstResp->set_iresult(T_EXCHANGE_PARA_ERROR);
		return -5;
	}

	MYSQL_ROW pstResult = NULL;
	unsigned long* pLengths = NULL;
	unsigned int uFields = 0;

	iRet = m_pDatabase->FetchOneRow(pstResult, pLengths, uFields);
	if (iRet)
	{
		TRACE_THREAD(m_iThreadIndex, "Failed to fetch rows, uin %u, ret %d\n", stReq.uin(), iRet);

		pstResp->set_iresult(T_EXCHANGE_PARA_ERROR);
		return -6;
	}

	//从结果中解析需要的字段

	//字段1是cardno, 字段2是cardpwd
	pstResp->set_strcardno(pstResult[0], pLengths[0]);
	pstResp->set_strcardpwd(pstResult[1], pLengths[1]);

	//更新DB中卡密信息
	iLength = SAFE_SPRINTF(pQueryBuff, MAX_QUERYBUFF_SIZE-1, "update %s set uin=%u,usetime=now() where cardno=\'%s\'", MYSQL_CARDNO_TABLE, stReq.uin(), pstResp->strcardno().c_str());

	iRet = m_pDatabase->ExecuteRealQuery(pQueryBuff, iLength, false);
	if (iRet)
	{
		TRACE_THREAD(m_iThreadIndex, "Failed to execute sql query, ret %d\n", iRet);

		pstResp->set_iresult(T_EXCHANGE_PARA_ERROR);
		return -4;
	}

	DEBUG_THREAD(m_iThreadIndex, "Success to get card no, uin %u, cardno %s\n", stReq.uin(), pstResp->strcardno().c_str());

	pstResp->set_iresult(T_SERVER_SUCCESS);

	return T_SERVER_SUCCESS;
}
