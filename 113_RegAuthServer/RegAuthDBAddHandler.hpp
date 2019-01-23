#pragma once

#include "MsgHandler.hpp"

// 处理来自RegAuthDB Server的 MSGID_REGAUTHDB_ADDACCOUNT_RESPONSE 消息
class CRegAuthDBAddHandler : public CMsgHandler
{
public:
    CRegAuthDBAddHandler();

	//第二个参数传空
	virtual void OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult = NULL, TNetHead_V2* pstNetHead = NULL);

private:
    int OnResponseRegAuthDBAdd();

private:
	// 客户连接
	TNetHead_V2* m_pstNetHead; 

	// 待处理的消息
	GameProtocolMsg* m_pstRequestMsg; 
	
	// 下面两个变量用于标识一个session
	unsigned int m_uiSessionFD;
	unsigned short m_unValue;
};
