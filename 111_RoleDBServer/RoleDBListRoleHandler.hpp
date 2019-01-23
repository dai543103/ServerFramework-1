#pragma once

#include "DBClientWrapper.hpp"
#include "MsgHandler.hpp"

// LISTROLE_ACCOUNT_REQUEST消息处理者
class CRoleDBListRoleHandler : public CMsgHandler
{
public:

	CRoleDBListRoleHandler(DBClientWrapper* pDatabase, char** ppQueryBuff);

	void SetThreadIdx(const int iThreadIdx){m_iThreadIdx = iThreadIdx;}

	//第三个参数不需要传
	virtual void OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult = NULL, TNetHead_V2* pstNetHead = NULL);

private:
    int QueryAndParseRole(const unsigned int uiUin, short nWorldID, short nNewWorldID, Account_ListRole_Response& stResp, int& iErrnoNum);
	int AccountListRole(SHandleResult& stHandleResult);

private:
	DBClientWrapper* m_pDatabase;		// 访问数据库的指针
	GameProtocolMsg* m_pstRequestMsg;	// 待处理的消息
	char** m_ppQueryBuff;				// 数据库操作的缓冲区					
	int m_iThreadIdx;					//所属线程idx
};
