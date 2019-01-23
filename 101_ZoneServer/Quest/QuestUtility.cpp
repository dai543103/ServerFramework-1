
#include "Kernel/ConfigManager.hpp"
#include "Kernel/GameRole.hpp"

#include "QuestUtility.h"

//捕鱼任务
void CQuestUtility::OnKillFish(CGameRoleObj& stRoleObj, int iModeID, int iRoomTypeID, int iFishID, int iKillNum, int iFishType)
{
	CQuestManager& stQuestMgr = stRoleObj.GetQuestManager();

	stQuestMgr.OnQuestNeedChange(QUEST_NEED_KILLFISH, iModeID, iRoomTypeID, iFishID, iKillNum, iFishType);
}

//捕鱼获得资源
void CQuestUtility::OnGetFishRes(CGameRoleObj& stRoleObj, int iModeID, int iRoomTypeID, int iResType, int iResNum)
{
	CQuestManager& stQuestMgr = stRoleObj.GetQuestManager();

	stQuestMgr.OnQuestNeedChange(QUEST_NEED_GETFISHRES, iModeID, iRoomTypeID, iResType, iResNum);
}

//使用技能
void CQuestUtility::OnUseSkill(CGameRoleObj& stRoleObj, int iModeID, int iRoomTypeID, int iSkillID, int iUseNum)
{
	CQuestManager& stQuestMgr = stRoleObj.GetQuestManager();

	stQuestMgr.OnQuestNeedChange(QUEST_NEED_USESKILL, iModeID, iRoomTypeID, iSkillID, iUseNum);
}

//切换操作模式
void CQuestUtility::OnChangeOpera(CGameRoleObj& stRoleObj, int iOperaMode, int iNum)
{
	CQuestManager& stQuestMgr = stRoleObj.GetQuestManager();

	stQuestMgr.OnQuestNeedChange(QUEST_NEED_CHANGEOPERA, iOperaMode, 0, 0, iNum);
}

//抽奖
void CQuestUtility::OnUserLottery(CGameRoleObj& stRoleObj, int iLotteryNum)
{
	CQuestManager& stQuestMgr = stRoleObj.GetQuestManager();

	stQuestMgr.OnQuestNeedChange(QUEST_NEED_LOTTERY, 0, 0, 0, iLotteryNum);
}

//完成任务
void CQuestUtility::OnFinQuest(CGameRoleObj& stRoleObj, int iQuestType, int iQuestID)
{
	CQuestManager& stQuestMgr = stRoleObj.GetQuestManager();

	stQuestMgr.OnQuestNeedChange(QUEST_NEED_FINQUEST, iQuestType, iQuestID, 0, 1);
}

//获得道具
void CQuestUtility::OnGetItem(CGameRoleObj& stRoleObj, int iItemType, int iItemID, int iItemNum)
{
	CQuestManager& stQuestMgr = stRoleObj.GetQuestManager();

	stQuestMgr.OnQuestNeedChange(QUEST_NEED_GETITEM, iItemType, iItemID, 0, iItemNum);
}

//登录天数
void CQuestUtility::OnLoginDay(CGameRoleObj& stRoleObj)
{
	CQuestManager& stQuestMgr = stRoleObj.GetQuestManager();

	//参数1：1表示累计登陆，2表示连续登陆
	stQuestMgr.OnQuestNeedChange(QUEST_NEED_LOGINDAY, 1, 0, 0, 1);
	stQuestMgr.OnQuestNeedChange(QUEST_NEED_LOGINDAY, 2, 0, 0, 1);
}

//在线时长
void CQuestUtility::OnRoleOnlineTime(CGameRoleObj& stRoleObj, int iAddOnlineTime)
{
	CQuestManager& stQuestMgr = stRoleObj.GetQuestManager();

	stQuestMgr.OnQuestNeedChange(QUEST_NEED_ONLINETIME, 1, 0, 0, iAddOnlineTime);
}

//发射子弹
void CQuestUtility::OnShootBullet(CGameRoleObj& stRoleObj, int iGunID, int iCost)
{
	CQuestManager& stQuestMgr = stRoleObj.GetQuestManager();

	stQuestMgr.OnShootBullet(iGunID, iCost);
}
