
#include <unistd.h>
#include <fstream>
#include <algorithm>

#include "json/json.h"
#include "LogAdapter.hpp"
#include "StringUtility.hpp"
#include "SectionConfig.hpp"

#include "Random.hpp"
#include "FileUtility.hpp"

#include "BaseConfigManager.hpp"

using namespace std;
using namespace ServerLib;

#define FISH_CONFIG_FILE			"Fish.json"			//鱼的配置
#define TRACE_CONFIG_FILE			"guiji.json"		//轨迹配置
#define GUN_CONFIG_FILE				"Battery.json"		//炮台配置
#define GLOBAL_CONFIG_FILE			"Global.json"		//全局配置
#define STATCONTROL_CONFIG_FILE		"Algorithm.json"	//个人算法配置
#define SERVERSTAT_CONFIG_FILE		"Stock.json"		//服务器算法配置
#define TRAJECTORY_CONFIG_FILE		"Trajectory.json"	//鱼阵配置
#define FORMGLOBAL_CONFIG_FILE		"Formationtime.json"//鱼阵全局配置
#define FISHITEM_CONFIG_FILE		"Item.json"			//捕鱼道具配置
#define SMALLFISH_CONFIG_FILE		"Smallfishgroup.json"//小鱼组配置
#define LOGINREWARD_CONFIG_FILE		"Loginreward.json"	//登录奖励配置
#define ROOMTYPE_CONFIG_FILE		"room.json"			//房间类型配置
#define BIRTHINIT_CONFIG_FILE		"Init.json"			//玩家初始化配置
#define LOTTERYCOST_CONFIG_FILE		"Lotterycost.json"	//玩家抽奖消耗配置
#define LOTTERYITEM_CONFIG_FILE		"Lotteryitem.json"	//玩家抽奖奖项配置
#define OPENBOX_CONFIG_FILE			"Box.json"			//玩家开宝箱配置
#define QUEST_CONFIG_FILE			"Quest.json"		//玩家任务配置
#define ADVENTUREREWARD_CONFIG_FILE	"Adventurereward.json"//玩家奇遇任务奖励配置
#define EXCHANGE_CONFIG_FILE		"Exchange.json"		//玩家兑换配置
#define EXPLINE_CONFIG_FILE			"Experience.json"	//玩家体验线配置
#define LIVNESS_CONFIG_FILE			"Liveness.json"		//玩家活跃度奖励配置
#define MAIL_CONFIG_FILE			"Mail.json"			//玩家邮件配置
#define VIPLEVEL_CONFIG_FILE		"Vip.json"			//玩家VIP特权配置
#define RECHARGELOTTERY_CONFIG_FILE	"Rechargedraw.json"	//玩家充值抽奖配置
#define HORSELAMP_CONFIG_FILE		"Rollingnotice.json"//玩家走马灯配置
#define LASVEGAS_CONFIG_FILE		"Lasvegas.json"		//拉斯维加斯转盘配置
#define RECHARGE_CONFIG_FILE		"Recharge.json"		//玩家充值配置
#define RANKREWARD_CONFIG_FILE		"yuwangrongyao.json"//鱼王荣耀配置
#define TICKETLOTTERY_CONFIG_FILE	"Tanbaoreward.json"	//玩家鱼票抽奖配置
#define MASKWORD_CONFIG_FILE		"pingbizi.xml"		//屏蔽字配置
#define QUESTIONFISH_CONFIG_FILE	"Ratefish.json"		//问号鱼配置

BaseConfigManager::~BaseConfigManager()
{
	Reset();
}

//加载配置
int BaseConfigManager::LoadAllConfig(const std::string& strConfigDir)
{
	Reset();

	int iRet = 0;

	//设置配置文件路径
	if ((access(strConfigDir.c_str(), 0)) != 0)
	{
		//文件夹访问失败
		LOGERROR("Failed to load base config, invalid config dir %s\n", strConfigDir.c_str());
		return -50;
	}

	m_strConfigDir = strConfigDir;

	//加载鱼的配置
	iRet = LoadFishConfig();
	if (iRet)
	{
		LOGERROR("Failed to load fish config, ret %d\n", iRet);
		return -1;
	}

	//加载轨迹的配置
	iRet = LoadTraceConfig();
	if(iRet)
	{
		LOGERROR("Failed to load trace config, ret %d\n", iRet);
		return -2;
	}

	//加载炮台的配置
	iRet = LoadGunConfig();
	if(iRet)
	{
		LOGERROR("Failed to load gun config, ret %d\n", iRet);
		return -3;
	}

	//加载全局配置
	iRet = LoadGlobalConfig();
	if(iRet)
	{
		LOGERROR("Failed to load global config, ret %d\n", iRet);
		return -4;
	}

	//加载个人状态控制配置
	iRet = LoadStatControlConfig();
	if(iRet)
	{
		LOGERROR("Failed to load stat control config, ret %d\n", iRet);
		return -5;
	}

	//加载服务器状态配置
	iRet = LoadServerStatConfig();
	if(iRet)
	{
		LOGERROR("Failed to load server stat config, ret %d\n", iRet);
		return -6;
	}

	//加载鱼阵的配置
	iRet = LoadTrajectoryConfig();
	if(iRet)
	{
		LOGERROR("Failed to load trajectory config, ret %d\n", iRet);
		return -7;
	}

	//加载鱼阵时间的配置
	iRet = LoadFormTimeConfig();
	if(iRet)
	{
		LOGERROR("Failed to load fish form time config, ret %d\n", iRet);
		return -8;
	}

	//加载捕鱼道具的配置
	iRet = LoadFishItemConfig();
	if (iRet)
	{
		LOGERROR("Failed to load fish item config, ret %d\n", iRet);
		return -9;
	}

	//加载小鱼组的配置
	iRet = LoadSmallFishConfig();
	if (iRet)
	{
		LOGERROR("Failed to load small fish config, ret %d\n", iRet);
		return -10;
	}

	//加载房间类型的配置
	iRet = LoadRoomTypeConfig();
	if (iRet)
	{
		LOGERROR("Failed to load room type config, ret %d\n", iRet);
		return -12;
	}

	//加载玩家初始化配置
	iRet = LoadBirthInitConfig();
	if (iRet)
	{
		LOGERROR("Failed to load birth init config, ret %d\n", iRet);
		return -13;
	}

	//加载抽奖消耗的配置
	iRet = LoadLotteryCostConfig();
	if (iRet)
	{
		LOGERROR("Failed to load lottery cost config, ret %d\n", iRet);
		return -14;
	}

	//加载抽奖奖项配置
	iRet = LoadLotteryItemConfig();
	if (iRet)
	{
		LOGERROR("Failed to load lottery item config, ret %d\n", iRet);
		return -15;
	}

	//加载开宝箱配置
	iRet = LoadOpenBoxConfig();
	if (iRet)
	{
		LOGERROR("Failed to load open box config, ret %d\n", iRet);
		return -16;
	}

	//加载任务配置
	iRet = LoadQuestConfig();
	if (iRet)
	{
		LOGERROR("Failed to load quest config, ret %d\n", iRet);
		return -17;
	}

	//加载奇遇任务奖励配置
	iRet = LoadAdventureRewardConfig();
	if (iRet)
	{
		LOGERROR("Failed to load adventure reward config, ret %d\n", iRet);
		return -18;
	}

	//加载兑换的配置
	iRet = LoadExchangeConfig();
	if (iRet)
	{
		LOGERROR("Failed to load exchange config, ret %d\n", iRet);
		return -19;
	}

	//加载体验线配置
	iRet = LoadExpLineConfig();
	if (iRet)
	{
		LOGERROR("Failed to load expline config, ret %d\n", iRet);
		return -20;
	}

	//加载活跃度奖励配置
	iRet = LoadLivnessRewardConfig();
	if (iRet)
	{
		LOGERROR("Failed to load livness reward config, ret %d\n", iRet);
		return -21;
	}

	//加载邮件配置
	iRet = LoadMailConfig();
	if (iRet)
	{
		LOGERROR("Failed to load mail config, ret %d\n", iRet);
		return -22;
	}

	//加载VIP特权配置
	iRet = LoadVIPConfig();
	if (iRet)
	{
		LOGERROR("Failed to load VIP config, ret %d\n", iRet);
		return -23;
	}

	//加载限量抽奖配置
	iRet = LoadLimitLotteryConfig();
	if (iRet)
	{
		LOGERROR("Failed to load limit lottery config, ret %d\n", iRet);
		return -24;
	}

	//加载走马灯配置
	iRet = LoadHorseLampConfig();
	if (iRet)
	{
		LOGERROR("Failed to load horse lamp config, ret %d\n", iRet);
		return -25;
	}

	//拉斯维加斯转盘配置
	iRet = LoadLasvegasConfig();
	if (iRet)
	{
		LOGERROR("Failed to load lasvegas config, ret %d\n", iRet);
		return -26;
	}

	//加载登录奖励配置
	iRet = LoadLoginRewardConfig();
	if (iRet)
	{
		LOGERROR("Failed to load login reward config, ret %d\n", iRet);
		return -29;
	}

	//加载玩家充值配置
	iRet = LoadRechargeConfig();
	if (iRet)
	{
		LOGERROR("Failed to load recharge config, ret %d\n", iRet);
		return -30;
	}

	//加载鱼王荣耀配置
	iRet = LoadRankRewardConfig();
	if (iRet)
	{
		LOGERROR("Failed to load rank reward config, ret %d\n", iRet);
		return -31;
	}

	//加载屏蔽字配置
	LoadMaskWordConfig();
	/*
	iRet = LoadMaskWordConfig();
	if (iRet)
	{
		LOGERROR("Failed to load mask word config, ret %d\n", iRet);
		return -32;
	}
	*/
	//加载多倍鱼配置
	iRet = LoadMultipleFishConfig();
	if (iRet)
	{
		LOGERROR("Failed to load multiple fish config, ret %d\n", iRet);
		return -33;
	}

	return 0;
}

