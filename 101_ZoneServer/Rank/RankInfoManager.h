#pragma once

#include "GameProtocol.hpp"
#include "CommDefine.h"
#include "Kernel/GameRole.hpp"

class CRankInfoManager
{
public:
	static CRankInfoManager* Instance();

	~CRankInfoManager();

	//初始化
	void Init();

	//拉取排行信息
	int GetRankInfo(unsigned uin, int iType, int iFromRank, int iNum, bool bLastRank, Zone_GetRankInfo_Response& stResp);

	//更新排行信息
	void UpdateRoleRank(CGameRoleObj& stRoleObj);

	//设置排行信息
	int SetRankListInfo(const World_GetRankInfo_Response& stResp);

	//定时器
	void OnTick();

private:
	CRankInfoManager();

	//更新排行信息
	void UpdateRankByType(int iType, const RankData& stData, bool bUpdateRank);

	//拉取排行榜信息
	void GetRankInfoFromWorld(int iType);

	//打包排行信息
	void PackRankInfo(unsigned uin, int iFromRank, int iNum, const std::vector<RankData>& vRankDatas, Zone_GetRankInfo_Response& stResp);

	//是否在排行中
	bool IsInRank(int iType, const RankData& stData);

	void Reset();

private:

	//上次排名更新时间
	int m_iLastUpdateTime;

	//排行榜信息
	RankList m_astRankLists[RANK_TYPE_MAX];
};
