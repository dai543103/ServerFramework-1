#pragma once

#include "Handler.hpp"
#include "AppDefW.hpp"

class CMailWorldHandler : public IHandler
{
public:
	virtual ~CMailWorldHandler();

public:

	virtual int OnClientMsg(GameProtocolMsg* pMsg);

private:

	//发送邮件的请求
	int OnRequestSendMail();

	//发送邮件的返回
	int OnResponseSendMail();

	//拉取系统邮件请求
	int OnRequestGetSystemMail();

private:
	GameProtocolMsg* m_pRequestMsg;
};
