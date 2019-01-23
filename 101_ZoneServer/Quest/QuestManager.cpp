
#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "ErrorNumDef.hpp"
#include "Random.hpp"
#include "Kernel/ConfigManager.hpp"
#include "Kernel/GameRole.hpp"
#include "Kernel/UnitUtility.hpp"
#include "Kernel/GameEventManager.hpp"
#include "Kernel/ZoneOssLog.hpp"
#include "RepThings/RepThingsUtility.hpp"
#include "Resource/ResourceUtility.h"
#include "Reward/RewardUtility.h"

#include "QuestManager.h"

using namespace ServerLib;

static GameProtocolMsg stMsg;

CQuestManager::CQuestManager()
{
	m_uiUin = 0;

	m_vQuestData.clear();

	//玩家奇遇任务过期时间
	m_iAdventEndTime = 0;

	//玩家奇遇任务获得的次数
	m_iAdventNum = 0;

	//玩家奇遇任务下次刷新时间
	m_iAdventNextUpdateTime = 0;

	//玩家每日任务下次刷新时间
	m_iDailyNextUpdateTime = 0;

	//玩家奇遇阶段总流水
	m_iAdventUserCost = 0;

	//玩家奇遇阶段总发炮
	m_iAdventShootNum = 0;

	//已领取的活跃度宝箱ID
	m_vGetLivnessRewards.clear();
}

CQuestManager::~CQuestManager()
{
	m_uiUin = 0;

	m_vQuestData.clear();

	//玩家奇遇任务过期时间
	m_iAdventEndTime = 0;

	//玩家奇遇任务获得的次数
	m_iAdventNum = 0;

	//玩家奇遇任务下次刷新时间
	m_iAdventNextUpdateTime = 0;

	//玩家每日任务下次刷新时间
	m_iDailyNextUpdateTime = 0;

	//玩家奇遇阶段总流水
	m_iAdventUserCost = 0;

	//玩家奇遇阶段总发炮
	m_iAdventShootNum = 0;

	//已领取的活跃度宝箱ID
	m_vGetLivnessRewards.clear();
}

//初始化
int CQuestManager::Initialize()
{
	return 0;
}

void CQuestManager::SetOwner(unsigned int uin)
{
	m_uiUin = uin;
}

CGameRoleObj* CQuestManager::GetOwner()
{
	return CUnitUtility::GetRoleByUin(m_uiUin);
}

