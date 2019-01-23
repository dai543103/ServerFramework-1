#pragma once

#include "CommDefine.h"
#include "Kernel/Handler.hpp"

class CGameRoleObj;
class CRechargeHandler : public IHandler
{
public:
	virtual ~CRechargeHandler();

	virtual int OnClientMsg();

private:

	//系统充值请求
	int OnRequestRecharge();

	//玩家拉取充值记录请求
	int OnRequestGetRecord();

	//玩家领取充值礼包请求
	int OnRequestGetPayGift();

	//玩家获取订单号请求
	int OnRequestGetPayOrder();

	//玩家获取订单号返回
	int OnResponseGetPayOrder();

	//玩家金币充值请求
	int OnRequestRechargeCoin();
};
