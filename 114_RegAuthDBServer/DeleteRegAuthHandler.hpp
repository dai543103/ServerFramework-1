#pragma once

#include "DBClientWrapper.hpp"
#include "MsgHandler.hpp"
#include "AppDef.hpp"
#include "ErrorNumDef.hpp"

// MSGID_REGAUTHDB_DELETE_REQUEST 消息处理者
class CDeleteRegAuthHandler : public CMsgHandler
{
public:
	CDeleteRegAuthHandler(DBClientWrapper* pDatabase, char** ppQueryBuff);

	void SetThreadIdx(const int iThreadIdx){m_iThreadIdx = iThreadIdx;}

	//第三个参数不需要传
	virtual void OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult = NULL, TNetHead_V2* pstNetHead = NULL);

private:

    //删除RegAuth帐号
    int DeleteAccount(const std::string& strAccount, int iType);

private:
	DBClientWrapper* m_pDatabase;		// 访问数据库的指针
	GameProtocolMsg* m_pstRequestMsg;	// 待处理的消息
	char** m_ppQueryBuff;				// DB操作缓冲区
	int m_iThreadIdx;
};
