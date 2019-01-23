#pragma once

#include "CommDefine.h"
#include "Kernel/Handler.hpp"

class CGameRoleObj;
class CVipPrivHandler : public IHandler
{
public:
	virtual ~CVipPrivHandler();

	virtual int OnClientMsg();

private:

	//玩家领取救济金的请求
	int OnRequestGetAlms();
};
