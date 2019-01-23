#pragma once

#include "MsgHandler.hpp"

// 处理来自LotusServer的 MSGID_REGAUTH_REGACCOUNT_REQUEST 消息
class CRegisterAccountHandler : public CMsgHandler
{
public:
    CRegisterAccountHandler();

	//第二个参数传空
	virtual void OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult = NULL, TNetHead_V2* pstNetHead = NULL);

private:
    int OnRequestRegAccount();

	//新增加帐号
	void AddAccount(unsigned uiSessionFd, const AccountInfo& stInfo);

    int CheckParam();

private:
	// 客户连接
	TNetHead_V2* m_pstNetHead;	

	// 待处理的消息
	GameProtocolMsg* m_pstRequestMsg;	
	
	// 下面两个变量用于标识一个session
	unsigned int m_uiSessionFD;
	unsigned short m_unValue;
};
