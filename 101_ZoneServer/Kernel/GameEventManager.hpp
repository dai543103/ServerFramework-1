#pragma once

#include "GameProtocol.hpp"
#include "GameObjCommDef.hpp"
#include "Kernel/GameRole.hpp"
#include "CommDefine.h"

//服务端所有事件通知处理的入口

class CGameEventManager
{
public:

	//捕鱼通知
	static void NotifyKillFish(CGameRoleObj& stRoleObj, int iFishID, int iFishType, int iKillNum, int iResType, int iResNum);

	//使用技能通知
	static void NotifyUseSkill(CGameRoleObj& stRoleObj, int iSkillID, int iUseNum);

	//切换操作模式通知
	static void NotifyChangeOpera(CGameRoleObj& stRoleObj, int iOperaMode, int iNum);

	//抽奖通知
	static void NotifyLottery(CGameRoleObj& stRoleObj, int iLotteryNum);

	//完成任务通知
	static void NotifyFinQuest(CGameRoleObj& stRoleObj, int iQuestType, int iQuestID);

	//获得道具通知
	static void NotifyGetItem(CGameRoleObj& stRoleObj, int iItemType, int iItemID, int iItemNum);

	//登录通知
	static void NotifyLogin(CGameRoleObj& stRoleObj);

	//下线通知
	static void NotifyLogout(CGameRoleObj& stRoleObj);

	//发射子弹通知
	static void NotifyShootBullet(CGameRoleObj& stRoleObj, int iGunID, int iCost);

	//在线时间更新通知
	static void NotifyOnlineTime(CGameRoleObj& stRoleObj, int iTimeNow, int iAddOnlineTime);

	//定时器通知
    static void NotifyTick();
};
