#pragma once

#include "CommDefine.h"
#include "Kernel/Handler.hpp"

class CGameRoleObj;
class CLotteryHandler : public IHandler
{
public:
	virtual ~CLotteryHandler();

	virtual int OnClientMsg();

private:

	//玩家海盗宝藏请求
	int OnRequestLottery();

	//玩家限量抽奖请求
	int OnRequestLimitLottery();

	//玩家限量抽奖返回
	int OnResponseLimitLottery();

	//玩家拉取充值抽奖请求
	int OnRequestPayLotteryRecord();

	//玩家拉取充值抽奖返回
	int OnResponsePayLotteryRecord();

	//玩家进出大转盘抽奖请求
	int OnRequestEnterLasvegas();

	//玩家大转盘下注的请求
	int OnRequestBetLasvegas();

	//玩家拉取中奖纪录的请求
	int OnRequestGetRewardInfo();

	//World更新大转盘信息
	int OnNotifyUpdateLasvegas();
};