//完成任务
int CQuestManager::FinQuest(int iQuestID)
{
	QuestData* pstQuest = GetQuestByID(iQuestID);
	if (!pstQuest || pstQuest->bIsFin)
	{
		//任务不存在或者已完成
		LOGERROR("Failed to fin quest, invalid quest id %d, uin %u\n", iQuestID, m_uiUin);
		return T_ZONE_PARA_ERROR;
	}

	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();
	const QuestConfig* pstConfig = stBaseCfgMgr.GetQuestConfig(iQuestID);
	if (!pstConfig)
	{
		//找不到配置
		LOGERROR("Failed to get quest config, invalid quest id %d, uin %u\n", iQuestID, m_uiUin);
		return T_ZONE_INVALID_CFG;
	}

	if (pstQuest->iNum < pstConfig->alParam[3])
	{
		//还未完成
		LOGERROR("Failed to fin quest, need:real %lld:%lld, quest id %d, uin %u\n", pstConfig->alParam[3], pstQuest->iNum, iQuestID, m_uiUin);
		return T_ZONE_PARA_ERROR;
	}

	static GameProtocolMsg stMsg;
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_QUESTCHANGE_NOTIFY);
	Zone_QuestChange_Notify* pstNotify = stMsg.mutable_stbody()->mutable_m_stzone_questchange_notify();

	//可以完成任务
	int iRet = T_SERVER_SUCCESS;
	switch (pstQuest->iType)
	{
	case QUEST_TYPE_NEW:
	{
		//完成新手任务,直接删除
		DeleteQuest(iQuestID, *pstNotify->add_stchanges());

		//领取奖励
		iRet = GetQuestReward(iQuestID, pstQuest->iType ,pstConfig->astRewards, sizeof(pstConfig->astRewards)/sizeof(RewardConfig));
		if (iRet)
		{
			LOGERROR("Failed to get quest reward, ret %d, uin %u, quest id %d\n", iRet, m_uiUin, iQuestID);
			return iRet;
		}
	}
	break;

	case QUEST_TYPE_DAILY:
	{
		//完成每日任务
		UpdateQuest(iQuestID, QUEST_CHANGE_FIN, 0, *pstNotify->add_stchanges());

		//领取奖励
		iRet = GetQuestReward(iQuestID, pstQuest->iType,  pstConfig->astRewards, sizeof(pstConfig->astRewards) / sizeof(RewardConfig));
		if (iRet)
		{
			LOGERROR("Failed to get quest reward, ret %d, uin %u, quest id %d\n", iRet, m_uiUin, iQuestID);
			return iRet;
		}
	}
	break;

	case QUEST_TYPE_ACHIEVE:
	{
		//完成成就任务
		if (pstConfig->iNextQuestID == 0)
		{
			//最后一个成就任务
			UpdateQuest(iQuestID, QUEST_CHANGE_FIN, 0, *pstNotify->add_stchanges());
		}
		else
		{
			//成就任务完成度要继承
			long8 iAchieveNum = pstQuest->iNum;

			//删除成就任务
			DeleteQuest(iQuestID, *pstNotify->add_stchanges());

			//添加下一个任务
			const QuestConfig* pstNextConfig = stBaseCfgMgr.GetQuestConfig(pstConfig->iNextQuestID);
			if (!pstNextConfig)
			{
				LOGERROR("Failed to get quest config, quest id %d\n", pstConfig->iNextQuestID);
				return T_ZONE_INVALID_CFG;
			}

			AddQuest(pstNextConfig->iID, pstNextConfig->iType, pstNextConfig->iNeedType, *pstNotify->add_stchanges(), iAchieveNum);
		}
		
		//领取奖励
		iRet = GetQuestReward(iQuestID, pstQuest->iType, pstConfig->astRewards, sizeof(pstConfig->astRewards) / sizeof(RewardConfig));
		if (iRet)
		{
			LOGERROR("Failed to get quest reward, ret %d, uin %u, quest id %d\n", iRet, m_uiUin, iQuestID);
			return iRet;
		}
	}
	break;

	case QUEST_TYPE_ADVENTURE:
	{
		//完成奇遇任务
		int iGunMultiple = (m_iAdventShootNum == 0) ? 0 : (m_iAdventUserCost/m_iAdventShootNum);

		//删除奇遇任务
		DeleteQuest(iQuestID, *(pstNotify->add_stchanges()));
		m_iAdventEndTime = 0;
		m_iAdventUserCost = 0;
		m_iAdventShootNum = 0;

		if (pstConfig->iNextQuestID != 0)
		{
			//有下一阶段，增加新的奇遇任务
			const QuestConfig* pstNextConfig = stBaseCfgMgr.GetAdventQuestConfig(pstConfig->iNextQuestID);
			if (!pstNextConfig)
			{
				LOGERROR("Failed to get quest config, quest id %d\n", pstConfig->iNextQuestID);
				return T_ZONE_INVALID_CFG;
			}

			//增加奇遇任务
			AddQuest(pstNextConfig->iID, pstNextConfig->iType, pstNextConfig->iNeedType, *pstNotify->add_stchanges());
			m_iAdventEndTime = CTimeUtility::GetNowTime() + pstNextConfig->iCountdownTime;
		}
		else
		{
			//没有下一阶段
			++m_iAdventNum;
		}

		//领取奇遇任务奖励
		const AdventureRewardConfig* pstRewardConfig = stBaseCfgMgr.GetAdventureRewardConfig(pstConfig->iQuestIndex, iGunMultiple);
		if (!pstRewardConfig)
		{
			LOGERROR("Failed to get adventure reward config, quest index %d, gun multiple %d, uin %u\n", pstConfig->iQuestIndex, iGunMultiple, m_uiUin);
			return T_ZONE_INVALID_CFG;
		}

		//领取奖励
		iRet = GetQuestReward(iQuestID, pstQuest->iType, pstRewardConfig->astRewards, sizeof(pstRewardConfig->astRewards) / sizeof(RewardConfig));
		if (iRet)
		{
			LOGERROR("Failed to get quest reward, ret %d, uin %u, quest id %d\n", iRet, m_uiUin, iQuestID);
			return iRet;
		}
	}
	break;

	default:
		break;
	}

	pstNotify->set_iadventendtime(m_iAdventEndTime);
	pstNotify->set_iadventnum(m_iAdventNum);

	//推送任务变化的通知
	CZoneMsgHelper::SendZoneMsgToRole(stMsg, GetOwner());

	//完成任务
	CGameEventManager::NotifyFinQuest(*GetOwner(), pstConfig->iType, pstConfig->iID);

	return T_SERVER_SUCCESS;
}