//获取所有鱼的配置
const FishSeqType& BaseConfigManager::GetAllFishConfig()
{
	return m_mFishConfig;
}

//获取所有轨迹的配置
const TraceConfigType& BaseConfigManager::GetAllTraceConfig()
{
	return m_mTraceConfig;
}

//获取轨迹配置
const TraceConfig* BaseConfigManager::GetTraceConfigByID(int iTraceID)
{
	for (TraceConfigType::iterator it=m_mTraceConfig.begin(); it!=m_mTraceConfig.end(); ++it)
	{
		for(unsigned i=0; i<it->second.size(); ++i)
		{
			if(it->second[i].iTraceID == iTraceID)
			{
				return &(it->second[i]);
			}
		}
	}

	return NULL;
}

//获取轨迹组对应的轨迹
void BaseConfigManager::GetTraceIDsByType(int iTraceType, std::vector<int>& vTraceIDs)
{
	vTraceIDs.clear();

	TraceConfigType::const_iterator it = m_mTraceConfig.find(iTraceType);
	if(it != m_mTraceConfig.end())
	{
		for(unsigned i=0; i<it->second.size(); ++i)
		{
			vTraceIDs.push_back(it->second[i].iTraceID);
		}
	}

	return;
}

//获取炮台配置
const GunConfig* BaseConfigManager::GetGunConfig(int iGunID)
{
	GunConfig stKey;
	stKey.iGunID = iGunID;

	std::vector<GunConfig>::iterator it = std::find(m_vGunConfig.begin(), m_vGunConfig.end(), stKey);
	if(it != m_vGunConfig.end())
	{
		//找到
		return &(*it);
	}

	return NULL;
}

//获取全局配置
int BaseConfigManager::GetGlobalConfig(int iTypeID)
{
	GlobalConfig stKey;
	stKey.iTypeID = iTypeID;

	std::vector<GlobalConfig>::iterator it = std::find(m_vGlobalConfig.begin(), m_vGlobalConfig.end(), stKey);
	if(it != m_vGlobalConfig.end())
	{
		//找到
		return (*it).iValue;
	}

	return 0;
}

//获取状态控制配置
const StatControlConfig* BaseConfigManager::GetStatConfig(int iRatio)
{
	for (unsigned i=0; i<m_vStatConfig.size(); ++i)
	{
		if (iRatio>=m_vStatConfig[i].iValueMin && (m_vStatConfig[i].iValueMax==0 || iRatio<m_vStatConfig[i].iValueMax))
		{
			return &(m_vStatConfig[i]);
		}
	}

	return NULL;
}

//获取服务器状态配置
const ServerStatConfig* BaseConfigManager::GetServerStatConfig(int iRatio)
{
	for (unsigned i=0; i<m_vServerStatConfig.size(); ++i)
	{
		if (iRatio>=m_vServerStatConfig[i].iValueMin && (m_vServerStatConfig[i].iValueMax==0 || iRatio<m_vServerStatConfig[i].iValueMax))
		{
			return &(m_vServerStatConfig[i]);
		}
	}

	return NULL;
}

//获取鱼阵时间的配置
const FormTimeConfig* BaseConfigManager::GetFormTimeConfig(int iFormID)
{
	for(unsigned i=0; i<m_vFormTimeConfig.size(); ++i)
	{
		if(m_vFormTimeConfig[i].ID == iFormID)
		{
			return &m_vFormTimeConfig[i];
		}
	}

	return NULL;
}

//获取鱼阵时间配置
const FormTimeConfig* BaseConfigManager::GetFormTimeConfigByWeight()
{
	int iTotalWeight = 0;
	for(unsigned i=0; i<m_vFormTimeConfig.size(); ++i)
	{
		iTotalWeight += m_vFormTimeConfig[i].iWeight;
	}

	int iRandNum = rand()%iTotalWeight;
	for(unsigned i=0; i<m_vFormTimeConfig.size(); ++i)
	{
		if(iRandNum <= m_vFormTimeConfig[i].iWeight)
		{
			return &m_vFormTimeConfig[i];
		}

		iRandNum -= m_vFormTimeConfig[i].iWeight;
	}

	return NULL;
}

//获取鱼阵的配置
const std::vector<TrajectoryConfig>* BaseConfigManager::GetTrajectoryConfigByType(int iType)
{
	TrajectoryType::iterator it = m_mTrajectoryConfig.find(iType);
	if(it != m_mTrajectoryConfig.end())
	{
		return &(it->second);
	}

	return NULL;
}

//获取捕鱼道具配置
const FishItemConfig* BaseConfigManager::GetFishItemConfig(int iItemID)
{
	for (unsigned i = 0; i < m_vFishItemConfig.size(); ++i)
	{
		if (m_vFishItemConfig[i].iItemID == iItemID)
		{
			return &m_vFishItemConfig[i];
		}
	}

	return NULL;
}

//获取小鱼组的配置
const SmallFishConfig* BaseConfigManager::GetSmallFishConfig(int iSmallFishID)
{
	for (unsigned i = 0; i < m_vSmallFishConfig.size(); ++i)
	{
		if (m_vSmallFishConfig[i].ID == iSmallFishID)
		{
			return &m_vSmallFishConfig[i];
		}
	}

	return NULL;
}

//获取房间类型的配置
const FishRoomConfig* BaseConfigManager::GetFishRoomConfig(int iRoomID)
{
	for (unsigned i = 0; i < m_vFishRoomConfig.size(); ++i)
	{
		if (m_vFishRoomConfig[i].iRoomID == iRoomID)
		{
			return &m_vFishRoomConfig[i];
		}
	}

	return NULL;
}

