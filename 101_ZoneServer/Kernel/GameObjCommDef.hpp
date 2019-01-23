#pragma once

//本文件中主要定义游戏中使用的单位和角色的基本数据结构

#include "GameProtocol.hpp"
#include "CommDefine.h"

//场景上单位的定义信息
struct TUNITINFO
{
    int iUnitID;                    //场景单位的ID
    unsigned int uiUnitStatus;      //场景单位当前的状态

    TUNITINFO()
    {
        memset(this, 0, sizeof(*this));
    };
};

//玩家炮台信息
struct TWEAPONINFO
{
	int iWeaponID;				//玩家炮台ID
	int iStyleID;				//玩家炮台样式ID
	int aiUnlockStyleIDs[16];	//已解锁炮台ID

	TWEAPONINFO()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

//玩家体验线信息
struct TEXPLINEINFO
{
	int iExpLineID;			//体验线ID
	int iExpLineType;		//体验线类型
	long8 lUserWinNum;		//体验线内收益
	long8 lIntervalWinNum;	//体验线内阶段收益
	int iBulletNum;			//体验线内发射子弹数量
	long8 lCostNum;			//体验线内发射子弹消耗

	TEXPLINEINFO()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

//玩家积分信息
struct TSCOREINFO
{
	int iLastScoreUpdate;		//上次积分更新时间
	long8 lDayScore;			//当日积分
	long8 lWeekScore;			//本周积分

	TSCOREINFO()
	{
		memset(this, 0, sizeof(*this));
	}
};

//玩家结算信息
struct TROLETALLYINFO 
{
	long8 alResource[RESOURCE_TYPE_MAX];	//上次结算资源数量
	int aiBindWarheadNum[WARHEAD_TYPE_MAX];	//上次结算绑定弹头数量
	int aiWarheadNum[WARHEAD_TYPE_MAX];		//上次结算非绑定弹头数量
	int aiSendGiftNum[WARHEAD_TYPE_MAX];	//本周期赠送非绑定弹头数量
	int aiRecvGiftNum[WARHEAD_TYPE_MAX];	//本周期接收非绑定弹头数量
	int iRechargeDiamonds;					//本周期玩家充值钻石数量
	long8 lCashTicketNum;					//本周期玩家套现鱼票价值，包括实物和话费道具
	long8 lUserWinNum;						//本周期玩家实际输赢
	bool bNeedLog;							//本周期是否需要记录

	TROLETALLYINFO()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

//玩家角色基本信息结构定义
struct TROLEBASEINFO
{
    char szNickName[MAX_NICK_NAME_LENGTH];  //玩家的名字
    int iLastLogin;							//玩家上次登录的时间
    int iLastLogout;						//玩家上次登出游戏的时间
    int iCreateTime;						//玩家帐号创建的时间
    int iOnlineTime;						//玩家的总在线时长
    int iLoginCount;						//玩家总的登录次数
    int iLoginTime;							//玩家本次登录的时间
    int iLogoutTime;						//玩家本次登出游戏的时间
	TWEAPONINFO stWeaponInfo;				//玩家使用炮台信息
	long8 alResource[RESOURCE_TYPE_MAX];	//玩家的资源信息
	int iVIPLevel;							//玩家VIP等级
	long8 lVIPExp;							//玩家VIP经验
	int iVIPPriv;							//玩家VIP特权
	int iLotteryNum;						//玩家当天已抽奖次数
	unsigned uTableID;						//玩家所在捕鱼桌子ID
	TEXPLINEINFO astExpLineInfo[MAX_ROOM_ALGORITHM_TYPE];//玩家体验线信息	
	TSCOREINFO stScoreInfo;					//玩家积分信息
	int iNextAlmsTime;						//救济金下次可领取时间
	int iAlmsNum;							//玩家当天救济金领取次数
	int iLastAlmsUpdateTime;				//玩家上次救济金信息更新时间
	int aiLotteryIDs[MAX_PAYLOTTERY_NUM];	//玩家充值抽奖记录
	int iLoginDays;							//玩家累计登录天数，不连续
	int iGetLoginReward;					//玩家登陆奖励是否领取，按位计算
	int iRemainNewRedNum;					//玩家剩余新手红包额度
	int iNowNewRedNum;						//玩家当前可用新手红包额度
	int iMonthEndTime;						//玩家月卡到期时间
	int iLastMonthTime;						//玩家上次领取月卡礼包时间
	char szChannel[32];						//玩家的渠道号
	char szPicID[256];						//玩家头像ID
	bool bUpdateRank;						//是否强制更新排行榜
	bool bGetVIPCoins;						//玩家是否获得VIP金币补足
	int iRealNameStat;						//玩家实名状态
	int iDayOnlineTime;						//玩家当天在线时长
	int iLastOnlineUpdateTime;				//玩家在线时长最近更新时间
	unsigned uFinGuideIndexes;				//玩家完成新手引导ID,位运算表示
	bool bNameChanged;						//玩家是否修改过名字
	char szAccount[128];					//玩家的账号
	char szDeviceID[128];					//玩家的设备号
	TROLETALLYINFO stTallyInfo;				//玩家结算信息
	long8 lUserWinNum;						//玩家实际输赢
	char szSign[128];						//玩家游戏签名

    TROLEBASEINFO()
    {
        memset(this, 0, sizeof(*this));
    };
};

//角色的数据信息
struct TROLEINFO
{
    RoleID stRoleID;            //角色ID
    TROLEBASEINFO stBaseInfo;   //角色的基础信息
    TUNITINFO stUnitInfo;       //角色的Unit单位信息
};