//领取活跃度奖励
int CQuestManager::GetLivnessReward(int iRewardID)
{
	//是否已经领取过
	if (std::find(m_vGetLivnessRewards.begin(), m_vGetLivnessRewards.end(), iRewardID) != m_vGetLivnessRewards.end())
	{
		//已经领取过
		LOGERROR("Failed to get livness reward, already get , uin %u, reward id %d\n", m_uiUin, iRewardID);
		return T_ZONE_PARA_ERROR;
	}

	//读取配置
	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();
	const LivnessRewardConfig* pstLivenessConfig = stBaseCfgMgr.GetLivnessRewardConfig(iRewardID);
	if (!pstLivenessConfig)
	{
		LOGERROR("Failed to get liveness config, id %d\n", iRewardID);
		return T_ZONE_INVALID_CFG;
	}

	CGameRoleObj* pstRoleObj = GetOwner();
	if (!pstRoleObj)
	{
		return T_ZONE_PARA_ERROR;
	}

	//活跃度是否满足
	int iLivnessNum = pstRoleObj->GetResource(RESOURCE_TYPE_LIVENESS);
	if (iLivnessNum < pstLivenessConfig->iLivnessNum)
	{
		//活跃度不满足
		LOGERROR("Failed to get livness reward, livess num real:need %d:%d, uin %u\n", iLivnessNum, pstLivenessConfig->iLivnessNum, m_uiUin);
		return T_ZONE_PARA_ERROR;
	}

	//领取奖励
	const OpenBoxConfig* pstBoxConfig = stBaseCfgMgr.GetOpenBoxConfig(pstLivenessConfig->iBoxItemID);
	if (!pstBoxConfig)
	{
		LOGERROR("Failed to get open box config, item id %d\n", pstLivenessConfig->iBoxItemID);
		return T_ZONE_INVALID_CFG;
	}

	int iRet = CRewardUtility::GetReward(*pstRoleObj, 1, pstBoxConfig->astRewards, sizeof(pstBoxConfig->astRewards) / sizeof(RewardConfig), ITEM_CHANNEL_QUEST);
	if (iRet)
	{
		LOGERROR("Failed to add open box item, ret %d, uin %u, box id %d\n", iRet, m_uiUin, pstBoxConfig->iID);
		return iRet;
	}

	//打印运营日志 活跃度宝箱
	for (unsigned i = 0; i < sizeof(pstBoxConfig->astRewards) / sizeof(RewardConfig); ++i)
	{
		//开活跃度宝箱之前对应 奖项的数量
		long8 lOldNum = 0;

		const RewardConfig& stRewardConfig = pstBoxConfig->astRewards[i];
		switch (stRewardConfig.iType)
		{
		case REWARD_TYPE_RES:
		{
			lOldNum = pstRoleObj->GetResource(stRewardConfig.iType);
		}
		break;

		case REWARD_TYPE_ITEM:
		{
			lOldNum = pstRoleObj->GetRepThingsManager().GetRepItemNum(stRewardConfig.iType);
		}
		break;

		default:
			break;
		}

		CZoneOssLog::TraceLiveness(pstRoleObj->GetUin(), pstRoleObj->GetChannel(), pstRoleObj->GetNickName(), pstBoxConfig->iID, stRewardConfig.iType,
			stRewardConfig.iRewardID, lOldNum, lOldNum + stRewardConfig.iRewardNum);
	}
	

	//增加到已完成列表
	m_vGetLivnessRewards.push_back(iRewardID);

	return T_SERVER_SUCCESS;
}