//获取房间抽水的配置
int BaseConfigManager::GetRoomX1Config(int iAlgorithmType)
{
	for (unsigned i = 0; i < m_vFishRoomConfig.size(); ++i)
	{
		if (m_vFishRoomConfig[i].iAlgorithmType == iAlgorithmType)
		{
			return m_vFishRoomConfig[i].iRoomX1;
		}
	}

	return 0;
}

//获取玩家初始化配置
int BaseConfigManager::GetBirthInitConfig(int iTypeID)
{
	for (unsigned i = 0; i < m_vBirthInitConfig.size(); ++i)
	{
		if (m_vBirthInitConfig[i].iTypeID == iTypeID)
		{
			return m_vBirthInitConfig[i].iValue;
		}
	}

	return 0;
}

//获取玩家抽奖消耗配置
const LotteryCostConfig* BaseConfigManager::GetLotteryCostConfig(int iLotteryCostID)
{
	for (unsigned i = 0; i < m_vLotteryCostConfig.size(); ++i)
	{
		if (m_vLotteryCostConfig[i].iCostID == iLotteryCostID)
		{
			return &m_vLotteryCostConfig[i];
		}
	}

	return NULL;
}

//获取抽奖奖项配置，算法随机抽取
const LotteryItemConfig* BaseConfigManager::GetLotteryItemConfig()
{
	int iRandMax = 0;
	for (unsigned i = 0; i < m_vLotteryItemConfig.size(); ++i)
	{
		iRandMax += m_vLotteryItemConfig[i].iWeight;
	}

	int iRandNum = CRandomCalculator::GetRandomNumberInRange(iRandMax);
	for (unsigned i = 0; i < m_vLotteryItemConfig.size(); ++i)
	{
		if (iRandNum <= m_vLotteryItemConfig[i].iWeight)
		{
			//随机到的奖项
			return &m_vLotteryItemConfig[i];
		}

		iRandNum -= m_vLotteryItemConfig[i].iWeight;
	}

	return NULL;
}

//获取开宝箱配置
const OpenBoxConfig* BaseConfigManager::GetOpenBoxConfig(int iBoxID)
{
	for (unsigned i = 0; i < m_vOpenBoxConfig.size(); ++i)
	{
		if (m_vOpenBoxConfig[i].iID == iBoxID)
		{
			return &m_vOpenBoxConfig[i];
		}
	}

	return NULL;
}

//获取任务配置
const QuestConfig* BaseConfigManager::GetQuestConfig(int iQuestID)
{
	for (unsigned i = 0; i < m_vQuestConfig.size(); ++i)
	{
		if (m_vQuestConfig[i].iID == iQuestID)
		{
			return &m_vQuestConfig[i];
		}
	}

	return NULL;
}

const std::vector<QuestConfig>& BaseConfigManager::GetQuestConfig()
{
	return m_vQuestConfig;
}

//获取奇遇任务
const QuestConfig* BaseConfigManager::GetAdventQuestConfig(int iQuestIndex)
{
	std::vector<int> vAdventQuestIndexes;
	for (unsigned i = 0; i < m_vQuestConfig.size(); ++i)
	{
		if (m_vQuestConfig[i].iType == QUEST_TYPE_ADVENTURE && m_vQuestConfig[i].iQuestIndex == iQuestIndex)
		{
			vAdventQuestIndexes.push_back(i);
		}
	}

	if (vAdventQuestIndexes.size() != 0)
	{
		return &m_vQuestConfig[vAdventQuestIndexes[CRandomCalculator::GetRandomNumberInRange(vAdventQuestIndexes.size())]];
	}

	return NULL;
}

//获取奇遇任务奖励配置
const AdventureRewardConfig* BaseConfigManager::GetAdventureRewardConfig(int iQuestIndex, int iGunMultiple)
{
	for (unsigned i = 0; i < m_vAdventureRewardConfig.size(); ++i)
	{
		if (m_vAdventureRewardConfig[i].iQuestIndex != iQuestIndex)
		{
			continue;
		}

		if (iGunMultiple >= m_vAdventureRewardConfig[i].iGunMultipleMin &&
			iGunMultiple < m_vAdventureRewardConfig[i].iGunMultipleMax)
		{
			return &m_vAdventureRewardConfig[i];
		}
	}

	return NULL;
}

//获取兑换的配置
const ExchangeConfig* BaseConfigManager::GetExchangeConfig(int iExchangeID)
{
	for (unsigned i = 0; i < m_vExchangeConfig.size(); ++i)
	{
		if (m_vExchangeConfig[i].iID == iExchangeID)
		{
			return &m_vExchangeConfig[i];
		}
	}

	return NULL;
}

void BaseConfigManager::GetExchangeConfig(bool bIsLimit, std::vector<ExchangeConfig>& vConfigs)
{
	vConfigs.clear();
	for (unsigned i = 0; i < m_vExchangeConfig.size(); ++i)
	{
		if (!(bIsLimit^m_vExchangeConfig[i].iIsLimit))
		{
			vConfigs.push_back(m_vExchangeConfig[i]);
		}
	}

	return;
}

//获取体验线配置
const ExpLineConfig* BaseConfigManager::GetExpLineConfig(int iExpLineID)
{
	for (unsigned i = 0; i < m_vExpLineConfig.size(); ++i)
	{
		if (m_vExpLineConfig[i].iID == iExpLineID)
		{
			return &m_vExpLineConfig[i];
		}
	}

	return NULL;
}

const ExpLineConfig* BaseConfigManager::GetExpLineConfig(int iExpLineType, int iAlgorithmType)
{
	std::vector<int> vExpLineIndexes;
	for (unsigned i = 0; i < m_vExpLineConfig.size(); ++i)
	{
		if (!m_vExpLineConfig[i].bIsBegin)
		{
			continue;
		}

		if (m_vExpLineConfig[i].iType == iExpLineType && m_vExpLineConfig[i].iAlgorithmType == iAlgorithmType)
		{
			vExpLineIndexes.push_back(i);
		}
	}

	if (vExpLineIndexes.size() == 0)
	{
		return NULL;
	}

	return &m_vExpLineConfig[vExpLineIndexes[CRandomCalculator::GetRandomNumberInRange(vExpLineIndexes.size())]];
}

//获取活跃度奖励配置
const LivnessRewardConfig* BaseConfigManager::GetLivnessRewardConfig(int iLivnessID)
{
	for (unsigned i = 0; i < m_vLivnessRewardConfig.size(); ++i)
	{
		if (m_vLivnessRewardConfig[i].iID == iLivnessID)
		{
			return &m_vLivnessRewardConfig[i];
		}
	}

	return NULL;
}

//获取邮件配置
const MailConfig* BaseConfigManager::GetMailConfig(int iMailID)
{
	for (unsigned i = 0; i < m_vMailConfig.size(); ++i)
	{
		if (m_vMailConfig[i].iID == iMailID)
		{
			return &m_vMailConfig[i];
		}
	}

	return NULL;
}

//获取VIP特权配置
const VipLevelConfig* BaseConfigManager::GetVipConfig(int iVipLevel)
{
	for (unsigned i = 0; i < m_vVipLevelConfig.size(); ++i)
	{
		if (m_vVipLevelConfig[i].iVIPLv == iVipLevel)
		{
			return &m_vVipLevelConfig[i];
		}
	}

	return NULL;
}

