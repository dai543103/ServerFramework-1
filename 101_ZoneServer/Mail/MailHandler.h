#pragma once

#include "CommDefine.h"
#include "Kernel/Handler.hpp"

class CGameRoleObj;
class CMailHandler : public IHandler
{
public:
	virtual ~CMailHandler();

	virtual int OnClientMsg();

private:

	//玩家操作邮件
	int OnRequestOperaMail();

	//发送邮件请求
	int OnRequestSendMail();

	//发送邮件的返回
	int OnResponseSendMail();

	//World系统邮件更新的通知
	int OnNotifySystemMailUniqID();

	//拉取系统邮件返回
	int OnResponseGetSystemMail();
};