//更新任务条件
int CQuestManager::OnQuestNeedChange(int iNeedType, int iParam1, int iParam2, int iParam3, int iParam4, int iExtraParam)
{
	static GameProtocolMsg stMsg;
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_QUESTCHANGE_NOTIFY);
	Zone_QuestChange_Notify* pstNotify = stMsg.mutable_stbody()->mutable_m_stzone_questchange_notify();

	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();
	for (unsigned i = 0; i < m_vQuestData.size(); ++i)
	{
		if (m_vQuestData[i].bIsFin || m_vQuestData[i].iNeedType != iNeedType)
		{
			continue;
		}

		//是对应任务
		const QuestConfig* pstQuestConfig = stBaseCfgMgr.GetQuestConfig(m_vQuestData[i].iQuestID);
		if (!pstQuestConfig)
		{
			LOGERROR("Failed to get quest config, invalid quest id %d, uin %u\n", m_vQuestData[i].iQuestID, m_uiUin);
			return T_ZONE_INVALID_CFG;
		}

		//判断是否满足条件
		switch (iNeedType)
		{
		case QUEST_NEED_KILLFISH:
		{
			//捕鱼任务
			if (pstQuestConfig->alParam[0] != 0 && (pstQuestConfig->alParam[0] & iParam1) == 0)
			{
				//不是对应房间模式
				continue;
			}

			if (pstQuestConfig->alParam[1] != 0 && pstQuestConfig->alParam[1] != iParam2)
			{
				//不是对应房间ID
				continue;
			}

			if (pstQuestConfig->alParam[2] > 100 && pstQuestConfig->alParam[2] != iParam3)
			{
				//鱼ID不匹配
				continue;
			}
			else if (pstQuestConfig->alParam[2] <= 100 && pstQuestConfig->alParam[2] > 0 && iExtraParam!=pstQuestConfig->alParam[2])
			{
				//鱼类型不匹配
				continue;
			}
		}
		break;

		case QUEST_NEED_GETFISHRES:
		case QUEST_NEED_USESKILL:
		case QUEST_NEED_FINQUEST:
		case QUEST_NEED_GETITEM:
		{
			if ((pstQuestConfig->alParam[0] != 0 && (pstQuestConfig->alParam[0] & iParam1) == 0) ||
				(pstQuestConfig->alParam[1] != 0 && pstQuestConfig->alParam[1] != iParam2) ||
				(pstQuestConfig->alParam[2] != 0 && pstQuestConfig->alParam[2] != iParam3))
			{
				//不是对应房间模式
				continue;
			}
		}
		break;

		case QUEST_NEED_CHANGEOPERA:
		case QUEST_NEED_LOTTERY:
		case QUEST_NEED_LOGINDAY:
		case QUEST_NEED_ONLINETIME:
		{
			if ((pstQuestConfig->alParam[0] != 0 && pstQuestConfig->alParam[0] != iParam1) ||
				(pstQuestConfig->alParam[1] != 0 && pstQuestConfig->alParam[1] != iParam2) ||
				(pstQuestConfig->alParam[2] != 0 && pstQuestConfig->alParam[2] != iParam3))
			{
				//不是对应房间模式
				continue;
			}
		}

		default:
			break;
		}

		//更新任务
		UpdateQuest(m_vQuestData[i].iQuestID, QUEST_CHANGE_UPDATE, iParam4, *pstNotify->add_stchanges());
	}

	if (pstNotify->stchanges_size() != 0)
	{
		pstNotify->set_iadventendtime(m_iAdventEndTime);
		pstNotify->set_iadventnum(m_iAdventNum);

		CZoneMsgHelper::SendZoneMsgToRole(stMsg, GetOwner());
	}

	return T_SERVER_SUCCESS;
}

//发射子弹通知
void CQuestManager::OnShootBullet(int iGunID, int iCost)
{
	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();

	int iDayAdventureNum = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_ADVENTURENUM);
	if (m_iAdventNum >= iDayAdventureNum)
	{
		//当天奇遇任务次数用完
		return;
	}

	//增加计数
	m_iAdventShootNum += 1;
	m_iAdventUserCost += iCost;

	if (m_iAdventEndTime != 0)
	{
		//当前有奇遇任务
		return;
	}

	//尝试领取新奇遇任务
	if (m_iAdventNum == 0)
	{
		int iBulletMin = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_ADVENTBULLETMIN);
		int iBulletMax = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_ADVENTBULLETMAX);

		if (m_iAdventShootNum < iBulletMin || m_iAdventShootNum >= CRandomCalculator::GetRandomNumber(iBulletMin, iBulletMax))
		{
			return;
		}
	}
	else
	{
		//第二次奇遇任务
		if (CRandomCalculator::GetRandomInRangeTenThousand() >= stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_SECONDADVENTRATE))
		{
			return;
		}
	}

	//领取奇遇任务
	const QuestConfig* pstNextConfig = stBaseCfgMgr.GetAdventQuestConfig(START_ADVENTURE_QUEST);
	if (!pstNextConfig)
	{
		LOGERROR("Failed to get adventure quest config, adventure quest id %d\n", START_ADVENTURE_QUEST);
		return;
	}

	//增加奇遇任务
	static GameProtocolMsg stMsg;
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_QUESTCHANGE_NOTIFY);
	Zone_QuestChange_Notify* pstNotify = stMsg.mutable_stbody()->mutable_m_stzone_questchange_notify();
	AddQuest(pstNextConfig->iID, pstNextConfig->iType, pstNextConfig->iNeedType, *pstNotify->add_stchanges());
	m_iAdventEndTime = CTimeUtility::GetNowTime() + pstNextConfig->iCountdownTime;

	CZoneMsgHelper::SendZoneMsgToRole(stMsg, GetOwner());

	return;
}

