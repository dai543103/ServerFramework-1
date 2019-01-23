#pragma once

#include <vector>
#include <map>
#include <string.h>
#include <string>
#include <stdint.h>
#include "json/json.h"
#include "CommDefine.h"
#include "MaskWordConfig.hpp"

//全局配置的类型定义
enum GlobalType
{
	GLOBAL_TYPE_INVALID = 0,			//非法
	GLOBAL_TYPE_SHOOTSPEED = 1,			//子弹射速
	GLOBAL_TYPE_BULLETSPEED = 3,		//子弹移动速度
	GLOBAL_TYPE_MAXBULLETNUM = 4,		//单个玩家最多存在子弹数量
	GLOBAL_TYPE_SERVERLOSS = 5,			//补差鱼服务器金币亏损数量
	GLOBAL_TYPE_ENVELOPERATIO = 6,		//红包个数占整体在线人数比例
	GLOBAL_TYPE_INITPUMPINGRATIO = 7,	//初始抽水比例
	GLOBAL_TYPE_MINPUMPINGRATIO = 8,	//最低营收抽水比例
	GLOBAL_TYPE_ONLINEENVELOPRATIO = 9,	//在线占红包比例
	GLOBAL_TYPE_GUNMULTIPLE = 10,		//平均炮台倍数较高占红包比例
	GLOBAL_TYPE_LOSSPLAYERRATIO = 11,	//亏损玩家占红包比例
	GLOBAL_TYPE_RANDOMENVELOPRATIO = 12,	//随机抽取占红包比例
	GLOBAL_TYPE_MAXTIMELIMIT = 13,		//红包生效缓冲时间上限
	GLOBAL_TYPE_MINTIMELIMIT = 14,		//红包生效缓冲时间下限
	GLOBAL_TYPE_TIMEDURATION = 15,		//红包持续时间
	GLOBAL_TYPE_RATIOBONUS = 16,		//红包概率加成
	GLOBAL_TYPE_GAMETIME = 17,			//单挑场时间
	GLOBAL_TYPE_INITSILVER = 18,		//初始库存数量
	GLOBAL_TYPE_SILVERMAXRATE = 19,		//库存上限占盈利的百分比
	GLOBAL_TYPE_SILVERPARAM = 20,		//库存计算系数值
	GLOBAL_TYPE_OUTFORMMIN = 21,		//鱼阵出现的时间下限
	GLOBAL_TYPE_OUTFORMMAX = 22,		//鱼阵出现的时间上限
	GLOBAL_TYPE_UPDATETIME = 24,		//服务器状态更新时间
	GLOBAL_TYPE_CHOUSHUIMODIFY = 25,	//服务器抽水修正值
	GLOBAL_TYPE_BFRETURNINTERVAL = 27,	//边锋平台发放低倍红包时间间隔
	GLOBAL_TYPE_BFRETURNSILVER = 28,	//每次发放的红包总金额
	GLOBAL_TYPE_BFTIMEDURATION = 29,	//边锋特殊红包持续时间
	GLOBAL_TYPE_BFRETURNNUM = 30,		//每次发放红包的人数占比,100-500倍炮玩家可领
	GLOBAL_TYPE_BOSSINITSR = 31,		//BOSS初始携带的银子数量
	GLOBAL_TYPE_BETTINGSR = 32,			//BOSS战下注金额
	GLOBAL_TYPE_BOSSTIME = 33,			//打BOSS持续时间
	GLOBAL_TYPE_WAITINGTIME = 34,		//BOSS打爆后的等待时间
	GLOBAL_TYPE_WILDBULLETNUM = 35,		//BOSS战狂暴触发命中子弹数
	GLOBAL_TYPE_BOSSADDRATIO = 36,		//火力全开提升的概率
	GLOBAL_TYPE_BETTINGTIME = 37,		//BOSS战下注时间
	GLOBAL_TYPE_HORSELAMPSILVER = 38,	//显示走马灯的金额
	GLOBAL_TYPE_PHONETICKETRATE = 39,	//普通鱼掉落奖券的概率
	GLOBAL_TYPE_TICKETRETURNRATE = 40,	//点券鱼流水返奖系数
	GLOBAL_TYPE_HIGHREDPACKETRATIO = 41,//高倍场红包个数占整体在线人数比例
	GLOBAL_TYPE_EXPEINITSILVER = 42,    //体验场初始金币
	GLOBAL_TYPE_DAILYRESETTIME = 47,    //日常任务每日更新时间
	GLOBAL_TYPE_ADVENTRESETTIME = 48,   //奇遇任务每日更新时间
	GLOBAL_TYPE_ADVENTBULLETMIN = 49,	//奇遇任务触发子弹数最小
	GLOBAL_TYPE_ADVENTBULLETMAX = 50,	//奇遇任务触发子弹数最大
	GLOBAL_TYPE_SECONDADVENTRATE = 51,  //第二次奇遇任务触发概率
	GLOBAL_TYPE_ADVENTURENUM = 52,		//一天内奇遇任务的次数
	GLOBAL_TYPE_EXPRANKNUM = 53,		//体验线收益排行
	GLOBAL_TYPE_EXPWINNUM = 54,			//体验线收益阈值
	GLOBAL_TYPE_DAYWINNUM = 55,			//每天收益阈值
	GLOBAL_TYPE_NEWREDNUM = 57,			//新手红包总额度
	GLOBAL_TYPE_NEWADDRATE = 58,		//新手红包命中概率加成
	GLOBAL_TYPE_LASVEGAS_BETTIME = 59,	//大转盘下注时间
	GLOBAL_TYPE_RECHARGELOTTERY = 60,	//一次充值抽奖需要的充值金额
	GLOBAL_TYPE_LASVEGAS_REWARDNUM = 61,//大转盘中奖上榜最小金额
	GLOBAL_TYPE_LASVEGAS_LOTTERYTIME = 62,//大转盘开奖时间
	GLOBAL_TYPE_LASVEGAS_REWARDTIME = 63,//大转盘结算倒计时，从下注结束开始计算
	GLOBAL_TYPE_RECHARGE_BOXID = 64,	//首冲礼包ID
	GLOBAL_TYPE_MONTHCARD_BOXID = 65,	//月卡礼包ID
	GLOBAL_TYPE_NEWREMAINCOINS = 66,	//新手红包生效额度
	GLOBAL_TYPE_NEWREDTIMES = 67,		//新手红包使用次数
	GLOBAL_TYPE_WALLOWSWITCH = 69,		//防沉迷开关，1表示开启防沉迷，0表示关闭
	GLOBAL_TYPE_TICKETLOTTERYCOST = 70,	//鱼票抽奖的消耗
	GLOBAL_TYPE_USEITEM_VIPLIMIT = 71,	//使用道具的VIP等级限制
	GLOBAL_TYPE_TENBILL_LIMIT = 72,		//十元话费每个账号每天限制兑换次数
	GLOBAL_TYPE_CMCCLIMIT = 73,			//移动话费兑换限制开关，为1限制兑换，0不限制
	GLOBAL_TYPE_MULTIPLEFISH = 74,		//多倍鱼命中倍数系数
	GLOBAL_TYPE_RETURNALLTIME = 75,		//周期红包发放完成的时间，单位ms
	GLOBAL_TYPE_MULTIPLEFISH_ADD = 76,	//多倍鱼命中倍数加成
	GLOBAL_TYPE_BINDBRONZE = 77,		//绑定青铜弹头ID
	GLOBAL_TYPE_BINDSILVER = 78,		//绑定白银弹头ID
	GLOBAL_TYPE_BINDGOLD = 79,			//绑定黄金弹头ID
	GLOBAL_TYPE_NORMALBRONZE = 80,		//非绑定青铜弹头ID
	GLOBAL_TYPE_NORMALSILVER = 81,		//非绑定白银弹头ID
	GLOBAL_TYPE_NORMALGOLD = 82,		//非绑定黄金弹头ID
	GLOBAL_TYPE_TICKTOCOIN = 83,		//鱼票兑换金币汇率
	GLOBAL_TYPE_DIAMONDTOCOIN = 84,		//钻石兑换金币汇率
	GLOBAL_TYPE_SERVERVER = 8888,		//服务器版本号，强制客户端更新版本
	GLOBAL_TYPE_PLATFORM = 9999,		//服务器平台类型

