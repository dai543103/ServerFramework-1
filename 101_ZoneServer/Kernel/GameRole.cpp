#include <algorithm>
#include <string.h>

#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "AppDef.hpp"
#include "TimeUtility.hpp"
#include "Random.hpp"
#include "FreqCtrl.hpp"
#include "StringUtility.hpp"
#include "Kernel/ZoneMsgHelper.hpp"
#include "Kernel/ModuleHelper.hpp"
#include "Kernel/UnitUtility.hpp"
#include "Kernel/UpdateRoleInfoHandler.hpp"
#include "Login/LogoutHandler.hpp"
#include "FishGame/FishpondObj.h"
#include "FishGame/FishUtility.h"
#include "Vip/VipUtility.h"
#include "Rank/RankInfoManager.h"

#include "GameRole.hpp"

using namespace ServerLib;

IMPLEMENT_DYN(CGameRoleObj)

CGameRoleObj::CGameRoleObj()
{
}

CGameRoleObj::~CGameRoleObj()
{
}

int CGameRoleObj::Initialize()
{
    m_stKicker.Clear();

    return 0;
}

// 初始化玩家的所有数值为默认数值
int CGameRoleObj::InitRole(const RoleID& stRoleID)
{
    time_t tNow = time(NULL);

    SetRoleID(stRoleID);

    memset(&m_stRoleInfo.stBaseInfo, 0, sizeof(m_stRoleInfo.stBaseInfo));

    // 登录和下线时间
	m_stRoleInfo.stBaseInfo.iLastLogin = 0;
	m_stRoleInfo.stBaseInfo.iLastLogout = 0;
    m_stRoleInfo.stBaseInfo.iLoginTime = tNow;
    m_stRoleInfo.stBaseInfo.iLogoutTime = tNow;

    m_iLastActiveTime = tNow;
    m_iLastTickTime = tNow;
    m_iLastUpdateDBTickTime = tNow;
    m_iLastMinTime = tNow;
    m_iLastTenSecondTime = tNow;
	m_iLastFiveSecondTime = tNow;

    m_enAAStatus = EAS_HEALTHY;
    SetOffline();

    memset(m_abLock, 0, sizeof(m_abLock));

    //初始化背包道具管理器
    m_stRepThingsManager.Initialize();
    m_stRepThingsManager.SetOwner(GetUin());

	//初始化任务管理器
	m_stQuestManager.Initialize();
	m_stQuestManager.SetOwner(GetUin());

	//初始化兑换管理器
	m_stExchangeManager.Initialize();
	m_stExchangeManager.SetOwner(GetUin());

	//初始化邮件管理器
	m_stMailManager.Initialize();
	m_stMailManager.SetOwner(GetUin());

	//初始化重置管理器
	m_stRechargeManager.Initialize();
	m_stRechargeManager.SetOwner(GetUin());

    m_stKicker.Clear();

    return 0;
}

void CGameRoleObj::SetNickName(const char* strNickName)
{
    ASSERT_AND_LOG_RTN_VOID(strNickName);
    
    SAFE_STRCPY(m_stRoleInfo.stBaseInfo.szNickName, strNickName, sizeof(m_stRoleInfo.stBaseInfo.szNickName)-1);
}

void CGameRoleObj::SetSessionID(int iSessionID)
{
    m_iSessionID = iSessionID;
}

int CGameRoleObj::GetSessionID()
{
    return m_iSessionID;
}

CGameSessionObj* CGameRoleObj::GetSession()
{
    return CModuleHelper::GetSessionManager()->FindSessionByID(m_iSessionID);
}

void CGameRoleObj::InitLoginTime()
{
    m_stRoleInfo.stBaseInfo.iLoginTime = time(NULL);
}

int CGameRoleObj::GetLoginTime()
{
    return m_stRoleInfo.stBaseInfo.iLoginTime;
}

int CGameRoleObj::GetLogoutTime()
{
    return m_stRoleInfo.stBaseInfo.iLogoutTime;
}

int CGameRoleObj::GetLastLoginTime()
{
    return m_stRoleInfo.stBaseInfo.iLastLogin;
}

void CGameRoleObj::SetLogoutTime(int iLogoutTime)
{
    m_stRoleInfo.stBaseInfo.iLogoutTime = iLogoutTime;
    return;
}

void CGameRoleObj::SetLastLoginTime(int iLastLoginTime)
{
    if (!iLastLoginTime)
    {
        return ;
    }

    m_stRoleInfo.stBaseInfo.iLastLogin = iLastLoginTime;
}

void CGameRoleObj::SetCreateTime(int iCreateTime)
{
    m_stRoleInfo.stBaseInfo.iCreateTime = iCreateTime;
}

int CGameRoleObj::GetCreateTime()
{
    return m_stRoleInfo.stBaseInfo.iCreateTime;
}

void CGameRoleObj::SetOnlineTotalTime(int iOnlineTime)
{
    m_stRoleInfo.stBaseInfo.iOnlineTime = iOnlineTime;
}

int CGameRoleObj::GetOnlineTotalTime()
{
    m_stRoleInfo.stBaseInfo.iOnlineTime += GetOnlineThisTime();

    return m_stRoleInfo.stBaseInfo.iOnlineTime;
}

int CGameRoleObj::GetOnlineThisTime()const
{
    int iOnlineThisTime = CTimeUtility::m_uiTimeTick - m_stRoleInfo.stBaseInfo.iLoginTime;
    if (iOnlineThisTime <= 0)
    {
        iOnlineThisTime = 0;
    }

    return iOnlineThisTime;
}

//玩家炮台信息
TWEAPONINFO& CGameRoleObj::GetWeaponInfo()
{
	return m_stRoleInfo.stBaseInfo.stWeaponInfo;
}

void CGameRoleObj::SetWeaponInfo(const WeaponInfo& stWeapon)
{
	TWEAPONINFO& stWeaponInfo = m_stRoleInfo.stBaseInfo.stWeaponInfo;
	stWeaponInfo.iWeaponID = stWeapon.iweaponid();
	stWeaponInfo.iStyleID = stWeapon.iweaponstyleid();

	memset(stWeaponInfo.aiUnlockStyleIDs, 0, sizeof(stWeaponInfo.aiUnlockStyleIDs));
	for (int i = 0; i < stWeapon.iunlockstyleids_size(); ++i)
	{
		stWeaponInfo.aiUnlockStyleIDs[i] = stWeapon.iunlockstyleids(i);
	}
}

