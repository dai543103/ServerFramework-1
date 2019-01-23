#pragma once

#include "Kernel/GameRole.hpp"

//奖励工具类
class RewardConfig;
class CRewardUtility
{
public:
	
	//获取奖励
	static int GetReward(CGameRoleObj& stRoleObj, int iMultiNum, const RewardConfig* pstConfig, int iRewardNum, int iItemChannel);
};