#pragma once

#include "CommDefine.h"
#include "Kernel/Handler.hpp"

class CGameRoleObj;
class CRankInfoHandler : public IHandler
{
public:
	virtual ~CRankInfoHandler();

	virtual int OnClientMsg();

private:

	//玩家拉取排行榜信息
	int OnRequestGetRankInfo();

	//拉取World排行榜的返回
	int OnResponseGetWorldRank();
};