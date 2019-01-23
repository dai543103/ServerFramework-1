#pragma once

#include "DBClientWrapper.hpp"
#include "MsgHandler.hpp"
#include "AppDef.hpp"
#include "ErrorNumDef.hpp"
#include "LogTargetProxy.hpp"

//写玩家日志的请求
// MSGID_WORLD_WRITELOG_REQUEST 消息处理者
class CWriteLogHandler : public CMsgHandler
{
public:
	CWriteLogHandler(DBClientWrapper* pDatabase, char** ppQueryBuff);

	void SetThreadIdx(const int iThreadIdx){m_iThreadIdx = iThreadIdx;}

	//第三个参数不需要传
	virtual void OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult = NULL, TNetHead_V2* pstNetHead = NULL);

private:

	//输出日志
    void OnWriteLogRequest(SHandleResult& stHandleResult);

private:
	DBClientWrapper* m_pDatabase;		//访问数据库的指针
	GameProtocolMsg* m_pstRequestMsg;	//待处理的消息
	char** m_ppQueryBuff;				//DB操作缓冲区					
	int m_iThreadIdx;					//所属线程idx
	LogTargetProxy* m_apstLogProxy[LOG_TARGET_MAX];	//写日志目标平台代理
};
