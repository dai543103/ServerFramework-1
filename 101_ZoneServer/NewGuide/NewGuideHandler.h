#pragma once

#include "GameProtocol.hpp"
#include "Kernel/Handler.hpp"
#include "CommDefine.h"
#include "ErrorNumDef.hpp"

class CGameRoleObj;
class CNewGuideHandler : public IHandler
{
public:
	virtual ~CNewGuideHandler(void);
	CNewGuideHandler();

public:
	int OnClientMsg();

private:

	//完成新手引导请求
	int OnRequestFinGuide();

	//玩家修改名字请求
	int OnRequestUpdateName();
};
