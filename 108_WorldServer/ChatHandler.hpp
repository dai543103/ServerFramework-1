#pragma once

#include "Handler.hpp"
#include "AppDefW.hpp"
#include "GameProtocol.hpp"

class CChatHandler : public IHandler
{
public:
    virtual ~CChatHandler();

public:
    virtual int OnClientMsg(GameProtocolMsg* pMsg);

private:

	//聊天消息
    int OnRequestChatNotify();

	//走马灯消息
	int OnRequestHorseLampNotify();

private:
    GameProtocolMsg* m_pMsg;
};
