#pragma once

#include <vector>
#include "GameProtocol.hpp"
#include "CommDefine.h"
#include "BaseConfigManager.hpp"

//限量抽奖信息
struct LimitLotteryInfo
{
	int iLotteryID;			//抽奖ID
	int iWeight;			//权重
	int iLimitType;			//限量类型
	int iConfigDayLimit;	//配置的日限量
	int iDailyLimit;		//当日剩余限量
	int iTotalLimit;		//总剩余限量

	LimitLotteryInfo()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

//充值抽奖记录
struct RechargeLotteryRecord
{
	std::string strNickName;	//抽奖玩家昵称
	int iLotteryID;				//抽中的ID

	RechargeLotteryRecord()
	{
		Reset();
	}

	void Reset()
	{
		strNickName.clear();
		iLotteryID = 0;
	}
};

class CWorldLimitLotteryManager
{
public:
	static CWorldLimitLotteryManager* Instance();

	~CWorldLimitLotteryManager();

	//初始化
	void Init();

	//限量抽奖
	void LimitLottery(int iLotteryType, const std::string& strNickName, bool bIsTenTimes, std::vector<int>& vLotteryIDs);

	//拉取充值记录
	void GetLotteryRecord(int iFrom, int iNum, Zone_PayLotteryRecord_Response& stResp);

private:

	CWorldLimitLotteryManager();

	//加载抽奖记录
	void LoadRechargeLotteryRecord();

	//保存抽奖记录
	void SaveRechargeLotterRecord();

	//重置重置抽奖信息
	void ResetLotteryInfo(bool bIsInit);

	//一次抽奖，返回抽奖结果
	int LotteryOneTime(int iLimitType);

	//增加抽奖记录
	void AddLotteryRecord(const std::string& strNickName, const std::vector<int>& vLotteryIDs);

	//重置
	void Reset();

private:

	//充值抽奖信息最近更新时间
	int m_iLastUpdateTime;

	//充值抽奖限量信息
	std::vector<LimitLotteryInfo> m_avLotteryInfo[LIMIT_LOTTERY_MAX];

	//充值抽奖记录
	std::vector<RechargeLotteryRecord> m_vLotteryRecord;
};
