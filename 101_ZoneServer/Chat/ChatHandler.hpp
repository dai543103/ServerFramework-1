
#ifndef __CHAT_HANDLER_HPP__
#define __CHAT_HANDLER_HPP__

#include "GameProtocol.hpp"
#include "Kernel/Handler.hpp"
#include "CommDefine.h"
#include "ErrorNumDef.hpp"

class CGameRoleObj;
class CChatHandler: public IHandler
{
public:
    virtual ~CChatHandler(void);
    CChatHandler();
public:
    int OnClientMsg();

private:

    //聊天消息的处理
    int OnRequestChat();

    //世界聊天消息的推送
    int OnNotifyChat();
	
	//世界走马灯的推送
	int OnNotifyHorseLamp();

	//发送聊天信息
	int SendChatMsg(CGameRoleObj& stRoleObj, int iChannel, const char* pMsg);

	//发送失败的回复
    int SendFailedResponse(int iMsgID, int iResultID, const TNetHead_V2& rstNetHead);

    //发送成功的回复
    int SendSuccessfulResponse();
};

#endif
