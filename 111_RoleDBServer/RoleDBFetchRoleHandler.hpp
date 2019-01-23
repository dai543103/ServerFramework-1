#pragma once

#include "DBClientWrapper.hpp"
#include "MsgHandler.hpp"
#include "SeqConverter.hpp"

// MSGID_ROLEDB_FETCHROLE_REQUEST消息处理者
class CRoleDBFetchRoleHandler : public CMsgHandler
{
public:
	CRoleDBFetchRoleHandler(DBClientWrapper* pDatabase, char** pQueryBuff);

	void SetThreadIdx(const int iThreadIdx){m_iThreadIdx = iThreadIdx;}

	//第三个参数不需要传
	virtual void OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult = NULL, TNetHead_V2* pstNetHead = NULL);

private:
    void OnFetchRoleRequest(SHandleResult& pstHandleResult);

    int QueryRole(const RoleID& stRoleID, World_FetchRole_Response& stResp);

private:
	DBClientWrapper* m_pDatabase;		//访问数据库的指针
	GameProtocolMsg* m_pstRequestMsg;	//待处理的消息
	char** m_ppQueryBuff;				//数据库操作的BUFF			
	int m_iThreadIdx;					//所属线程idx
};
