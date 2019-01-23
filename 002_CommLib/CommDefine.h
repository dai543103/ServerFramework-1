#pragma once

#include <string>
#include <vector>
#include <string.h>

//配置公用变量和结构定义

typedef long long long8;			//long8类型定义
typedef unsigned long long ulong8;	//ulong8类型定义
typedef unsigned char BYTE;			//BYTE类型定义
typedef unsigned short USHORT;		//USHORT类型定义

static const unsigned SERVER_BUSID_LEN = 64;			// 服务器BUS ID的长度
static const unsigned DATE_TIME_LEN = 64;				// 配置中时间字符串的长度
static const unsigned MAX_IP_ADDRESS_LENGTH = 64;		// IP地址最大长度
static const unsigned MAX_SERVER_BUS_ADDR_LEN = 64;		// 服务器的ZMQ BUS的地址的最大长度，格式为 ip:port
static const unsigned MAX_SESSIONKEY_LENGTH = 64;		// 玩家session key的最大长度
static const unsigned ROLE_TABLE_SPLIT_FACTOR = 8;		// ROLEDB数据库访问的分表因子
static const unsigned REGAUTH_TABLE_SPLIT_FACTOR = 8;	// RegAuthDB数据库访问的分表因子
static const unsigned NAME_TABLE_SPLIT_FACTOR = 8;		// NameDB数据库访问的分表因子
static const unsigned LOG_TABLE_SPLIT_FACTOR = 8;		// LogServer数据库访问的分表因子
static const unsigned MAX_NICK_NAME_LENGTH = 128;		// 玩家名字的最大长度
static const unsigned GAME_START_YEAR = 2017;			// 游戏正式开始运营的年份
static const unsigned UNACTIVE_ROLE_TIMEOUT = 7200;		// 判断玩家不活跃的超时时间,120*60s
static const unsigned TICK_TIMEVAL_UPDATEDB = 300;		// 更新玩家数据库的时间间隔，5*60s
static const unsigned ONE_MIN_TIMEOUT = 60;				// 一分钟定时
static const unsigned TEN_SECOND_TIMER = 10;			// 10秒钟定时

static const int MAX_HANDLER_NUMBER = 16384;			// 服务器支持的最大的消息处理Handler的数量
static const int MAX_ZONE_PER_WORLD = 8;				// 当前单个世界能支持的最大分区
static const int MAX_REP_BLOCK_NUM = 500;				// 玩家最大背包格子数
static const int MAX_EXCHANGE_ORDER_NUM = 50;			//最大能显示的兑换订单数
static const int MAX_ROOM_ALGORITHM_TYPE = 6;			//最大支持的房间算法类型
static const int MAX_APPENDIX_NUM = 4;					//单个邮件附件最大数量
static const int MAX_ROLE_MAIL_NUM = 500;				//玩家最大邮件数量
static const int MAX_RANK_INFO_NUM = 100;				//最大支持的排行玩家数量
static const int GOLD_WARHEAD_ID = 10003;				//黄金弹头ID
static const int WARHEAD_INRANK_NUM = 10;				//入排行榜需要的黄金弹头数
static const int AIM_FISH_ITEM_ID = 30001;				//瞄准道具的ID
static const int MAX_PAYLOTTERY_NUM = 20;				//玩家抽奖记录上限
static const int RECHARGE_LOTTERY_COSTNUM = 30;			//充值抽奖消耗30充值积分，1积分=充值1RMB
static const int MAX_RECHARGE_LOTTERY_RECORD = 200;		//充值抽奖记录上限
static const int MAX_LASVEGAS_RECORD_NUM = 100;			//大转盘玩家中奖纪录上限
static const int MAX_LASVEGAS_LOTTERY_NUM = 20;			//大转盘开奖记录上限
static const int MAX_RECHARGE_RECORD_NUM = 50;			//玩家充值记录上限
static const int MAX_APP_THREAD_NUM = 8;				//AppThread最大线程数	

enum ServerState
{
	SERVER_STATE_IDLE = 1,
	SERVER_STATE_BUSY = 2,
	SERVER_STATE_FULL = 3,
};

// Server服务器ID
typedef enum enGameServerID
{
	GAME_SERVER_UNDEFINE	= 0,
	GAME_SERVER_WORLD		= 1,
	GAME_SERVER_ZONE		= 2,
	GAME_SERVER_ROLEDB		= 3,
	GAME_SERVER_LOTUS		= 4,
	GAME_SERVER_REGAUTH		= 5,
	GAME_SERVER_REGAUTHDB	= 6,
	GAME_SERVER_NAMEDB		= 7,
	GAME_SERVER_RECHARGE	= 8,
	GAME_SERVER_PLATFORM	= 9,
	GAME_SERVER_EXCHANGE	= 10,
	GAME_SERVER_LOGSERVER	= 11,
	GAME_SERVER_MAX,
} EGameServerID;

