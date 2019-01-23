#pragma once

//玩家任务管理器
#include <string.h>
#include <vector>

#include "GameProtocol.hpp"

//起始奇遇任务阶段
const static int START_ADVENTURE_QUEST = 1;

//任务数据
struct QuestData
{
	int iQuestID;	//任务ID
	int iType;		//任务类型
	int iNeedType;	//任务完成条件类型
	long8 iNum;		//任务当前进度数量
	bool bIsFin;	//任务是否已完成

	QuestData()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

class CGameRoleObj;
class RewardConfig;
class CQuestManager
{
public:
	CQuestManager();
	~CQuestManager();

public:

	//初始化
	int Initialize();

	void SetOwner(unsigned int uin);
	CGameRoleObj* GetOwner();

	//完成任务
	int FinQuest(int iQuestID);

	//领取活跃度奖励
	int GetLivnessReward(int iRewardID);

	//更新任务条件
	int OnQuestNeedChange(int iNeedType, int iParam1, int iParam2, int iParam3, int iParam4, int iExtraParam=0);

	//发射子弹通知
	void OnShootBullet(int iGunID, int iCost);

	//定时器
	void OnTick(int iTimeNow);

	//更新任务到DB
	void UpdateQuestToDB(QUESTDBINFO& stQuestDBInfo);

	//从DB初始化任务
	void InitQuestFromDB(const QUESTDBINFO& stQuestDBInfo);

private:

	//添加任务
	void AddQuest(int iQuestID, int iType, int iNeedType, QuestChange& stChangeInfo, long8 iNum=0);

	//更新任务
	void UpdateQuest(int iQuestID, int iChangeType, int iAddNum, QuestChange& stChangeInfo);

	//删除任务
	void DeleteQuest(int iQuestID, QuestChange& stChangeInfo);

	//获取任务
	QuestData* GetQuestByID(int iQuestID);

	//领取任务奖励
	int GetQuestReward(int iQuestID, int iQuestType, const RewardConfig* pstRewardConfig, int iNum);

	//重置任务
	void ResetQuest(bool bIsInit);

private:

	//玩家uin
	unsigned m_uiUin;

	//玩家拥有的任务
	std::vector<QuestData> m_vQuestData;

	//玩家奇遇任务过期时间
	int m_iAdventEndTime;

	//玩家奇遇任务获得的次数
	int m_iAdventNum;

	//玩家奇遇任务下次刷新时间
	int m_iAdventNextUpdateTime;

	//玩家每日任务下次刷新时间
	int m_iDailyNextUpdateTime;

	//玩家奇遇阶段总流水
	int m_iAdventUserCost;

	//玩家奇遇阶段总发炮
	int m_iAdventShootNum;

	//已领取的活跃度宝箱ID
	std::vector<int> m_vGetLivnessRewards;
};
