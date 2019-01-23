#pragma once

#include "DBClientWrapper.hpp"
#include "MsgHandler.hpp"
#include "AppDef.hpp"
#include "ErrorNumDef.hpp"

//MSGID_REGAUTHDB_FETCH_REQUEST 消息处理者
class CFetchRegAuthHandler : public CMsgHandler
{
public:
	CFetchRegAuthHandler(DBClientWrapper* pDatabase, char** ppQueryBuff);

	void SetThreadIdx(const int iThreadIdx){m_iThreadIdx = iThreadIdx;}

	//第三个参数不需要传
	virtual void OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult = NULL, TNetHead_V2* pstNetHead = NULL);

private:

    //拉取返回帐号详细信息
    int FetchAccountInfo(const AccountInfo& stInfo, RegAuthDB_GetAccount_Response& stResp);

private:
	DBClientWrapper* m_pDatabase;
	GameProtocolMsg* m_pstRequestMsg; // 待处理的消息
	char** m_ppQueryBuff;
	int m_iThreadIdx;
};