const LimitLotteryConfig* BaseConfigManager::GetLimitLotteryConfig(int iLimitLotteryType, int iLotteryID)
{
	if (iLimitLotteryType <= LIMIT_LOTTERY_INVALID || iLimitLotteryType >= LIMIT_LOTTERY_MAX)
	{
		return NULL;
	}

	for (unsigned i = 0; i < m_avLimitLotteryConfig[iLimitLotteryType].size(); ++i)
	{
		if (m_avLimitLotteryConfig[iLimitLotteryType][i].iID == iLotteryID)
		{
			return &m_avLimitLotteryConfig[iLimitLotteryType][i];
		}
	}

	return NULL;
}

//获取充值抽奖配置
const std::vector<LimitLotteryConfig>* BaseConfigManager::GetLimitLotteryConfig(int iLimitLotteryType)
{
	if (iLimitLotteryType <= LIMIT_LOTTERY_INVALID || iLimitLotteryType >= LIMIT_LOTTERY_MAX)
	{
		return NULL;
	}

	return &m_avLimitLotteryConfig[iLimitLotteryType];
}

//获取走马灯配置
const HorseLampConfig* BaseConfigManager::GetHorseLampConfig(int iID)
{
	for (unsigned i = 0; i < m_vHorseLampConfig.size(); ++i)
	{
		if (m_vHorseLampConfig[i].iID == iID)
		{
			return &m_vHorseLampConfig[i];
		}
	}

	return NULL;
}

//获取拉斯维加斯转盘配置
const LasvegasConfig* BaseConfigManager::GetLasvegasConfig()
{
	//根据随机权重来随机一个
	int iRandMax = 0;
	for (unsigned i = 0; i < m_vLasvegasConfig.size(); ++i)
	{
		iRandMax += m_vLasvegasConfig[i].iWeight;
	}

	int iRandNum = CRandomCalculator::GetRandomNumberInRange(iRandMax);
	for (unsigned i = 0; i < m_vLasvegasConfig.size(); ++i)
	{
		if (iRandNum < m_vLasvegasConfig[i].iWeight)
		{
			//落在对应区间
			return &m_vLasvegasConfig[i];
		}

		iRandNum -= m_vLasvegasConfig[i].iWeight;
	}

	return NULL;
}

const LasvegasConfig* BaseConfigManager::GetLasvegasConfig(int iID)
{
	for (unsigned i = 0; i < m_vLasvegasConfig.size(); ++i)
	{
		if (m_vLasvegasConfig[i].iID == iID)
		{
			return &m_vLasvegasConfig[i];
		}
	}

	return NULL;
}

const LasvegasConfig* BaseConfigManager::GetLasvegasConfigByNumber(int iNumber)
{
	for (unsigned i = 0; i < m_vLasvegasConfig.size(); ++i)
	{
		if (m_vLasvegasConfig[i].iNumber == iNumber)
		{
			return &m_vLasvegasConfig[i];
		}
	}

	return NULL;
}

//获取登录奖励配置
const LoginRewardConfig* BaseConfigManager::GetLoginRewardConfig(int iLoginDays)
{
	for (unsigned i = 0; i < m_vLoginRewardConfig.size(); ++i)
	{
		if (m_vLoginRewardConfig[i].iID == iLoginDays)
		{
			return &m_vLoginRewardConfig[i];
		}
	}

	return NULL;
}

//获取玩家充值配置
const RechargeConfig* BaseConfigManager::GetRechargeConfig(int iRechargeID)
{
	for (unsigned i = 0; i < m_vRechargeConfig.size(); ++i)
	{
		if (m_vRechargeConfig[i].iID == iRechargeID)
		{
			return &m_vRechargeConfig[i];
		}
	}

	return NULL;
}

//获取鱼王荣耀配置
const RankRewardConfig* BaseConfigManager::GetRankRewardConfig(int iRank)
{
	for (unsigned i = 0; i < m_vRankRewardConfig.size(); ++i)
	{
		if (m_vRankRewardConfig[i].iID == iRank)
		{
			return &m_vRankRewardConfig[i];
		}
	}

	return NULL;
}

//是否包含屏蔽字
bool BaseConfigManager::IsContainMaskWord(const std::string& strWord)
{
	return m_stMaskWordConfig.HasMaskWord(strWord);
}

//获取问号鱼倍率配置，算法随机抽取
const MultipleFishConfig* BaseConfigManager::GetMultipleFishConfig()
{
	int iRandMax = 0;
	for (unsigned i = 0; i < m_vMultipleFishConfig.size(); ++i)
	{
		iRandMax += m_vMultipleFishConfig[i].iPower;
	}

	int iRandNum = CRandomCalculator::GetRandomNumberInRange(iRandMax);
	for (unsigned i = 0; i < m_vMultipleFishConfig.size(); ++i)
	{
		if (iRandNum <= m_vMultipleFishConfig[i].iPower)
		{
			//随机到的倍率
			return &m_vMultipleFishConfig[i];
		}

		iRandNum -= m_vMultipleFishConfig[i].iPower;
	}

	return NULL;
}

//加载鱼的配置
int BaseConfigManager::LoadFishConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(FISH_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_mFishConfig.clear();

	//解析内容
	Json::Reader jReader;
	Json::Value jTraceType;
	Json::Value jMultiple;
	for(unsigned i=0; i<jValue.size(); ++i)
	{
		FishConfig stConfig;

		stConfig.Time_min_i = jValue[i]["Time_min_i"].asInt();
		stConfig.Time_max_i = jValue[i]["Time_max_i"].asInt();
		if (stConfig.Time_min_i == 0 && stConfig.Time_max_i == 0)
		{
			//这种是不能出的鱼，不读配置
			continue;
		}

		stConfig.id = jValue[i]["id"].asInt();
		stConfig.Sequence_i = jValue[i]["Sequence_i"].asInt();
		stConfig.Type_i = jValue[i]["Type_i"].asInt();

		stConfig.Adjust_i = jValue[i]["Adjust_i"].asInt();
		stConfig.Occurrence_i = jValue[i]["Occurrence_i"].asInt();
		stConfig.iWidth = jValue[i]["X_i"].asInt();
		stConfig.iHeight = jValue[i]["Y_i"].asInt();

		stConfig.aiTimeParam[0] = 1000;
		stConfig.aiTimeParam[1] = 1000;
		stConfig.aiTimeParam[2] = jValue[i]["X2_i"].asInt();
		stConfig.aiTimeParam[3] = jValue[i]["X3_i"].asInt();
		stConfig.aiTimeParam[4] = jValue[i]["X4_i"].asInt();

		stConfig.iHighAdjust = jValue[i]["Highadjust_i"].asInt();
		stConfig.iRoomTypeID = jValue[i]["Roomtype_i"].asInt();
		stConfig.iPower = jValue[i]["Power_i"].asInt();
		stConfig.iLimitType = jValue[i]["Limittype_i"].asInt();
		stConfig.iLimitNum = jValue[i]["Limitnumber_i"].asInt();

		//解析倍数
		if (!jReader.parse(jValue[i]["Multiple_i"].asString(), jMultiple) || jMultiple.size()<2)
		{
			//解析失败
			return -1;
		}

		stConfig.Multiple_i_min = jMultiple[0u].asInt();
		stConfig.Multiple_i_max = jMultiple[1u].asInt();

		//解析轨迹
		if(!jReader.parse(jValue[i]["Tracetype_i"].asString(), jTraceType))
		{
			//解析失败
			return -2;
		}

		for(unsigned j=0; j<jTraceType.size(); ++j)
		{
			stConfig.vTraceType_i.push_back(jTraceType[j].asInt());
		}

		m_mFishConfig[stConfig.Sequence_i].push_back(stConfig);
	}

	return 0;
}

