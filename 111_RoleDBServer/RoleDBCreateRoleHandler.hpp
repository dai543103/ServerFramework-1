#pragma once

#include "DBClientWrapper.hpp"
#include "MsgHandler.hpp"

//创建角色消息
class CRoleDBCreateRoleHandler : public CMsgHandler
{
public:
	CRoleDBCreateRoleHandler(DBClientWrapper* pDatabase, char** ppQueryBuff);

	void SetThreadIdx(const int iThreadIdx){m_iThreadIdx = iThreadIdx;}

	//第三个参数不需要传
	virtual void OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult = NULL, TNetHead_V2* pstNetHead = NULL);

private:
    int InsertNewRoleRecord(const World_CreateRole_Request& stReq, unsigned int uiSeq);

    //获取玩家角色创建的时间
    void GetRoleCreateTime(unsigned short& usCreateTime);

    //生成插入的SQL Query语句
    int GenerateQueryString(const World_CreateRole_Request& stReq, unsigned int uiSeq, char* pszBuff, int iBuffLen, int& iLength);

private:
	DBClientWrapper* m_pDatabase;
	GameProtocolMsg* m_pstRequestMsg;	// 待处理的消息
	char** m_ppQueryBuff;				//数据库操作的BUFF
	int m_iThreadIdx;
};