	GLOBAL_TYPE_MAX,
};

//初始化配表类型定义
enum BirthInitType
{
	BIRTH_INIT_INVALID	= 0,			//非法的类型
	BIRTH_INIT_COIN		= 1,			//初始化金币
	BIRTH_INIT_WEAPON	= 2,			//初始化炮台ID
	BIRTH_INIT_VIPLEVEL = 3,			//初始VIP等级
	BIRTH_INIT_EXPTYPE	= 4,			//初始体验线类型
	BIRTH_INIT_PICID	= 5,			//初始玩家头像
	BIRTH_INIT_WEAPONSTYLE = 6,			//初始化炮台样式ID
	BIRTH_INIT_MONTHCARD= 7,			//初始化体验月卡ID

	BIRTH_INIT_MAX,
};

//捕鱼房间类型定义
enum FishRoomType
{
	FISH_ROOM_INVALID		= 0x0000,	//非法的房间类型
	FISH_ROOM_NORMAL		= 0x0001,	//经典场低倍场
	FISH_ROOM_MIDNORMAL		= 0x0002,	//经典场中倍场
	FISH_ROOM_HIGHNORMAL	= 0x0004,	//经典场高倍场
	FISH_ROOM_WARHEADLOW	= 0x0008,	//弹头低倍场
	FISH_ROOM_WARHEADMID	= 0x0010,	//弹头中倍场
	FISH_ROOM_WARHEADHIGH	= 0x0020,	//弹头高倍场
	FISH_ROOM_SUPERNORMAL	= 0x0040,	//经典场超高倍场
	
