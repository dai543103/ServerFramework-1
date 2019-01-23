#ifndef __GAME_ROLE_HPP__
#define __GAME_ROLE_HPP__

#include <string.h>

#include "GameProtocol.hpp"
#include "CommDefine.h"
#include "ObjAllocator.hpp"
#include "TimeUtility.hpp"
#include "LogAdapter.hpp"
#include "AppTick.hpp"
#include "GameObjCommDef.hpp"

#include "RepThings/RepThingsManager.hpp"
#include "Exchange/ExchangeManager.h"
#include "Quest/QuestManager.h"
#include "Mail/MailManager.h"
#include "Recharge/RechargeManager.h"

using namespace ServerLib;

// Zone维护 ：角色数据重试写数据库时间间隔（秒）
const int ROLEDATA_REWRITE_SECONDS = 10;

// 防沉迷状态
typedef enum tagEnumAAStatus
{
    EAS_HEALTHY     = 0,        // 健康0 ~ 3小时
    EAS_TIRED       = 1,        // 疲劳3 ~ 5小时
    EAS_UNHEALTHY   = 2,        // 不健康 >5小时
} TAAStatus;

// 消息记录
typedef struct
{
    unsigned short m_wMsgID;    //消息ID
    unsigned char m_ucSource;   //来源
    timeval m_stTimeval;        //时间
}MsgInfo;

const int MAX_MSG_QUEUE_LEN = 400;

typedef struct tagMsgInfoQueue
{
    int m_iMsgQueueIndex;
    MsgInfo m_astMsgInfo[MAX_MSG_QUEUE_LEN];

    tagMsgInfoQueue()
    {
        memset(this, 0, sizeof(*this));
    };
}MsgInfoQueue;

// 模块锁
typedef enum
{
    MAIL_LOCK = 0,
    MARKET_LOCK = 1,
    MAX_LOCK_NUM = 2
}ENUM_LOCK;

class CGameSessionObj;
class CGameRoleObj : public CObj
{
public:
    CGameRoleObj();
    virtual ~CGameRoleObj();
    virtual int Initialize();
    virtual int Resume();
    DECLARE_DYN

///////////////////////////////////////////////////////////////////////
public:

    // 初始化玩家属性
    int InitRole(const RoleID& stRoleID);

    void SetRoleID(const RoleID& stRoleID) { m_stRoleInfo.stRoleID.CopyFrom(stRoleID); };
    const RoleID& GetRoleID() { return m_stRoleInfo.stRoleID; };
    unsigned int GetUin() const { return m_stRoleInfo.stRoleID.uin(); };
    void SetUin(unsigned int uiUin) { m_stRoleInfo.stRoleID.set_uin(uiUin); };

    TROLEINFO& GetRoleInfo() { return m_stRoleInfo; };
    TUNITINFO& GetUnitInfo() { return m_stRoleInfo.stUnitInfo; };

    int GetSessionID();
    void SetSessionID(int iSessionID);
    CGameSessionObj* GetSession();

    //sznickname 名字
    void SetNickName(const char* strNickName);
    const char* GetNickName();

    //uStatus 状态
    unsigned GetRoleStatus() { return m_stRoleInfo.stUnitInfo.uiUnitStatus; };
    void SetRoleStatus(int uStatus) { m_stRoleInfo.stUnitInfo.uiUnitStatus = uStatus; };

    //iLastLogin 上次登录时间
    int GetLastLoginTime() ;
    void SetLastLoginTime(int iLastLoginTime) ;

    //iLastLogout 上次下线时间
    int GetLastLogoutTime(){ return m_stRoleInfo.stBaseInfo.iLastLogout;}
    void SetLastLogoutTime(int iLastLogoutTime){m_stRoleInfo.stBaseInfo.iLastLogout = iLastLogoutTime;}

    //iCreateTime 帐号创建的时间
    void SetCreateTime(int iCreateTime);
    int GetCreateTime();
    
    //总在线时间
    void SetOnlineTotalTime(int iOnlineTime);
    int GetOnlineTotalTime();
    int GetOnlineThisTime()const;

    //iLoginCount 玩家总的登录次数
    int GetLoginCount() {return m_stRoleInfo.stBaseInfo.iLoginCount;}
    void SetLoginCount(int iCount) { m_stRoleInfo.stBaseInfo.iLoginCount = iCount;}

    //iLoginTime 本次登录的时间
    void InitLoginTime();
    int GetLoginTime() ;

    //iLogoutTime 本次登出游戏的时间
    int GetLogoutTime() ;
    void SetLogoutTime(int iLogoutTime) ;

