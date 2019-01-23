
#include "GameProtocol.hpp"
#include "StringUtility.hpp"
#include "LogAdapter.hpp"
#include "Kernel/UnitUtility.hpp"
#include "Kernel/ConfigManager.hpp"
#include "Kernel/GameRole.hpp"
#include "Kernel/ZoneOssLog.hpp"
#include "FishpondObj.h"
#include "FishUtility.h"

#include "FishAlgorithm.h"

using namespace ServerLib;

//算法数据
//服务器算法数据
RoomAlgorithmData FishAlgorithm::astData[MAX_ROOM_ALGORITHM_TYPE];

//算法初始化
void FishAlgorithm::Initalize()
{
	for (int i = 0; i < MAX_ROOM_ALGORITHM_TYPE; ++i)
	{
		astData[i].lLastUpdateTime = CTimeUtility::GetMSTime();
		astData[i].iX = CConfigManager::Instance()->GetBaseCfgMgr().GetGlobalConfig(GLOBAL_TYPE_INITPUMPINGRATIO);
	}
}

void FishAlgorithm::OnTick()
{
	long lTimeNow = CTimeUtility::GetMSTime();

	for (int i = 0; i < MAX_ROOM_ALGORITHM_TYPE; ++i)
	{
		//更新服务器状态
		UpdateServerStat(lTimeNow, i);

		//更新大额红包状态
		UpdateBigReturnStat(lTimeNow, i);
	}
}

void FishAlgorithm::GetOneRedPacket(CGameRoleObj& stRoleObj, SeatUserData& stUserInfo, int iAlgorithmType)
{
	long lTimeNow = CTimeUtility::GetMSTime();
	int iRedPacketType = RETURN_TYPE_SILVERFLOW;

	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();
	int iServerUpdateTime = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_UPDATETIME) * 1000;

	RoomAlgorithmData& stData = astData[iAlgorithmType];
	if (stData.uReturnID == stUserInfo.uReturnID)
	{
		//本周期已经获得过红包
		return;
	}

	stUserInfo.iEffectCountdown = 0;
	stUserInfo.lUnEffectTime = 0;

	CGameRoleObj* pstRoleObj = CUnitUtility::GetRoleByUin(stUserInfo.uiUin);
	if (!pstRoleObj)
	{
		LOGERROR("Failed to get game role obj, uin %u\n", stUserInfo.uiUin);
		return;
	}

	bool bGetReturn = false;
	if (stData.aiReturnNum[RETURN_TYPE_SILVERFLOW]>0 && stUserInfo.lUserFlow[CYCLE_TIME_LAST] >= stData.lLastAverageFlow)
	{
		//可以获得高流水红包

		//只有在间隔的人才能领取红包
		if (stData.lReturnBulletInterval==0 || stData.lTotalBulletNum==0 || stData.lTotalBulletNum%stData.lReturnBulletInterval != 0)
		{
			return;
		}

		bGetReturn = true;
		stUserInfo.iReturnType = RETURN_TYPE_SILVERFLOW;
		iRedPacketType = RETURN_TYPE_SILVERFLOW;
		--stData.aiReturnNum[RETURN_TYPE_SILVERFLOW];
	}
	else if (stData.aiReturnNum[RETURN_TYPE_ONLINEENVELOPRATIO]>0 && (lTimeNow - pstRoleObj->GetLoginTime()) >= iServerUpdateTime)
	{
		//能获得在线红包

		//只有在间隔的人才能领取红包
		if (stData.lReturnBulletInterval == 0 || stData.lTotalBulletNum == 0 || stData.lTotalBulletNum%stData.lReturnBulletInterval != 0)
		{
			return;
		}

		bGetReturn = true;
		stUserInfo.iReturnType = RETURN_TYPE_ONLINEENVELOPRATIO;
		iRedPacketType = RETURN_TYPE_ONLINEENVELOPRATIO;
		--stData.aiReturnNum[RETURN_TYPE_ONLINEENVELOPRATIO];
	}
	else if (stData.aiReturnNum[RETURN_TYPE_LOSSPLAYERRATIO]>0 && stUserInfo.iLossNum>0)
	{
		//能获得亏损红包

		//只有在间隔的人才能领取红包
		if (stData.lReturnBulletInterval == 0 || stData.lTotalBulletNum == 0 || stData.lTotalBulletNum%stData.lReturnBulletInterval != 0)
		{
			return;
		}

		bGetReturn = true;
		stUserInfo.iReturnType = RETURN_TYPE_LOSSPLAYERRATIO;
		iRedPacketType = RETURN_TYPE_LOSSPLAYERRATIO;
		--stData.aiReturnNum[RETURN_TYPE_LOSSPLAYERRATIO];
	}
	else if (stData.aiReturnNum[RETURN_TYPE_RANDOMENVELOPRATIO]>0)
	{
		//能获得随机红包

		//只有在间隔的人才能领取红包
		if (stData.lReturnBulletInterval == 0 || stData.lTotalBulletNum == 0 || stData.lTotalBulletNum%stData.lReturnBulletInterval != 0)
		{
			return;
		}

		bGetReturn = true;
		stUserInfo.iReturnType = RETURN_TYPE_RANDOMENVELOPRATIO;
		iRedPacketType = RETURN_TYPE_RANDOMENVELOPRATIO;
		--stData.aiReturnNum[RETURN_TYPE_RANDOMENVELOPRATIO];
	}

	if (bGetReturn)
	{
		//获得了红包
		LOGDEBUG("GetType %d, Return Packet Num: %d|%d|%d|%d|%d\n", iRedPacketType, stData.aiReturnNum[0],
			stData.aiReturnNum[1], stData.aiReturnNum[2], stData.aiReturnNum[3],
			stData.aiReturnNum[4]);

		stUserInfo.uReturnID = stData.uReturnID;

		//打印红包运营日志
		CZoneOssLog::TraceGetRedPacket(stUserInfo.uiUin, stRoleObj.GetNickName(), iRedPacketType);

		//更新普通红包信息
		int iRandMax = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_MAXTIMELIMIT);
		int iRandMin = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_MINTIMELIMIT);
		stUserInfo.iEffectCountdown = (rand() % (iRandMax - iRandMin) + iRandMin) * 1000;

		//获取到新的红包
		stUserInfo.iLossNum = 0;
	}

	return;
}