	FISH_ROOM_MAX,
};

//捕鱼房间模式定义
enum RoomPatternType
{
	ROOM_PATTERN_INVALID	= 0,		//非法的模式
	ROOM_PATTERN_NORMAL		= 1,		//经典模式
	ROOM_PATTERN_MATCH		= 2,		//比赛竞技模式
	ROOM_PATTERN_WARHEAD	= 3,		//弹头模式
	ROOM_PATTERN_CRYSTAL	= 4,		//水晶宫模式
};

//抽奖奖项类型
enum LotteryItemType
{
	LOTTERY_ITEM_INVALID	= 0,		//非法的类型
	LOTTERY_ITEM_COINS		= 1,		//抽奖金币
	LOTTERY_ITEM_TICKET		= 2,		//抽奖鱼票
	LOTTERY_ITEM_PROP		= 3,		//抽奖道具
};

//任务类型
enum QuestType
{
	QUEST_TYPE_INVALID = 0,		//非法的任务类型
	QUEST_TYPE_NEW = 1,			//新手任务
	QUEST_TYPE_DAILY = 2,		//日常任务
	QUEST_TYPE_ACHIEVE = 4,		//成就任务
	QUEST_TYPE_ADVENTURE = 8,	//奇遇任务
};

//奖励类型
enum RewardType
{
	REWARD_TYPE_INVALID	= 0,	//非法的奖励类型
	REWARD_TYPE_RES		= 1,	//奖励资源
	REWARD_TYPE_ITEM	= 2,	//奖励道具
	REWARD_TYPE_ENTITY	= 3,	//奖励实物
	REWARD_TYPE_CARDNO	= 4,	//卡密道具
};

//任务条件类型
enum QuestNeedType
{
	QUEST_NEED_INVALID		= 0,	//非法的条件
	QUEST_NEED_KILLFISH		= 1,	//捕到鱼
	QUEST_NEED_GETFISHRES	= 2,	//捕鱼获得资源
	QUEST_NEED_USESKILL		= 3,	//使用技能
	QUEST_NEED_CHANGEOPERA	= 4,	//切换操作模式
	QUEST_NEED_LOTTERY		= 5,	//抽奖
	QUEST_NEED_FINQUEST		= 6,	//完成任务
	QUEST_NEED_GETITEM		= 7,	//获得道具
	QUEST_NEED_LOGINDAY		= 8,	//登录天数
	QUEST_NEED_ONLINETIME	= 9,	//在线时长
};

//兑换消耗类型
enum ExchangeCostType
{
	EXCHANGE_COST_INVALID	= 0,	//非法的消耗类型
	EXCHANGE_COST_TICKET	= 1,	//消耗鱼票
	EXCHANGE_COST_WARHEAD	= 2,	//消耗弹头
};

//走马灯ID类型
enum HorseLampType
{
	HORSELAMP_TYPE_INVALID	= 0,	//非法的走马灯
	HORSELAMP_TYPE_COINS	= 1,	//捕鱼获得金币
	HORSELAMP_TYPE_TICKETS	= 2,	//捕鱼获得鱼票
	HORSELAMP_TYPE_WARHEADS	= 3,	//捕鱼获得弹头
	HORSELAMP_TYPE_EXCHANGE = 4,	//兑换道具走马灯
	HORSELAMP_TYPE_GM		= 99,	//管理员走马灯
};

//限量类型
enum LimitType
{
	LIMIT_TYPE_CLOSED		= 0,	//关闭，不可用
	LIMIT_TYPE_UNLIMIT		= 1,	//不限量
	LIMIT_TYPE_DAYLIMIT		= 2,	//每日限量
	LIMIT_TYPE_TOTALLIMIT	= 3,	//总数限量
	LIMIT_TYPE_LIMITALL		= 4,	//所有限量都生效
};