	bool IsGM() { return false; };

	//玩家炮台信息
	TWEAPONINFO& GetWeaponInfo();
	void SetWeaponInfo(const WeaponInfo& stWeapon);
	void UnlockWeaponStyle(int iStyleID);

	//玩家资源信息
	long8 GetResource(int iType);
	void SetResource(int iType, long8 lResource);
	bool AddResource(int iType, long8 lAddResource);

	//玩家VIP等级
	int GetVIPLevel();
	void SetVIPLevel(int iLevel);

	//玩家VIP经验
	long8 GetVIPExp();
	void SetVIPExp(long8 iExp);
	void AddVIPExp(int iExp);

	//玩家VIP特权
	int GetVIPPriv();
	void SetVIPPriv(int iPriv);
	bool HasVIPPriv(int iPrivType);

	//玩家救济金下次可领取时间
	int GetNextAlmsTime();
	void SetNextAlmsTime(int iTime);

	//玩家当日已领取救济金次数
	int GetAlmsNum();
	void SetAlmsNum(int iNum);

	//玩家上次救济金更新时间
	int GetLastAlmsUpdateTime();
	void SetLastAlmsUpdateTime(int iTime);

	//玩家剩余抽奖次数
	int GetLotteryNum();
	void SetLotteryNum(int iRemainNum);

	//玩家充值抽奖记录
	void SetRechargeLotteryRecord(const BASEDBINFO& stBaseInfo);
	void GetRechargeLotteryRecord(std::vector<int>& vRecords);
	void AddRechargeLotteryRecord(int iLotteryID);

	//玩家捕鱼桌子ID
	unsigned GetTableID();
	void SetTableID(unsigned uTableID);

	//玩家体验线信息
	TEXPLINEINFO* GetExpLineInfo(int iAlgorithmType);
	void SetExpLineInfo(const BASEDBINFO& stBaseInfo);

	//玩家积分信息
	TSCOREINFO& GetScoreInfo();
	void SetScoreInfo(const BASEDBINFO& stBaseInfo);
	void AddFishScore(int iScoreNum);

	//玩家登陆天数
	int GetLoginDays();
	void SetLoginDays(int iLoginDays);

	//玩家领取登录奖励
	int GetLoginReward();
	void SetLoginReward(int iLoginReward);
	bool HasGetLoginReward(int iGetDay);
	void SetLoginRewardStat(int iGetDay);

	//玩家新手红包
	int GetRemainNewRedNum();
	void SetRemainNewRedNum(int iNum);
	int GetNowNewRedNum();
	void SetNowNewRedNum(int iNum);

	//玩家月卡
	int GetMonthEndTime();
	void SetMonthEndTime(int iTime);
	void AddMonthEndTime(int iAddTime);

	//玩家月卡礼包时间
	int GetLastMonthTime();
	void SetLastMonthTime(int iTime);

	//玩家渠道号
	const char* GetChannel();
	void SetChannel(const std::string& strChannel);

	//玩家头像
	const char* GetPicID();
	void SetPicID(const std::string& strPicID);

	//是否强制更新排行榜
	bool GetUpdateRank();
	void SetUpdateRank(bool bUpdateRank);

	//是否获得VIP金币补足
	bool GetIsVIPAddCoins();
	void SetIsVIPAddCoins(bool bVIPAddCoins);

	//玩家实名状态
	int GetRealNameStat();
	void SetRealNameStat(int iRealNameStat);

	//玩家当天在线时长
	int GetDayOnlineTime();
	void SetDayOnlineTime(int iDayOnlineTime);

	//玩家在线时长最近更新时间
	int GetLastOnlineUpdate();
	void SetLastOnlineUpdate(int iTime);

	//玩家已完成新手引导ID
	unsigned GetFinGuideIndexes();
	void SetFinGuideIndexes(unsigned uIndexes);
	void SetFinGuide(int iGuideID);
	bool IsGuideFin(int iGuideID);

	//玩家是否改名
	bool GetNameChanged();
	void SetNameChanged(bool bChanged);

	//玩家第三方账号
	const char* GetAccount();
	void SetAccount(const char* szAccount);

	//玩家设备号
	const char* GetDeviceID();
	void SetDeviceID(const char* szDeviceID);

	//玩家累计输赢
	long8 GetUserWinNum();
	void SetUserWinNum(long8 lWinNum);
	void AddUserWinNum(long8 lAddNum);
	void UpdateRoleWinNum();

	//玩家游戏签名
	const char* GetSign();
	void SetSign(const char* szSign);