//更新服务器状态
void FishAlgorithm::UpdateServerStat(long lTimeNow, int iAlgorithmType)
{
	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();
	int iServerUpdateTime = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_UPDATETIME) * 1000;

	if (lTimeNow < (astData[iAlgorithmType].lLastUpdateTime + iServerUpdateTime))
	{
		//未到时间
		return;
	}

	//更新结算时间
	astData[iAlgorithmType].lLastUpdateTime = lTimeNow;
	astData[iAlgorithmType].uReturnID = CFishUtility::GetReturnUniqID();
	astData[iAlgorithmType].uCycleID = CFishUtility::GetCycleUniqID();

	//读取配置的X1和X2,Y
	//int iX1ConfigValue = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_MINPUMPINGRATIO);	//不分房间
	int iX1ConfigValue = stBaseCfgMgr.GetRoomX1Config(iAlgorithmType);	//分房间
	int iX2ConfigValue = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_INITPUMPINGRATIO);
	int iYConfigValue = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_CHOUSHUIMODIFY);

	//先计算服务器盈利
	long8 lServerWinNum = astData[iAlgorithmType].lRewardSilver - (astData[iAlgorithmType].lCostSilver - astData[iAlgorithmType].lUsedReturnSilver);
	
	//打印服务器 周期收益情况 运营日志
	//计算抽水
	int iPump = 0;
	if (astData[iAlgorithmType].lRewardSilver != 0)
	{
		iPump = lServerWinNum / astData[iAlgorithmType].lRewardSilver * 1000;
	}

	//计算平均炮台倍数
	int iAverageGunMultiple = 0;
	if (astData[iAlgorithmType].lTotalBulletNum != 0)
	{
		iAverageGunMultiple = astData[iAlgorithmType].lRewardSilver / astData[iAlgorithmType].lTotalBulletNum;
	}
	
	if (lServerWinNum < 0)
	{
		//计入服务器亏损
		astData[iAlgorithmType].lTotalServerLossNum -= lServerWinNum;

		//不发红包
	}
	else
	{
		if (astData[iAlgorithmType].iX == iX2ConfigValue)
		{
			//If X=X2，红包数量=G*（X2-X1）/X2，K=0

			//亏损清0
			astData[iAlgorithmType].lTotalServerLossNum = 0;

			//计算红包 红包数量=G*（X2-X1）/X2
			astData[iAlgorithmType].lTotalReturnSilver += lServerWinNum*(iX2ConfigValue - iX1ConfigValue) / iX2ConfigValue;
		}
		else
		{
			//红包数量=G*（X2-X1）/（X2+10%），K=K-G*10%/（X2+10%）

			astData[iAlgorithmType].lTotalServerLossNum -= lServerWinNum*iYConfigValue / (iX2ConfigValue + iYConfigValue);
			astData[iAlgorithmType].lTotalServerLossNum = astData[iAlgorithmType].lTotalServerLossNum > 0 ? astData[iAlgorithmType].lTotalServerLossNum : 0;
			astData[iAlgorithmType].lTotalReturnSilver += lServerWinNum*(iX2ConfigValue - iX1ConfigValue) / (iX2ConfigValue + iYConfigValue);
		}
	}

	//处理点券鱼流水返利
	const int iTicketFishReturnRate = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_TICKETRETURNRATE);
	long8 lTicketFishReturnNum = 0;
	if (astData[iAlgorithmType].lTicketFishRewardSilver > (astData[iAlgorithmType].lTicketFishDropNum*iTicketFishReturnRate))
	{
		lTicketFishReturnNum = (astData[iAlgorithmType].lTicketFishRewardSilver - (astData[iAlgorithmType].lTicketFishDropNum*iTicketFishReturnRate));
		astData[iAlgorithmType].lTotalReturnSilver += lTicketFishReturnNum;
	}

	//If K>0时，抽水比例X = X2 + 10 % ; If K = 0 抽水比例X = X2
	if (astData[iAlgorithmType].lTotalServerLossNum > 0)
	{
		astData[iAlgorithmType].iX = iX2ConfigValue + iYConfigValue;
	}
	else
	{
		astData[iAlgorithmType].iX = iX2ConfigValue;
	}

	//3.发放产生的红包

	//红包发放总人数比例
	int iReturnRatio = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_ENVELOPERATIO);

	//发放红包的人数
	long8 lReturnPlayerNum = astData[iAlgorithmType].lPlayingNum;

	//单个红包个数
	if (lReturnPlayerNum > 0 && astData[iAlgorithmType].lTotalReturnSilver > 0)
	{
		//按比例分配红包
		astData[iAlgorithmType].aiReturnNum[RETURN_TYPE_SILVERFLOW] = lReturnPlayerNum * iReturnRatio * stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_GUNMULTIPLE) / (1000 * 1000);
		astData[iAlgorithmType].aiReturnNum[RETURN_TYPE_ONLINEENVELOPRATIO] = lReturnPlayerNum * iReturnRatio * stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_ONLINEENVELOPRATIO) / (1000 * 1000);
		astData[iAlgorithmType].aiReturnNum[RETURN_TYPE_LOSSPLAYERRATIO] = lReturnPlayerNum * iReturnRatio * stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_LOSSPLAYERRATIO) / (1000 * 1000);
		astData[iAlgorithmType].aiReturnNum[RETURN_TYPE_RANDOMENVELOPRATIO] = lReturnPlayerNum * iReturnRatio * stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_RANDOMENVELOPRATIO) / (1000 * 1000);
	}

	//最少发2个
	if (astData[iAlgorithmType].aiReturnNum[RETURN_TYPE_RANDOMENVELOPRATIO] <= 1)
	{
		astData[iAlgorithmType].aiReturnNum[RETURN_TYPE_RANDOMENVELOPRATIO] = 2;
	}

	//4.计算服务器状态

	//计算服务器收益比
	astData[iAlgorithmType].iServerX = 0;
	astData[iAlgorithmType].lServerWinInventory = (astData[iAlgorithmType].lTotalRewardSilver - astData[iAlgorithmType].lTotalCostSliver + astData[iAlgorithmType].lTotalNewRedUseNum);
	long8 lTotalUserCost = 0;
	if (astData[iAlgorithmType].lPlayingNum > 0)
	{
		lTotalUserCost = astData[iAlgorithmType].lTotalBulletNum * astData[iAlgorithmType].lTotalGunMultiple * 
			stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_SILVERPARAM) / (astData[iAlgorithmType].lPlayingNum * 1000);
	}
	
	long lInitWinSilver = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_INITSILVER);
	astData[iAlgorithmType].lDynamicInventory = (lTotalUserCost >= lInitWinSilver) ? lTotalUserCost : lInitWinSilver;
	long8 lServerRatio = 0;
	if (astData[iAlgorithmType].lDynamicInventory > 0)
	{
		lServerRatio = (astData[iAlgorithmType].lServerWinInventory + astData[iAlgorithmType].lDynamicInventory) * 1000 / astData[iAlgorithmType].lDynamicInventory;
	}

	const ServerStatConfig* pstStatConfig = stBaseCfgMgr.GetServerStatConfig(lServerRatio);
	if (pstStatConfig)
	{
		astData[iAlgorithmType].iServerX = pstStatConfig->iDecrease;
	}

	//更新上个周期的服务器平均流水
	astData[iAlgorithmType].lLastAverageFlow = astData[iAlgorithmType].lPlayingNum>0 ? (astData[iAlgorithmType].lRewardSilver / astData[iAlgorithmType].lPlayingNum) : 0;

	//计算红包发放子弹间隔
	astData[iAlgorithmType].lReturnBulletInterval = astData[iAlgorithmType].lTotalBulletNum*stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_RETURNALLTIME) /
		(stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_UPDATETIME) * 1000);

	//更新多倍鱼亏损池
	astData[iAlgorithmType].lMultiFishLossNum += (astData[iAlgorithmType].lMultiFishFlow - astData[iAlgorithmType].lMultiFishHitFlow);
	if (astData[iAlgorithmType].lMultiFishLossNum > 0)
	{
		astData[iAlgorithmType].lMultiFishLossNum = 0;
	}

	//打印运营日志
	CZoneOssLog::TraceServerCycleProfit(iAlgorithmType, astData[iAlgorithmType].lRewardSilver,
		astData[iAlgorithmType].lCostSilver, lServerWinNum, astData[iAlgorithmType].lTicketFishDropNum,
		iPump, astData[iAlgorithmType].lPlayingNum, iAverageGunMultiple,
		astData[iAlgorithmType].lTotalReturnSilver, lTicketFishReturnNum, astData[iAlgorithmType].lLastUpdateTime+ iServerUpdateTime,
		astData[iAlgorithmType].aiReturnNum[RETURN_TYPE_RANDOMENVELOPRATIO], astData[iAlgorithmType].lUsedReturnSilver);

	astData[iAlgorithmType].lCostSilver = 0;
	astData[iAlgorithmType].lRewardSilver = 0;
	astData[iAlgorithmType].lTotalBulletNum = 0;
	astData[iAlgorithmType].lUsedReturnSilver = 0;
	astData[iAlgorithmType].lTicketFishRewardSilver = 0;
	astData[iAlgorithmType].lTicketFishDropNum = 0;
	astData[iAlgorithmType].lMultiFishFlow = 0;
	astData[iAlgorithmType].lMultiFishHitFlow = 0;

	return;
}