//加载轨迹配置
int BaseConfigManager::LoadTraceConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(TRACE_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_mTraceConfig.clear();

	TraceConfig stOneConfig;
	TracePoint stOnePoint;
	for(unsigned i=0; i<jValue.size(); ++i)
	{
		stOneConfig.iTraceID = jValue[i]["Id"].asInt();
		stOneConfig.iTraceType = jValue[i]["Type"].asInt();

		stOneConfig.vPoints.clear();

		//轨迹上的点
		for(unsigned j=0; j<jValue[i]["Points"].size(); ++j)
		{
			stOnePoint.iPosX = jValue[i]["Points"][j]["X"].asInt();
			stOnePoint.iPosY = jValue[i]["Points"][j]["Y"].asInt();
			stOnePoint.iTime = jValue[i]["Points"][j]["Time"].asInt();
			stOnePoint.iRatio = jValue[i]["Points"][j]["Ratio"].asInt();

			stOneConfig.vPoints.push_back(stOnePoint);
		}

		m_mTraceConfig[stOneConfig.iTraceType].push_back(stOneConfig);
	}

	return 0;
}

//加载炮台的配置
int BaseConfigManager::LoadGunConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(GUN_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vGunConfig.clear();

	GunConfig stOneConfig;
	for(unsigned i=0; i<jValue.size(); ++i)
	{
		stOneConfig.iGunID = jValue[i]["id"].asInt();
		stOneConfig.iMultiple = jValue[i]["Multiple_i"].asInt();
		stOneConfig.iConsume = jValue[i]["Consume_i"].asInt();

		m_vGunConfig.push_back(stOneConfig);
	}
	
	return 0;
}

//加载全局的配置
int BaseConfigManager::LoadGlobalConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(GLOBAL_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vGlobalConfig.clear();

	GlobalConfig stOneConfig;
	for(unsigned i=0; i<jValue.size(); ++i)
	{
		stOneConfig.iTypeID = jValue[i]["type"].asInt();
		stOneConfig.iValue = jValue[i]["value"].asInt();

		m_vGlobalConfig.push_back(stOneConfig);
	}

	return 0;
}

//加载状态控制配置
int BaseConfigManager::LoadStatControlConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(STATCONTROL_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vStatConfig.clear();

	StatControlConfig stOneConfig;
	for(unsigned i=0; i<jValue.size(); ++i)
	{
		stOneConfig.iStatID = jValue[i]["id"].asInt();
		stOneConfig.iValueMin = jValue[i]["value_i_min"].asInt();
		stOneConfig.iValueMax = jValue[i]["value_i_max"].asInt();
		stOneConfig.iStep = jValue[i]["step_i"].asInt();
		stOneConfig.iDecrease = jValue[i]["decrease_i"].asInt();

		m_vStatConfig.push_back(stOneConfig);
	}

	return 0;
}

//加载服务器状态配置
int BaseConfigManager::LoadServerStatConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(SERVERSTAT_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vServerStatConfig.clear();

	ServerStatConfig stOneConfig;
	for(unsigned i=0; i<jValue.size(); ++i)
	{
		stOneConfig.ID = jValue[i]["id"].asInt();
		stOneConfig.iValueMin = jValue[i]["Threshold_i_min"].asInt();
		stOneConfig.iValueMax = jValue[i]["Threshold_i_max"].asInt();
		stOneConfig.iDecrease = jValue[i]["Decrease_i"].asInt();

		m_vServerStatConfig.push_back(stOneConfig);
	}

	return 0;
}

//加载鱼阵的配置
int BaseConfigManager::LoadTrajectoryConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(TRAJECTORY_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_mTrajectoryConfig.clear();

	TrajectoryConfig stOneConfig;
	for(unsigned i=0; i<jValue.size(); ++i)
	{
		stOneConfig.ID = jValue[i]["Key"].asInt();
		stOneConfig.iType = jValue[i]["Id_i"].asInt();
		stOneConfig.iTraceID = jValue[i]["Trajectoryid_i"].asInt();
		stOneConfig.iStartTime = jValue[i]["Starttime_i"].asInt();
		stOneConfig.iFishID = jValue[i]["Fishid_i"].asInt();	
		stOneConfig.iInterval = jValue[i]["Intervaltime_i"].asInt();
		stOneConfig.iFishNumMax = jValue[i]["Number_i"].asInt();

		m_mTrajectoryConfig[stOneConfig.iType].push_back(stOneConfig);
	}

	return 0;
}

//加载鱼阵时间的配置
int BaseConfigManager::LoadFormTimeConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(FORMGLOBAL_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vFormTimeConfig.clear();

	FormTimeConfig stOneConfig;
	for(unsigned i=0; i<jValue.size(); ++i)
	{
		stOneConfig.ID = jValue[i]["id"].asInt();
		stOneConfig.iWeight = jValue[i]["weight_i"].asInt();
		stOneConfig.iPauseTime = jValue[i]["Pausetime_i"].asInt();

		m_vFormTimeConfig.push_back(stOneConfig);
	}

	return 0;
}

//加载捕鱼道具的配置
int BaseConfigManager::LoadFishItemConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(FISHITEM_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vFishItemConfig.clear();

	FishItemConfig stOneConfig;
	for (unsigned i = 0; i<jValue.size(); ++i)
	{
		stOneConfig.iItemID = jValue[i]["Id"].asInt();
		stOneConfig.iType = jValue[i]["Type_i"].asInt();
		stOneConfig.iCanSend = jValue[i]["Give_i"].asInt();
		stOneConfig.aiParam[0] = jValue[i]["Value_1_i"].asInt();
		stOneConfig.aiParam[1] = jValue[i]["Value_2_i"].asInt();
		stOneConfig.iTicketValue = jValue[i]["TicketValue"].asInt();
		//stOneConfig.aiParam[2] = jValue[i]["Value_3_i"].asInt();

		m_vFishItemConfig.push_back(stOneConfig);
	}

	return 0;
}

//加载小鱼组的配置
int BaseConfigManager::LoadSmallFishConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(SMALLFISH_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vSmallFishConfig.clear();

	SmallFishConfig stOneConfig;
	for (unsigned i = 0; i<jValue.size(); ++i)
	{
		stOneConfig.ID = jValue[i]["Id"].asInt();
		
		//小鱼组中鱼的位置
		//解析内容
		Json::Reader jReader;
		Json::Value jSmallFishTrackIDs;
		if (!jReader.parse(jValue[i]["Trackid"].asString(), jSmallFishTrackIDs))
		{
			//解析失败
			return -1;
		}

		stOneConfig.vTrackIDs.clear();
		for (unsigned j = 0; j < jSmallFishTrackIDs.size(); ++j)
		{
			stOneConfig.vTrackIDs.push_back(jSmallFishTrackIDs[j].asInt());
		}

		m_vSmallFishConfig.push_back(stOneConfig);
	}

	return 0;
}

//加载房间类型的配置
int BaseConfigManager::LoadRoomTypeConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(ROOMTYPE_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vFishRoomConfig.clear();

	FishRoomConfig stOneConfig;
	for (unsigned i = 0; i<jValue.size(); ++i)
	{
		stOneConfig.iRoomID = jValue[i]["Id"].asInt();
		stOneConfig.iRoomPattern = jValue[i]["Pattern_i"].asInt();
		stOneConfig.iRoomTypeID = jValue[i]["Type_i"].asInt();
		stOneConfig.iPlayerNum = jValue[i]["Player_i"].asInt();
		stOneConfig.iCoinLimit = jValue[i]["CoinLimitmin_i"].asInt();
		stOneConfig.iMinBatteryID = jValue[i]["Min_battery_i"].asInt();
		stOneConfig.iMaxBatteryID = jValue[i]["Max_battery_i"].asInt();
		stOneConfig.astDrops[0].iItemID = jValue[i]["Drop1_i"].asInt();
		stOneConfig.astDrops[0].iItemNum = jValue[i]["Drop1_number_i"].asInt();
		stOneConfig.astDrops[1].iItemID = jValue[i]["Drop2_i"].asInt();
		stOneConfig.astDrops[1].iItemNum = jValue[i]["Drop2_number_i"].asInt();
		stOneConfig.iAlgorithmType = jValue[i]["Algorithmtype_i"].asInt();
		stOneConfig.iRoomX1 = jValue[i]["Probability_i"].asInt();

		m_vFishRoomConfig.push_back(stOneConfig);
	}

	return 0;
}