void CGameRoleObj::UnlockWeaponStyle(int iStyleID)
{
	TWEAPONINFO& stWeaponInfo = m_stRoleInfo.stBaseInfo.stWeaponInfo;

	//增加到已解锁列表
	for (unsigned i = 0; i < sizeof(stWeaponInfo.aiUnlockStyleIDs)/sizeof(int); ++i)
	{
		if (stWeaponInfo.aiUnlockStyleIDs[i] == 0 ||
			stWeaponInfo.aiUnlockStyleIDs[i] == iStyleID)
		{
			stWeaponInfo.aiUnlockStyleIDs[i] = iStyleID;
			break;
		}
	}

	return;
}

//玩家资源信息
long8 CGameRoleObj::GetResource(int iType)
{
	if (iType < RESOURCE_TYPE_INVALID || iType >= RESOURCE_TYPE_MAX)
	{
		return 0;
	}

	return m_stRoleInfo.stBaseInfo.alResource[iType];
}

void CGameRoleObj::SetResource(int iType, long8 lResource)
{
	if (iType < RESOURCE_TYPE_INVALID || iType >= RESOURCE_TYPE_MAX || lResource<0)
	{
		return;
	}

	m_stRoleInfo.stBaseInfo.alResource[iType] = lResource;
}

bool CGameRoleObj::AddResource(int iType, long8 lAddResource)
{
	if (iType < RESOURCE_TYPE_INVALID || iType >= RESOURCE_TYPE_MAX)
	{
		return false;
	}

	if (m_stRoleInfo.stBaseInfo.alResource[iType] + lAddResource < 0)
	{
		//不能扣成负的
		return false;
	}

	m_stRoleInfo.stBaseInfo.alResource[iType] += lAddResource;

	return true;
}

//玩家VIP等级
int CGameRoleObj::GetVIPLevel()
{
	return m_stRoleInfo.stBaseInfo.iVIPLevel;
}

void CGameRoleObj::SetVIPLevel(int iLevel)
{
	m_stRoleInfo.stBaseInfo.iVIPLevel = iLevel;
}

//玩家VIP经验
long8 CGameRoleObj::GetVIPExp()
{
	return m_stRoleInfo.stBaseInfo.lVIPExp;
}

void CGameRoleObj::SetVIPExp(long8 iExp)
{
	m_stRoleInfo.stBaseInfo.lVIPExp = iExp;
}

void CGameRoleObj::AddVIPExp(int iExp)
{
	m_stRoleInfo.stBaseInfo.lVIPExp += iExp;
}

//玩家VIP特权
int CGameRoleObj::GetVIPPriv()
{
	return m_stRoleInfo.stBaseInfo.iVIPPriv;
}

void CGameRoleObj::SetVIPPriv(int iPriv)
{
	m_stRoleInfo.stBaseInfo.iVIPPriv = iPriv;
}

bool CGameRoleObj::HasVIPPriv(int iPrivType)
{
	return (m_stRoleInfo.stBaseInfo.iVIPPriv & iPrivType);
}

//玩家救济金下次可领取时间
int CGameRoleObj::GetNextAlmsTime()
{
	return m_stRoleInfo.stBaseInfo.iNextAlmsTime;
}

void CGameRoleObj::SetNextAlmsTime(int iTime)
{
	m_stRoleInfo.stBaseInfo.iNextAlmsTime = iTime;
}

//玩家当日已领取救济金次数
int CGameRoleObj::GetAlmsNum()
{
	return m_stRoleInfo.stBaseInfo.iAlmsNum;
}

void CGameRoleObj::SetAlmsNum(int iNum)
{
	m_stRoleInfo.stBaseInfo.iAlmsNum = iNum;
}

//玩家上次救济金更新时间
int CGameRoleObj::GetLastAlmsUpdateTime()
{
	return m_stRoleInfo.stBaseInfo.iLastAlmsUpdateTime;
}

void CGameRoleObj::SetLastAlmsUpdateTime(int iTime)
{
	m_stRoleInfo.stBaseInfo.iLastAlmsUpdateTime = iTime;
}

//玩家当天已抽奖次数
int CGameRoleObj::GetLotteryNum()
{
	return m_stRoleInfo.stBaseInfo.iLotteryNum;
}

void CGameRoleObj::SetLotteryNum(int iRemainNum)
{
	m_stRoleInfo.stBaseInfo.iLotteryNum = iRemainNum;
}

//玩家充值抽奖记录
void CGameRoleObj::SetRechargeLotteryRecord(const BASEDBINFO& stBaseInfo)
{
	for (int i = 0; i < stBaseInfo.irechargelotteryids_size(); ++i)
	{
		m_stRoleInfo.stBaseInfo.aiLotteryIDs[i] = stBaseInfo.irechargelotteryids(i);
	}
}

void CGameRoleObj::GetRechargeLotteryRecord(std::vector<int>& vRecords)
{
	vRecords.clear();
	for (int i = 0; i < MAX_PAYLOTTERY_NUM; ++i)
	{
		if (m_stRoleInfo.stBaseInfo.aiLotteryIDs[i] == 0)
		{
			break;
		}

		vRecords.push_back(m_stRoleInfo.stBaseInfo.aiLotteryIDs[i]);
	}
}

void CGameRoleObj::AddRechargeLotteryRecord(int iLotteryID)
{
	std::vector<int> vRecords;

	GetRechargeLotteryRecord(vRecords);
	vRecords.push_back(iLotteryID);

	if (vRecords.size() > (unsigned)MAX_PAYLOTTERY_NUM)
	{
		//删除第一个
		vRecords.erase(vRecords.begin());
	}

	//写回到玩家数据
	for (unsigned i = 0; i < vRecords.size(); ++i)
	{
		m_stRoleInfo.stBaseInfo.aiLotteryIDs[i] = vRecords[i];
	}
}

