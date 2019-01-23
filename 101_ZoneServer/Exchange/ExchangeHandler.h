#pragma once

#include "CommDefine.h"
#include "Kernel/Handler.hpp"
#include "ExchangeHandler.h"

class CGameRoleObj;
class CExchangeHandler : public IHandler
{
public:
	virtual ~CExchangeHandler();

	virtual int OnClientMsg();

private:

	//玩家修改信息的请求
	int OnRequestChangeUserInfo();

	//玩家兑换的请求
	int OnRequestExchange();

	//玩家拉取限限量的请求
	int OnRequestGetLimitInfo();

	//World拉取限量的返回
	int OnResponseGetLimitInfo();

	//World更新限量的返回
	int OnResponseUpdateLimit();

	//World拉取卡密信息的返回
	int OnResponseGetCardNo();

	//玩家拉取兑换记录的请求
	int OnRequestGetExchangeRec();

	//World拉取兑换记录的返回
	int OnResponseGetExchangeRec();
};
