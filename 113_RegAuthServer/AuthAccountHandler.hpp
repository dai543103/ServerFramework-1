#pragma once

#include "MsgHandler.hpp"

// 处理来自LotusServer的MSGID_AUTHACCOUNT_REQUEST消息
class CAuthAccountHandler : public CMsgHandler
{
public:
    CAuthAccountHandler();
    
	//第二个参数传空
	virtual void OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult = NULL, TNetHead_V2* pstNetHead = NULL);

private:

	//认证帐号的请求
    int OnRequestAuthAccount();

	//平台认证返回
	int OnResponsePlatformAuth();

	//RegAuthDB认证返回
	int OnResponseRegDBAuth();

	//发送平台认证
	int PlatformAuthAccount(const AccountInfo& stInfo);

	//发送认证请求到RegAuthDB
	int SendAuthToRegAuthDB(const AccountInfo& stInfo);

private:
	// 客户连接
	TNetHead_V2* m_pstNetHead;  
	
	// 待处理的消息
	GameProtocolMsg* m_pstRequestMsg; 
	
	// 下面两个变量用于标识一个session
	unsigned int m_uiSessionFD;
	unsigned short m_unValue;
};