//玩家捕鱼桌子ID
unsigned CGameRoleObj::GetTableID()
{
	return m_stRoleInfo.stBaseInfo.uTableID;
}

void CGameRoleObj::SetTableID(unsigned uTableID)
{
	m_stRoleInfo.stBaseInfo.uTableID = uTableID;
}

//玩家体验线信息
TEXPLINEINFO* CGameRoleObj::GetExpLineInfo(int iAlgorithmType)
{
	if (iAlgorithmType < 0 || iAlgorithmType >= MAX_ROOM_ALGORITHM_TYPE)
	{
		return NULL;
	}

	return &m_stRoleInfo.stBaseInfo.astExpLineInfo[iAlgorithmType];
}

void CGameRoleObj::SetExpLineInfo(const BASEDBINFO& stBaseInfo)
{
	for (int i = 0; i < stBaseInfo.stexplines_size(); ++i)
	{
		m_stRoleInfo.stBaseInfo.astExpLineInfo[i].iExpLineID = stBaseInfo.stexplines(i).iexplineid();
		m_stRoleInfo.stBaseInfo.astExpLineInfo[i].iExpLineType = stBaseInfo.stexplines(i).iexplinetype();
		m_stRoleInfo.stBaseInfo.astExpLineInfo[i].lUserWinNum = stBaseInfo.stexplines(i).luserwinnum();
		m_stRoleInfo.stBaseInfo.astExpLineInfo[i].iBulletNum = stBaseInfo.stexplines(i).ibulletnum();
		m_stRoleInfo.stBaseInfo.astExpLineInfo[i].lCostNum = stBaseInfo.stexplines(i).lusercostnum();
	}
}

//玩家积分信息
TSCOREINFO& CGameRoleObj::GetScoreInfo()
{
	return m_stRoleInfo.stBaseInfo.stScoreInfo;
}

void CGameRoleObj::SetScoreInfo(const BASEDBINFO& stBaseInfo)
{
	m_stRoleInfo.stBaseInfo.stScoreInfo.iLastScoreUpdate = stBaseInfo.ilastscoreupdate();
	m_stRoleInfo.stBaseInfo.stScoreInfo.lDayScore = stBaseInfo.idayscore();
	m_stRoleInfo.stBaseInfo.stScoreInfo.lWeekScore = stBaseInfo.iweekscore();
}

void CGameRoleObj::AddFishScore(int iScoreNum)
{
	int iTimeNow = CTimeUtility::GetNowTime();

	TSCOREINFO& stScoreInfo = GetScoreInfo();
	if (!CTimeUtility::IsInSameDay(iTimeNow, stScoreInfo.iLastScoreUpdate))
	{
		//隔天
		stScoreInfo.lDayScore = 0;
	}

	if (!CTimeUtility::IsInSameWeek(iTimeNow, stScoreInfo.iLastScoreUpdate))
	{
		//隔周
		stScoreInfo.lWeekScore = 0;
	}

	stScoreInfo.lDayScore += iScoreNum;
	stScoreInfo.lWeekScore += iScoreNum;
	stScoreInfo.iLastScoreUpdate = iTimeNow;

	return;
}

//玩家登陆天数
int CGameRoleObj::GetLoginDays()
{
	return m_stRoleInfo.stBaseInfo.iLoginDays;
}

void CGameRoleObj::SetLoginDays(int iLoginDays)
{
	m_stRoleInfo.stBaseInfo.iLoginDays = iLoginDays;
}

//玩家领取登录奖励
int CGameRoleObj::GetLoginReward()
{
	return m_stRoleInfo.stBaseInfo.iGetLoginReward;
}

void CGameRoleObj::SetLoginReward(int iLoginReward)
{
	m_stRoleInfo.stBaseInfo.iGetLoginReward = iLoginReward;
}

bool CGameRoleObj::HasGetLoginReward(int iGetDay)
{
	return (m_stRoleInfo.stBaseInfo.iGetLoginReward & (0x01 << (iGetDay-1)));
}

void CGameRoleObj::SetLoginRewardStat(int iGetDay)
{
	m_stRoleInfo.stBaseInfo.iGetLoginReward |= (0x01<<(iGetDay-1));
}

//玩家新手红包
int CGameRoleObj::GetRemainNewRedNum()
{
	return m_stRoleInfo.stBaseInfo.iRemainNewRedNum;
}

void CGameRoleObj::SetRemainNewRedNum(int iNum)
{
	m_stRoleInfo.stBaseInfo.iRemainNewRedNum = iNum;
}

int CGameRoleObj::GetNowNewRedNum()
{
	//不能小于炮台最小倍数
	if (m_stRoleInfo.stBaseInfo.iNowNewRedNum < 100)
	{
		m_stRoleInfo.stBaseInfo.iNowNewRedNum = 0;
	}

	return m_stRoleInfo.stBaseInfo.iNowNewRedNum;
}

void CGameRoleObj::SetNowNewRedNum(int iNum)
{
	m_stRoleInfo.stBaseInfo.iNowNewRedNum = iNum;
}

//玩家月卡
int CGameRoleObj::GetMonthEndTime()
{
	return m_stRoleInfo.stBaseInfo.iMonthEndTime;
}

void CGameRoleObj::SetMonthEndTime(int iTime)
{
	m_stRoleInfo.stBaseInfo.iMonthEndTime = iTime;
}

void CGameRoleObj::AddMonthEndTime(int iAddTime)
{
	m_stRoleInfo.stBaseInfo.iMonthEndTime += iAddTime;
}

//玩家月卡礼包时间
int CGameRoleObj::GetLastMonthTime()
{
	return m_stRoleInfo.stBaseInfo.iLastMonthTime;
}

void CGameRoleObj::SetLastMonthTime(int iTime)
{
	m_stRoleInfo.stBaseInfo.iLastMonthTime = iTime;
}

//玩家渠道号
const char* CGameRoleObj::GetChannel()
{
	return m_stRoleInfo.stBaseInfo.szChannel;
}