typedef enum enServerStatus
{
	// 正常启动中
	// 1) Zone向World发送启动消息
	// 2) World向Cluster发送启动消息
	GAME_SERVER_STATUS_INIT = 1,

	// 恢复启动中
	GAME_SERVER_STATUS_RESUME = 2,

	// 空闲运行中
	GAME_SERVER_STATUS_IDLE = 3,

	// 忙运行中
	GAME_SERVER_STATUS_BUSY = 4,

	// 准备停止
	// 1) 通知客户端准备停服下线
	// 2) 开始进入STOP状态倒计时
	GAME_SERVER_STATUS_PRESTOP = 5,

	// 停止中
	// 1) 通知Lotus停止所有的输入接收
	// 2) 处理所有的Lotus数据
	// 3) 处理所有的TBus数据
	// 4) 将所有在线玩家踢下线, 并通知Lotus断开连接
	// 5) 停止Lotus和Zone服务器
	GAME_SERVER_STATUS_STOP = 6,

} EGameServerStatus;

enum GameUnitType
{
	EUT_ROLE = 1, // 角色
};

enum EUnitStatus
{
	EGUS_ONLINE = 1,
	EGUS_LOGOUT = 2,
	EGUS_ISGM = 4,
	EGUS_DELETE = 8,
};

static const int NETHEAD_V2_SIZE = 24;	//网络头

struct TNetHead_V2
{
	unsigned int	m_uiSocketFD;	//套接字
	unsigned int	m_uiSocketTime;	//套接字创建时刻
	unsigned int	m_uiSrcIP;		//源地址
	unsigned short	m_ushSrcPort;	//源端口
	unsigned short	m_ushReservedValue01; //字节对齐，未用
	unsigned int	m_uiCodeTime;	//消息时刻
	unsigned int	m_uiCodeFlag;	//消息标志，用于实现套接字控制

	TNetHead_V2()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 动态内存对象使用配置

//////////////////////////////////////////
// Zone
/////////////////////////////////////////

// 角色对象
const int MAX_ROLE_OBJECT_NUMBER_IN_ZONE = 4000;

// 单位ID
const int MAX_UNIT_NUMBER_IN_ZONE = (MAX_ROLE_OBJECT_NUMBER_IN_ZONE) * 2;

//鱼池对象
const int MAX_FISHPOND_NUMBER_IN_ZONE = MAX_ROLE_OBJECT_NUMBER_IN_ZONE;

//////////////////////////////////////////
// World
/////////////////////////////////////////
const int MAX_ROLE_OBJECT_NUMBER_IN_WORLD = MAX_ROLE_OBJECT_NUMBER_IN_ZONE * 4;

//////////////////////////////////////////
// Name
///////////////////////////////////////
//主HashMap节点大小
const int MAX_ROLE_NAME_NUMBER = 100000;

//////////////////////////////////////////
//RegAuth
const int MAX_ACCOUNT_OBJ_CACHE_NUMBER = 5000;

enum ResourceType
{
	RESOURCE_TYPE_INVALID	= 0,	// 非法的资源类型
	RESOURCE_TYPE_COIN		= 1,	// 游戏金币
	RESOURCE_TYPE_DIAMOND	= 2,	// 游戏充值钻石
	RESOURCE_TYPE_TICKET	= 3,	//游戏内鱼票
	RESOURCE_TYPE_LIVENESS	= 4,	//游戏内活跃度
	RESOURCE_TYPE_ACHEIVE	= 5,	//游戏内成就点
	RESOURCE_TYPE_PAYSCORE	= 6,	//充值抽奖积分
	RESOURCE_TYPE_MAX,				// 游戏资源最大
};

//捕鱼道具类型
enum FishItemType
{
	FISH_ITEM_INVALID	= 0,	//非法的道具类型
	FISH_ITEM_BOX		= 1,	//宝箱道具类型
	FISH_ITEM_WARHEAD	= 2,	//弹头道具类型
	FISH_ITEM_SKILL		= 3,	//技能道具类型
	FISH_ITEM_STYLE		= 5,	//炮台样式道具类型
	FISH_ITEM_MONTH		= 6,	//月卡道具
	FISH_ITEM_USE		= 7,	//可使用道具类型
};

//捕鱼VIP特权类型
enum VIPPrivType
{
	VIP_PRIV_GETALMS	= 0x01,	//领取救济金
	VIP_PRIV_GETITEM	= 0x02,	//升级道具奖励
	VIP_PRIV_LOTTERY	= 0x04,	//海盗宝藏抽奖
	VIP_PRIV_WILDNUM	= 0x08,	//狂暴叠加次数
	VIP_PRIV_ADDCOINS	= 0x10,	//每日首次登陆补充金币
	VIP_PRIV_SENDGIFT	= 0x20,	//开启赠送功能
	VIP_PRIV_EXCHANGE	= 0x40,	//开启弹头兑换功能
	VIP_PRIV_CUSTOMIZE	= 0x80,	//开启私人定制
};

//上报日志类型
enum ReportLogType
{
	REPORT_LOG_INVALID		= 0,	//非法的日志类型
	REPORT_LOG_MOFANGLOGIN	= 1,	//魔方登录日志
	REPORT_LOG_MOFANGPAY	= 2,	//魔方充值日志
	REPORT_LOG_TALLYINFO	= 3,	//玩家结算日志
};

//玩家弹头类型
enum WarheadType
{
	WARHEAD_TYPE_INVALID	= 0,	//非法的弹头类型
	WARHEAD_TYPE_BRONZE		= 1,	//青铜弹头
	WARHEAD_TYPE_SILVER		= 2,	//白银弹头
	WARHEAD_TYPE_GOLD		= 3,	//黄金弹头
	WARHEAD_TYPE_MAX		= 4,	//弹头最大，程序使用
};

//玩家道具渠道类型
enum ItemChannelType
{
	ITEM_CHANNEL_INVALID	= 0,	//非法的渠道
	ITEM_CHANNEL_EXCHANGE	= 1,	//兑换消耗道具
	ITEM_CHANNEL_GMADD		= 2,	//GM修改道具
	ITEM_CHANNEL_FISHADD	= 3,	//捕鱼获得道具
	ITEM_CHANNEL_USEITEM	= 4,	//使用道具
	ITEM_CHANNEL_QUEST		= 5,	//任务获得道具
	ITEM_CHANNEL_SENDGIFT	= 6,	//赠送修改道具
	ITEM_CHANNEL_ROLEMAIL	= 7,	//个人邮件获得
	ITEM_CHANNEL_SYSMAIL	= 8,	//系统邮件获得
	ITEM_CHANNEL_OPENBOX	= 9,	//开宝箱获得道具
	ITEM_CHANNEL_LOTTERY	= 10,	//抽奖获得道具
	ITEM_CHANNEL_LOGINADD	= 11,	//登录奖励道具
};

//鱼的位置
struct FISHPOS
{
	int iX;		//X坐标
	int iY;		//Y坐标

