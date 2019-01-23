#pragma once

#include <string>
#include <vector>
#include "GameProtocol.hpp"
#include "CommDefine.h"

class CWorldLasvegasManager
{
public:
	static CWorldLasvegasManager* Instance();

	~CWorldLasvegasManager();

	//初始化Lasvegas
	void Init();

	//更新数字下注信息
	void UpdateBetNumber(int iNumber, int iBetCoins);

	//更新中奖信息
	void UpdatePrizeInfo(const World_UpdatePrizeInfo_Request& stReq);

	//定时器
	void OnTick(int iTimeNow);

private:

	CWorldLasvegasManager();

	//加载Lasvegas信息
	void LoadLasvegasInfo();

	//保存Lasvegas信息
	void SaveLasvegasInfo();

	//推送大转盘信息
	void SendUpdateLasvegasNotify(int iTimeNow);

	//重置
	void Reset();

private:

	//当前阶段
	int m_iStepType;

	//当前阶段结束时间
	int m_iStepEndTime;

	//是否需要推送更新
	bool m_bSendUpdate;

	//上次更新推送时间
	int m_iLastUpdateTime;

	//最近开奖信息
	std::vector<int> m_vLotteryIDs;

	//最近中奖纪录
	std::vector<LotteryPrizeData> m_vPrizeDatas;

	//当前下注信息
	std::vector<LasvegasBetData> m_vBetDatas;
};
