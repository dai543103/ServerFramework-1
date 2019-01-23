#pragma once

#include "DBClientWrapper.hpp"
#include "MsgHandler.hpp"

// MSGID_WORLD_SENDMAIL_REQUEST 消息处理者
class CRoleDBMailHandler : public CMsgHandler
{
public:
	CRoleDBMailHandler(DBClientWrapper* pDatabase, char** ppQueryBuff);

	void SetThreadIdx(const int iThreadIdx) { m_iThreadIdx = iThreadIdx; }

	//第三个参数不需要传
	virtual void OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult = NULL, TNetHead_V2* pstNetHead = NULL);

private:

	//发送离线邮件
	int OfflineMail(SHandleResult& stHandleResult);

	//拉取离线数据
	int GetOfflineInfo(unsigned uin, RESERVED1DBINFO& stOfflineInfo);

	//增加离线邮件信息
	void AddOfflineMailInfo(const OneMailInfo& stMailInfo, RESERVED1DBINFO& stOfflineInfo);

	//更新离线数据
	int UpdateOfflineInfo(unsigned uin, RESERVED1DBINFO& stOfflineInfo);

private:
	DBClientWrapper* m_pDatabase;		// 访问数据库的指针
	GameProtocolMsg* m_pstRequestMsg;	// 待处理的消息
	char** m_ppQueryBuff;				// 数据库操作的缓冲区			
	int m_iThreadIdx;					//所属线程idx
};
