#pragma once

#include <string.h>
#include "CommDefine.h"

//捕鱼算法定义

//红包类型
enum ReturnType
{
	RETURN_TYPE_SILVERFLOW = 0,			//发炮流水较高的人
	RETURN_TYPE_ONLINEENVELOPRATIO = 1,	//上个时间段持续在线的人
	RETURN_TYPE_LOSSPLAYERRATIO = 2,	//上个时间段亏损的人
	RETURN_TYPE_RANDOMENVELOPRATIO = 3,	//随机获取的人
	RETURN_TYPE_BIGBOSS = 5,			//BOSS战类型
	RETURN_TYPE_MAX = 6,				//类型最大
};

struct RoomAlgorithmData
{
	//上次结算时间
	long lLastUpdateTime;

	//上次大额红包结算时间
	long lBigReturnUpdateTime;

	//当前结算周期ID
	unsigned uCycleID;

	//服务器总金币消耗
	long8 lTotalCostSliver;

	//服务器总金币收益
	long8 lTotalRewardSilver;

	//服务器总新手红包使用量
	long8 lTotalNewRedUseNum;

	//服务器金币消耗
	long8 lCostSilver;

	//服务器金币收益
	long8 lRewardSilver;

	//该算法类型当前玩家数
	long8 lPlayingNum;

	//服务器上个周期平均流水
	long8 lLastAverageFlow;

	//服务器当前炮台总倍数
	long8 lTotalGunMultiple;

	//服务器当前发射子弹总数目
	long8 lTotalBulletNum;

	//红包发放子弹间隔
	long8 lReturnBulletInterval;

	//上个结算周期的X
	int iX;

	//上个结算周期服务器的状态
	int iServerX;

	//服务器盈利库存
	long8 lServerWinInventory;

	//服务器动态库存
	long8 lDynamicInventory;

	//总的红包发放金额
	long8 lTotalReturnSilver;

	//周期红包使用金额
	long8 lUsedReturnSilver;

	//服务器当日金币消耗
	long8 lTodayCostSliver;

	//服务器当日金币收益
	long8 lTodayRewardSilver;

	//服务器当日已发放大额红包
	long8 lTodayBigReturnSilver;

	//当前红包ID
	unsigned uReturnID;

	//每种类型红包个数
	int aiReturnNum[RETURN_TYPE_MAX];

	//服务器累计亏损值
	long8 lTotalServerLossNum;

	//周期点券鱼服务器收益
	long8 lTicketFishRewardSilver;

	//周期点券鱼掉落鱼票数量
	long8 lTicketFishDropNum;

	//周期多倍鱼发炮流水
	long8 lMultiFishFlow;

	//周期多倍鱼命中流水
	long8 lMultiFishHitFlow;

	//周期多倍鱼亏损池
	long8 lMultiFishLossNum;

	RoomAlgorithmData()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

//捕鱼算法
struct SeatUserData;
class FishAlgorithm
{
public:

	//算法初始化
	static void Initalize();

	//算法定时器
	static void OnTick();

	static void GetOneRedPacket(CGameRoleObj& stRoleObj, SeatUserData& stUserInfo, int iAlgorithmType);

private:

	//更新服务器状态
	static void UpdateServerStat(long lTimeNow, int iAlgorithmType);

	//更新大额红包状态
	static void UpdateBigReturnStat(long lTimeNow, int iAlgorithmType);

public:

	//服务器算法数据
	static RoomAlgorithmData astData[MAX_ROOM_ALGORITHM_TYPE];
};