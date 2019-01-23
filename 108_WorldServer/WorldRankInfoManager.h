#pragma once

#include <vector>
#include "GameProtocol.hpp"
#include "CommDefine.h"

class CWorldRankInfoManager
{
public:
	static CWorldRankInfoManager* Instance();

	~CWorldRankInfoManager();

	//初始化RankInfo
	void Init();

	//拉取排行信息
	int GetWorldRank(int iRankType, unsigned uVersionID, World_GetRankInfo_Response& stResp);

	//更新排行信息
	int UpdateWorldRank(int iRankType, const RankInfo& stRankInfo);

	//定时器
	void OnTick(int iTimeNow);

private:

	CWorldRankInfoManager();

	//设置RankData
	void SetRankData(RankData& stData, const RankInfo& stInfo);

	//发放周榜奖励
	void SendRankReward(int iRankType, int iTimeNow);

	//加载Rank信息
	void LoadRankInfo();

	//保存Rank信息
	void SaveRankInfo();

	//重置
	void Reset();

private:

	//排行列表
	RankList m_astRankLists[RANK_TYPE_MAX];
};