//加载玩家初始化的配置
int BaseConfigManager::LoadBirthInitConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(BIRTHINIT_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vBirthInitConfig.clear();

	BirthInitConfig stOneConfig;
	for (unsigned i = 0; i<jValue.size(); ++i)
	{
		stOneConfig.iTypeID = jValue[i]["Type_i"].asInt();
		stOneConfig.iValue = jValue[i]["Value_i"].asInt();

		m_vBirthInitConfig.push_back(stOneConfig);
	}

	return 0;
}

//加载抽奖消耗的配置
int BaseConfigManager::LoadLotteryCostConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(LOTTERYCOST_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vLotteryCostConfig.clear();

	LotteryCostConfig stOneConfig;
	for (unsigned i = 0; i<jValue.size(); ++i)
	{
		stOneConfig.iCostID = jValue[i]["Id"].asInt();
		stOneConfig.iCost = jValue[i]["Cost_i"].asInt();
		stOneConfig.iVIPLevel = jValue[i]["Vip_i"].asInt();

		m_vLotteryCostConfig.push_back(stOneConfig);
	}

	return 0;
}

//加载抽奖奖项配置
int BaseConfigManager::LoadLotteryItemConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(LOTTERYITEM_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vLotteryItemConfig.clear();

	LotteryItemConfig stOneConfig;
	for (unsigned i = 0; i<jValue.size(); ++i)
	{
		stOneConfig.iLotteryItemID = jValue[i]["Id"].asInt();
		stOneConfig.stReward.iType = jValue[i]["Type_i"].asInt();
		stOneConfig.stReward.iRewardID = jValue[i]["Itemid_i"].asInt();
		stOneConfig.stReward.iRewardNum = jValue[i]["Value_i"].asInt();
		stOneConfig.iWeight = jValue[i]["Weight_i"].asInt();

		m_vLotteryItemConfig.push_back(stOneConfig);
	}

	return 0;
}

//加载开宝箱配置
int BaseConfigManager::LoadOpenBoxConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(OPENBOX_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vOpenBoxConfig.clear();

	OpenBoxConfig stOneConfig;
	for (unsigned i = 0; i<jValue.size(); ++i)
	{
		stOneConfig.iID = jValue[i]["Id"].asInt();

		char szKey[32] = {0};
		for (unsigned j = 0; j < sizeof(stOneConfig.astRewards) / sizeof(RewardConfig); ++j)
		{
			SAFE_SPRINTF(szKey, sizeof(szKey) - 1, "Itemtype%d_i", (j + 1));
			stOneConfig.astRewards[j].iType = jValue[i][szKey].asInt();

			SAFE_SPRINTF(szKey, sizeof(szKey) - 1, "Itemid%d_i", (j + 1));
			stOneConfig.astRewards[j].iRewardID = jValue[i][szKey].asInt();

			SAFE_SPRINTF(szKey, sizeof(szKey) - 1, "ItemNum%d_i", (j + 1));
			stOneConfig.astRewards[j].iRewardNum = jValue[i][szKey].asInt();
		}

		m_vOpenBoxConfig.push_back(stOneConfig);
	}

	return 0;
}

///加载任务配置
int BaseConfigManager::LoadQuestConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(QUEST_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vQuestConfig.clear();

	QuestConfig stOneConfig;
	for (unsigned i = 0; i<jValue.size(); ++i)
	{
		stOneConfig.iID = jValue[i]["Id"].asInt();
		stOneConfig.iType = jValue[i]["Type_i"].asInt();
		stOneConfig.iQuestIndex = jValue[i]["Questindex_i"].asInt();
		stOneConfig.iNextQuestID = jValue[i]["NextQuest_i"].asInt();
		stOneConfig.iNeedType = jValue[i]["QuestID_i"].asInt();
		stOneConfig.iCountdownTime = jValue[i]["Time_i"].asInt();

		char szKey[32] = { 0 };
		for (unsigned j = 0; j < sizeof(stOneConfig.alParam) / sizeof(long8); ++j)
		{
			SAFE_SPRINTF(szKey, sizeof(szKey)-1, "NeedParam%d_i", (j + 1));
			stOneConfig.alParam[j] = jValue[i][szKey].asDouble();
		}

		for (unsigned j = 0; j < sizeof(stOneConfig.astRewards) / sizeof(RewardConfig); ++j)
		{
			SAFE_SPRINTF(szKey, sizeof(szKey) - 1, "RewardType%d_i", (j+1));
			stOneConfig.astRewards[j].iType = jValue[i][szKey].asInt();

			SAFE_SPRINTF(szKey, sizeof(szKey) - 1, "RewardID%d_i", (j + 1));
			stOneConfig.astRewards[j].iRewardID = jValue[i][szKey].asInt();

			SAFE_SPRINTF(szKey, sizeof(szKey) - 1, "RewardNum%d_i", (j + 1));
			stOneConfig.astRewards[j].iRewardNum = jValue[i][szKey].asInt();
		}

		m_vQuestConfig.push_back(stOneConfig);
	}

	return 0;
}

//加载奇遇任务奖励配置
int BaseConfigManager::LoadAdventureRewardConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(ADVENTUREREWARD_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vAdventureRewardConfig.clear();

	AdventureRewardConfig stOneConfig;
	for (unsigned i = 0; i<jValue.size(); ++i)
	{
		stOneConfig.iID = jValue[i]["Id"].asInt();
		stOneConfig.iQuestIndex = jValue[i]["Questindex_i"].asInt();
		stOneConfig.iGunMultipleMin = jValue[i]["NumMin_i"].asInt();
		stOneConfig.iGunMultipleMax = jValue[i]["NumMax_i"].asInt();

		char szKey[32] = { 0 };
		for (unsigned j = 0; j < sizeof(stOneConfig.astRewards) / sizeof(RewardConfig); ++j)
		{
			SAFE_SPRINTF(szKey, sizeof(szKey) - 1, "RewardType%d_i", (j + 1));
			stOneConfig.astRewards[j].iType = jValue[i][szKey].asInt();

			SAFE_SPRINTF(szKey, sizeof(szKey) - 1, "RewardID%d_i", (j + 1));
			stOneConfig.astRewards[j].iRewardID = jValue[i][szKey].asInt();

			SAFE_SPRINTF(szKey, sizeof(szKey) - 1, "RewardNum%d_i", (j + 1));
			stOneConfig.astRewards[j].iRewardNum = jValue[i][szKey].asInt();
		}

		m_vAdventureRewardConfig.push_back(stOneConfig);
	}

	return 0;
}

//加载兑换的配置
int BaseConfigManager::LoadExchangeConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(EXCHANGE_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vExchangeConfig.clear();

	ExchangeConfig stOneConfig;
	for (unsigned i = 0; i<jValue.size(); ++i)
	{
		stOneConfig.iID = jValue[i]["ID"].asInt();
		stOneConfig.iType = jValue[i]["Type_i"].asInt();
		stOneConfig.iVIPLv = jValue[i]["VIPLv_i"].asInt();
		stOneConfig.iIsLimit = jValue[i]["Limited_i"].asInt();
		stOneConfig.iLimitNum = jValue[i]["Number_i"].asInt();
		stOneConfig.iCostType = jValue[i]["CostType_i"].asInt();
		stOneConfig.lCostNum = jValue[i]["CostNum_i"].asDouble();
		stOneConfig.stReward.iType = jValue[i]["ItemType_i"].asInt();
		stOneConfig.stReward.iRewardID = jValue[i]["ItemID_i"].asInt();
		stOneConfig.stReward.iRewardNum = jValue[i]["ItemNum_i"].asInt();
		stOneConfig.bIsNotice = jValue[i]["Notice_i"].asInt();

		m_vExchangeConfig.push_back(stOneConfig);
	}

	return 0;
}