void CGameRoleObj::SetChannel(const std::string& strChannel)
{
	SAFE_SPRINTF(m_stRoleInfo.stBaseInfo.szChannel, sizeof(m_stRoleInfo.stBaseInfo.szChannel)-1, "%s", strChannel.c_str());
}

//玩家头像
const char* CGameRoleObj::GetPicID()
{
	return m_stRoleInfo.stBaseInfo.szPicID;
}

void CGameRoleObj::SetPicID(const std::string& strPicID)
{
	SAFE_SPRINTF(m_stRoleInfo.stBaseInfo.szPicID, sizeof(m_stRoleInfo.stBaseInfo.szPicID)-1, "%s", strPicID.c_str());
}

//是否强制更新排行榜
bool CGameRoleObj::GetUpdateRank()
{
	return m_stRoleInfo.stBaseInfo.bUpdateRank;
}

void CGameRoleObj::SetUpdateRank(bool bUpdateRank)
{
	m_stRoleInfo.stBaseInfo.bUpdateRank = bUpdateRank;
}

//是否获得VIP金币补足
bool CGameRoleObj::GetIsVIPAddCoins()
{
	return m_stRoleInfo.stBaseInfo.bGetVIPCoins;
}

void CGameRoleObj::SetIsVIPAddCoins(bool bVIPAddCoins)
{
	m_stRoleInfo.stBaseInfo.bGetVIPCoins = bVIPAddCoins;
}

//玩家实名状态
int CGameRoleObj::GetRealNameStat()
{
	return m_stRoleInfo.stBaseInfo.iRealNameStat;
}

void CGameRoleObj::SetRealNameStat(int iRealNameStat)
{
	m_stRoleInfo.stBaseInfo.iRealNameStat = iRealNameStat;
}

//玩家当天在线时长
int CGameRoleObj::GetDayOnlineTime()
{
	return m_stRoleInfo.stBaseInfo.iDayOnlineTime;
}

void CGameRoleObj::SetDayOnlineTime(int iDayOnlineTime)
{
	m_stRoleInfo.stBaseInfo.iDayOnlineTime = iDayOnlineTime;
}

//玩家在线时长最近更新时间
int CGameRoleObj::GetLastOnlineUpdate()
{
	return m_stRoleInfo.stBaseInfo.iLastOnlineUpdateTime;
}

void CGameRoleObj::SetLastOnlineUpdate(int iTime)
{
	m_stRoleInfo.stBaseInfo.iLastOnlineUpdateTime = iTime;
}

//玩家已完成新手引导ID
unsigned CGameRoleObj::GetFinGuideIndexes()
{
	return m_stRoleInfo.stBaseInfo.uFinGuideIndexes;
}

void CGameRoleObj::SetFinGuideIndexes(unsigned uIndexes)
{
	m_stRoleInfo.stBaseInfo.uFinGuideIndexes = uIndexes;
}

void CGameRoleObj::SetFinGuide(int iGuideID)
{
	m_stRoleInfo.stBaseInfo.uFinGuideIndexes |= (0x01 << (iGuideID - 1));
}

bool CGameRoleObj::IsGuideFin(int iGuideID)
{
	return (m_stRoleInfo.stBaseInfo.uFinGuideIndexes & (0x01 << (iGuideID - 1)));
}

//玩家是否改名
bool CGameRoleObj::GetNameChanged()
{
	return m_stRoleInfo.stBaseInfo.bNameChanged;
}

void CGameRoleObj::SetNameChanged(bool bChanged)
{
	m_stRoleInfo.stBaseInfo.bNameChanged = bChanged;
}

//玩家第三方账号
const char* CGameRoleObj::GetAccount()
{
	return m_stRoleInfo.stBaseInfo.szAccount;
}

void CGameRoleObj::SetAccount(const char* szAccount)
{
	SAFE_SPRINTF(m_stRoleInfo.stBaseInfo.szAccount, sizeof(m_stRoleInfo.stBaseInfo.szAccount)-1, "%s", szAccount);
}

//玩家设备号
const char* CGameRoleObj::GetDeviceID()
{
	return m_stRoleInfo.stBaseInfo.szDeviceID;
}

void CGameRoleObj::SetDeviceID(const char* szDeviceID)
{
	SAFE_SPRINTF(m_stRoleInfo.stBaseInfo.szDeviceID, sizeof(m_stRoleInfo.stBaseInfo.szDeviceID) - 1, "%s", szDeviceID);
}

//玩家累计输赢
long8 CGameRoleObj::GetUserWinNum()
{
	return m_stRoleInfo.stBaseInfo.lUserWinNum;
}

void CGameRoleObj::SetUserWinNum(long8 lWinNum)
{
	m_stRoleInfo.stBaseInfo.lUserWinNum = lWinNum;
}

void CGameRoleObj::AddUserWinNum(long8 lAddNum)
{
	m_stRoleInfo.stBaseInfo.lUserWinNum += lAddNum;
}

