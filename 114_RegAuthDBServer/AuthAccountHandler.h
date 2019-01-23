#pragma once

#include "DBClientWrapper.hpp"
#include "MsgHandler.hpp"
#include "AppDef.hpp"

//认证账号的请求

// MSGID_REGAUTH_AUTHACCOUNT_REQUEST 消息处理者
class CAuthAccountHandler : public CMsgHandler
{
public:
	CAuthAccountHandler(DBClientWrapper* pDatabase, char** ppQueryBuff);

	void SetThreadIdx(const int iThreadIdx) { m_iThreadIdx = iThreadIdx; }

	//第三个参数不需要传
	virtual void OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult = NULL, TNetHead_V2* pstNetHead = NULL);

private:

	//认证玩家账号请求
	int OnRequestAuthAccount(SHandleResult& stHandleResult);

	//认证玩家账号
	//bIsExist返回账号是否存在
	int AuthAccount(const AccountInfo& stInfo, RegAuth_AuthAccount_Response& stResp, bool& bIsExist);

	//账号不存在，新增账号
	int AddNewAccount(const AccountInfo& stInfo, RegAuth_AuthAccount_Response& stResp);

	//获取可用uin
	int GetAvaliableUin(unsigned int& uin);

	//插入新的记录
	int AddNewRecord(const AccountInfo& stInfo, unsigned int uin, RegAuth_AuthAccount_Response& stResp);

private:
	DBClientWrapper* m_pDatabase;		//访问数据库的指针
	GameProtocolMsg* m_pstRequestMsg;	//待处理的消息
	char** m_ppQueryBuff;				//DB操作缓冲区					
	int m_iThreadIdx;					//所属线程idx
};