//限量抽奖类型
enum LimitLotteryType
{
	LIMIT_LOTTERY_INVALID	= 0,	//非法限量抽奖
	LIMIT_LOTTERY_RECHARGE	= 1,	//充值限量抽奖
	LIMIT_LOTTERY_TICKET	= 2,	//鱼票限量抽奖
	LIMIT_LOTTERY_MAX		= 3,	//限量抽奖类型最大
};

//鱼类型最大
static const int FISHTYPE_MAX_NUM	= 100;

//鱼的配置
struct FishConfig
{
	int id;					//鱼的ID
	int Sequence_i;			//鱼组ID
	int Multiple_i_min;		//鱼的倍数最小
	int Multiple_i_max;		//鱼的倍数最大
	int Type_i;				//鱼的类型
	int Time_min_i;			//出现时间随机最小
	int Time_max_i;			//出现时间随机最大
	int Adjust_i;			//调整概率
	int Occurrence_i;		//出现概率
	std::vector<int> vTraceType_i;	//轨迹类型ID
	int iWidth;				//鱼的宽度
	int iHeight;			//鱼的高度
	int aiTimeParam[5];		//出鱼的时间参数
	int iHighAdjust;		//高倍场出鱼调整
	int iRoomTypeID;		//该鱼出现的房间类型ID
	int iPower;				//出鱼权重，权重大的优先出
	int iLimitType;			//出鱼上限类型
	int iLimitNum;			//出鱼上限数量

	FishConfig()
	{
		id = 0;					//鱼的ID
		Sequence_i = 0;			//鱼组ID
		Multiple_i_min = 0;		//鱼的倍数最小
		Multiple_i_max = 0;		//余的倍数最大
		Type_i = 0;				//鱼的类型
		Time_min_i = 0;			//出现时间随机最小
		Time_max_i = 0;			//出现时间随机最大
		Adjust_i = 0;			//调整概率
		Occurrence_i = 0;		//出现概率
		vTraceType_i.clear();	//轨迹类型ID
		iWidth = 0;
		iHeight = 0;

		memset(aiTimeParam, 0, sizeof(aiTimeParam));

		iHighAdjust = 0;
		iRoomTypeID = 0;
		iPower = 0;
		iLimitType = 0;
		iLimitNum = 0;
	}
};

typedef std::map<int, std::vector<FishConfig> > FishSeqType;

//轨迹点坐标
struct TracePoint
{
	int iPosX;		//X坐标
	int iPosY;		//Y坐标
	int iTime;		//坐标点的时间
	int iRatio;		//缩放比例，乘了1000的

	TracePoint()
	{
		memset(this, 0, sizeof(*this));
	}
};

//单条轨迹的配置
struct TraceConfig  
{
	int iTraceID;	//轨迹ID
	int iTraceType;	//轨迹类型
	std::vector<TracePoint> vPoints;	//轨迹点

	TraceConfig()
	{
		iTraceID = 0;
		iTraceType = 0;
		vPoints.clear();
	}
};

//轨迹类型 -> 轨迹组
typedef std::map<int, std::vector<TraceConfig> > TraceConfigType;

//炮台的配置
struct GunConfig
{
	int iGunID;		//炮台ID
	int iMultiple;	//炮台的倍数
	int iConsume;	//炮台的消耗

	GunConfig()
	{
		memset(this, 0, sizeof(*this));
	}

	inline bool operator==(const GunConfig& other) const
	{
		return (this->iGunID == other.iGunID);
	}
};

//全局的配置
struct GlobalConfig
{
	int iTypeID;	//全局配置类型ID
	int iValue;		//该类型配置的值

	GlobalConfig()
	{
		memset(this, 0, sizeof(*this));
	}

	inline bool operator==(const GlobalConfig& other) const
	{
		return (this->iTypeID == other.iTypeID);
	}
};

//状态控制的配置
struct StatControlConfig
{
	int iStatID;		//状态控制ID
	int iValueMin;		//状态控制最小值
	int iValueMax;		//状态控制最大值
	int iStep;			//状态计算的Step值
	int iDecrease;		//状态控制的衰减值

	StatControlConfig()
	{
		memset(this, 0, sizeof(*this));
	}
};

//服务器状态控制配置
struct ServerStatConfig
{
	int ID;				//状态控制的ID
	int iValueMin;		//控制区间最小值
	int iValueMax;		//控制区间最大值
	int iDecrease;		//服务器状态衰减

	ServerStatConfig()
	{
		memset(this, 0, sizeof(*this));
	}
};

