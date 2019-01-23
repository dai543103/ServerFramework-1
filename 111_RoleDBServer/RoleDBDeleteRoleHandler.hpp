#pragma once

#include "DBClientWrapper.hpp"
#include "MsgHandler.hpp"

// MSGID_ACCOUNT_DELETEROLE_REQUEST消息处理者
class CRoleDBDeleteRoleHandler : public CMsgHandler
{
public:
	CRoleDBDeleteRoleHandler(DBClientWrapper* pDatabase, char** ppQueryBuff);

	void SetThreadIdx(const int iThreadIdx){m_iThreadIdx = iThreadIdx;}

	//第三个参数不需要传
	virtual void OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult = NULL, TNetHead_V2* pstNetHead = NULL);

private:
    int DeleteRole(const RoleID& stRoleID);
    int QueryRoleInfo(const RoleID& stRoleID);

private:
	DBClientWrapper* m_pDatabase;
	GameProtocolMsg* m_pstRequestMsg;	// 待处理的消息
	char** m_ppQueryBuff;				//数据库操作的BUFF
	int m_iThreadIdx;
};

