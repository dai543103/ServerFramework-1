#pragma once

#include "Handler.hpp"
#include "AppDefW.hpp"

class CLimitLotteryWorldHandler : public IHandler
{
public:
	virtual ~CLimitLotteryWorldHandler();

public:

	virtual int OnClientMsg(GameProtocolMsg* pMsg);

private:

	//玩家限量抽奖
	int OnRequestLimitLottery();

	//玩家拉取重置抽奖记录
	int OnRequestPayLotteryRecord();

private:
	GameProtocolMsg* m_pRequestMsg;
};