	FISHPOS()
	{
		memset(this, 0, sizeof(*this));
	}

	FISHPOS(int iPosX, int iPosY)
	{
		iX = iPosX;
		iY = iPosY;
	}

	FISHPOS& operator=(const FISHPOS& stPos)
	{
		if (this == &stPos)
		{
			return *this;
		}

		this->iX = stPos.iX;
		this->iY = stPos.iY;

		return *this;
	}

	bool operator==(const FISHPOS& stPos)
	{
		if (this == &stPos)
		{
			return true;
		}

		return ((this->iX == stPos.iX) && (this->iY == stPos.iY));
	}
};

//排行信息
struct RankData
{
	unsigned uiUin;				//玩家ID
	std::string strNickName;	//玩家昵称
	std::string strPicID;		//玩家头像ID
	int iVIPLevel;				//玩家VIP等级
	long8 iNum;					//玩家拥有的数量，金币,弹头或积分
	std::string strSign;		//玩家游戏签名

	RankData()
	{
		Reset();
	}

	void Reset()
	{
		uiUin = 0;
		strNickName.clear();
		strPicID.clear();
		iVIPLevel = 0;
		iNum = 0;
		strSign.clear();
	}

	bool operator==(const RankData& stData) const
	{
		return (this->uiUin == stData.uiUin);
	}
};

//排行榜列表
struct RankList
{
	unsigned uVersionID;					//当前排行榜版本ID，有更新+1
	std::vector<RankData> vRankInfos;		//排行信息
	std::vector<RankData> vLastRankInfos;	//上个周期排行榜信息
	int iLastUpdateTime;					//排行榜上次更新时间

	RankList()
	{
		Reset();
	}

	void Reset()
	{
		uVersionID = 1;		//从1开始
		iLastUpdateTime = 0;
		vRankInfos.clear();
		vLastRankInfos.clear();
	}
};

//玩家转盘中奖信息
struct LotteryPrizeData
{
	std::string strNickName;	//玩家名字
	int iNumber;				//奖项数字
	int iRewardCoins;			//中奖金额

	LotteryPrizeData()
	{
		Reset();
	}

	void Reset()
	{
		strNickName.clear();
		iNumber = 0;
		iRewardCoins = 0;
	}
};

//转盘奖项下注信息
struct LasvegasBetData
{
	int iNumber;		//奖项倍数数字
	int iBetCoins;	//下注金额

	LasvegasBetData()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

//奖励配置
struct RewardConfig
{
	int iType;		//奖励类型
	int iRewardID;	//奖励ID，根据类型表示道具或资源
	int iRewardNum;	//奖励数量

	RewardConfig()
	{
		memset(this, 0, sizeof(*this));
	}
};

//邮件信息
struct MailData
{
	unsigned uUniqID;					//邮件唯一ID
	int iMailID;						//邮件配置ID
	int iSendTime;						//邮件发送时间
	int iMailStat;						//邮件状态
	std::string strTitle;				//邮件标题
	std::vector<std::string> vParams;	//邮件参数
	std::vector<RewardConfig> vAppendixes;//邮件附件

	MailData()
	{
		Reset();
	}

	void Reset()
	{
		uUniqID = 0;
		iMailID = 0;
		iSendTime = 0;
		iMailStat = 0;
		strTitle.clear();
		vParams.clear();
		vAppendixes.clear();
	}
};