void CGameRoleObj::UpdateRoleWinNum()
{
	TROLETALLYINFO& stTallyInfo = GetTallyInfo();

	//金币输赢
	stTallyInfo.lUserWinNum += (GetResource(RESOURCE_TYPE_COIN) - stTallyInfo.alResource[RESOURCE_TYPE_COIN]);
	if (stTallyInfo.lUserWinNum != 0)
	{
		stTallyInfo.bNeedLog = true;
	}

	//弹头折算金币变化
	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();
	int aiBindWarheadIDs[WARHEAD_TYPE_MAX] = { 0 };
	int aiNormalWarheadIDs[WARHEAD_TYPE_MAX] = { 0 };
	aiBindWarheadIDs[WARHEAD_TYPE_BRONZE] = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_BINDBRONZE);
	aiBindWarheadIDs[WARHEAD_TYPE_SILVER] = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_BINDSILVER);
	aiBindWarheadIDs[WARHEAD_TYPE_GOLD] = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_BINDGOLD);
	aiNormalWarheadIDs[WARHEAD_TYPE_BRONZE] = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_NORMALBRONZE);
	aiNormalWarheadIDs[WARHEAD_TYPE_SILVER] = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_NORMALSILVER);
	aiNormalWarheadIDs[WARHEAD_TYPE_GOLD] = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_NORMALGOLD);

	CRepThingsManager& stRepThingsMgr = GetRepThingsManager();
	const FishItemConfig* pstItemConfig = NULL;
	for (int i = (WARHEAD_TYPE_INVALID + 1); i < WARHEAD_TYPE_MAX; ++i)
	{
		//绑定弹头
		pstItemConfig = stBaseCfgMgr.GetFishItemConfig(aiBindWarheadIDs[i]);
		if (pstItemConfig)
		{
			stTallyInfo.lUserWinNum += (stRepThingsMgr.GetRepItemNum(aiBindWarheadIDs[i]) - stTallyInfo.aiBindWarheadNum[i])*pstItemConfig->aiParam[0];
		}
		
		if (stTallyInfo.lUserWinNum != 0)
		{
			stTallyInfo.bNeedLog = true;
		}

		//非绑定弹头,有赠送
		pstItemConfig = stBaseCfgMgr.GetFishItemConfig(aiNormalWarheadIDs[i]);
		if (pstItemConfig)
		{
			stTallyInfo.lUserWinNum += (stRepThingsMgr.GetRepItemNum(aiNormalWarheadIDs[i]) - stTallyInfo.aiWarheadNum[i])*pstItemConfig->aiParam[0];
			if (stTallyInfo.lUserWinNum != 0)
			{
				stTallyInfo.bNeedLog = true;
			}

			stTallyInfo.lUserWinNum += stTallyInfo.aiSendGiftNum[i] * pstItemConfig->aiParam[0];
			if (stTallyInfo.lUserWinNum != 0)
			{
				stTallyInfo.bNeedLog = true;
			}
			
			stTallyInfo.lUserWinNum -= stTallyInfo.aiRecvGiftNum[i] * pstItemConfig->aiParam[0];
			if (stTallyInfo.lUserWinNum != 0)
			{
				stTallyInfo.bNeedLog = true;
			}
		}
	}

	//鱼票变化
	stTallyInfo.lUserWinNum += (GetResource(RESOURCE_TYPE_TICKET)-stTallyInfo.alResource[RESOURCE_TYPE_TICKET])*stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_TICKTOCOIN);
	if (stTallyInfo.lUserWinNum != 0)
	{
		stTallyInfo.bNeedLog = true;
	}

	stTallyInfo.lUserWinNum += stTallyInfo.lCashTicketNum * stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_TICKTOCOIN);
	if (stTallyInfo.lUserWinNum != 0)
	{
		stTallyInfo.bNeedLog = true;
	}

	//钻石变化
	stTallyInfo.lUserWinNum += (GetResource(RESOURCE_TYPE_DIAMOND) - stTallyInfo.alResource[RESOURCE_TYPE_DIAMOND])*stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_DIAMONDTOCOIN);
	if (stTallyInfo.lUserWinNum != 0)
	{
		stTallyInfo.bNeedLog = true;
	}

	stTallyInfo.lUserWinNum -= stTallyInfo.iRechargeDiamonds * stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_DIAMONDTOCOIN);
	if (stTallyInfo.lUserWinNum != 0)
	{
		stTallyInfo.bNeedLog = true;
	}

	//增加玩家输赢
	AddUserWinNum(stTallyInfo.lUserWinNum);

	return;
}

//玩家游戏签名
const char* CGameRoleObj::GetSign()
{
	return m_stRoleInfo.stBaseInfo.szSign;
}

void CGameRoleObj::SetSign(const char* szSign)
{
	SAFE_SPRINTF(m_stRoleInfo.stBaseInfo.szSign, sizeof(m_stRoleInfo.stBaseInfo.szSign)-1, "%s", szSign);
}

//玩家结算信息
TROLETALLYINFO& CGameRoleObj::GetTallyInfo()
{
	return m_stRoleInfo.stBaseInfo.stTallyInfo;
}

//更新结算信息
void CGameRoleObj::UpdateTallyInfo()
{
	TROLETALLYINFO& stTallyInfo = GetTallyInfo();
	stTallyInfo.Reset();

	//设置资源
	for (int i = (RESOURCE_TYPE_INVALID + 1); i < RESOURCE_TYPE_MAX; ++i)
	{
		stTallyInfo.alResource[i] = GetResource(i);
	}

	//设置弹头数量
	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();
	CRepThingsManager& stRepThingsMgr = GetRepThingsManager();
	stTallyInfo.aiBindWarheadNum[WARHEAD_TYPE_BRONZE] = stRepThingsMgr.GetRepItemNum(stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_BINDBRONZE));
	stTallyInfo.aiBindWarheadNum[WARHEAD_TYPE_SILVER] = stRepThingsMgr.GetRepItemNum(stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_BINDSILVER));
	stTallyInfo.aiBindWarheadNum[WARHEAD_TYPE_GOLD] = stRepThingsMgr.GetRepItemNum(stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_BINDGOLD));
	stTallyInfo.aiWarheadNum[WARHEAD_TYPE_BRONZE] = stRepThingsMgr.GetRepItemNum(stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_NORMALBRONZE));
	stTallyInfo.aiWarheadNum[WARHEAD_TYPE_SILVER] = stRepThingsMgr.GetRepItemNum(stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_NORMALSILVER));
	stTallyInfo.aiWarheadNum[WARHEAD_TYPE_GOLD] = stRepThingsMgr.GetRepItemNum(stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_NORMALGOLD));

	return;
}

void CGameRoleObj::PrintMyself()
{
    return;
}

int CGameRoleObj::Resume()
{
    TRACESVR("role obj resume\n");

    return 0;
}

const char* CGameRoleObj::GetNickName()
{
    return m_stRoleInfo.stBaseInfo.szNickName;
}