//鱼阵的配置
struct TrajectoryConfig
{
	int ID;				//唯一ID
	int iType;			//轨迹所属鱼阵类型
	int iTraceID;		//鱼阵对应轨迹ID
	int iStartTime;		//开始出鱼时间, ms
	int iFishID;		//对应的鱼ID
	int iInterval;		//出鱼间隔时间, ms
	int iFishNumMax;	//最大出鱼数量

	TrajectoryConfig()
	{
		memset(this, 0, sizeof(*this));
	}
};

//鱼阵类型 -> 鱼阵信息
typedef std::map<int, std::vector<TrajectoryConfig> > TrajectoryType;

//鱼阵时间配置
struct FormTimeConfig
{
	int ID;				//鱼阵类型ID
	int iWeight;		//出鱼的权重
	int iPauseTime;		//鱼阵持续时间

	FormTimeConfig()
	{
		memset(this, 0, sizeof(*this));
	}
};

//道具的配置
struct FishItemConfig
{
	int iItemID;		//道具ID
	int iType;			//道具类型
	int iCanSend;		//道具是否可赠送
	int aiParam[3];		//暂定最多支持3个参数
	int iTicketValue;	//道具的鱼票价值

	FishItemConfig()
	{
		memset(this, 0, sizeof(*this));
	}
};

//小鱼组的配置
struct SmallFishConfig
{
	int ID;							//小鱼组ID，设定和鱼表的一致
	std::vector<int> vTrackIDs;		//小鱼组中鱼的轨迹

	SmallFishConfig()
	{
		ID = 0;

		vTrackIDs.clear();
	}
};

//掉落物品配置
struct DropItemConfig
{
	int iItemID;		//掉落的物品ID
	int iItemNum;		//掉落的物品数量

	DropItemConfig()
	{
		memset(this, 0, sizeof(*this));
	}
};

//房间类型配置
struct FishRoomConfig
{
	int iRoomID;		//房间ID
	int iRoomPattern;	//房间模式ID
	int iRoomTypeID;	//房间类型ID
	int iPlayerNum;		//房间桌子最大玩家数量
	int iCoinLimit;		//房间进入金币限制
	int iMinBatteryID;	//最小炮台ID
	int iMaxBatteryID;	//最大炮台ID
	DropItemConfig astDrops[2];	//掉落道具的配置
	int iAlgorithmType;	//房间算法类型
	int iRoomX1;		//房间抽水，相同算法类型抽水必须相同

	FishRoomConfig()
	{
		memset(this, 0, sizeof(*this));
	}
};

//玩家初始化的配置
struct BirthInitConfig
{
	int iTypeID;	//玩家属性初始化类型ID
	int iValue;		//玩家属性初始化值

	BirthInitConfig()
	{
		memset(this, 0, sizeof(*this));
	}
};

//玩家抽奖消耗配置
struct LotteryCostConfig
{
	int iCostID;	//抽奖消耗的ID
	int iCost;		//抽奖消耗
	int iVIPLevel;	//抽奖需要的VIP等级

	LotteryCostConfig()
	{
		memset(this, 0, sizeof(*this));
	}
};

//玩家抽奖奖项配置
struct LotteryItemConfig
{
	int iLotteryItemID;		//抽奖奖项ID
	RewardConfig stReward;	//奖项的奖励
	int iWeight;			//奖项的权重

	LotteryItemConfig()
	{
		memset(this, 0, sizeof(*this));
	}
};

//玩家开宝箱的配置
struct OpenBoxConfig
{
	int iID;			//宝箱的ID
	RewardConfig astRewards[7];	//开出的宝箱获得

	OpenBoxConfig()
	{
		memset(this, 0, sizeof(*this));
	}
};

//玩家任务配置
struct QuestConfig
{
	int iID;					//任务ID
	int iType;					//任务类型
	int iQuestIndex;			//任务阶段
	int iNextQuestID;			//下一个任务ID，0表示没有
	int iNeedType;				//完成任务的条件类型
	long8 alParam[4];				//完成任务的条件,默认参数4为累计数量
	int iCountdownTime;			//任务过期时间
	RewardConfig astRewards[2];	//任务奖励

	QuestConfig()
	{
		memset(this, 0, sizeof(*this));
	}
};

//玩家奇遇任务奖励
struct AdventureRewardConfig
{
	int iID;					//奇遇奖励ID
	int iQuestIndex;			//奇遇任务阶段
	int iGunMultipleMin;		//平均炮台倍数最低
	int iGunMultipleMax;		//平均炮台倍数最高
	RewardConfig astRewards[4];	//奇遇任务奖励

	AdventureRewardConfig()
	{
		memset(this, 0, sizeof(*this));
	}
};