//定时器
void CQuestManager::OnTick(int iTimeNow)
{
	if (m_iAdventEndTime > iTimeNow && m_iAdventNextUpdateTime > iTimeNow && m_iDailyNextUpdateTime > iTimeNow)
	{
		//不需要更新
		return;
	}

	ResetQuest(false);

	return;
}

//更新任务到DB
void CQuestManager::UpdateQuestToDB(QUESTDBINFO& stQuestDBInfo)
{
	stQuestDBInfo.Clear();

	//任务数据
	for (unsigned i = 0; i < m_vQuestData.size(); ++i)
	{
		OneQuest* pstQuestInfo = stQuestDBInfo.add_stquestinfos();

		pstQuestInfo->set_iquestid(m_vQuestData[i].iQuestID);
		pstQuestInfo->set_iquesttype(m_vQuestData[i].iType);
		pstQuestInfo->set_ineedtype(m_vQuestData[i].iNeedType);
		pstQuestInfo->set_inum(m_vQuestData[i].iNum);
		pstQuestInfo->set_bisfin(m_vQuestData[i].bIsFin);
	}

	//奇遇任务刷新数据
	stQuestDBInfo.set_iadventureendtime(m_iAdventEndTime);
	stQuestDBInfo.set_iadventurenum(m_iAdventNum);
	stQuestDBInfo.set_iadventnextupdatetime(m_iAdventNextUpdateTime);
	stQuestDBInfo.set_iadventusercost(m_iAdventUserCost);
	stQuestDBInfo.set_iadventshootnum(m_iAdventShootNum);

	//每日任务刷新数据
	stQuestDBInfo.set_idailynextupdatetime(m_iDailyNextUpdateTime);

	//已领取活跃度奖励ID
	for (unsigned i = 0; i < m_vGetLivnessRewards.size(); ++i)
	{
		stQuestDBInfo.add_igetliverewardids(m_vGetLivnessRewards[i]);
	}

	return;
}

//从DB初始化任务
void CQuestManager::InitQuestFromDB(const QUESTDBINFO& stQuestDBInfo)
{
	//任务数据
	m_vQuestData.clear();

	QuestData stQuest;
	for (int i = 0; i < stQuestDBInfo.stquestinfos().size(); ++i)
	{
		stQuest.Reset();

		stQuest.iQuestID = stQuestDBInfo.stquestinfos(i).iquestid();
		stQuest.iType = stQuestDBInfo.stquestinfos(i).iquesttype();
		stQuest.iNeedType = stQuestDBInfo.stquestinfos(i).ineedtype();
		stQuest.iNum = stQuestDBInfo.stquestinfos(i).inum();
		stQuest.bIsFin = stQuestDBInfo.stquestinfos(i).bisfin();

		m_vQuestData.push_back(stQuest);
	}

	//奇遇任务刷新数据
	m_iAdventEndTime = stQuestDBInfo.iadventureendtime();
	m_iAdventNum = stQuestDBInfo.iadventurenum();
	m_iAdventNextUpdateTime = stQuestDBInfo.iadventnextupdatetime();
	m_iAdventUserCost = stQuestDBInfo.iadventusercost();
	m_iAdventShootNum = stQuestDBInfo.iadventshootnum();

	//每日任务刷新数据
	m_iDailyNextUpdateTime = stQuestDBInfo.idailynextupdatetime();

	//已领取活跃度奖励数据
	m_vGetLivnessRewards.clear();
	for (int i = 0; i < stQuestDBInfo.igetliverewardids_size(); ++i)
	{
		m_vGetLivnessRewards.push_back(stQuestDBInfo.igetliverewardids(i));
	}

	ResetQuest(true);

	return;
}