void CGameRoleObj::UpdateBaseInfoToDB(BASEDBINFO& rstBaseInfo)
{
    rstBaseInfo.set_sznickname(GetNickName());
    rstBaseInfo.set_ustatus(GetRoleStatus()&(!EGUS_LOGOUT));    //清除掉Logout标志
    rstBaseInfo.set_ilastlogin(GetLastLoginTime());
    rstBaseInfo.set_ilastlogout(GetLastLogoutTime());
    rstBaseInfo.set_icreatetime(GetCreateTime());
    rstBaseInfo.set_ionlinetime(GetOnlineTotalTime());
    rstBaseInfo.set_ilogincount(GetLoginCount());
    rstBaseInfo.set_ilogintime(GetLoginTime());
    rstBaseInfo.set_ilogouttime(GetLogoutTime());
	rstBaseInfo.set_iviplevel(GetVIPLevel());
	rstBaseInfo.set_lvipexp(GetVIPExp());
	rstBaseInfo.set_ivippriv(GetVIPPriv());
	rstBaseInfo.set_ilotterynum(GetLotteryNum());
	rstBaseInfo.set_inextalmstime(GetNextAlmsTime());
	rstBaseInfo.set_ialmsnum(GetAlmsNum());
	rstBaseInfo.set_ilastalmsupdatetime(GetLastAlmsUpdateTime());
	rstBaseInfo.set_iremainnewrednum(GetRemainNewRedNum());
	rstBaseInfo.set_inownewrednum(GetNowNewRedNum());
	rstBaseInfo.set_imonthendtime(GetMonthEndTime());
	rstBaseInfo.set_ilastgetmonthtime(GetLastMonthTime());
	rstBaseInfo.set_strchannel(GetChannel());
	rstBaseInfo.set_strpicid(GetPicID());
	rstBaseInfo.set_bupdaterank(GetUpdateRank());
	rstBaseInfo.set_bgetvipcoins(GetIsVIPAddCoins());
	rstBaseInfo.set_idayonlinetime(GetDayOnlineTime());
	rstBaseInfo.set_ilastonlineupdate(GetLastOnlineUpdate());
	rstBaseInfo.set_ufinguideindexes(GetFinGuideIndexes());
	rstBaseInfo.set_bnamechanged(GetNameChanged());
	rstBaseInfo.set_luserwinnum(GetUserWinNum());
	rstBaseInfo.set_strsign(GetSign());

	//玩家炮台数据
	const TWEAPONINFO& stWeaponInfo = GetWeaponInfo();
	rstBaseInfo.mutable_stweapon()->set_iweaponid(stWeaponInfo.iWeaponID);
	rstBaseInfo.mutable_stweapon()->set_iweaponstyleid(stWeaponInfo.iStyleID);

	for (unsigned i = 0; i < sizeof(stWeaponInfo.aiUnlockStyleIDs) / sizeof(int); ++i)
	{
		if (stWeaponInfo.aiUnlockStyleIDs[i] == 0)
		{
			break;
		}
		
		rstBaseInfo.mutable_stweapon()->add_iunlockstyleids(stWeaponInfo.aiUnlockStyleIDs[i]);
	}

	//玩家体验线
	for (int i = 0; i < MAX_ROOM_ALGORITHM_TYPE; ++i)
	{
		ExpLineInfo* pstExpLine = rstBaseInfo.add_stexplines();
		pstExpLine->set_iexplineid(GetExpLineInfo(i)->iExpLineID);
		pstExpLine->set_iexplinetype(GetExpLineInfo(i)->iExpLineType);
		pstExpLine->set_luserwinnum(GetExpLineInfo(i)->lUserWinNum);
		pstExpLine->set_ibulletnum(GetExpLineInfo(i)->iBulletNum);
		pstExpLine->set_lusercostnum(GetExpLineInfo(i)->lCostNum);
	}

	//玩家充值抽奖
	std::vector<int> vLotteryRecord;
	GetRechargeLotteryRecord(vLotteryRecord);
	for (unsigned i = 0; i < vLotteryRecord.size(); ++i)
	{
		rstBaseInfo.add_irechargelotteryids(vLotteryRecord[i]);
	}

	//玩家积分
	rstBaseInfo.set_ilastscoreupdate(GetScoreInfo().iLastScoreUpdate);
	rstBaseInfo.set_idayscore(GetScoreInfo().lDayScore);
	rstBaseInfo.set_iweekscore(GetScoreInfo().lWeekScore);

	//玩家登陆奖励
	rstBaseInfo.set_ilogindays(GetLoginDays());
	rstBaseInfo.set_igetloginreward(GetLoginReward());

	for (int i = 0; i < RESOURCE_TYPE_MAX; ++i)
	{
		rstBaseInfo.add_iresources(GetResource(i));
	}

    return;
}

void CGameRoleObj::InitBaseInfoFromDB(const BASEDBINFO& rstBaseInfo, const KickerInfo& stKicker)
{
    SetNickName(rstBaseInfo.sznickname().c_str());
    SetRoleStatus(rstBaseInfo.ustatus());
    SetLastLoginTime(rstBaseInfo.ilastlogin());
    SetLastLogoutTime(rstBaseInfo.ilastlogout());
    SetCreateTime(rstBaseInfo.icreatetime());
    SetOnlineTotalTime(rstBaseInfo.ionlinetime());
    SetLoginCount(rstBaseInfo.ilogincount());
    InitLoginTime();
    SetLogoutTime(rstBaseInfo.ilogouttime());
	SetWeaponInfo(rstBaseInfo.stweapon());
	SetVIPLevel(rstBaseInfo.iviplevel());
	SetVIPExp(rstBaseInfo.lvipexp());
	SetVIPPriv(rstBaseInfo.ivippriv());
	SetLotteryNum(rstBaseInfo.ilotterynum());
	SetExpLineInfo(rstBaseInfo);
	SetScoreInfo(rstBaseInfo);
	SetNextAlmsTime(rstBaseInfo.inextalmstime());
	SetAlmsNum(rstBaseInfo.ialmsnum());
	SetLastAlmsUpdateTime(rstBaseInfo.ilastalmsupdatetime());
	SetLoginDays(rstBaseInfo.ilogindays());
	SetLoginReward(rstBaseInfo.igetloginreward());
	SetRemainNewRedNum(rstBaseInfo.iremainnewrednum());
	SetNowNewRedNum(rstBaseInfo.inownewrednum());
	SetMonthEndTime(rstBaseInfo.imonthendtime());
	SetLastMonthTime(rstBaseInfo.ilastgetmonthtime());
	SetChannel(rstBaseInfo.strchannel());
	SetPicID(rstBaseInfo.strpicid());
	SetUpdateRank(rstBaseInfo.bupdaterank());
	SetIsVIPAddCoins(rstBaseInfo.bgetvipcoins());
	SetRealNameStat(stKicker.irealnamestat());
	SetDayOnlineTime(rstBaseInfo.idayonlinetime());
	SetLastOnlineUpdate(rstBaseInfo.ilastonlineupdate());
	SetFinGuideIndexes(rstBaseInfo.ufinguideindexes());
	SetNameChanged(rstBaseInfo.bnamechanged());
	SetAccount(stKicker.straccount().c_str());
	SetDeviceID(stKicker.strdeviceid().c_str());
	SetUserWinNum(rstBaseInfo.luserwinnum());
	SetSign(rstBaseInfo.strsign().c_str());

	for (int i = 0; i < RESOURCE_TYPE_MAX; ++i)
	{
		SetResource(i, rstBaseInfo.iresources(i));
	}

	//玩家充值抽奖
	SetRechargeLotteryRecord(rstBaseInfo);

	SetTableID(0);

	//如果每日第一次登录，重置已抽奖次数
	if (!CTimeUtility::IsInSameDay(GetLoginTime(), GetLastLoginTime()))
	{
		SetLotteryNum(0);
	}

    return;
}