//玩家兑换的配置
struct ExchangeConfig
{
	int iID;			//兑换ID
	int iType;			//兑换类型
	int iVIPLv;			//VIP等级要求
	int iIsLimit;		//是否限量兑换
	int iLimitNum;		//如果是限量，限量数量
	int iCostType;		//消耗类型
	long8 lCostNum;		//消耗数量
	RewardConfig stReward;	//兑换获得
	bool bIsNotice;		//是否播放走马灯

	ExchangeConfig()
	{
		memset(this, 0, sizeof(*this));
	}
};

//玩家体验线配置
struct ExpLineConfig
{
	int iID;			//体验线ID
	int iBulletMax;		//体验线适用的最大体验线数量
	int iAddRate;		//体验线增加的概率
	int iType;			//体验线的类型
	int iAlgorithmType;	//对应的算法类型
	int iStandardNum;	//基准子弹倍数
	int iNextLine;		//下一个体验线配置
	bool bIsBegin;		//是否开始的体验线

	ExpLineConfig()
	{
		memset(this, 0, sizeof(*this));
	}
};

//活跃度奖励配置
struct LivnessRewardConfig
{
	int iID;			//奖励ID
	int iLivnessNum;	//需要的活跃度数量
	int iBoxItemID;		//奖励的宝箱ID

	LivnessRewardConfig()
	{
		memset(this, 0, sizeof(*this));
	}
};

//邮件配置
struct MailConfig
{
	int iID;					//邮件配置ID
	int iType;					//邮件类型，普通邮件还是附件邮件
	//std::string strTitle;		//邮件标题

	MailConfig()
	{
		iID = 0;
		iType = 0;
	}
};

//VIP特权
struct VipPriv
{
	int iPrivType;				//特权类型
	int aiParams[3];			//特权参数

	VipPriv()
	{
		memset(this, 0, sizeof(*this));
	}
};

//VIP特权配置
struct VipLevelConfig
{
	int iVIPLv;					//VIP等级
	int iNeedExp;				//该等级需要的经验
	std::vector<VipPriv> vPrivs;//等级对应的特权

	VipLevelConfig()
	{
		Reset();
	}

	void Reset()
	{
		iVIPLv = 0;
		iNeedExp = 0;
		vPrivs.clear();
	}
};

//限量抽奖的配置
struct LimitLotteryConfig
{
	int iID;				//充值抽奖ID
	RewardConfig stReward;	//抽奖奖项
	int iWeight;			//奖项权重
	int iDayNum;			//奖项日数量
	int iTotalNum;			//奖项总数量
	int iLimitType;			//限量类型

	LimitLotteryConfig()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

//走马灯配置
struct HorseLampConfig
{
	int iID;			//走马灯ID
	int iType;			//走马灯类型
	int aiNeeds[3];		//走马灯要求条件，和类型对应
	int iTimes;			//走马灯播放次数

	HorseLampConfig()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

//拉斯维拉斯转盘配置
struct LasvegasConfig
{
	int iID;			//转盘奖项ID
	int iNumber;		//转盘奖项赔率
	int iMinBet;		//单人最小下注
	int iMaxBet;		//单人最大下注
	int iWeight;		//奖项转盘中的权重

	LasvegasConfig()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

//登录奖励配置
struct LoginRewardConfig
{
	int iID;				//登录奖励ID,和天数对应
	RewardConfig stReward;	//登录奖励内容

	LoginRewardConfig()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

//玩家充值配置
struct RechargeConfig
{
	int iID;				//充值ID
	int iCostRMB;			//玩家花费人民币
	RewardConfig stReward;	//充值获得
	int iExtraNum;			//首冲额外赠送数量

	RechargeConfig()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

//鱼王荣耀配置
struct RankRewardConfig
{
	int iID;					//排名ID
	RewardConfig stDayReward;	//日排行奖励
	RewardConfig stWeekReward;	//周排行奖励
	
	RankRewardConfig()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

//多倍鱼倍率配置
struct MultipleFishConfig
{
	int iID;		//倍率ID
	int iRate;		//倍率
	int iPower;		//权重

	MultipleFishConfig()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

class BaseConfigManager
{
public:
	BaseConfigManager();
	~BaseConfigManager();

	//加载配置
	int LoadAllConfig(const std::string& strConfigDir);

	//重置配置
	void Reset();

public:

	//获取鱼的配置
	const FishConfig* GetFishConfig(int iSeqID, int ID);

	//获取鱼的配置
	const FishConfig* GetFishConfig(int ID);

	//获取所有鱼的配置
	const FishSeqType& GetAllFishConfig();

	//获取所有轨迹的配置
	const TraceConfigType& GetAllTraceConfig();

