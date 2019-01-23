#pragma once

#include "Handler.hpp"
#include "AppDefW.hpp"

class CExchangeWorldHandler : public IHandler
{
public:
	virtual ~CExchangeWorldHandler();

public:

	virtual int OnClientMsg(GameProtocolMsg* pMsg);

private:

	//拉取限量兑换信息
	int OnRequestGetExchange();

	//修改限量兑换信息
	int OnRequestUpdateExchange();

	//在线兑换请求
	int OnRequestOnlineExchange();

	//在线兑换返回
	int OnResponseOnlineExchange();

	//增加兑换记录请求
	int OnRequestAddExchangeRec();

	//拉取兑换记录请求
	int OnRequestGetExchangeRec();

private:
	GameProtocolMsg* m_pRequestMsg;
};
