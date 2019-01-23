#pragma once

#include "MsgHandler.hpp"

// 处理来自LotusServer的MSGID_LOGOUTSERVER_REQUEST消息
class CClientClosedHandler : public CMsgHandler
{
public:
    CClientClosedHandler();

	//第二个参数传空
	virtual void OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult = NULL, TNetHead_V2* pstNetHead = NULL);
};
