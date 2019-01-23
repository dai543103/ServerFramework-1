#pragma once

#include "CommDefine.h"
#include "Kernel/Handler.hpp"
#include "RepThingsManager.hpp"

class CGameRoleObj;

class CRepThingsHandler : public IHandler
{
public:
	virtual ~CRepThingsHandler();

	virtual int OnClientMsg();

private:

    //玩家对背包物品的操作
    int OnRequestOperaRepItem();

	//玩家道具使用返回
	int OnResponseUseRepItem();
};