	//获取轨迹配置
	const TraceConfig* GetTraceConfigByID(int iTraceID);

	//获取轨迹组对应的轨迹
	void GetTraceIDsByType(int iTraceType, std::vector<int>& vTraceIDs);

	//获取炮台配置
	const GunConfig* GetGunConfig(int iGunID);

	//获取全局配置
	int GetGlobalConfig(int iTypeID);

	//获取状态控制配置
	const StatControlConfig* GetStatConfig(int iRatio);

	//获取服务器状态配置
	const ServerStatConfig* GetServerStatConfig(int iRatio);

	//获取鱼阵的配置
	const std::vector<TrajectoryConfig>* GetTrajectoryConfigByType(int iType);

	//获取鱼阵时间的配置
	const FormTimeConfig* GetFormTimeConfig(int iFormID);

	//获取鱼阵时间配置
	const FormTimeConfig* GetFormTimeConfigByWeight();

	//获取捕鱼道具配置
	const FishItemConfig* GetFishItemConfig(int iItemID);

	//获取小鱼组的配置
	const SmallFishConfig* GetSmallFishConfig(int iSmallFishID);

	//获取房间类型的配置
	const FishRoomConfig* GetFishRoomConfig(int iRoomID);

	//获取房间抽水的配置
	int GetRoomX1Config(int iAlgorithmType);

	//获取玩家初始化配置
	int GetBirthInitConfig(int iTypeID);

	//获取玩家抽奖消耗配置
	const LotteryCostConfig* GetLotteryCostConfig(int iLotteryCostID);

	//获取抽奖奖项配置，算法随机抽取
	const LotteryItemConfig* GetLotteryItemConfig();

	//获取开宝箱配置
	const OpenBoxConfig* GetOpenBoxConfig(int iBoxID);

	//获取任务配置
	const QuestConfig* GetQuestConfig(int iQuestID);
	const std::vector<QuestConfig>& GetQuestConfig();

	//获取奇遇任务
	const QuestConfig* GetAdventQuestConfig(int iQuestIndex);

	//获取奇遇任务奖励配置
	const AdventureRewardConfig* GetAdventureRewardConfig(int iQuestIndex, int iGunMultiple);

	//获取兑换的配置
	const ExchangeConfig* GetExchangeConfig(int iExchangeID);
	void GetExchangeConfig(bool bIsLimit, std::vector<ExchangeConfig>& vConfigs);

	//获取体验线配置
	const ExpLineConfig* GetExpLineConfig(int iExpLineID);
	const ExpLineConfig* GetExpLineConfig(int iExpLineType, int iAlgorithmType);

	//获取活跃度奖励配置
	const LivnessRewardConfig* GetLivnessRewardConfig(int iLivnessID);

	//获取邮件配置
	const MailConfig* GetMailConfig(int iMailID);

	//获取VIP特权配置
	const VipLevelConfig* GetVipConfig(int iVipLevel);

	//获取限量抽奖配置
	const LimitLotteryConfig* GetLimitLotteryConfig(int iLimitLotteryType, int iLotteryID);
	const std::vector<LimitLotteryConfig>* GetLimitLotteryConfig(int iLimitLotteryType);

	//获取走马灯配置
	const HorseLampConfig* GetHorseLampConfig(int iID);

	//获取拉斯维加斯转盘配置
	const LasvegasConfig* GetLasvegasConfig();
	const LasvegasConfig* GetLasvegasConfig(int iID);
	const LasvegasConfig* GetLasvegasConfigByNumber(int iNumber);

	//获取登录奖励配置
	const LoginRewardConfig* GetLoginRewardConfig(int iLoginDays);

	//获取玩家充值配置
	const RechargeConfig* GetRechargeConfig(int iRechargeID);

	//获取鱼王荣耀配置
	const RankRewardConfig* GetRankRewardConfig(int iRank);

	//是否包含屏蔽字
	bool IsContainMaskWord(const std::string& strWord);

	//获取多倍鱼配置，算法随机抽取
	const MultipleFishConfig* GetMultipleFishConfig();

private:
	
	//加载鱼配置
	int LoadFishConfig();

	//加载轨迹配置
	int LoadTraceConfig();

	//加载炮台的配置
	int LoadGunConfig();

	//加载全局的配置
	int LoadGlobalConfig();

	//加载状态控制配置
	int LoadStatControlConfig();

	//加载服务器状态配置
	int LoadServerStatConfig();

	//加载鱼阵的配置
	int LoadTrajectoryConfig();

	//加载鱼阵时间的配置
	int LoadFormTimeConfig();

	//加载捕鱼道具的配置
	int LoadFishItemConfig();