//更新大额红包状态
void FishAlgorithm::UpdateBigReturnStat(long lTimeNow, int iAlgorithmType)
{
	//先判断是否隔天
	if (!CFishUtility::IsSameDay(lTimeNow, astData[iAlgorithmType].lBigReturnUpdateTime) && astData[iAlgorithmType].lBigReturnUpdateTime != 0)
	{
		astData[iAlgorithmType].lTodayCostSliver = 0;
		astData[iAlgorithmType].lTodayRewardSilver = 0;
		astData[iAlgorithmType].lTodayBigReturnSilver = 0;

		return;
	}
	
	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();
	int iBigReturnUpdateTime = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_UPDATETIME) * 1000;
	if (lTimeNow < (astData[iAlgorithmType].lBigReturnUpdateTime + iBigReturnUpdateTime))
	{
		//未到时间
		return;
	}

	astData[iAlgorithmType].lBigReturnUpdateTime = lTimeNow;
	
	//int iX1ConfigValue = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_MINPUMPINGRATIO);	//不分房间
	int iX1ConfigValue = stBaseCfgMgr.GetRoomX1Config(iAlgorithmType);	//分房间

	//计算额外盈利
	long8 lExtraWinSilver = astData[iAlgorithmType].lTodayRewardSilver*(1000 - iX1ConfigValue) / 1000 - astData[iAlgorithmType].lTodayCostSliver
		- astData[iAlgorithmType].lTodayBigReturnSilver - astData[iAlgorithmType].lTotalReturnSilver;
	if (lExtraWinSilver <= 0)
	{
		//不满足发放要求
		return;
	}

	astData[iAlgorithmType].lTodayBigReturnSilver += lExtraWinSilver;
	astData[iAlgorithmType].lTotalReturnSilver += lExtraWinSilver;

	return;
}
