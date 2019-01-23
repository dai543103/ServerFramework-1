
#include "GameProtocol.hpp"
#include "AppDef.hpp"
#include "LogAdapter.hpp"
#include "Kernel/ModuleHelper.hpp"
#include "Kernel/ZoneObjectHelper.hpp"
#include "Kernel/GameSession.hpp"
#include "Kernel/GameRole.hpp"
#include "Kernel/SessionManager.hpp"
#include "Kernel/UnitUtility.hpp"
#include "Kernel/ZoneMsgHelper.hpp"
#include "Kernel/ZoneOssLog.hpp"
#include "FishGame/FishAlgorithm.h"
#include "FishGame/FishUtility.h"
#include "FishGame/FishpondObj.h"
#include "Quest/QuestUtility.h"
#include "Rank/RankInfoManager.h"
#include "Lottery/LasvegasManager.h"
#include "Mail/MailUtility.h"
#include "Login/LogoutHandler.hpp"

#include "GameEventManager.hpp"

//捕鱼通知
void CGameEventManager::NotifyKillFish(CGameRoleObj& stRoleObj, int iFishID, int iFishType, int iKillNum, int iResType, int iResNum)
{
	CFishpondObj* pstFishpondObj = CFishUtility::GetFishpondByID(stRoleObj.GetTableID());
	if (!pstFishpondObj)
	{
		LOGERROR("Failed to notify kill fish, uin %u, table id %d\n", stRoleObj.GetUin(), stRoleObj.GetTableID());
		return;
	}

	int iModeID = pstFishpondObj->GetRoomConfig()->iRoomPattern;
	int iRoomTypeID = pstFishpondObj->GetRoomConfig()->iRoomTypeID;

	//捕鱼条数
	CQuestUtility::OnKillFish(stRoleObj, iModeID, iRoomTypeID, iFishID, iKillNum, iFishType);

	//捕鱼获得资源
	CQuestUtility::OnGetFishRes(stRoleObj, iModeID, iRoomTypeID, iResType, iResNum);

	return;
}

//使用技能通知
void CGameEventManager::NotifyUseSkill(CGameRoleObj& stRoleObj, int iSkillID, int iUseNum)
{
	CFishpondObj* pstFishpondObj = CFishUtility::GetFishpondByID(stRoleObj.GetTableID());
	if (!pstFishpondObj)
	{
		LOGERROR("Failed to notify kill fish, uin %u, table id %d\n", stRoleObj.GetUin(), stRoleObj.GetTableID());
		return;
	}

	int iModeID = pstFishpondObj->GetRoomConfig()->iRoomPattern;
	int iRoomTypeID = pstFishpondObj->GetRoomConfig()->iRoomTypeID;

	//使用技能
	CQuestUtility::OnUseSkill(stRoleObj, iModeID, iRoomTypeID, iSkillID, iUseNum);

	return;
}

//切换操作模式通知
void CGameEventManager::NotifyChangeOpera(CGameRoleObj& stRoleObj, int iOperaMode, int iNum)
{
	//切换操作模式
	CQuestUtility::OnChangeOpera(stRoleObj, iOperaMode, iNum);
}

//抽奖通知
void CGameEventManager::NotifyLottery(CGameRoleObj& stRoleObj, int iLotteryNum)
{
	//抽奖
	CQuestUtility::OnUserLottery(stRoleObj, iLotteryNum);
}

//完成任务通知
void CGameEventManager::NotifyFinQuest(CGameRoleObj& stRoleObj, int iQuestType, int iQuestID)
{
	//完成任务
	CQuestUtility::OnFinQuest(stRoleObj, iQuestType, iQuestID);
}

//获得道具通知
void CGameEventManager::NotifyGetItem(CGameRoleObj& stRoleObj, int iItemType, int iItemID, int iItemNum)
{
	//使用道具
	CQuestUtility::OnGetItem(stRoleObj, iItemType, iItemID, iItemNum);
}