	//加载小鱼组的配置
	int LoadSmallFishConfig();

	//加载房间类型的配置
	int LoadRoomTypeConfig();

	//加载玩家初始化配置
	int LoadBirthInitConfig();

	//加载抽奖消耗的配置
	int LoadLotteryCostConfig();

	//加载抽奖奖项配置
	int LoadLotteryItemConfig();

	//加载开宝箱配置
	int LoadOpenBoxConfig();

	//加载任务配置
	int LoadQuestConfig();

	//加载奇遇任务奖励配置
	int LoadAdventureRewardConfig();

	//加载兑换的配置
	int LoadExchangeConfig();

	//加载体验线配置
	int LoadExpLineConfig();

	//加载活跃度奖励配置
	int LoadLivnessRewardConfig();

	//加载邮件配置
	int LoadMailConfig();

	//加载VIP特权配置
	int LoadVIPConfig();

	//加载限量抽奖配置
	int LoadLimitLotteryConfig();

	//加载走马灯配置
	int LoadHorseLampConfig();

	//拉斯维加斯转盘配置
	int LoadLasvegasConfig();

	//加载登录奖励配置
	int LoadLoginRewardConfig();

	//加载玩家充值配置
	int LoadRechargeConfig();

	//加载鱼王荣耀配置
	int LoadRankRewardConfig();

	//加载屏蔽字配置
	int LoadMaskWordConfig();

	//加载多倍鱼配置
	int LoadMultipleFishConfig();

private:

	//设置配置文件路径
	void SetConfigDir(const std::string& strConfigDir);

	//加载配置文件
	int LoadFile(const std::string& strFileName, Json::Value& jValue);

private:
	
	//鱼的配置
	FishSeqType m_mFishConfig;

	//轨迹点的配置
	TraceConfigType m_mTraceConfig;

	//炮台的配置
	std::vector<GunConfig> m_vGunConfig;

	//全局的配置
	std::vector<GlobalConfig> m_vGlobalConfig;

	//状态控制的配置
	std::vector<StatControlConfig> m_vStatConfig;

	//服务器状态的配置
	std::vector<ServerStatConfig> m_vServerStatConfig;

	//鱼阵的配置
	TrajectoryType m_mTrajectoryConfig;

	//鱼阵时间的配置
	std::vector<FormTimeConfig> m_vFormTimeConfig;

	//道具表的配置
	std::vector<FishItemConfig> m_vFishItemConfig;

	//小鱼组的配置
	std::vector<SmallFishConfig> m_vSmallFishConfig;

	//房间类型的配置
	std::vector<FishRoomConfig> m_vFishRoomConfig;

	//玩家初始化配置
	std::vector<BirthInitConfig> m_vBirthInitConfig;

	//玩家抽奖消耗配置
	std::vector<LotteryCostConfig> m_vLotteryCostConfig;

	//玩家抽奖奖项配置
	std::vector<LotteryItemConfig> m_vLotteryItemConfig;

	//开宝箱的配置
	std::vector<OpenBoxConfig> m_vOpenBoxConfig;

	//任务的配置
	std::vector<QuestConfig> m_vQuestConfig;

	//奇遇任务奖励配置
	std::vector<AdventureRewardConfig> m_vAdventureRewardConfig;

	//兑换功能的配置
	std::vector<ExchangeConfig> m_vExchangeConfig;

	//玩家体验线配置
	std::vector<ExpLineConfig> m_vExpLineConfig;

	//玩家活跃度奖励配置
	std::vector<LivnessRewardConfig> m_vLivnessRewardConfig;

	//邮件配置
	std::vector<MailConfig> m_vMailConfig;

	//VIP特权配置
	std::vector<VipLevelConfig> m_vVipLevelConfig;

	//限量抽奖配置
	std::vector<LimitLotteryConfig> m_avLimitLotteryConfig[LIMIT_LOTTERY_MAX];

	//走马灯配置
	std::vector<HorseLampConfig> m_vHorseLampConfig;

	//拉斯维加斯转盘配置
	std::vector<LasvegasConfig> m_vLasvegasConfig;

	//登录奖励配置
	std::vector<LoginRewardConfig> m_vLoginRewardConfig;

	//玩家充值配置
	std::vector<RechargeConfig> m_vRechargeConfig;

	//鱼王荣耀配置
	std::vector<RankRewardConfig> m_vRankRewardConfig;

	//屏蔽字配置
	MaskWordConfig m_stMaskWordConfig;

	//多倍鱼配置
	std::vector<MultipleFishConfig> m_vMultipleFishConfig;

	//配置文件的路径
	std::string m_strConfigDir;
};