// 刷新活动状态
void CGameRoleObj::ActiveRefresh()
{
    m_iLastActiveTime = time(NULL);
}

bool CGameRoleObj::IsUnactive()
{
    return time(NULL) > (m_iLastActiveTime + UNACTIVE_ROLE_TIMEOUT);
}

void CGameRoleObj::OnTick()
{
    time_t tNow = time(NULL);
    int iTickTime = tNow - m_iLastTickTime;
    if (iTickTime < 1)
    {
        return;
    }

    m_iLastTickTime = tNow;

    // 检查是否长时间不活跃的玩家
    if (IsUnactive())
    {
        CLogoutHandler::LogoutRole(this, LOGOUT_REASON_KICKED);
        
        //刷新玩家活跃的时间
        ActiveRefresh();

        return;
    }

    // 检查下线通知
    if (!IsOnline())
    {
        CLogoutHandler::LogoutAction(this);
        return;
    }

    // 检查下线超时重发
    if (CUnitUtility::IsUnitStatusSet(&m_stRoleInfo.stUnitInfo, EGUS_LOGOUT))
    {
        if (tNow - m_stRoleInfo.stBaseInfo.iLogoutTime > ROLEDATA_REWRITE_SECONDS)
        {
            if (CFreqencyCtroller<0>::Instance()->CheckFrequency(1))
            {
                CLogoutHandler::LogoutRole(this, LOGOUT_REASON_NORMAL);
                m_stRoleInfo.stBaseInfo.iLogoutTime = tNow;
            }
        }

        return;
    }

    // 检查停机下线
    if (GAME_SERVER_STATUS_STOP == g_enServerStatus)
    {
        if (!CUnitUtility::IsUnitStatusSet(&m_stRoleInfo.stUnitInfo, EGUS_LOGOUT))
        {
            if (CFreqencyCtroller<0>::Instance()->CheckFrequency(1))
            {
                CLogoutHandler::LogoutRole(this, LOGOUT_REASON_KICKED);
            }
        }

        return;
    }

    // 更新DB
    if ((tNow - m_iLastUpdateDBTickTime) >= TICK_TIMEVAL_UPDATEDB
            && g_enServerStatus != GAME_SERVER_STATUS_STOP)
    {
        CUpdateRoleInfo_Handler::UpdateRoleInfo(this, 0);
        m_iLastUpdateDBTickTime = tNow; 
    }

	if (GetTableID() > 0)
	{
		//调用鱼池的定时器
		CFishpondObj* pstFishpondObj = CFishUtility::GetFishpondByID(GetTableID());
		if (pstFishpondObj)
		{
			pstFishpondObj->OnTick(*this);
		}
	}

	//任务的Tick
	m_stQuestManager.OnTick(tNow);

	//邮件的Tick
	m_stMailManager.OnTick(tNow);

	//玩家排行
	CRankInfoManager::Instance()->UpdateRoleRank(*this);

	//救济金
	if (!CTimeUtility::IsInSameDay(tNow, GetLastAlmsUpdateTime()))
	{
		//更新救济金信息
		CVipUtility::TriggerAlmsUpdate(*this);
	}

	//玩家在线时间相关
	CGameEventManager::NotifyOnlineTime(*this, tNow, 1);

	//5秒定时器
	if ((tNow - m_iLastFiveSecondTime) >= 5)
	{
		//推送时间同步消息
		CUnitUtility::SendSyncTimeNotify(this, CTimeUtility::GetMSTime());

		m_iLastFiveSecondTime = tNow;
	}

	//10秒定时器
	if ((tNow - m_iLastTenSecondTime) >= TEN_SECOND_TIMER)
	{
		m_iLastTenSecondTime = tNow;
	}

    //一分钟定时器
    if ((tNow - m_iLastMinTime) >= ONE_MIN_TIMEOUT)
    {
        m_iLastMinTime = tNow;
    }

    return;
}

//重置GameRole
void CGameRoleObj::ResetRole()
{
	m_stRoleInfo.stRoleID.Clear();

	memset(&m_stRoleInfo.stBaseInfo, 0, sizeof(m_stRoleInfo.stBaseInfo));

	// 登录和下线时间
	m_stRoleInfo.stBaseInfo.iLastLogin = 0;
	m_stRoleInfo.stBaseInfo.iLastLogout = 0;
	m_stRoleInfo.stBaseInfo.iLoginTime = 0;
	m_stRoleInfo.stBaseInfo.iLogoutTime = 0;

	m_iLastActiveTime = 0;
	m_iLastTickTime = 0;
	m_iLastUpdateDBTickTime = 0;
	m_iLastMinTime = 0;
	m_iLastTenSecondTime = 0;
	m_iLastFiveSecondTime = 0;

	m_enAAStatus = EAS_HEALTHY;
	SetOffline();

	memset(m_abLock, 0, sizeof(m_abLock));

	//初始化背包道具管理器
	m_stRepThingsManager.Initialize();

	//初始化任务管理器
	m_stQuestManager.Initialize();

	//初始化兑换管理器
	m_stExchangeManager.Initialize();

	//初始化邮件管理器
	m_stMailManager.Initialize();

	//初始化充值管理器
	m_stRechargeManager.Initialize();

	m_stKicker.Clear();

	return;
}

