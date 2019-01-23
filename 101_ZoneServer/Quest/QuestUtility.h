#pragma once

//任务帮助类

class CGameRoleObj;
class CQuestUtility
{
public:

	//捕鱼任务
	static void OnKillFish(CGameRoleObj& stRoleObj, int iModeID, int iRoomTypeID, int iFishID, int iKillNum, int iFishType);

	//捕鱼获得资源
	static void OnGetFishRes(CGameRoleObj& stRoleObj, int iModeID, int iRoomTypeID, int iResType, int iResNum);

	//使用技能
	static void OnUseSkill(CGameRoleObj& stRoleObj, int iModeID, int iRoomTypeID, int iSkillID, int iUseNum);

	//切换操作模式
	static void OnChangeOpera(CGameRoleObj& stRoleObj, int iOperaMode, int iNum);

	//抽奖
	static void OnUserLottery(CGameRoleObj& stRoleObj, int iLotteryNum);

	//完成任务
	static void OnFinQuest(CGameRoleObj& stRoleObj, int iQuestType, int iQuestID);

	//获得道具
	static void OnGetItem(CGameRoleObj& stRoleObj, int iItemType, int iItemID, int iItemNum);

	//登录天数
	static void OnLoginDay(CGameRoleObj& stRoleObj);

	//在线时长
	static void OnRoleOnlineTime(CGameRoleObj& stRoleObj, int iAddOnlineTime);

	//发射子弹
	static void OnShootBullet(CGameRoleObj& stRoleObj, int iGunID, int iCost);
};