//加载体验线配置
int BaseConfigManager::LoadExpLineConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(EXPLINE_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vExpLineConfig.clear();

	ExpLineConfig stOneConfig;
	for (unsigned i = 0; i<jValue.size(); ++i)
	{
		stOneConfig.iID = jValue[i]["Id"].asInt();
		stOneConfig.iBulletMax = jValue[i]["Numbermax_i"].asInt();
		stOneConfig.iAddRate = jValue[i]["Add_i"].asInt();
		stOneConfig.iType = jValue[i]["Type_i"].asInt();
		stOneConfig.iAlgorithmType = jValue[i]["Algorithmtype_i"].asInt();
		stOneConfig.iStandardNum = jValue[i]["Standard_i"].asInt();
		stOneConfig.iNextLine = jValue[i]["Next_i"].asInt();
		stOneConfig.bIsBegin = jValue[i]["IsBegin_i"].asInt();

		m_vExpLineConfig.push_back(stOneConfig);
	}

	return 0;
}

//加载活跃度奖励配置
int BaseConfigManager::LoadLivnessRewardConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(LIVNESS_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vLivnessRewardConfig.clear();

	LivnessRewardConfig stOneConfig;
	for (unsigned i = 0; i<jValue.size(); ++i)
	{
		stOneConfig.iID = jValue[i]["ID"].asInt();
		stOneConfig.iLivnessNum = jValue[i]["Liveness_i"].asInt();
		stOneConfig.iBoxItemID = jValue[i]["Reward_i"].asInt();

		m_vLivnessRewardConfig.push_back(stOneConfig);
	}

	return 0;
}

//加载邮件配置
int BaseConfigManager::LoadMailConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(MAIL_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vMailConfig.clear();

	MailConfig stOneConfig;
	for (unsigned i = 0; i<jValue.size(); ++i)
	{
		stOneConfig.iID = jValue[i]["Id"].asInt();
		stOneConfig.iType = jValue[i]["Type_i"].asInt();

		m_vMailConfig.push_back(stOneConfig);
	}

	return 0;
}

//加载VIP特权配置
int BaseConfigManager::LoadVIPConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(VIPLEVEL_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vVipLevelConfig.clear();

	VipLevelConfig stOneConfig;
	VipPriv stOnePriv;
	for (unsigned i = 0; i<jValue.size(); ++i)
	{
		stOneConfig.Reset();
		stOneConfig.iVIPLv = jValue[i]["Viplevel_i"].asInt();

		//查找是否已经出现过
		bool bExist = false;
		for (unsigned j = 0; j < m_vVipLevelConfig.size(); ++j)
		{
			if (m_vVipLevelConfig[j].iVIPLv != stOneConfig.iVIPLv)
			{
				continue;
			}

			//已经存在
			bExist = true;

			stOnePriv.iPrivType = jValue[i]["Contentid_i"].asInt();
			char szKey[32] = { 0 };
			for (unsigned k = 0; k < sizeof(stOnePriv.aiParams) / sizeof(int); ++k)
			{
				SAFE_SPRINTF(szKey, sizeof(szKey) - 1, "Variable%d_i", (k + 1));
				stOnePriv.aiParams[k] = jValue[i][szKey].asInt();
			}

			m_vVipLevelConfig[j].vPrivs.push_back(stOnePriv);
			break;
		}

		if (bExist)
		{
			//已经处理过
			continue;
		}

		//不存在
		stOneConfig.iNeedExp = jValue[i]["Experience_i"].asInt();

		stOnePriv.iPrivType = jValue[i]["Contentid_i"].asInt();
		char szKey[32] = { 0 };
		for (unsigned j = 0; j < sizeof(stOnePriv.aiParams) / sizeof(int); ++j)
		{
			SAFE_SPRINTF(szKey, sizeof(szKey) - 1, "Variable%d_i", (j + 1));
			stOnePriv.aiParams[j] = jValue[i][szKey].asInt();
		}
		stOneConfig.vPrivs.push_back(stOnePriv);

		m_vVipLevelConfig.push_back(stOneConfig);
	}

	return 0;
}

//加载充值抽奖配置
int BaseConfigManager::LoadLimitLotteryConfig()
{
	static const char* szLimitLotteryFile[LIMIT_LOTTERY_MAX] = {"", RECHARGELOTTERY_CONFIG_FILE, TICKETLOTTERY_CONFIG_FILE};

	for (int i = LIMIT_LOTTERY_INVALID + 1; i < LIMIT_LOTTERY_MAX; ++i)
	{
		Json::Value jValue;
		int iRet = LoadFile(szLimitLotteryFile[i], jValue);
		if (iRet)
		{
			//加载失败
			return iRet;
		}

		m_avLimitLotteryConfig[i].clear();

		LimitLotteryConfig stOneConfig;
		for (unsigned j = 0; j < jValue.size(); ++j)
		{
			stOneConfig.Reset();

			stOneConfig.iID = jValue[j]["Id"].asInt();
			stOneConfig.iWeight = jValue[j]["Weight_i"].asInt();
			stOneConfig.iDayNum = jValue[j]["DayTotal_i"].asInt();
			stOneConfig.iTotalNum = jValue[j]["Total_i"].asInt();
			stOneConfig.iLimitType = jValue[j]["LimitType_i"].asInt();
			stOneConfig.stReward.iType = jValue[j]["Type_i"].asInt();
			stOneConfig.stReward.iRewardID = jValue[j]["Item_i"].asInt();
			stOneConfig.stReward.iRewardNum = jValue[j]["Number_i"].asInt();

			m_avLimitLotteryConfig[i].push_back(stOneConfig);
		}
	}

	return 0;
}

//加载走马灯配置
int BaseConfigManager::LoadHorseLampConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(HORSELAMP_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vHorseLampConfig.clear();

	HorseLampConfig stOneConfig;
	for (unsigned i = 0; i<jValue.size(); ++i)
	{
		stOneConfig.Reset();

		stOneConfig.iID = jValue[i]["Id"].asInt();
		stOneConfig.iType = jValue[i]["Type_i"].asInt();
		stOneConfig.iTimes = jValue[i]["Scroll_i"].asInt();
		stOneConfig.aiNeeds[0] = jValue[i]["Condition1_i"].asInt();
		stOneConfig.aiNeeds[1] = jValue[i]["Condition2_i"].asInt();
		stOneConfig.aiNeeds[2] = jValue[i]["Condition3_i"].asInt();

		m_vHorseLampConfig.push_back(stOneConfig);
	}

	return 0;
}

//拉斯维加斯转盘配置
int BaseConfigManager::LoadLasvegasConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(LASVEGAS_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vLasvegasConfig.clear();

	LasvegasConfig stOneConfig;
	for (unsigned i = 0; i<jValue.size(); ++i)
	{
		stOneConfig.Reset();

		stOneConfig.iID = jValue[i]["Id"].asInt();
		stOneConfig.iNumber = jValue[i]["Number_i"].asInt();
		stOneConfig.iMinBet = jValue[i]["Min_i"].asInt();
		stOneConfig.iMaxBet = jValue[i]["Max_i"].asInt();
		stOneConfig.iWeight = jValue[i]["Probability_i"].asInt();

		m_vLasvegasConfig.push_back(stOneConfig);
	}

	return 0;
}

