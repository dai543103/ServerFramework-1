
#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "ErrorNumDef.hpp"
#include "Kernel/ConfigManager.hpp"
#include "RepThings/RepThingsUtility.hpp"
#include "Resource/ResourceUtility.h"

#include "RewardUtility.h"

using namespace ServerLib;

//获取奖励
int CRewardUtility::GetReward(CGameRoleObj& stRoleObj, int iMultiNum, const RewardConfig* pstConfig, int iRewardNum, int iItemChannel)
{
	if (!pstConfig || iRewardNum <= 0 || iMultiNum<=0)
	{
		return T_ZONE_PARA_ERROR;
	}

	for (int i = 0; i < iRewardNum; ++i)
	{
		if (pstConfig[i].iType == 0)
		{
			break;
		}

		//增加兑换获得
		switch (pstConfig[i].iType)
		{
		case REWARD_TYPE_RES:
		{
			//兑换获得资源
			if (!CResourceUtility::AddUserRes(stRoleObj, pstConfig[i].iRewardID, pstConfig[i].iRewardNum*iMultiNum))
			{
				LOGERROR("Failed to add reward resource, uin %u, reward id %d, num %d\n", stRoleObj.GetUin(), pstConfig[i].iRewardID, pstConfig[i].iRewardNum*iMultiNum);
				return T_ZONE_PARA_ERROR;
			}
		}
		break;

		case REWARD_TYPE_ITEM:
		{
			//兑换获得道具
			int iRet = CRepThingsUtility::AddItemNum(stRoleObj, pstConfig[i].iRewardID, pstConfig[i].iRewardNum*iMultiNum, iItemChannel);
			if (iRet)
			{
				LOGERROR("Failed to add reward item, uin %u, reward id %d, num %d\n", stRoleObj.GetUin(), pstConfig[i].iRewardID, pstConfig[i].iRewardNum*iMultiNum);
				return iRet;
			}
		}
		break;

		case REWARD_TYPE_ENTITY:
		{
			//兑换获得实物
			;
		}
		break;

		default:
			break;
		}
	}

	return T_SERVER_SUCCESS;
};