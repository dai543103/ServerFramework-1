#pragma once

#include "Handler.hpp"
#include "AppDefW.hpp"

class CLasvegasWorldHandler : public IHandler
{
public:
	virtual ~CLasvegasWorldHandler();

public:

	virtual int OnClientMsg(GameProtocolMsg* pMsg);

private:

	//更新数字下注信息
	int OnRequestUpdateBetInfo();

	//更新中奖信息
	int OnRequestUpdatePrizeInfo();

private:
	GameProtocolMsg* m_pRequestMsg;
};