void CGameRoleObj::SetOnline()
{
    CUnitUtility::SetUnitStatus(&m_stRoleInfo.stUnitInfo, EGUS_ONLINE);
}

void CGameRoleObj::SetOffline()
{
    CUnitUtility::ClearUnitStatus(&m_stRoleInfo.stUnitInfo, EGUS_ONLINE);
}

bool CGameRoleObj::IsOnline()
{
    return CUnitUtility::IsUnitStatusSet(&m_stRoleInfo.stUnitInfo, EGUS_ONLINE);
}

CRepThingsManager& CGameRoleObj::GetRepThingsManager()
{
    return m_stRepThingsManager;
}

void CGameRoleObj::UpdateRepThingsToDB(ITEMDBINFO& rstItemDBInfo)
{
    m_stRepThingsManager.UpdateRepItemToDB(rstItemDBInfo);

    return;
}

void CGameRoleObj::InitRepThingsFromDB(const ITEMDBINFO& rstItemDBInfo)
{
   m_stRepThingsManager.InitRepItemFromDB(rstItemDBInfo);

   return;
}

//玩家任务管理器
CQuestManager& CGameRoleObj::GetQuestManager()
{
	return m_stQuestManager;
}

void CGameRoleObj::UpdateQuestToDB(QUESTDBINFO& stQuestDBInfo)
{
	m_stQuestManager.UpdateQuestToDB(stQuestDBInfo);
}

void CGameRoleObj::InitQuestFromDB(const QUESTDBINFO& stQuestDBInfo)
{
	m_stQuestManager.InitQuestFromDB(stQuestDBInfo);
}

//玩家兑换管理器
CExchangeManager& CGameRoleObj::GetExchangeManager()
{
	return m_stExchangeManager;
}

void CGameRoleObj::UpdateExchangeToDB(EXCHANGEDBINFO& stExchangeDBInfo)
{
	m_stExchangeManager.UpdateExchangeToDB(stExchangeDBInfo);
}

void CGameRoleObj::InitExchangeFromDB(const EXCHANGEDBINFO& stExchangeDBInfo)
{
	m_stExchangeManager.InitExchangeFromDB(stExchangeDBInfo);
}

//玩家邮件管理器
CMailManager& CGameRoleObj::GetMailManager()
{
	return m_stMailManager;
}

void CGameRoleObj::UpdateMailToDB(MAILDBINFO& stMailDBInfo)
{
	return m_stMailManager.UpdateMailToDB(stMailDBInfo);
}

void CGameRoleObj::InitMailFromDB(const MAILDBINFO& stMailDBInfo)
{
	return m_stMailManager.InitMailFromDB(stMailDBInfo);
}

//玩家充值管理器
CRechargeManager& CGameRoleObj::GetRechargeManager()
{
	return m_stRechargeManager;
}

void CGameRoleObj::UpdateRechargeToDB(RECHARGEDBINFO& stRechargeInfo)
{
	m_stRechargeManager.UpdateRechargeToDB(stRechargeInfo);
}

void CGameRoleObj::InitRechargeFromDB(const RECHARGEDBINFO& stRechargeInfo)
{
	m_stRechargeManager.InitRechargeFromDB(stRechargeInfo);
}

void CGameRoleObj::SetLogoutReason(int iReason)
{
    m_iLogoutReason = iReason;
}

bool CGameRoleObj::IsMsgFreqLimit(const int iMsgID) const
{
    int iLastPushIdx = (MAX_MSG_QUEUE_LEN + m_stMsgInfoQueue.m_iMsgQueueIndex - 1) % MAX_MSG_QUEUE_LEN;
    if (CConfigManager::Instance()->GetMsgInterval(iMsgID) == 0)
    {
        return false;
    }

    while (iLastPushIdx != m_stMsgInfoQueue.m_iMsgQueueIndex)
    {
        const MsgInfo& rstMsgInfo = m_stMsgInfoQueue.m_astMsgInfo[iLastPushIdx];
        //空的格子
        if ((rstMsgInfo.m_stTimeval.tv_sec == 0)||(rstMsgInfo.m_wMsgID == 0))
        {
            return false;
        }

        //超过最大间隔了
        CTimeValue stTimeDiff = CTimeUtility::m_stTimeValueTick - CTimeValue(rstMsgInfo.m_stTimeval);
        if (stTimeDiff.GetMilliSec() >= MAX_MSG_INTERVAL_TIME)
        {
            return false;
        }

        //找到了相同的消息ID, 并且也是lotus的
        if ((rstMsgInfo.m_wMsgID == iMsgID)&& (rstMsgInfo.m_ucSource == (unsigned char)GAME_SERVER_LOTUS))
        {
            //小于间隔值
            if (stTimeDiff.GetMilliSec() < CConfigManager::Instance()->GetMsgInterval(iMsgID))
            {
                return true;
            }
        }

        iLastPushIdx = (MAX_MSG_QUEUE_LEN + iLastPushIdx - 1) % MAX_MSG_QUEUE_LEN;
    }

    return false;
}

void CGameRoleObj::PushMsgID(const int iMsgID, const unsigned char ucSource)
{
    MsgInfo& rstMsgInfo = m_stMsgInfoQueue.m_astMsgInfo[m_stMsgInfoQueue.m_iMsgQueueIndex];
    rstMsgInfo.m_wMsgID = iMsgID;
    rstMsgInfo.m_ucSource = ucSource;
    rstMsgInfo.m_stTimeval = CTimeUtility::m_stTimeValueTick.GetTimeValue();

    m_stMsgInfoQueue.m_iMsgQueueIndex = (m_stMsgInfoQueue.m_iMsgQueueIndex + 1) % MAX_MSG_QUEUE_LEN;
}

