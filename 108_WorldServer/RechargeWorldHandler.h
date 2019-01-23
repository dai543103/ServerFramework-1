#pragma once

#include "Handler.hpp"
#include "AppDefW.hpp"

class CRechargeWorldHandler : public IHandler
{
public:
	virtual ~CRechargeWorldHandler();

public:

	virtual int OnClientMsg(GameProtocolMsg* pMsg);

private:

	//系统充值的请求
	int OnRequestUserRecharge();

	//系统充值的返回
	int OnResponseUserRecharge();

	//玩家拉取订单号
	int OnRequestGetPayOrder();

	//拉取账号信息的请求
	int OnRequestGetUserInfo();

	//拉取账号信息的返回
	int OnResponseGetUserInfo();

	//获取充值订单号
	std::string GetNewOrderID(unsigned uiUin);

private:
	GameProtocolMsg* m_pRequestMsg;
};