//添加任务
void CQuestManager::AddQuest(int iQuestID, int iType, int iNeedType, QuestChange& stChangeInfo, long8 iNum)
{
	QuestData stData;
	stData.iQuestID = iQuestID;
	stData.iType = iType;
	stData.iNeedType = iNeedType;
	stData.iNum = iNum;
	stData.bIsFin = false;

	m_vQuestData.push_back(stData);

	stChangeInfo.set_iquestid(iQuestID);
	stChangeInfo.set_ichangetype(QUEST_CHANGE_ADD);
	stChangeInfo.set_inum(iNum);

	return;
}

//更新任务
void CQuestManager::UpdateQuest(int iQuestID, int iChangeType, int iAddNum, QuestChange& stChangeInfo)
{
	for (unsigned i = 0; i < m_vQuestData.size(); ++i)
	{
		if (m_vQuestData[i].iQuestID != iQuestID)
		{
			continue;
		}

		m_vQuestData[i].iNum += iAddNum;
		if (iChangeType == QUEST_CHANGE_FIN)
		{
			m_vQuestData[i].bIsFin = true;
		}

		stChangeInfo.set_iquestid(iQuestID);
		stChangeInfo.set_ichangetype(iChangeType);
		stChangeInfo.set_inum(m_vQuestData[i].iNum);

		break;
	}

	return;
}

//删除任务
void CQuestManager::DeleteQuest(int iQuestID, QuestChange& stChangeInfo)
{
	for (unsigned i = 0; i < m_vQuestData.size(); ++i)
	{
		if (m_vQuestData[i].iQuestID == iQuestID)
		{
			m_vQuestData.erase(m_vQuestData.begin()+i);
			break;
		}
	}

	stChangeInfo.set_iquestid(iQuestID);
	stChangeInfo.set_ichangetype(QUEST_CHANGE_DELETE);

	return;
}

//获取任务
QuestData* CQuestManager::GetQuestByID(int iQuestID)
{
	for (unsigned i = 0; i < m_vQuestData.size(); ++i)
	{
		if (m_vQuestData[i].iQuestID == iQuestID)
		{
			return &m_vQuestData[i];
		}
	}

	return NULL;
}

//领取任务奖励
int CQuestManager::GetQuestReward(int iQuestID, int iQuestType, const RewardConfig* pstRewardConfig, int iNum)
{
	CGameRoleObj* pstRoleObj = GetOwner();

	if (!pstRewardConfig || !pstRoleObj)
	{
		return T_ZONE_PARA_ERROR;
	}

	static GameProtocolMsg stMsg;
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_GETREWARD_NOTIFY);
	Zone_GetReward_Notify* pstNotify = stMsg.mutable_stbody()->mutable_m_stzone_getreward_notify();
	pstNotify->set_iquestid(iQuestID);

	int iRet = T_SERVER_SUCCESS;
	for (int i = 0; i < iNum; ++i)
	{
		switch (pstRewardConfig[i].iType)
		{
		case REWARD_TYPE_ITEM:
		{
			//道具
			iRet = CRepThingsUtility::AddItemNum(*pstRoleObj, pstRewardConfig[i].iRewardID, pstRewardConfig[i].iRewardNum, ITEM_CHANNEL_QUEST);
			if (iRet)
			{
				LOGERROR("Failed to get reward, ret %d, uin %u, quest id %d, reward id %d\n", iRet, m_uiUin, iQuestID, pstRewardConfig[i].iRewardID);
				return iRet;
			}

			//打印运营日志
			long8 lNewNum = pstRoleObj->GetRepThingsManager().GetRepItemNum(pstRewardConfig[i].iRewardID);
			CZoneOssLog::TraceQuest(pstRoleObj->GetUin(), pstRoleObj->GetChannel(), pstRoleObj->GetNickName(), iQuestID, iQuestType, pstRewardConfig[i].iType, pstRewardConfig[i].iRewardID,
				lNewNum - pstRewardConfig[i].iRewardNum, lNewNum);

			RewardInfo* pstReward = pstNotify->add_strewards();
			pstReward->set_itype(pstRewardConfig[i].iType);
			pstReward->set_iid(pstRewardConfig[i].iRewardID);
			pstReward->set_inum(pstRewardConfig[i].iRewardNum);
		}
		break;

		case REWARD_TYPE_RES:
		{
			//资源
			if(!CResourceUtility::AddUserRes(*pstRoleObj, pstRewardConfig[i].iRewardID, pstRewardConfig[i].iRewardNum))
			{
				LOGERROR("Failed to get reward, ret %d, uin %u, quest id %d, reward id %d\n", iRet, m_uiUin, iQuestID, pstRewardConfig[i].iRewardID);
				return T_ZONE_INVALID_CFG;
			}

			//打印运营日志
			long8 lNewResNum = pstRoleObj->GetResource(pstRewardConfig[i].iRewardID);
			CZoneOssLog::TraceQuest(pstRoleObj->GetUin(), pstRoleObj->GetChannel(), pstRoleObj->GetNickName(), iQuestID, iQuestType,  pstRewardConfig[i].iType, pstRewardConfig[i].iRewardID,
				lNewResNum - pstRewardConfig[i].iRewardNum, lNewResNum);

			RewardInfo* pstReward = pstNotify->add_strewards();
			pstReward->set_itype(pstRewardConfig[i].iType);
			pstReward->set_iid(pstRewardConfig[i].iRewardID);
			pstReward->set_inum(pstRewardConfig[i].iRewardNum);
		}
		break;

		default:
			break;
		}
	}

	if (pstNotify->strewards_size() != 0)
	{
		//推送通知
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, GetOwner());
	}

	return T_SERVER_SUCCESS;
}

