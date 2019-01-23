#pragma once

#include <vector>

#include "GameProtocol.hpp"
#include "CommDefine.h"
#include "Kernel/GameRole.hpp"

//玩家下注信息
struct UserBetData
{
	unsigned uin;	//玩家uin
	std::vector<LasvegasBetData> vBets;

	UserBetData()
	{
		Reset();
	}

	bool operator== (const UserBetData& stData)
	{
		return (this->uin == stData.uin);
	}

	void Reset()
	{
		uin = 0;
		vBets.clear();
	}
};

class CLasvegasManager
{
public:
	static CLasvegasManager* Instance();

	~CLasvegasManager();

	//初始化
	void Init();

	//玩家进出转盘
	void EnterLasvegas(CGameRoleObj& stRoleObj, bool bIsEnter);

	//玩家下注
	int BetLasvegas(CGameRoleObj& stRoleObj, int iNumber, int iBetCoins);

	//更新转盘信息
	void UpdateLasvegasInfo(const World_UpdateLasvegas_Notify& stNotify);

	//拉取转盘中奖信息
	int GetRewardInfo(CGameRoleObj& stRoleObj, Zone_GetRewardInfo_Response& stResp, int iFrom, int iNum);

	//定时器
	void OnTick();

private:
	CLasvegasManager();

	//发放奖励
	void SendLasvegasReward();

	//推送转盘信息
	void SendLasvegasInfoNotify(CGameRoleObj* pstRoleObj);

	//更新World下注信息
	void SendUpdateBetInfoToWorld(int iNumber, int iBetCoins);

	//打包转盘信息,不包括中奖纪录
	void PackLasvegasInfo(LasvegasInfo& stInfo);

	void Reset();

private:

	//当前阶段
	int m_iStepType;

	//当前阶段结束时间
	int m_iStepEndTime;

	//是否推送更新给玩家
	bool m_bSendUserNotify;

	//最近推送更新给玩家的时间
	int m_iLastSendNotifyTime;

	//是否需要发放奖励
	bool m_bNeedSendReward;

	//开奖发放奖励时间
	int m_iLotteryRewardTime;

	//本次开奖信息
	int m_iLotteryID;

	//最近开奖信息
	std::vector<int> m_vLotteryIDs;

	//最近中奖纪录
	std::vector<LotteryPrizeData> m_vPrizeDatas;

	//当前下注信息
	std::vector<LasvegasBetData> m_vBetDatas;

	//转盘玩家列表
	std::vector<UserBetData> m_vUserList;
};