	//玩家结算信息
	TROLETALLYINFO& GetTallyInfo();
	
	//更新结算信息
	void UpdateTallyInfo();

public:
    //基础信息的数据库操作函数
    void UpdateBaseInfoToDB(BASEDBINFO& rstBaseInfo);
    void InitBaseInfoFromDB(const BASEDBINFO& rstBaseInfo, const KickerInfo& stKicker);

///////////////////////////////////////////////////////////////////////
public:

    void SetKicker(const World_KickRole_Request& rstKicker) {m_stKicker.CopyFrom(rstKicker);}
    World_KickRole_Request& GetKicker() {return m_stKicker;}

///////////////////////////////////////////////////////////////////////
///
public:
    // 刷新玩家的最后活动的信息
    void ActiveRefresh();
    bool IsUnactive();

public:
    // Tick 计算
    void OnTick();

	//重置GameRole
	void ResetRole();

public:
    // 防沉迷状态
    void SetAAStatus(TAAStatus enAAStatus) {m_enAAStatus = enAAStatus;};
    TAAStatus GetAAStatus() {return m_enAAStatus;};

/////////////////////////////////////////////////////////////////////////////////////////
public:
    // 调试使用
    void PrintMyself();

public:
    // 在线状态
    void SetOnline();
    void SetOffline();
    bool IsOnline();

/////////////////////////////////////////////////////////////////////////////////////////
// 成员变量
private:
    int m_iSessionID;

    //玩家角色的数据信息
    TROLEINFO m_stRoleInfo;

    // 操作记录
    int m_iLastActiveTime;

    // 记录踢我下线的人
    World_KickRole_Request m_stKicker;

    // Tick记录
    int m_iLastTickTime;
    int m_iLastUpdateDBTickTime;
    int m_iLastMinTime;			//1min 定时器
    int m_iLastTenSecondTime;	//10s 定时器
	int m_iLastFiveSecondTime;	//5s 定时器
        
    // 防沉迷状态
    TAAStatus m_enAAStatus;

public:
    bool IsLock(ENUM_LOCK eLock)
    {
        return m_abLock[eLock];
    }

    void UnLock(ENUM_LOCK eLock)
    {
        m_abLock[eLock] = false;
    }

    void Lock(ENUM_LOCK eLock)
    {
        m_abLock[eLock] = true;
    }

private:
    //行为锁
    //  请求限次，防止恶意请求造成异步事务异常，出现道具复制等问题
    bool m_abLock[MAX_LOCK_NUM];

public:
    void SetLogoutReason(int iReason);
    int GetLogoutReason() {return m_iLogoutReason;}

private:
    int m_iLogoutReason;

public:
    bool IsMsgFreqLimit(const int iMsgID) const;
    void PushMsgID(const int iMsgID, const unsigned char ucSource);
private:

    //消息队列
    MsgInfoQueue m_stMsgInfoQueue;

public:
	
	//玩家背包管理器
	CRepThingsManager& GetRepThingsManager();
	void UpdateRepThingsToDB(ITEMDBINFO& rstItemDBInfo);
	void InitRepThingsFromDB(const ITEMDBINFO& rstItemDBInfo);

	//玩家任务管理器
	CQuestManager& GetQuestManager();
	void UpdateQuestToDB(QUESTDBINFO& stQuestDBInfo);
	void InitQuestFromDB(const QUESTDBINFO& stQuestDBInfo);

	//玩家兑换管理器
	CExchangeManager& GetExchangeManager();
	void UpdateExchangeToDB(EXCHANGEDBINFO& stExchangeDBInfo);
	void InitExchangeFromDB(const EXCHANGEDBINFO& stExchangeDBInfo);

	//玩家邮件管理器
	CMailManager& GetMailManager();
	void UpdateMailToDB(MAILDBINFO& stMailDBInfo);
	void InitMailFromDB(const MAILDBINFO& stMailDBInfo);

	//玩家充值管理器
	CRechargeManager& GetRechargeManager();
	void UpdateRechargeToDB(RECHARGEDBINFO& stRechargeInfo);
	void InitRechargeFromDB(const RECHARGEDBINFO& stRechargeInfo);

private:

	//背包管理器
	CRepThingsManager	m_stRepThingsManager;

	//任务管理器
	CQuestManager		m_stQuestManager;

	//兑换管理器
	CExchangeManager	m_stExchangeManager;

	//邮件管理器
	CMailManager		m_stMailManager;

	//充值管理器
	CRechargeManager	m_stRechargeManager;
};

#endif
