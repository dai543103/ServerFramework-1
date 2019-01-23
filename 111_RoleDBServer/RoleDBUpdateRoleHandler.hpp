#pragma once

#include "DBClientWrapper.hpp"
#include "MsgHandler.hpp"

// MSGID_ROLEDB_UPDATEROLE_REQUEST消息处理者
class CRoleDBUpdateRoleHandler : public CMsgHandler
{
public:
	CRoleDBUpdateRoleHandler(DBClientWrapper* pDatabase, char** ppQueryBuff);
	
	void SetThreadIdx(const int iThreadIdx){m_iThreadIdx = iThreadIdx;}

	//第三个参数不需要传
	virtual void OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult = NULL, TNetHead_V2* pstNetHead = NULL);

private:
    void OnUpdateRoleRequest(SHandleResult& stHandleResult);

private:
    int UpdateRole(const World_UpdateRole_Request& stReq);
    int GenerateQueryString(const World_UpdateRole_Request& iRet, char* pszBuff, int iBuffLen, int& iLength);

private:
	DBClientWrapper* m_pDatabase;		// 访问数据库的指针
	GameProtocolMsg* m_pstRequestMsg;	// 待处理的消息
	char** m_ppQueryBuff;				// 数据库操作缓冲区			
	int m_iThreadIdx;					//所属线程idx
};