//重置任务
void CQuestManager::ResetQuest(bool bIsInit)
{
	int iTimeNow = CTimeUtility::GetNowTime();
	int iNextDayNowTime = iTimeNow + 24 * 60 * 60;

	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();

	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_QUESTCHANGE_NOTIFY);
	Zone_QuestChange_Notify* pstNotify = stMsg.mutable_stbody()->mutable_m_stzone_questchange_notify();

	bool bSendNotify = false;

	//日常任务
	if (m_iDailyNextUpdateTime <= iTimeNow)
	{
		//重置日常任务
		bSendNotify = true;
		m_iDailyNextUpdateTime = CTimeUtility::GetTodayTime(iNextDayNowTime, stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_DAILYRESETTIME));

		//刷新日常任务
		for (unsigned i = 0; i < m_vQuestData.size(); ++i)
		{
			if (m_vQuestData[i].iType != QUEST_TYPE_DAILY)
			{
				continue;
			}

			m_vQuestData[i].bIsFin = false;
			m_vQuestData[i].iNum = 0;

			QuestChange* pstOneChange = pstNotify->add_stchanges();
			pstOneChange->set_iquestid(m_vQuestData[i].iQuestID);
			pstOneChange->set_inum(m_vQuestData[i].iNum);
			pstOneChange->set_ichangetype(QUEST_CHANGE_UPDATE);
		}

		//刷新活跃度宝箱
		m_vGetLivnessRewards.clear();

		//重置活跃度
		CResourceUtility::AddUserRes(*GetOwner(), RESOURCE_TYPE_LIVENESS, -GetOwner()->GetResource(RESOURCE_TYPE_LIVENESS));
	}

	//奇遇任务
	if (m_iAdventNextUpdateTime <= iTimeNow)
	{
		//重置奇遇任务次数
		bSendNotify = true;
		m_iAdventNextUpdateTime = CTimeUtility::GetTodayTime(iNextDayNowTime, stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_ADVENTRESETTIME));

		//刷新奇遇任务次数
		m_iAdventNum = 0;
	}

	if (m_iAdventEndTime!=0 && m_iAdventEndTime<=iTimeNow)
	{
		//重置奇遇任务
		bSendNotify = true;
		m_iAdventEndTime = 0;
		m_iAdventUserCost = 0;
		m_iAdventShootNum = 0;

		for (unsigned i = 0; i < m_vQuestData.size(); ++i)
		{
			if (m_vQuestData[i].iType != QUEST_TYPE_ADVENTURE)
			{
				continue;
			}

			//删除奇遇任务
			DeleteQuest(m_vQuestData[i].iQuestID, *pstNotify->add_stchanges());
			break;
		}
	}

	if (!bIsInit && bSendNotify)
	{
		//需要推送通知
		pstNotify->set_iadventnum(m_iAdventNum);
		pstNotify->set_iadventendtime(m_iAdventEndTime);

		CZoneMsgHelper::SendZoneMsgToRole(stMsg, GetOwner());
	}

	return;
}
