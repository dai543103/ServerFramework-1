#include <stdlib.h>
#include <string.h>

#include "GameProtocol.hpp"
#include "AppDef.hpp"
#include "ThreadLogManager.hpp"
#include "UnixTime.hpp"
#include "NowTime.hpp"
#include "StringUtility.hpp"
#include "LogServerApp.hpp"
#include "ConfigManager.hpp"
#include "MofangLogProxy.hpp"
#include "TapDBLogProxy.hpp"

#include "WriteLogHandler.hpp"

using namespace ServerLib;


CWriteLogHandler::CWriteLogHandler(DBClientWrapper* pDatabase, char** ppQueryBuff)
{
	m_pDatabase = pDatabase;
	m_ppQueryBuff = ppQueryBuff;

	//初始化写日志代理
	memset(m_apstLogProxy, 0, sizeof(m_apstLogProxy));

	m_apstLogProxy[LOG_TARGET_TAPDB] = new CTapDBLogProxy;
	m_apstLogProxy[LOG_TARGET_MOFANG] = new CMofangLogProxy;
}

void CWriteLogHandler::OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult, TNetHead_V2* pstNetHead)
{
	// 写日志的请求
	m_pstRequestMsg = &stReqMsg;
	switch (m_pstRequestMsg->sthead().uimsgid())
	{
	case MSGID_WORLD_WRITELOG_REQUEST:
	{
		OnWriteLogRequest(*pstHandleResult);
	}
	break;

	default:
	{

	}
	break;
	}

	return;
}

//输出日志
void CWriteLogHandler::OnWriteLogRequest(SHandleResult& stHandleResult)
{
	//设置为不需要回复
	stHandleResult.iNeedResponse = 0;

	//获取请求
	const World_WriteLog_Request& stReq = m_pstRequestMsg->stbody().m_stworld_writelog_request();

	//检查参数
	if (stReq.ilogtargettype() <= LOG_TARGET_INVALID || stReq.ilogtargettype() >= LOG_TARGET_MAX)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to write log, invalid target %d, log %s\n", stReq.ilogtargettype(), stReq.strlogdata().c_str());
		return;
	}

	//根据目标类型输出日志
	int iRet = m_apstLogProxy[stReq.ilogtargettype()]->WriteLog(m_pDatabase, *m_ppQueryBuff, m_iThreadIdx, stReq.strlogdata());
	if (iRet)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to write log, ret %d, target %d, log %s\n", iRet, stReq.ilogtargettype(), stReq.strlogdata().c_str());
		return;
	}

	return;
}