//加载登录奖励配置
int BaseConfigManager::LoadLoginRewardConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(LOGINREWARD_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vLoginRewardConfig.clear();

	LoginRewardConfig stOneConfig;
	for (unsigned i = 0; i<jValue.size(); ++i)
	{
		stOneConfig.Reset();

		stOneConfig.iID = jValue[i]["Id"].asInt();
		stOneConfig.stReward.iType = jValue[i]["RewardType_i"].asInt();
		stOneConfig.stReward.iRewardID = jValue[i]["RewardID_i"].asInt();
		stOneConfig.stReward.iRewardNum = jValue[i]["RewardNum_i"].asInt();

		m_vLoginRewardConfig.push_back(stOneConfig);
	}

	return 0;
}

//加载玩家充值配置
int BaseConfigManager::LoadRechargeConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(RECHARGE_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vRechargeConfig.clear();

	RechargeConfig stOneConfig;
	for (unsigned i = 0; i<jValue.size(); ++i)
	{
		stOneConfig.Reset();

		stOneConfig.iID = jValue[i]["Id"].asInt();
		stOneConfig.iCostRMB = jValue[i]["Rmb_i"].asInt();
		stOneConfig.stReward.iType = jValue[i]["RechargeType_i"].asInt();
		stOneConfig.stReward.iRewardID = jValue[i]["RechargeID_i"].asInt();
		stOneConfig.stReward.iRewardNum = jValue[i]["RechargeNum_i"].asInt();
		stOneConfig.iExtraNum = jValue[i]["ExtraNum_i"].asInt();

		m_vRechargeConfig.push_back(stOneConfig);
	}

	return 0;
}

//加载鱼王荣耀配置
int BaseConfigManager::LoadRankRewardConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(RANKREWARD_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vRankRewardConfig.clear();

	RankRewardConfig stOneConfig;
	for (unsigned i = 0; i<jValue.size(); ++i)
	{
		stOneConfig.Reset();

		stOneConfig.iID = jValue[i]["Id"].asInt();
		stOneConfig.stDayReward.iType = jValue[i]["Daily1Type_i"].asInt();
		stOneConfig.stDayReward.iRewardID = jValue[i]["Daily1Id_i"].asInt();
		stOneConfig.stDayReward.iRewardNum = jValue[i]["Daily1Num_i"].asInt();
		stOneConfig.stWeekReward.iType = jValue[i]["Week1Type_i"].asInt();
		stOneConfig.stWeekReward.iRewardID = jValue[i]["Week1Id_i"].asInt();
		stOneConfig.stWeekReward.iRewardNum = jValue[i]["Week1Num_i"].asInt();

		m_vRankRewardConfig.push_back(stOneConfig);
	}

	return 0;
}

//加载屏蔽字配置
int BaseConfigManager::LoadMaskWordConfig()
{
	std::string strConfigFile = m_strConfigDir + MASKWORD_CONFIG_FILE;

	return m_stMaskWordConfig.LoadConfig(strConfigFile.c_str());
}

//加载问号鱼配置
int BaseConfigManager::LoadMultipleFishConfig()
{
	Json::Value jValue;
	int iRet = LoadFile(QUESTIONFISH_CONFIG_FILE, jValue);
	if (iRet)
	{
		//加载失败
		return iRet;
	}

	m_vMultipleFishConfig.clear();

	MultipleFishConfig stOneConfig;
	for (unsigned i = 0; i < jValue.size(); ++i)
	{
		stOneConfig.iID = jValue[i]["Id"].asInt();
		stOneConfig.iRate = jValue[i]["Rate_i"].asInt();
		stOneConfig.iPower = jValue[i]["Power_i"].asInt();

		m_vMultipleFishConfig.push_back(stOneConfig);
	}

	return 0;
}

//重置配置
void BaseConfigManager::Reset()
{
	//配置文件路径
	m_strConfigDir.clear();

	//鱼的配置
	m_mFishConfig.clear();

	//轨迹点的配置
	m_mTraceConfig.clear();

	//炮台的配置
	m_vGunConfig.clear();

	//全局的配置
	m_vGlobalConfig.clear();

	//状态控制的配置
	m_vStatConfig.clear();

	//服务器状态的配置
	m_vServerStatConfig.clear();

	//鱼阵的配置
	m_mTrajectoryConfig.clear();

	//鱼阵时间的配置
	m_vFormTimeConfig.clear();

	//道具表的配置
	m_vFishItemConfig.clear();

	//小鱼组的配置
	m_vSmallFishConfig.clear();

	//捕鱼房间配置
	m_vFishRoomConfig.clear();

	//玩家初始配置
	m_vBirthInitConfig.clear();

	//玩家抽奖消耗配置
	m_vLotteryCostConfig.clear();

	//玩家抽奖奖项配置
	m_vLotteryItemConfig.clear();

	//玩家开宝箱配置
	m_vOpenBoxConfig.clear();

	//任务的配置
	m_vQuestConfig.clear();

	//奇遇任务奖励的配置
	m_vAdventureRewardConfig.clear();

	//兑换功能的配置
	m_vExchangeConfig.clear();

	//玩家体验线配置
	m_vExpLineConfig.clear();

	//玩家活跃度奖励配置
	m_vLivnessRewardConfig.clear();

	//邮件配置
	m_vMailConfig.clear();

	//VIP特权配置
	m_vVipLevelConfig.clear();

	//限量抽奖配置
	for (int i = 0; i < LIMIT_LOTTERY_MAX; ++i)
	{
		m_avLimitLotteryConfig[i].clear();
	}

	//走马灯配置
	m_vHorseLampConfig.clear();

	//拉斯维加斯转盘配置
	m_vLasvegasConfig.clear();

	//登录奖励配置
	m_vLoginRewardConfig.clear();

	//玩家充值配置
	m_vRechargeConfig.clear();

	//鱼王荣耀配置
	m_vRankRewardConfig.clear();

	//屏蔽字配置
	m_stMaskWordConfig.Reset();

	//多倍鱼配置
	m_vMultipleFishConfig.clear();
}

//获取鱼的配置
const FishConfig* BaseConfigManager::GetFishConfig(int iSeqID, int ID)
{
	FishSeqType::iterator it = m_mFishConfig.find(iSeqID);
	if(it == m_mFishConfig.end())
	{
		return NULL;
	}

	for(unsigned i=0; i<it->second.size(); ++i)
	{
		if((it->second)[i].id == ID)
		{
			//找到
			return &((it->second)[i]);
		}
	}

	return NULL;
}

//获取鱼的配置
const FishConfig* BaseConfigManager::GetFishConfig(int ID)
{
	for(FishSeqType::iterator it=m_mFishConfig.begin(); it!=m_mFishConfig.end(); ++it)
	{
		for(unsigned i=0; i<it->second.size(); ++i)
		{
			if((it->second)[i].id == ID)
			{
				//找到
				return &((it->second)[i]);
			}
		}
	}

	return NULL;
}

BaseConfigManager::BaseConfigManager()
{

}

//设置配置文件路径
void BaseConfigManager::SetConfigDir(const std::string& strConfigDir)
{
	m_strConfigDir = strConfigDir;
}

//加载配置文件
int BaseConfigManager::LoadFile(const std::string& strFileName, Json::Value& jValue)
{
	Json::Reader reader;

	std::string strRealFile = m_strConfigDir + strFileName;

	//从文件中读取
	std::ifstream is;
	is.open(strRealFile.c_str(), ios::binary);

	if (!reader.parse(is, jValue))
	{
		//解析json文件失败
		is.close();
		return -1;
	}

	is.close();

	return 0;
}