//登录通知
void CGameEventManager::NotifyLogin(CGameRoleObj& stRoleObj)
{
	int iLastLoginTime = stRoleObj.GetLastLoginTime();
	int iLoginTime = stRoleObj.GetLoginTime();

	//处理头像
	CGameSessionObj* pstSessionObj = CModuleHelper::GetSessionManager()->FindSessionByRoleID(stRoleObj.GetRoleID());
	if (pstSessionObj)
	{
		const char* pstrNewPicID = pstSessionObj->GetPictureID();
		if (strlen(pstrNewPicID) != 0 && stRoleObj.GetPicID() != pstrNewPicID)
		{
			//更新头像
			stRoleObj.SetPicID(pstrNewPicID);

			//设置为强制更新排行榜
			stRoleObj.SetUpdateRank(true);
		}
	}

	//设置为没有VIP金币补足
	stRoleObj.SetIsVIPAddCoins(false);

	if (!CTimeUtility::IsInSameDay(iLastLoginTime, iLoginTime))
	{
		//当天第一次登录

		//刷新10元话费兑换限量
		stRoleObj.GetExchangeManager().SetPersonLimit(PERSONLIMIT_TYPE_TENBILL, 0);

		//增加登录天数
		CQuestUtility::OnLoginDay(stRoleObj);

		//增加玩家登陆天数
		stRoleObj.SetLoginDays(stRoleObj.GetLoginDays()+1);

		//重置海盗宝藏抽奖次数
		stRoleObj.SetLotteryNum(0);

		//VIP特权金币补足
		const VipLevelConfig* pstVipConfig = CConfigManager::Instance()->GetBaseCfgMgr().GetVipConfig(stRoleObj.GetVIPLevel());
		if (pstVipConfig)
		{
			for (unsigned i = 0; i < pstVipConfig->vPrivs.size(); ++i)
			{
				if (pstVipConfig->vPrivs[i].iPrivType != VIP_PRIV_ADDCOINS)
				{
					continue;
				}

				if (stRoleObj.GetResource(RESOURCE_TYPE_COIN) < pstVipConfig->vPrivs[i].aiParams[2])
				{
					//获取补足金币之前的金币数量 运营日志使用
					long8 lOldCoinNum = stRoleObj.GetResource(RESOURCE_TYPE_COIN);

					//补足金币
					stRoleObj.SetResource(RESOURCE_TYPE_COIN, pstVipConfig->vPrivs[i].aiParams[2]);
					stRoleObj.SetIsVIPAddCoins(true);

					//打印运营日志 上线补足金币
					CZoneOssLog::TraceVipReward(stRoleObj.GetUin(), stRoleObj.GetChannel(), stRoleObj.GetNickName(), stRoleObj.GetVIPLevel(),
						lOldCoinNum, pstVipConfig->vPrivs[i].aiParams[2]);
				}

				break;
			}
		}

		//月卡礼包邮件
		int iBoxMailEndTime = (stRoleObj.GetMonthEndTime() < iLoginTime) ? stRoleObj.GetMonthEndTime() : (iLoginTime-24*60*60);
		int iLastMonthCardTime = stRoleObj.GetLastMonthTime();
		int iMonthBoxID = CConfigManager::Instance()->GetBaseCfgMgr().GetGlobalConfig(GLOBAL_TYPE_MONTHCARD_BOXID);
		while (!CTimeUtility::IsInSameDay(iLastMonthCardTime, iBoxMailEndTime))
		{
			if (iLastMonthCardTime > iBoxMailEndTime)
			{
				break;
			}

			//发送邮件
			CMailUtility::SendMonthCardMail(stRoleObj, iMonthBoxID, 1);
			iLastMonthCardTime += 24 * 60 * 60;
		}

		stRoleObj.SetLastMonthTime(iLastMonthCardTime);
	}
}

//下线通知
void CGameEventManager::NotifyLogout(CGameRoleObj& stRoleObj)
{
	int iTimeNow = CTimeUtility::GetNowTime();

	//在线时长
	CQuestUtility::OnRoleOnlineTime(stRoleObj, iTimeNow-stRoleObj.GetLoginTime());
}

//发射子弹通知
void CGameEventManager::NotifyShootBullet(CGameRoleObj& stRoleObj, int iGunID, int iCost)
{
	//发射子弹
	CQuestUtility::OnShootBullet(stRoleObj, iGunID, iCost);
}

//在线时间更新通知
void CGameEventManager::NotifyOnlineTime(CGameRoleObj& stRoleObj, int iTimeNow, int iAddOnlineTime)
{
	if (!CTimeUtility::IsInSameDay(stRoleObj.GetLastOnlineUpdate(), iTimeNow))
	{
		//已经隔天
		stRoleObj.SetLastOnlineUpdate(iTimeNow);
		stRoleObj.SetDayOnlineTime(0);

		return;
	}

	stRoleObj.SetLastOnlineUpdate(iTimeNow);
	stRoleObj.SetDayOnlineTime(stRoleObj.GetDayOnlineTime()+iAddOnlineTime);
	stRoleObj.SetOnlineTotalTime(stRoleObj.GetOnlineTotalTime() + iAddOnlineTime);

	//防沉迷开关是否开启
	if (CConfigManager::Instance()->GetBaseCfgMgr().GetGlobalConfig(GLOBAL_TYPE_WALLOWSWITCH))
	{
		//不是成年人且当天在线时长超过3小时
		if (stRoleObj.GetRealNameStat() != REAL_STAT_ADULT && stRoleObj.GetDayOnlineTime() >= 3 * 60 * 60)
		{
			//踢玩家下线
			CLogoutHandler::LogoutRole(&stRoleObj, LOGOUT_REASON_WALLOW);
		}
	}

	return;
}

void CGameEventManager::NotifyTick()
{
    // 根据系统系统的闲忙状态, 动态调整系统负载
    EGameServerStatus enServerStatus = g_enServerStatus;

	//全局算法Tick
	FishAlgorithm::OnTick();

	//排行榜Tick
	CRankInfoManager::Instance()->OnTick();

	//大转盘Tick
	CLasvegasManager::Instance()->OnTick();

    int iNumber = (enServerStatus == GAME_SERVER_STATUS_BUSY) ? 2 : 200;
    for (int i = 0; i < iNumber; i++)
    {
        int iRoleIdx = CUnitUtility::IterateRoleIdx();
        if (iRoleIdx < 0)
        {
            break;
        }

        CGameRoleObj* pRoleObj = GameTypeK32<CGameRoleObj>::GetByIdx(iRoleIdx);
        if (!pRoleObj)
        {
            continue;
        }

        TUNITINFO* pUnitInfo = &pRoleObj->GetRoleInfo().stUnitInfo;
        // 尝试删除单位
        if (CUnitUtility::IsUnitStatusSet(pUnitInfo, EGUS_DELETE))
        {
            CUnitUtility::DeleteUnit(pUnitInfo);
            continue;
        }

        // 角色Tick
        pRoleObj->OnTick();
    }

    return;
}
