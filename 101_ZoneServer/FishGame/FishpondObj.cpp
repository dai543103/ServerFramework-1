
#include "GameProtocol.hpp"
#include "ErrorNumDef.hpp"
#include "Kernel/GameRole.hpp"
#include "Kernel/ZoneObjectHelper.hpp"
#include "Kernel/ZoneMsgHelper.hpp"
#include "Kernel/ModuleHelper.hpp"
#include "Kernel/ConfigManager.hpp"
#include "Kernel/UnitUtility.hpp"
#include "Kernel/GameEventManager.hpp"
#include "Kernel/ZoneOssLog.hpp"
#include "Resource/ResourceUtility.h"
#include "RepThings/RepThingsUtility.hpp"
#include "Chat/ChatUtility.h"

#include "FishUtility.h"
#include "FishAlgorithm.h"

#include "FishpondObj.h"

using namespace ServerLib;

//发送给客户端的消息
GameProtocolMsg CFishpondObj::ms_stZoneMsg;

IMPLEMENT_DYN(CFishpondObj)

CFishpondObj::CFishpondObj()
{
	Reset();
}

CFishpondObj::~CFishpondObj()
{
	Reset();
}

int CFishpondObj::Initialize()
{
	return T_SERVER_SUCCESS;
}

int CFishpondObj::Resume()
{
	return T_SERVER_SUCCESS;
}

//设置鱼池信息
void CFishpondObj::SetTableInfo(unsigned uTableUniqID, int iFishRoomID, const FishRoomConfig& stConfig)
{
	//鱼池ID
	m_uTableID = uTableUniqID;

	//鱼池所在房间ID
	m_iFishRoomID = iFishRoomID;

	//鱼池配置
	m_pstRoomConfig = &stConfig;

	return;
}

//玩家进入鱼池
int CFishpondObj::EnterFishpond(CGameRoleObj& stRoleObj)
{
	//玩家坐下
	int iRet = PlayerSitDown(stRoleObj);
	if (iRet)
	{
		LOGERROR("Failed to sit in fishpond, room id %d, uin %u, ret %d\n", m_pstRoomConfig->iRoomID, stRoleObj.GetUin(), iRet);
		return iRet;
	}

	if (m_vSeatUserData.size() <= 1)
	{
		//重新初始化鱼
		iRet = InitFishInfo();
		if (iRet)
		{
			LOGERROR("Failed to init fishpond data, table id %d, ret %d\n", m_uTableID, iRet);
			return iRet;
		}

		LOGDEBUG("Success to init fishpond data, table id %d!\n", m_uTableID);
	}

	//推送服务器时间给玩家
	CUnitUtility::SendSyncTimeNotify(&stRoleObj, CTimeUtility::GetMSTime());

	//推送自己座位信息给所有
	SendSeatUserInfo(NULL, &stRoleObj);

	//推送桌子信息给玩家
	SendFishpondInfoToUser(stRoleObj);

	//推送鱼阵时间
	SendFishFormTime(&stRoleObj, true);
	SendFishFormTime(&stRoleObj, false);

	//推送鱼阵详细信息
	SendFishFormInfo(&stRoleObj);

	//设置玩家桌子
	stRoleObj.SetTableID(m_uTableID);

	//获取基础配置管理器
	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();

	//获取房间算法数据
	RoomAlgorithmData& stData = FishAlgorithm::astData[m_pstRoomConfig->iAlgorithmType];

	//增加在线玩家数
	++stData.lPlayingNum;

	//增加全局炮台倍数
	const GunConfig* pstGunConfig = stBaseCfgMgr.GetGunConfig(stRoleObj.GetWeaponInfo().iWeaponID);
	if (pstGunConfig)
	{
		stData.lTotalGunMultiple += pstGunConfig->iMultiple;
	}

	//打印运营日志 todo jasonxiong 整理后统一添加
	//pGameLogic->OSSUserEnterLeaveTable(player, true, pstUserInfo->lTotalSilver, pstUserInfo->lTokens);

	return T_SERVER_SUCCESS;
}

//玩家退出鱼池
void CFishpondObj::ExitFishpond(CGameRoleObj& stRoleObj, bool bForceExit)
{
	SeatUserData* pstUserData = GetSeatUserByUin(stRoleObj.GetUin());
	if (!pstUserData)
	{
		return;
	}

	//删除玩家所有子弹
	ClearUserBullets(pstUserData->iSeat);

	//玩家周期结算日志
	CZoneOssLog::TraceCycleProfit(stRoleObj.GetUin(), stRoleObj.GetChannel(), stRoleObj.GetNickName(), -pstUserData->iCycleWinNum, pstUserData->lCycleNewUsedNum,
		stRoleObj.GetResource(RESOURCE_TYPE_COIN), stRoleObj.GetResource(RESOURCE_TYPE_TICKET));

	//获取基础配置管理器
	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();

	//获取房间算法数据
	RoomAlgorithmData& stData = FishAlgorithm::astData[m_pstRoomConfig->iAlgorithmType];

	//减少全局炮台倍数
	const GunConfig* pstGunConfig = stBaseCfgMgr.GetGunConfig(stRoleObj.GetWeaponInfo().iWeaponID);
	if (pstGunConfig && stData.lTotalGunMultiple >= pstGunConfig->iMultiple)
	{
		stData.lTotalGunMultiple -= pstGunConfig->iMultiple;
	}

	//推送退出椅子消息
	SendExitFishpondAll(pstUserData->iSeat, bForceExit);

	//删除玩家鱼池数据
	for (unsigned i = 0; i < m_vSeatUserData.size(); ++i)
	{
		if (m_vSeatUserData[i].iSeat == pstUserData->iSeat)
		{
			m_vSeatUserData.erase(m_vSeatUserData.begin() + i);
			break;
		}
	}

	stRoleObj.SetTableID(0);

	//减少在线人数
	--stData.lPlayingNum;

	//打印运营日志 todo jasonxiong 统一再来处理
	//m_pGameLogic->OSSUserEnterLeaveTable(player, false, pstUserData->lTotalSilver, pstUserData->lTokens);

	return;
}

//玩家切换炮台
int CFishpondObj::ChangeGun(CGameRoleObj& stRoleObj, int iNewGunID, bool bIsStyle)
{
	SeatUserData* pstUserData = GetSeatUserByUin(stRoleObj.GetUin());
	if (!pstUserData)
	{
		//不在该鱼池中
		LOGERROR("Failed to change gun, table id %u, uin %u\n", m_uTableID, stRoleObj.GetUin());
		return T_ZONE_PARA_ERROR;
	}

	//刷新活跃时间
	pstUserData->iActiveTime = CTimeUtility::GetNowTime();

	if (bIsStyle)
	{
		//切换炮台样式
		TWEAPONINFO& stWeaponInfo = stRoleObj.GetWeaponInfo();

		bool bHasStyle = false;
		for (unsigned i = 0; i < sizeof(stWeaponInfo.aiUnlockStyleIDs) / sizeof(int); ++i)
		{
			if (stWeaponInfo.aiUnlockStyleIDs[i] == 0)
			{
				break;
			}

			if (stWeaponInfo.aiUnlockStyleIDs[i] == iNewGunID)
			{
				//已经解锁
				bHasStyle = true;
				break;
			}
		}

		if (!bHasStyle)
		{
			LOGERROR("Failed to change gun style, new style id %d, uin %u\n", iNewGunID, stRoleObj.GetUin());
			return T_ZONE_PARA_ERROR;
		}

		//切换样式
		stWeaponInfo.iStyleID = iNewGunID;
	}
	else
	{
		//切换炮台倍数

		if (iNewGunID<m_pstRoomConfig->iMinBatteryID || iNewGunID>m_pstRoomConfig->iMaxBatteryID)
		{
			LOGERROR("Failed to change gun, invalid gun id %d\n", iNewGunID);
			return T_ZONE_PARA_ERROR;;
		}

		//获取基础配置管理器
		BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();

		//读取炮台配置
		const GunConfig* pstConfig = stBaseCfgMgr.GetGunConfig(iNewGunID);
		if (!pstConfig)
		{
			//找不到配置
			return T_ZONE_INVALID_CFG;
		}

		//读取老的炮台配置
		const GunConfig* pstOldConfig = stBaseCfgMgr.GetGunConfig(stRoleObj.GetWeaponInfo().iWeaponID);
		if (!pstOldConfig)
		{
			//找不到配置
			return T_ZONE_INVALID_CFG;
		}

		stRoleObj.GetWeaponInfo().iWeaponID = iNewGunID;
		long8 lTimeNow = CTimeUtility::GetMSTime();
		if (pstOldConfig->iMultiple < pstConfig->iMultiple && pstUserData->lUnEffectTime >= lTimeNow)
		{
			//向上切炮台并且有红包,红包失效
			pstUserData->lUnEffectTime = 0;
		}

		//修改全局炮台倍数
		FishAlgorithm::astData[m_pstRoomConfig->iAlgorithmType].lTotalGunMultiple += (pstConfig->iMultiple - pstOldConfig->iMultiple);
	}

	//推送炮台切换的通知
	SendChangeGunNotify(*pstUserData, iNewGunID, bIsStyle);

	return T_SERVER_SUCCESS;
}

//玩家发射子弹
int CFishpondObj::ShootBullet(CGameRoleObj& stRoleObj, long lShootTime, int iPosX, int iPosY, bool bAutoShoot)
{
	SeatUserData* pstUserData = GetSeatUserByUin(stRoleObj.GetUin());
	if (!pstUserData)
	{
		return T_ZONE_PARA_ERROR;
	}

	long lTimeNow = CTimeUtility::GetMSTime();

	//刷新活跃时间
	pstUserData->iActiveTime = lTimeNow/1000;

	if ((bAutoShoot && !pstUserData->bAutoShoot))
	{
		//自动发炮状态错误
		LOGERROR("Failed to shoot bullet, uin %u, auto shoot server:client %d:%d\n", stRoleObj.GetUin(), pstUserData->bAutoShoot, bAutoShoot);
		return T_ZONE_PARA_ERROR;
	}

	/*
	if(abs(iPosX)!=CLIENT_SCREEN_WIDTH/2 && abs(iPosY)!=CLIENT_SCREEN_HEIGHT/2)
	{
	//目标位置不对
	return GAME_ERRORNO_INVALIDPOS;
	}
	*/

	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();

	//是否超过发射子弹最大数量
	if (pstUserData->iBulletNum >= stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_MAXBULLETNUM))
	{
		//超过最大发射数量
		return T_ZONE_INVALID_BULLETNUM;
	}

	//是否符合发射速度要求
	if ((pstUserData->alShootTime[pstUserData->iIndex] + 1000) >= lTimeNow)
	{
		//不满足速度要求
		return T_ZONE_INVALID_BULLETNUM;
	}

	//判断积分是否满足要求
	const GunConfig* pstGunConfig = stBaseCfgMgr.GetGunConfig(stRoleObj.GetWeaponInfo().iWeaponID);
	if (!pstGunConfig)
	{
		return T_ZONE_INVALID_CFG;
	}

	//是否狂暴
	if (pstUserData->lWildEndTime + 100 < lTimeNow)
	{
		//不在狂暴状态
		pstUserData->iWildNum = 0;
	}

	int iConsume = (pstUserData->iWildNum == 0) ? pstGunConfig->iConsume : (pstGunConfig->iConsume*pstUserData->iWildNum * 2);
	if (stRoleObj.GetResource(RESOURCE_TYPE_COIN) < iConsume)
	{
		if (stRoleObj.GetResource(RESOURCE_TYPE_COIN) <= 0 ||
			stRoleObj.GetWeaponInfo().iWeaponID != m_pstRoomConfig->iMinBatteryID)
		{
			return T_ZONE_INVALID_NOSILVER;
		}
		else
		{
			iConsume = stRoleObj.GetResource(RESOURCE_TYPE_COIN);
		}
	}

	//扣除银子
	if (!CResourceUtility::AddUserRes(stRoleObj, RESOURCE_TYPE_COIN, -iConsume))
	{
		//扣除银子失败
		LOGERROR("Failed to add user silver, uin %u, add silver %d\n", stRoleObj.GetUin(), -iConsume);
		return T_ZONE_INVALID_NOSILVER;
	}

	pstUserData->lUserSilverCost += iConsume;	//增加玩家积分消耗
	pstUserData->lUserFlow[CYCLE_TIME_NOW] += iConsume;
	pstUserData->iLossNum += iConsume;
	pstUserData->iCycleWinNum -= iConsume;

	//获取房间算法数据
	RoomAlgorithmData& stData = FishAlgorithm::astData[m_pstRoomConfig->iAlgorithmType];
	stData.lTotalRewardSilver += iConsume;
	stData.lRewardSilver += iConsume;
	stData.lTodayRewardSilver += iConsume;

	//加总发射的子弹数
	++stData.lTotalBulletNum;

	//更新红包信息
	UpdateRedPacketInfo(stRoleObj, *pstUserData);

	//增加子弹信息
	BulletData stBulletInfo;
	stBulletInfo.uUniqueID = CFishUtility::GetBulletUniqID();
	stBulletInfo.iGunID = stRoleObj.GetWeaponInfo().iWeaponID;
	stBulletInfo.iSeat = pstUserData->iSeat;
	stBulletInfo.stTargetPos.iX = iPosX;
	stBulletInfo.stTargetPos.iY = iPosY;
	stBulletInfo.lShootTime = lShootTime;
	stBulletInfo.uFishUniqID = (pstUserData->lAimEndTime + 100 < lTimeNow) ? 0 : pstUserData->uAimFishUniqID;
	stBulletInfo.iWildNum = pstUserData->iWildNum;
	stBulletInfo.iCost = iConsume;
	stBulletInfo.iFishIndex = pstUserData->iAimFishIndex;

	m_vBulletData.push_back(stBulletInfo);

	//打印运营日志
	CZoneOssLog::TraceShootBullet(stRoleObj.GetUin(), stRoleObj.GetChannel(), stRoleObj.GetNickName(), iConsume, m_pstRoomConfig->iRoomID);

	//更新玩家发射信息
	++pstUserData->iBulletNum;

	pstUserData->alShootTime[pstUserData->iIndex] = lTimeNow;
	pstUserData->iIndex = (++pstUserData->iIndex) % MAX_BULLET_PER_SECOND;

	//体验线相关处理
	const ExpLineConfig* pstExpLineConfig = NULL;
	TEXPLINEINFO* pstExpLineInfo = GetRoleExpLineInfo(stRoleObj.GetUin(), pstExpLineConfig);
	if (pstExpLineInfo)
	{
		pstExpLineInfo->lUserWinNum -= iConsume;
		pstExpLineInfo->lIntervalWinNum -= iConsume;
		pstExpLineInfo->lCostNum += iConsume;
		++pstExpLineInfo->iBulletNum;
	}

	//处理新手红包
	if (stRoleObj.GetNowNewRedNum() == 0 && stRoleObj.GetRemainNewRedNum()>0 &&
		stRoleObj.GetResource(RESOURCE_TYPE_COIN) <= stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_NEWREMAINCOINS))
	{
		int iAddNewRedNum = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_NEWREDNUM) / stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_NEWREDTIMES);
		if (stRoleObj.GetRemainNewRedNum() >= iAddNewRedNum)
		{
			//增加玩家新手红包可用额度
			stRoleObj.SetRemainNewRedNum(stRoleObj.GetRemainNewRedNum() - iAddNewRedNum);
			stRoleObj.SetNowNewRedNum(stRoleObj.GetNowNewRedNum() + iAddNewRedNum);
		}
	}

	//LOGDEBUG("ShootBullet, uin %u, seat %d, shoot time %ld, pos %d:%d, cost %d\n",
	//	stRoleObj.GetUin(), pstUserData->iSeat, lShootTime, iPosX, iPosY, iConsume);

	//todo jasonxiong 统一再来整
	//打印运营日志 玩家发射子弹 ID = 2
	//m_pGameLogic->OSSUserShootBullet(player, pstUserData->iGunID, stBulletInfo.uUniqueID, iConsume);

	//推送子弹消息
	SendShootBulletNotify(stBulletInfo);

	if (!IsRoomPattern(ROOM_PATTERN_WARHEAD))
	{
		//发射子弹的通知,非弹头场才通知
		CGameEventManager::NotifyShootBullet(stRoleObj, stRoleObj.GetWeaponInfo().iWeaponID, iConsume);
	}

	return T_SERVER_SUCCESS;
}

//玩家命中鱼
int CFishpondObj::HitFish(CGameRoleObj& stRoleObj, long lHitTime, unsigned uBulletUniqID, unsigned uFishUniqID, int iFishIndex)
{
	int iFishID = 0;
	int iCostSilver = 0;
	int iRewardSilver = 0;

	//获取玩家信息
	SeatUserData* pstUserData = GetSeatUserByUin(stRoleObj.GetUin());
	if (!pstUserData)
	{
		return T_ZONE_PARA_ERROR;
	}

	long lTimeNow = CTimeUtility::GetMSTime();

	//刷新活跃时间
	pstUserData->iActiveTime = lTimeNow / 1000;

	//获取子弹信息
	BulletData* pBulletInfo = GetBulletData(uBulletUniqID);
	if (!pBulletInfo || pBulletInfo->iSeat != pstUserData->iSeat)
	{
		//找不到子弹或子弹不是玩家发出的
		return T_ZONE_INVALID_BULLETNUM;
	}

	//减玩家的子弹数
	if (pstUserData->iBulletNum > 0)
	{
		--pstUserData->iBulletNum;
	}

	iCostSilver = pBulletInfo->iCost;

	//获取鱼的信息
	FishData* pFishInfo = GetFishData(uFishUniqID);
	if (!pFishInfo)
	{
		//找不到鱼
		DeleteBulletData(uBulletUniqID);	//删除子弹

											//推送命中鱼的消息
		SendHitFishInfoNotify(*pstUserData, uBulletUniqID, uFishUniqID, 0, false, iCostSilver, iFishIndex, 0);

		return 0;
	}

	iFishID = pFishInfo->iFishID;
	pFishInfo->aiCostSilver[pstUserData->iSeat] += iCostSilver;

	if (abs(lHitTime - lTimeNow) >= 1000)
	{
		//命中时间非法,删除子弹
		DeleteBulletData(uBulletUniqID);
		
		//推送命中鱼的消息
		SendHitFishInfoNotify(*pstUserData, uBulletUniqID, uFishUniqID, 0, false, iCostSilver, iFishIndex, 0);

		return 0;
	}

	//获取配置
	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();

	const FishConfig* pstFishConfig = stBaseCfgMgr.GetFishConfig(pFishInfo->iFishSeqID, pFishInfo->iFishID);
	const GunConfig* pstGunConfig = stBaseCfgMgr.GetGunConfig(pBulletInfo->iGunID);
	if (!pstGunConfig || !pstFishConfig)
	{
		//找不到配置
		DeleteBulletData(uBulletUniqID);

		//推送命中鱼的消息
		SendHitFishInfoNotify(*pstUserData, uBulletUniqID, uFishUniqID, 0, false, iCostSilver, iFishIndex, 0);
		return 0;
	}

	//获取体验线
	const ExpLineConfig* pstConfig = NULL;
	TEXPLINEINFO* pstExpLineInfo = GetRoleExpLineInfo(stRoleObj.GetUin(), pstConfig);
	if (!pstExpLineInfo)
	{
		LOGERROR("Failed to get exp line info, uin %u, algorithm type %d\n", stRoleObj.GetUin(), m_pstRoomConfig->iAlgorithmType);
		return T_ZONE_PARA_ERROR;
	}

	//获取房间算法数据
	RoomAlgorithmData& stData = FishAlgorithm::astData[m_pstRoomConfig->iAlgorithmType];
	if (pFishInfo->iType == FISH_TYPE_PHONETICKET)
	{
		//点券鱼

		//点券鱼的子弹不计算服务器状态
		pstUserData->lUserSilverCost -= iCostSilver;	//增加玩家积分消耗
		pstUserData->lUserTicketSilverCost += iCostSilver;//增加点券鱼积分消耗
		stData.lTotalRewardSilver -= iCostSilver;
		stData.lRewardSilver -= iCostSilver;
		stData.lTodayRewardSilver -= iCostSilver;

		//点券鱼子弹不计算体验线
		pstExpLineInfo->lUserWinNum += iCostSilver;
		pstExpLineInfo->lIntervalWinNum += iCostSilver;

		//打印运营日志 记录点券鱼流水
		CZoneOssLog::TraceShootCostByFishType(stRoleObj.GetUin(), stRoleObj.GetChannel(), stRoleObj.GetNickName(), m_pstRoomConfig->iRoomID,
			pBulletInfo->iCost, FISH_TYPE_PHONETICKET);

		//计算是否命中
		if (rand() % 1000 < (pstGunConfig->iMultiple * 1000 / pstFishConfig->Multiple_i_max))
		{
			//获取初始鱼票数
			long8 lOldTicketNum = stRoleObj.GetResource(RESOURCE_TYPE_TICKET);

			//打中了点券鱼
			iRewardSilver = pstFishConfig->Multiple_i_min;
			if (!AddUserTicket(stRoleObj, uFishUniqID, false, pstGunConfig->iMultiple, iRewardSilver, true))
			{
				LOGERROR("Failed to add ticket fish user ticket, numid %u, ticket num %d\n", stRoleObj.GetUin(), iRewardSilver);
			}

			//增加点券鱼掉落鱼票数量
			stData.lTicketFishDropNum += iRewardSilver;

			//删除鱼
			DeleteFishData(uFishUniqID);

			//命中点券鱼
			CGameEventManager::NotifyKillFish(stRoleObj, pstFishConfig->Sequence_i, pstFishConfig->Type_i, 1, RESOURCE_TYPE_TICKET, iRewardSilver);

			//打印运营日志
			CZoneOssLog::TraceCatchFish(stRoleObj.GetUin(), stRoleObj.GetChannel(), stRoleObj.GetNickName(), pBulletInfo->iCost, FISH_TYPE_PHONETICKET,
				RESOURCE_TYPE_TICKET, m_pstRoomConfig->iRoomID, lOldTicketNum, stRoleObj.GetResource(RESOURCE_TYPE_TICKET));
		}

		//删除子弹
		DeleteBulletData(uBulletUniqID);

		//增加点券鱼流水
		stData.lTicketFishRewardSilver += iCostSilver;

		//推送命中鱼的消息
		SendHitFishInfoNotify(*pstUserData, uBulletUniqID, uFishUniqID, iRewardSilver, false, iCostSilver, iFishIndex, 0);

		//发送点券走马灯
		const HorseLampConfig* pstHorseLampConfig = stBaseCfgMgr.GetHorseLampConfig(HORSELAMP_TYPE_TICKETS);
		if (pstHorseLampConfig && (iRewardSilver >= pstHorseLampConfig->aiNeeds[0]))
		{
			//发送走马灯
			SendFishHorseLamp(stRoleObj, HORSELAMP_TYPE_TICKETS, pstGunConfig->iMultiple, iRewardSilver, 0);
		}

		return 0;
	}
	else if (pFishInfo->iType == FISH_TYPE_WARHEAD)
	{
		//弹头鱼

		//弹头鱼的子弹不计算服务器状态
		pstUserData->lUserSilverCost -= iCostSilver;	//增加玩家积分消耗
		pstUserData->lUserTicketSilverCost += iCostSilver;	//增加点券鱼积分消耗
		stData.lTotalRewardSilver -= iCostSilver;
		stData.lRewardSilver -= iCostSilver;
		stData.lTodayRewardSilver -= iCostSilver;

		//弹头鱼子弹不计算体验线
		pstExpLineInfo->lUserWinNum += iCostSilver;
		pstExpLineInfo->lIntervalWinNum += iCostSilver;

		//打印运营日志 记录弹头鱼流水
		CZoneOssLog::TraceShootCostByFishType(stRoleObj.GetUin(), stRoleObj.GetChannel(), stRoleObj.GetNickName(), m_pstRoomConfig->iRoomID,
			pBulletInfo->iCost, FISH_TYPE_WARHEAD);

		//计算是否命中,公式 P<1/倍数
		if ((rand() % 10000)*pFishInfo->iMultiple < 10000)
		{
			//打中了弹头鱼,增加弹头
			for (unsigned i = 0; i < sizeof(m_pstRoomConfig->astDrops) / sizeof(DropItemConfig); ++i)
			{
				const DropItemConfig& stDropConfig = m_pstRoomConfig->astDrops[i];
				if (stDropConfig.iItemID == 0)
				{
					break;
				}

				int iRet = CRepThingsUtility::AddItemNum(stRoleObj, stDropConfig.iItemID, stDropConfig.iItemNum, ITEM_CHANNEL_FISHADD);
				if (iRet)
				{
					LOGERROR("Failed to add fish drop item, ret %d, uin %u, item id %d, num %d\n", iRet, stRoleObj.GetUin(), stDropConfig.iItemID, stDropConfig.iItemNum);
					return iRet;
				}

				//增加玩家积分
				const FishItemConfig* pstItemConfig = stBaseCfgMgr.GetFishItemConfig(stDropConfig.iItemID);
				if (pstItemConfig)
				{
					stRoleObj.AddFishScore(pstItemConfig->aiParam[0] * stDropConfig.iItemNum);
				}

				//获取背包当前道具数量，打印运营日志使用
				CRepThingsManager& rstThingsManager = stRoleObj.GetRepThingsManager();
				long8 lNewItemNum = rstThingsManager.GetRepItemNum(stDropConfig.iItemID);

				//打印运营日志
				CZoneOssLog::TraceCatchWarHeadFish(stRoleObj.GetUin(), stRoleObj.GetChannel(), stRoleObj.GetNickName(), pBulletInfo->iCost, FISH_TYPE_WARHEAD,
					FISH_ITEM_WARHEAD, stDropConfig.iItemID, m_pstRoomConfig->iRoomID, lNewItemNum - stDropConfig.iItemNum, lNewItemNum);

				//发送弹头走马灯
				const HorseLampConfig* pstHorseLampConfig = stBaseCfgMgr.GetHorseLampConfig(HORSELAMP_TYPE_WARHEADS);
				if (pstHorseLampConfig &&
					(m_pstRoomConfig->iRoomID == pstHorseLampConfig->aiNeeds[0] || m_pstRoomConfig->iRoomID == pstHorseLampConfig->aiNeeds[1]))
				{
					//发送走马灯
					SendFishHorseLamp(stRoleObj, HORSELAMP_TYPE_WARHEADS, m_pstRoomConfig->iRoomID, stDropConfig.iItemNum, stDropConfig.iItemID);
				}
			}

			//恢复出鱼
			m_bStopOutFish = false;

			//删除鱼
			DeleteFishData(uFishUniqID);

			//命中弹头鱼
			CGameEventManager::NotifyKillFish(stRoleObj, pstFishConfig->Sequence_i, pstFishConfig->Type_i, 1, 0, 0);

			//设置为命中
			iRewardSilver = 1;
		}

		//删除子弹
		DeleteBulletData(uBulletUniqID);

		//推送命中鱼的消息
		SendHitFishInfoNotify(*pstUserData, uBulletUniqID, uFishUniqID, iRewardSilver, false, iCostSilver, iFishIndex, 0);

		return 0;
	}
	else if (pFishInfo->iType == FISH_TYPE_SMALLGROUP)
	{
		//小鱼组
		const SmallFishConfig* pstSmallConfig = stBaseCfgMgr.GetSmallFishConfig(pFishInfo->iFishID);
		if (!pstSmallConfig)
		{
			LOGERROR("Failed to get small fish config, fish id %d\n", pFishInfo->iFishID);
			return T_ZONE_INVALID_CFG;
		}

		//该鱼是否有效
		if (iFishIndex < 0 || iFishIndex >= (int)pstSmallConfig->vTrackIDs.size())
		{
			//删除子弹
			DeleteBulletData(uBulletUniqID);
			
			//推送命中鱼的消息
			SendHitFishInfoNotify(*pstUserData, uBulletUniqID, uFishUniqID, 0, false, iCostSilver, iFishIndex, 0);

			return 0;
		}

		//该鱼是否死亡
		if (!(pFishInfo->cIndex&(0x01 << iFishIndex)))
		{
			//删除子弹
			DeleteBulletData(uBulletUniqID);
			
			//推送命中鱼的消息
			SendHitFishInfoNotify(*pstUserData, uBulletUniqID, uFishUniqID, 0, false, iCostSilver, iFishIndex, 0);

			return 0;
		}
	}
	else if (pFishInfo->iType == FISH_TYPE_MULTIPLE)
	{
		//增加多倍鱼命中流水
		stData.lMultiFishFlow += iCostSilver;
	}

	//非瞄准才检查位置
	if (pBulletInfo->uFishUniqID == 0)
	{
		//验证子弹和鱼的位置
		if (!CheckIsValidHit(lHitTime, *pBulletInfo, *pFishInfo, iFishIndex))
		{
			//该位置未能命中
			DeleteBulletData(uBulletUniqID);

			//推送命中鱼的消息
			SendHitFishInfoNotify(*pstUserData, uBulletUniqID, uFishUniqID, 0, false, iCostSilver, iFishIndex, 0);

			return 0;
		}
	}

	//计算命中倍数

	//打印运营日志 普通鱼发炮流水
	CZoneOssLog::TraceShootCostByFishType(stRoleObj.GetUin(), stRoleObj.GetChannel(), stRoleObj.GetNickName(), m_pstRoomConfig->iRoomID,
		pBulletInfo->iCost, pFishInfo->iType);

	//计算服务器命中概率
	if (!CheckIsLogicHit(stRoleObj, *pstUserData, *pstGunConfig, *pFishInfo, pstFishConfig->Adjust_i, false))
	{
		//服务器逻辑未命中
		DeleteBulletData(uBulletUniqID);

		//推送命中鱼的消息
		SendHitFishInfoNotify(*pstUserData, uBulletUniqID, uFishUniqID, 0, false, iCostSilver, iFishIndex, 0);

		return 0;
	}

	//命中该鱼

	//删除子弹
	DeleteBulletData(uBulletUniqID);

	int iFishCostSilver = pFishInfo->aiCostSilver[pstUserData->iSeat];
	pFishInfo->aiCostSilver[pstUserData->iSeat] = 0;

	if (pFishInfo->iType == FISH_TYPE_BOOMFISH || pFishInfo->iType == FISH_TYPE_FLASHFISH)
	{
		//处理电鳗和炸弹鱼逻辑
		BoomFish(*pstUserData, pFishInfo, false);
	}

	//计算奖励的计分
	int iFishMultiple = pFishInfo->iMultiple;
	iRewardSilver = iCostSilver * pFishInfo->iMultiple;

	//多倍鱼奖励倍乘
	int iMultipleFish = 0;
	if (pFishInfo->iType == FISH_TYPE_MULTIPLE)
	{
		const MultipleFishConfig* pstMultipleFishConfig = stBaseCfgMgr.GetMultipleFishConfig();
		if (!pstMultipleFishConfig)
		{
			LOGERROR("Failed to get multiple fish config, uin %u\n", stRoleObj.GetUin());
			return T_ZONE_INVALID_CFG;
		}
		iRewardSilver = iRewardSilver * pstMultipleFishConfig->iRate;
		iMultipleFish = pstMultipleFishConfig->iRate;

		//增加周期多倍鱼命中流水
		stData.lMultiFishHitFlow += iRewardSilver;
	}

	if (pFishInfo->iType == FISH_TYPE_SMALLGROUP)
	{
		//小鱼组
		pFishInfo->cIndex &= ~(0x01 << iFishIndex);
		if (pFishInfo->cIndex == 0)
		{
			//小鱼组被打完,删除鱼
			DeleteFishData(uFishUniqID);
		}
	}
	else
	{
		//删除鱼
		DeleteFishData(uFishUniqID);
	}

	//计算体验线
	pstExpLineInfo->lUserWinNum += iRewardSilver;
	pstExpLineInfo->lIntervalWinNum += iRewardSilver;

	//增加玩家积分
	stRoleObj.AddFishScore(iRewardSilver);

	//获取玩家当前金币数量，后面运营日志使用
	long8 lOldCoinNum = stRoleObj.GetResource(RESOURCE_TYPE_COIN);

	//增加积分
	if (!CResourceUtility::AddUserRes(stRoleObj, RESOURCE_TYPE_COIN, iRewardSilver))
	{
		LOGERROR("Failed to add user silver, uin %u, add silver %d\n", stRoleObj.GetUin(), iRewardSilver);
		return T_ZONE_INVALID_NOSILVER;
	}

	//是否使用红包
	if (iRewardSilver > iFishCostSilver)
	{
		int iUseRedSilver = iRewardSilver - iFishCostSilver;
		if (pstUserData->uReturnID != 0 && pstUserData->lUnEffectTime >= lTimeNow && (stData.lTotalReturnSilver + stRoleObj.GetNowNewRedNum()) >= iRewardSilver)
		{
			if (stRoleObj.GetNowNewRedNum() >= iUseRedSilver)
			{
				//只使用新手红包
				stRoleObj.SetNowNewRedNum(stRoleObj.GetNowNewRedNum() - iUseRedSilver);
				stData.lTotalNewRedUseNum += iUseRedSilver;
				pstUserData->lCycleNewUsedNum += iUseRedSilver;
			}
			else
			{
				//先扣除新手红包
				int iNewRedUsedNum = stRoleObj.GetNowNewRedNum();
				stRoleObj.SetNowNewRedNum(0);
				stData.lTotalNewRedUseNum += iNewRedUsedNum;
				pstUserData->lCycleNewUsedNum += iNewRedUsedNum;

				iUseRedSilver -= iNewRedUsedNum;

				//再扣除普通红包
				stData.lTotalReturnSilver -= iUseRedSilver;
				stData.lUsedReturnSilver += iUseRedSilver;
			}
		}
		else if (stRoleObj.GetNowNewRedNum() >= iRewardSilver)
		{
			//新手红包
			stData.lUsedReturnSilver += iUseRedSilver;
			stRoleObj.SetNowNewRedNum(stRoleObj.GetNowNewRedNum() - iUseRedSilver);
			stData.lTotalNewRedUseNum += iUseRedSilver;
		}
	}

	pstUserData->iLossNum -= iRewardSilver;
	pstUserData->iCycleWinNum += iRewardSilver;
	pstUserData->lUserSilverReward += iRewardSilver;

	stData.lTotalCostSliver += iRewardSilver;
	stData.lCostSilver += iRewardSilver;
	stData.lTodayCostSliver += iRewardSilver;

	//普通鱼掉落点券
	if (!AddUserTicket(stRoleObj, uFishUniqID, false, pstGunConfig->iMultiple, iRewardSilver))
	{
		LOGERROR("Failed to add normal user ticket, numid %u, reward silver %d\n", stRoleObj.GetUin(), iRewardSilver);
		return T_ZONE_INVALID_CFG;
	}

	//推送命中鱼的消息
	SendHitFishInfoNotify(*pstUserData, uBulletUniqID, uFishUniqID, iRewardSilver, false, iCostSilver, iFishIndex, iMultipleFish);

	//发送金币走马灯
	const HorseLampConfig* pstHorseLampConfig = stBaseCfgMgr.GetHorseLampConfig(HORSELAMP_TYPE_COINS);
	if (pstHorseLampConfig &&
		iRewardSilver >= pstHorseLampConfig->aiNeeds[0] &&
		iFishMultiple >= pstHorseLampConfig->aiNeeds[1])
	{
		//发送走马灯
		SendFishHorseLamp(stRoleObj, HORSELAMP_TYPE_COINS, pstGunConfig->iMultiple, iFishID, iRewardSilver);
	}

	//打印运营日志
	CZoneOssLog::TraceCatchFish(stRoleObj.GetUin(), stRoleObj.GetChannel(), stRoleObj.GetNickName(), pBulletInfo->iCost, pstFishConfig->Type_i,
		RESOURCE_TYPE_COIN, m_pstRoomConfig->iRoomID, lOldCoinNum, stRoleObj.GetResource(RESOURCE_TYPE_COIN));

	//命中普通鱼
	CGameEventManager::NotifyKillFish(stRoleObj, pstFishConfig->Sequence_i, pstFishConfig->Type_i, 1, RESOURCE_TYPE_COIN, iRewardSilver);

	LOGDEBUG("HitFish, uin %u, reward %d, fish id %d, fish multiple %d, gun multiple %d\n", stRoleObj.GetUin(), iRewardSilver,
		iFishID, iFishMultiple, pstGunConfig->iMultiple);

	return 0;
}

//命中鱼阵中的鱼
int CFishpondObj::HitFormFish(CGameRoleObj& stRoleObj, long lHitTime, unsigned uBulletUniqID, int iFishOutID, int iFishIndex)
{
	int iFishID = 0;
	int iCostSilver = 0;
	int iRewardSilver = 0;

	//获取玩家信息
	SeatUserData* pstUserData = GetSeatUserByUin(stRoleObj.GetUin());
	if (!pstUserData)
	{
		return T_ZONE_PARA_ERROR;
	}

	//客户端FishIndex从1开始
	if (iFishIndex < 1 || iFishIndex >(int)MAX_FORM_FISHKILL_NUM)
	{
		return T_ZONE_INVALID_BULLET;
	}

	iFishIndex = iFishIndex - 1;

	//获取子弹信息
	BulletData* pBulletInfo = GetBulletData(uBulletUniqID);
	if (!pBulletInfo || pBulletInfo->iSeat != pstUserData->iSeat)
	{
		//找不到子弹或子弹不是玩家发出的
		return T_ZONE_INVALID_BULLET;
	}

	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();

	const GunConfig* pstGunConfig = stBaseCfgMgr.GetGunConfig(stRoleObj.GetWeaponInfo().iWeaponID);
	if (!pstGunConfig)
	{
		return T_ZONE_INVALID_CFG;
	}

	iCostSilver = pstGunConfig->iMultiple;

	//减玩家的子弹数
	if (pstUserData->iBulletNum > 0)
	{
		--pstUserData->iBulletNum;
	}

	long lTimeNow = CTimeUtility::GetMSTime();
	if (abs(lHitTime - lTimeNow) >= 1000)
	{
		//命中时间非法		

		LOGERROR("HitFormFish, uin %u, hittime:servertime %ld:%ld\n", stRoleObj.GetUin(), lHitTime, lTimeNow);
		DeleteBulletData(uBulletUniqID);	//删除子弹

											//推送命中消息
		SendHitFishInfoNotify(*pstUserData, uBulletUniqID, iFishOutID * 1000 + iFishIndex, 0, true, iCostSilver, 0, 0);

		return T_SERVER_SUCCESS;
	}

	//获取鱼的信息
	FishData stFishInfo;
	FormFishOutData* pstFormOutData = GetFormFishInfo(iFishOutID, iFishIndex, stFishInfo);
	if (!pstFormOutData)
	{
		//获取鱼阵的鱼失败

		DeleteBulletData(uBulletUniqID);	//删除子弹

											//推送命中消息
		SendHitFishInfoNotify(*pstUserData, uBulletUniqID, iFishOutID * 1000 + iFishIndex, 0, true, iCostSilver, 0, 0);

		return 0;
	}

	pstFormOutData->aiCostSilver[pstUserData->iSeat][iFishIndex] += pstGunConfig->iMultiple;
	iFishID = stFishInfo.iFishID;

	//不是瞄准才检查位置
	if (pBulletInfo->uFishUniqID == 0)
	{
		//验证子弹和鱼的位置
		if (!CheckIsValidHit(lHitTime, *pBulletInfo, stFishInfo))
		{
			//该位置未能命中
			DeleteBulletData(uBulletUniqID);

			//推送命中消息
			SendHitFishInfoNotify(*pstUserData, uBulletUniqID, iFishOutID * 1000 + iFishIndex, 0, true, iCostSilver, 0, 0);

			return 0;
		}
	}

	//计算命中倍数
	const FishConfig* pstFishConfig = stBaseCfgMgr.GetFishConfig(stFishInfo.iFishSeqID, stFishInfo.iFishID);
	if (!pstGunConfig || !pstFishConfig)
	{
		//找不到配置
		DeleteBulletData(uBulletUniqID);

		//推送命中消息
		SendHitFishInfoNotify(*pstUserData, uBulletUniqID, iFishOutID * 1000 + iFishIndex, 0, true, iCostSilver, 0, 0);

		return 0;
	}

	//计算服务器命中概率
	if (!CheckIsLogicHit(stRoleObj, *pstUserData, *pstGunConfig, stFishInfo, pstFishConfig->Adjust_i, true))
	{
		//服务器逻辑未命中
		DeleteBulletData(uBulletUniqID);

		//推送命中消息
		SendHitFishInfoNotify(*pstUserData, uBulletUniqID, iFishOutID * 1000 + iFishIndex, 0, true, iCostSilver, 0, 0);

		return 0;
	}

	//命中该鱼

	//删除子弹
	DeleteBulletData(uBulletUniqID);

	//设置鱼死亡信息
	pstFormOutData->SetFishAlive(iFishIndex, false);

	if (stFishInfo.iType == FISH_TYPE_BOOMFISH || stFishInfo.iType == FISH_TYPE_FLASHFISH)
	{
		//处理电鳗和炸弹鱼逻辑
		FishData* pstFishInfo = &stFishInfo;
		BoomFish(*pstUserData, pstFishInfo, true);
	}

	//计算奖励的计分
	iRewardSilver = pstGunConfig->iMultiple * stFishInfo.iMultiple;

	//增加积分
	if (!CResourceUtility::AddUserRes(stRoleObj, RESOURCE_TYPE_COIN, iRewardSilver))
	{
		LOGERROR("Failed to add user silver, uin %u, add silver %d\n", stRoleObj.GetUin(), iRewardSilver);
		return T_ZONE_INVALID_NOSILVER;
	}

	int iFishCostSilver = pstFormOutData->aiCostSilver[pstUserData->iSeat][iFishIndex];

	//获取房间算法数据
	RoomAlgorithmData& stData = FishAlgorithm::astData[m_pstRoomConfig->iAlgorithmType];

	//使用红包
	if (pstUserData->uReturnID != 0 && pstUserData->lUnEffectTime >= lTimeNow)
	{
		if (stData.lTotalReturnSilver >= iRewardSilver && iRewardSilver > iFishCostSilver)
		{
			//当前使用了红包，不计算服务器亏损，个人亏损，个人收益
			stData.lTotalReturnSilver -= (iRewardSilver - iFishCostSilver);
			stData.lUsedReturnSilver += (iRewardSilver - iFishCostSilver);

			//numid|nickname|seat|iFishID|uFishUniqID|uBulletUniqID|iRewardSilver|iFishCostSilver|lTotalReturnSilver
			LOGDEBUG("UseRedPacket, %u|%s|%d|%d|%u|%u|%d|%d|%lld\n", stRoleObj.GetUin(), stRoleObj.GetNickName(), pstUserData->iSeat, iFishID,
				iFishOutID * 1000 + iFishIndex, uBulletUniqID, iRewardSilver, iFishCostSilver, stData.lTotalReturnSilver);
		}
	}

	pstUserData->iLossNum -= iRewardSilver;
	pstUserData->iCycleWinNum += iRewardSilver;
	pstUserData->lUserSilverReward += iRewardSilver;

	stData.lTotalCostSliver += iRewardSilver;
	stData.lCostSilver += iRewardSilver;
	stData.lTodayCostSliver += iRewardSilver;

	//普通鱼掉落点券
	if (!AddUserTicket(stRoleObj, stFishInfo.uUniqueID, true, pstGunConfig->iMultiple, iRewardSilver))
	{
		LOGERROR("Failed to add normal user ticket, numid %u, reward silver %d\n", stRoleObj.GetUin(), iRewardSilver);
		return T_ZONE_INVALID_CFG;
	}

	//推送命中消息
	SendHitFishInfoNotify(*pstUserData, uBulletUniqID, iFishOutID * 1000 + iFishIndex, iRewardSilver, true, iCostSilver, 0, 0);

	LOGDEBUG("HitFormFish, uin %u, reward %d\n", stRoleObj.GetUin(), iRewardSilver);

	return 0;
}

//玩家使用技能
int CFishpondObj::UseSkill(CGameRoleObj& stRoleObj, const Zone_UseSkill_Request& stReq)
{
	SeatUserData* pstUserData = GetSeatUserByUin(stRoleObj.GetUin());
	if (!pstUserData)
	{
		LOGERROR("Failed to get seat user, uin %u, table id %u\n", stRoleObj.GetUin(), m_uTableID);
		return T_ZONE_PARA_ERROR;
	}

	//刷新玩家活跃时间
	pstUserData->iActiveTime = CTimeUtility::GetNowTime();

	CZoneMsgHelper::GenerateMsgHead(ms_stZoneMsg, MSGID_ZONE_USESKILL_NOTIFY);
	Zone_UseSkill_Notify* pstNotify = ms_stZoneMsg.mutable_stbody()->mutable_m_stzone_useskill_notify();
	pstNotify->set_itype(stReq.itype());
	pstNotify->set_uin(stRoleObj.GetUin());

	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();

	switch (stReq.itype())
	{
	case SKILL_TYPE_AUTOSHOOT:
	{
		//使用自动发炮
		int iTimeNow = CTimeUtility::GetNowTime();
		if (!pstUserData->bAutoShoot && stRoleObj.GetMonthEndTime() < iTimeNow)
		{
			//月卡状态不满足，不能激活自动发炮
			LOGERROR("Failed to use autoshoot, uin %u, monthend:now %d:%d\n", stRoleObj.GetUin(), stRoleObj.GetMonthEndTime(), iTimeNow);
			return T_ZONE_PARA_ERROR;
		}

		//打印运营日志， 自动发炮不消耗道具
		CZoneOssLog::TraceUseSkill(stRoleObj.GetUin(), stRoleObj.GetChannel(), stRoleObj.GetNickName(), SKILL_TYPE_AUTOSHOOT, m_pstRoomConfig->iRoomID, 0, 0);

		//切换状态
		pstUserData->bAutoShoot = ~pstUserData->bAutoShoot;
		pstNotify->set_bisopen(pstUserData->bAutoShoot);
	}
	break;

	case SKILL_TYPE_AIMFISH:
	{
		//瞄准鱼
		long lTimeNow = CTimeUtility::GetMSTime();

		//是否在瞄准中
		if (pstUserData->lAimEndTime > lTimeNow)
		{
			//不能使用瞄准
			LOGERROR("Failed to use aim item, in aim fish, uin %u\n", stRoleObj.GetUin());
			return T_ZONE_PARA_ERROR;
		}

		//读取配置
		const FishItemConfig* pstItemConfig = stBaseCfgMgr.GetFishItemConfig(AIM_FISH_ITEM_ID);
		if (!pstItemConfig)
		{
			LOGERROR("Failed to get fish item config, invalid id %d\n", AIM_FISH_ITEM_ID);
			return T_ZONE_INVALID_CFG;
		}

		//获取玩家当前瞄准道具数量 下面运营日志使用
		long8 lOldItemNum = stRoleObj.GetRepThingsManager().GetRepItemNum(AIM_FISH_ITEM_ID);

		//先扣除消耗
		int iRet = CRepThingsUtility::AddItemNum(stRoleObj, AIM_FISH_ITEM_ID, -1, ITEM_CHANNEL_USEITEM);
		if (iRet)
		{
			//不能扣道具，尝试扣钱
			if (!CResourceUtility::AddUserRes(stRoleObj, RESOURCE_TYPE_COIN, -pstItemConfig->aiParam[0]))
			{
				//不能扣除消耗
				LOGERROR("Failed to use aim item, no enough cost, uin %u\n", stRoleObj.GetUin());
				return T_ZONE_INVALID_NOSILVER;
			}
		}

		//生效瞄准
		pstUserData->lAimEndTime = lTimeNow + pstItemConfig->aiParam[1] * 1000;

		//打印运营日志 使用瞄准技能
		CZoneOssLog::TraceUseSkill(stRoleObj.GetUin(), stRoleObj.GetChannel(), stRoleObj.GetNickName(), SKILL_TYPE_AIMFISH, m_pstRoomConfig->iRoomID, lOldItemNum, stRoleObj.GetRepThingsManager().GetRepItemNum(AIM_FISH_ITEM_ID));

		pstNotify->set_laimendtime(pstUserData->lAimEndTime);
	}
	break;

	case SKILL_TYPE_WARHEAD:
	{
		//使用弹头

		//读取配置
		const FishItemConfig* pstItemConfig = stBaseCfgMgr.GetFishItemConfig(stReq.iitemid());
		if (!pstItemConfig || pstItemConfig->iType != FISH_ITEM_WARHEAD)
		{
			LOGERROR("Failed to get warhead item config, invalid id %d\n", stReq.iitemid());
			return T_ZONE_PARA_ERROR;
		}

		//先获取使用前弹头数量，运营日志使用
		long8 lOldWarHeadNum = stRoleObj.GetRepThingsManager().GetRepItemNum(stReq.iitemid());
		long8 lOldCoinNum = stRoleObj.GetResource(RESOURCE_TYPE_COIN);

		//先扣除道具
		int iRet = CRepThingsUtility::AddItemNum(stRoleObj, stReq.iitemid(), -1, ITEM_CHANNEL_USEITEM);
		if (iRet)
		{
			LOGERROR("Failed to use warhead, ret %d, uin %u, item id %d\n", iRet, stRoleObj.GetUin(), stReq.iitemid());
			return iRet;
		}

		//删除炸死的鱼
		for (int i = 0; i < stReq.ufishuniqids_size(); ++i)
		{
			DeleteFishData(stReq.ufishuniqids(i));
		}

		//加炸鱼奖励
		if (!CResourceUtility::AddUserRes(stRoleObj, RESOURCE_TYPE_COIN, pstItemConfig->aiParam[0]))
		{
			LOGERROR("Failed to add warhead reward, uin %u, item id %d\n", stRoleObj.GetUin(), stReq.iitemid());
			return T_ZONE_PARA_ERROR;
		}

		//打印运营日志 弹头使用日志
		CZoneOssLog::TraceUseWarHead(stRoleObj.GetUin(), stRoleObj.GetChannel(), stRoleObj.GetNickName(), stReq.iitemid(), pstItemConfig->aiParam[0],
			m_pstRoomConfig->iRoomID, lOldWarHeadNum, stRoleObj.GetRepThingsManager().GetRepItemNum(stReq.iitemid()),
			lOldCoinNum, stRoleObj.GetResource(RESOURCE_TYPE_COIN));

		//设置推送消息参数
		pstNotify->mutable_stwarheadpos()->CopyFrom(stReq.stwarheadpos());
		pstNotify->mutable_ufishuniqids()->CopyFrom(stReq.ufishuniqids());
		pstNotify->set_lrewardcoins(pstItemConfig->aiParam[0]);
	}
	break;

	case SKILL_TYPE_WILD:
	{
		//狂暴

		//先判断是否能使用
		const VipLevelConfig* pstVipConfig = stBaseCfgMgr.GetVipConfig(stRoleObj.GetVIPLevel());
		if (!pstVipConfig)
		{
			LOGERROR("Failed to get vip config, invalid vip level %d, uin %u\n", stRoleObj.GetVIPLevel(), stRoleObj.GetUin());
			return T_ZONE_INVALID_CFG;
		}

		const VipPriv* pstPriv = NULL;
		for (unsigned i = 0; i < pstVipConfig->vPrivs.size(); ++i)
		{
			if (pstVipConfig->vPrivs[i].iPrivType == VIP_PRIV_WILDNUM)
			{
				pstPriv = &pstVipConfig->vPrivs[i];
				break;
			}
		}

		//是否配置中可用
		if (!pstPriv || pstPriv->aiParams[0] <= 0)
		{
			LOGERROR("Failed to use wild skill, no valid config, uin %u, vip level %d\n", stRoleObj.GetUin(), stRoleObj.GetVIPLevel());
			return T_ZONE_PARA_ERROR;
		}

		//是否达到狂暴次数上限
		if (pstUserData->iWildNum >= pstPriv->aiParams[2])
		{
			LOGERROR("Failed to use wild skill, already reach max num, uin %u\n", stRoleObj.GetUin());
			return T_ZONE_PARA_ERROR;
		}

		pstUserData->lWildEndTime = CTimeUtility::GetMSTime() + pstPriv->aiParams[0] * 1000;
		pstUserData->iWildNum += 1;

		//取消玩家红包状态
		pstUserData->lUnEffectTime = 0;

		//打印运营日志 道具使用 狂暴 不消耗道具
		CZoneOssLog::TraceUseSkill(stRoleObj.GetUin(), stRoleObj.GetChannel(), stRoleObj.GetNickName(), SKILL_TYPE_WILD, m_pstRoomConfig->iRoomID, 0, 0);

		//设置推送消息的参数
		pstNotify->set_lwildendtime(pstUserData->lWildEndTime);
		pstNotify->set_iwildnum(pstUserData->iWildNum);
	}
	break;

	case SKILL_TYPE_PREWARHEAD:
	{
		//预使用弹头

		//读取配置
		const FishItemConfig* pstItemConfig = stBaseCfgMgr.GetFishItemConfig(stReq.iitemid());
		if (!pstItemConfig || pstItemConfig->iType != FISH_ITEM_WARHEAD)
		{
			LOGERROR("Failed to get warhead item config, invalid id %d\n", stReq.iitemid());
			return T_ZONE_PARA_ERROR;
		}

		//设置推送消息参数
		pstNotify->mutable_stwarheadpos()->CopyFrom(stReq.stwarheadpos());
	}
	break;

	default:
	{
		LOGERROR("Failed to use skill, uin %u, skill type %d\n", stRoleObj.GetUin(), stReq.itype());
		return T_ZONE_INVALID_SKILLTYPE;
	}
	break;
	}

	//推送技能使用通知
	SendZoneMsgToFishpond(ms_stZoneMsg);

	//技能使用的通知
	if (stReq.itype() != SKILL_TYPE_AUTOSHOOT && stReq.itype() != SKILL_TYPE_PREWARHEAD)
	{
		//自动发炮和预使用弹头不计算使用次数
		CGameEventManager::NotifyUseSkill(stRoleObj, stReq.itype(), 1);
	}

	return T_SERVER_SUCCESS;
}

//玩家选择瞄准鱼
int CFishpondObj::ChooseAimFish(CGameRoleObj& stRoleObj, unsigned uFishUniqID, int iFishIndex)
{
	SeatUserData* pstUserData = GetSeatUserByUin(stRoleObj.GetUin());
	if (!pstUserData)
	{
		LOGERROR("Failed to get fish user, table id %d, uin %u\n", m_uTableID, stRoleObj.GetUin());
		return T_ZONE_GAMEROLE_NOT_EXIST;
	}

	long lTimeNow = CTimeUtility::GetMSTime();

	//更新玩家活跃时间
	pstUserData->iActiveTime = lTimeNow / 1000;

	//是否在瞄准状态
	if (pstUserData->lAimEndTime + 100 < lTimeNow)
	{
		//不在瞄准状态
		pstUserData->uAimFishUniqID = 0;

		return T_ZONE_INVALID_AIMSTAT;
	}

	//鱼是否存在
	FishData* pstFishData = GetFishData(uFishUniqID);
	if (!pstFishData || pstFishData->iType == FISH_TYPE_PHONETICKET)
	{
		//鱼不存在或者点券鱼
		pstUserData->uAimFishUniqID = 0;

		return T_ZONE_INVALID_FISH;
	}

	//判断iFishIndex
	if (iFishIndex < 0)
	{
		//非法的index
		pstUserData->uAimFishUniqID = 0;
		return T_ZONE_PARA_ERROR;
	}
	else if (iFishIndex == 0)
	{
		if (pstFishData->iType == FISH_TYPE_SMALLGROUP)
		{
			//非法的index
			pstUserData->uAimFishUniqID = 0;
			return T_ZONE_PARA_ERROR;
		}
	}
	else
	{
		if (pstFishData->iType != FISH_TYPE_SMALLGROUP || !(pstFishData->cIndex&(0x01 << (iFishIndex - 1))))
		{
			//非法的index
			pstUserData->uAimFishUniqID = 0;
			return T_ZONE_PARA_ERROR;
		}
	}

	//设置瞄准鱼信息
	pstUserData->uAimFishUniqID = uFishUniqID;
	pstUserData->iAimFishIndex = iFishIndex;

	//更新玩家子弹的瞄准信息
	for (unsigned i = 0; i < m_vBulletData.size(); ++i)
	{
		if (m_vBulletData[i].iSeat != pstUserData->iSeat || m_vBulletData[i].uFishUniqID == 0)
		{
			continue;
		}

		m_vBulletData[i].uFishUniqID = uFishUniqID;
		m_vBulletData[i].iFishIndex = iFishIndex;
	}

	//封装返回
	CZoneMsgHelper::GenerateMsgHead(ms_stZoneMsg, MSGID_ZONE_CHOOSEAIMFISH_NOTIFY);
	Zone_ChooseAimFish_Notify* pstNotify = ms_stZoneMsg.mutable_stbody()->mutable_m_stzone_chooseaimfish_notify();
	pstNotify->set_uin(stRoleObj.GetUin());
	pstNotify->set_ufishuniqid(uFishUniqID);
	pstNotify->set_ifishindex(iFishIndex);

	SendZoneMsgToFishpond(ms_stZoneMsg);

	return T_SERVER_SUCCESS;
}

//获取桌子ID
unsigned CFishpondObj::GetTableID()
{
	return m_uTableID;
}

//获取桌子房间ID
int CFishpondObj::GetFishRoomID()
{
	return m_iFishRoomID;
}

//获取玩家人数
int CFishpondObj::GetPlayerNum()
{
	return m_vSeatUserData.size();
}

//获取鱼池配置
const FishRoomConfig* CFishpondObj::GetRoomConfig()
{
	return m_pstRoomConfig;
}

//定时器
int CFishpondObj::OnTick(CGameRoleObj& stRoleObj)
{
	long lTimeNow = CTimeUtility::GetMSTime();

	//更新玩家个人信息
	UpdateSeatUserInfo(stRoleObj, lTimeNow);

	if (lTimeNow - m_lLastTickTime < 1000)
	{
		//1s钟执行一次鱼池的Tick
		return 0;
	}

	m_lLastTickTime = lTimeNow;

	//更新出鱼信息
	int iRet = UpdateOutFishRule(lTimeNow);
	if (iRet)
	{
		return iRet;
	}

	//更新鱼的信息
	iRet = UpdateFishInfo(lTimeNow);
	if (iRet)
	{
		return iRet;
	}

	//更新鱼阵信息
	iRet = UpdateFishFormInfo(lTimeNow);
	if (iRet)
	{
		return iRet;
	}

	//更新捕鱼玩家信息
	iRet = UpdateFishUserInfo(lTimeNow);
	if (iRet)
	{
		return iRet;
	}

	return 0;
}

//推送捕鱼玩家信息更新
void CFishpondObj::SendFishUserUpdate(unsigned uiUin, int iResType, long8 lAddNum, int iItemID, int iItemNum)
{
	static GameProtocolMsg stMsg;

	SeatUserData* pstUserData = GetSeatUserByUin(uiUin);

	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_FISHUSERUPDATE_NOTIFY);
	Zone_FishUserUpdate_Notify* pstNotify = stMsg.mutable_stbody()->mutable_m_stzone_fishuserupdate_notify();
	pstNotify->set_iseat(pstUserData->iSeat);

	if (iResType == RESOURCE_TYPE_COIN)
	{
		pstNotify->set_iaddcoins(lAddNum);
	}
	else if (iResType == RESOURCE_TYPE_TICKET)
	{
		pstNotify->set_iaddtickets(lAddNum);
	}

	if (iItemID != 0)
	{
		pstNotify->mutable_stadditem()->set_iitemid(iItemID);
		pstNotify->mutable_stadditem()->set_iitemnum(iItemNum);
	}

	SendZoneMsgToFishpond(stMsg);

	return;
}

//重置Fishpond
void CFishpondObj::ResetFishpond()
{
	//鱼池ID
	m_uTableID = 0;

	//鱼池所在房间ID
	m_iFishRoomID = 0;

	//鱼池配置
	m_pstRoomConfig = NULL;

	//座位信息
	m_vSeatUserData.clear();

	//鱼池出鱼规则
	m_vOutFishRule.clear();

	//鱼池中的子弹
	m_vBulletData.clear();

	//鱼阵信息
	m_stFishFormData.Reset();

	//鱼池中鱼的信息
	m_vFishData.clear();

	//鱼池出鱼上限信息
	m_mFishLimitData.clear();

	//是否停止出鱼
	m_bStopOutFish = false;

	//发送给客户端的消息
	ms_stZoneMsg.Clear();

	//鱼池上次tick时间,单位ms
	m_lLastTickTime = 0;

	return;
}

//初始化鱼信息
int CFishpondObj::InitFishInfo()
{
	//初始化出鱼信息
	int iRet = InitOutFishRule();
	if (iRet)
	{
		LOGERROR("Failed to init out fish rule, ret %d\n", iRet);
		return iRet;
	}

	//初始化鱼阵信息
	iRet = InitFishFormRule(false);
	if (iRet)
	{
		LOGERROR("Failed to init fish form info, ret %d\n", iRet);
		return iRet;
	}

	return T_SERVER_SUCCESS;
}

bool CompareFishPower(const OutFishRule& stRule1, const OutFishRule& stRule2)
{
	return (stRule1.iPower > stRule2.iPower);
}

//初始化出鱼信息
int CFishpondObj::InitOutFishRule()
{
	//获取配置管理器
	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();

	OutFishRule stOneRule;
	const FishSeqType& stFishSeqConfigs = stBaseCfgMgr.GetAllFishConfig();

	//遍历鱼组
	FishLimitData stLimit;
	for (FishSeqType::const_iterator it = stFishSeqConfigs.begin(); it != stFishSeqConfigs.end(); ++it)
	{
		UpdateOneOutFishRule(it->first, stOneRule, -10);

		if (stOneRule.iFishSeqID != 0)
		{
			m_vOutFishRule.push_back(stOneRule);
		}

		//初始化出鱼上限
		if (it->second.size() != 0)
		{
			stLimit.iLimitMaxNum = it->second[0].iLimitNum;
			stLimit.iRemainNum = it->second[0].iLimitNum;
			m_mFishLimitData[it->second[0].iLimitType] = stLimit;
		}
	}

	//按出鱼权重排序
	std::sort(m_vOutFishRule.begin(), m_vOutFishRule.end(), CompareFishPower);

	return 0;
}

//初始化鱼阵信息
int CFishpondObj::InitFishFormRule(bool bIsFormEnd)
{
	//当前没有鱼阵
	return 0;

	if (!bIsFormEnd)
	{
		//非鱼阵结束才清空出鱼信息
		m_stFishFormData.vFishOutData.clear();

		//非鱼阵结束才清空冰封信息
		m_stFishFormData.vFormFreezeInfo.clear();
	}

	m_stFishFormData.lNextUpdateTime = 0;	//下次出现鱼阵的时间
	m_stFishFormData.iFormTypeID = 0;		//出的鱼阵的ID
	m_stFishFormData.lFormEndTime = 0;		//鱼阵消失的时间
	m_stFishFormData.bIsCleared = false;	//是否已经清场
	m_stFishFormData.bIsInForm = false;		//是否在鱼阵中

											//读取配置
	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();
	int iRandTimeMin = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_OUTFORMMIN);
	int iRandTimeMax = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_OUTFORMMAX);
	if (iRandTimeMax == iRandTimeMin)
	{
		return -1;
	}

	m_stFishFormData.lNextUpdateTime = CTimeUtility::GetMSTime() + (rand() % (iRandTimeMax - iRandTimeMin) + iRandTimeMin) * 1000;

	//推送鱼阵出现时间给客户端
	SendFishFormTime(NULL, true);

	return 0;
}

//更新单个出鱼信息
void CFishpondObj::UpdateOneOutFishRule(int iFishSeqID, OutFishRule& stFishRule, int iAddTime)
{
	long lTimeNow = CTimeUtility::GetMSTime();

	stFishRule.Reset();

	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();

	//鱼组配置
	FishSeqType::const_iterator it = stBaseCfgMgr.GetAllFishConfig().find(iFishSeqID);
	if (it == stBaseCfgMgr.GetAllFishConfig().end())
	{
		//没找到配置
		stFishRule.Reset();
		return;
	}

	const std::vector<FishConfig>& stSeqConfig = it->second;

	//鱼组ID
	stFishRule.iFishSeqID = iFishSeqID;

	//没有有效的配置
	if (stSeqConfig.size() == 0 || stSeqConfig[0].Time_max_i == stSeqConfig[0].Time_min_i)
	{
		stFishRule.Reset();
		return;
	}

	//判断是否对应房间
	if (!IsRoomType(stSeqConfig[0].iRoomTypeID))
	{
		//不是对应房间的鱼
		stFishRule.Reset();
		return;
	}

	//鱼出现的时间
	int iTimeInterval = 1000 * (rand() % (stSeqConfig[0].Time_max_i - stSeqConfig[0].Time_min_i) + stSeqConfig[0].Time_min_i);

	//加上人数对时间系数的影响
	iTimeInterval = iTimeInterval*stSeqConfig[0].aiTimeParam[GetPlayerNum()] / 1000;

	if (iTimeInterval == 0)
	{
		//不出鱼
		stFishRule.Reset();
		return;
	}
	
	stFishRule.lOutTime = lTimeNow + iTimeInterval + iAddTime * 1000;
	stFishRule.iDeltaTime = iAddTime * 1000;

	stFishRule.iPower = stSeqConfig[0].iPower;
	stFishRule.iLimitType = stSeqConfig[0].iLimitType;
	stFishRule.iType = stSeqConfig[0].Type_i;

	//遍历一个鱼组中的鱼
	int iRandMax = 0;
	for (unsigned i = 0; i<stSeqConfig.size(); ++i)
	{
		iRandMax += stSeqConfig[i].Occurrence_i;
	}

	int iRandNum = rand() % iRandMax;

	for (unsigned i = 0; i<stSeqConfig.size(); ++i)
	{
		if (iRandNum < stSeqConfig[i].Occurrence_i)
		{
			//生成的鱼
			stFishRule.iFishID = stSeqConfig[i].id;
			break;
		}

		iRandNum -= stSeqConfig[i].Occurrence_i;
	}

	return;
}

//更新玩家信息
void CFishpondObj::UpdateSeatUserInfo(CGameRoleObj& stRoleObj, long lTimeNow)
{
	SeatUserData* pstUserData = GetSeatUserByUin(stRoleObj.GetUin());
	if (!pstUserData)
	{
		return;
	}

	//获取房间算法数据
	RoomAlgorithmData& stData = FishAlgorithm::astData[m_pstRoomConfig->iAlgorithmType];

	//处理玩家发炮流水信息
	if (pstUserData->uCycleID != stData.uCycleID)
	{
		//结算周期已经更新
		pstUserData->uCycleID = stData.uCycleID;
		pstUserData->lUserFlow[CYCLE_TIME_LAST] = pstUserData->lUserFlow[CYCLE_TIME_NOW];
		pstUserData->lUserFlow[CYCLE_TIME_NOW] = 0;

		//玩家周期结算日志
		CZoneOssLog::TraceCycleProfit(stRoleObj.GetUin(), stRoleObj.GetChannel(), stRoleObj.GetNickName(), -pstUserData->iCycleWinNum, pstUserData->lCycleNewUsedNum,
			stRoleObj.GetResource(RESOURCE_TYPE_COIN), stRoleObj.GetResource(RESOURCE_TYPE_TICKET));

		pstUserData->iCycleWinNum = 0;
		pstUserData->lCycleNewUsedNum = 0;
	}

	//处理红包信息
	if (pstUserData->iEffectCountdown > 0)
	{
		//有未生效的红包
		pstUserData->iEffectCountdown -= 1000;
		if (pstUserData->iEffectCountdown <= 0)
		{
			BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();

			//处理红包生效效果
			int iEffectTime = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_TIMEDURATION);
			pstUserData->lUnEffectTime = lTimeNow + iEffectTime * 1000;
		}
	}

	return;
}

//更新出鱼信息
int CFishpondObj::UpdateOutFishRule(long lTimeNow)
{
	if (m_bStopOutFish)
	{
		//停止出鱼
		return 0;
	}

	//重新计算出鱼上限
	for (std::map<int, FishLimitData>::iterator it = m_mFishLimitData.begin(); it != m_mFishLimitData.end(); ++it)
	{
		it->second.iRemainNum = it->second.iLimitMaxNum;
	}

	for (unsigned i = 0; i < m_vFishData.size(); ++i)
	{
		--m_mFishLimitData[m_vFishData[i].iLimitType].iRemainNum;
	}

	//新出的鱼信息
	std::vector<FishData> vNewFishInfo;

	//遍历出鱼规则信息
	int iFishSeqID = 0;
	int iFishID = 0;
	long lOutTime = 0;
	int iRet = 0;
	for (unsigned i = 0; i<m_vOutFishRule.size(); ++i)
	{
		//是否鱼池已满
		if (m_vFishData.size() >= MAX_FISH_NUM)
		{
			break;
		}

		//是否超过出鱼上限
		if (m_mFishLimitData[m_vOutFishRule[i].iLimitType].iRemainNum <= 0)
		{
			continue;
		}

		if (m_vOutFishRule[i].lOutTime > lTimeNow)
		{
			//还未到出鱼时间
			continue;
		}

		iFishSeqID = m_vOutFishRule[i].iFishSeqID;
		iFishID = m_vOutFishRule[i].iFishID;
		lOutTime = lTimeNow + m_vOutFishRule[i].iDeltaTime;	//要用当前时间-出鱼差值时间

															//重新获取下一条鱼
		UpdateOneOutFishRule(m_vOutFishRule[i].iFishSeqID, m_vOutFishRule[i], 0);

		//鱼池中加鱼
		iRet = AddNewFish(iFishSeqID, iFishID, lOutTime);
		if (iRet)
		{
			LOGERROR("Failed to add new fish, ret %d, fish seq id %d, fish id %d, out time %ld, now %ld\n", iRet, iFishSeqID, iFishID,
				lOutTime, lTimeNow);
			continue;
		}

		//增加到新出鱼列表
		vNewFishInfo.push_back(*m_vFishData.rbegin());

		//修改出鱼上限
		--m_mFishLimitData[m_vOutFishRule[i].iLimitType].iRemainNum;
	}

	if (vNewFishInfo.size() > 0)
	{
		//推送新出鱼信息
		SendFishInfoToUser(NULL, vNewFishInfo);
	}

	return 0;
}

//更新鱼池中鱼的信息
int CFishpondObj::UpdateFishInfo(long lTimeNow)
{
	//清理所有死掉的鱼
	int iToBeDeleteNum = 0;
	for (unsigned i = 0; i<(m_vFishData.size() - iToBeDeleteNum);)
	{
		if (m_vFishData[i].lDeadTime > lTimeNow)
		{
			//未死亡
			++i;
			continue;
		}

		if (m_vFishData[i].iType == FISH_TYPE_WARHEAD)
		{
			//死亡的是弹头鱼,恢复出鱼
			m_bStopOutFish = false;
		}

		//该位置的鱼已经死亡
		m_vFishData[i] = m_vFishData[(m_vFishData.size() - 1) - iToBeDeleteNum];

		++iToBeDeleteNum;
	}

	m_vFishData.resize(m_vFishData.size() - iToBeDeleteNum);

	return 0;
}

//更新鱼阵信息
int CFishpondObj::UpdateFishFormInfo(long lTimeNow)
{
	//当前没有鱼阵
	return 0;

	//获取配置管理器
	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();

	if (!m_stFishFormData.bIsCleared && m_stFishFormData.lNextUpdateTime <= (lTimeNow + 5 * 1000))
	{
		m_stFishFormData.bIsCleared = true;

		//鱼阵开始前5s不出鱼
		LOGDEBUG("No fish before fishform, time %ld, form time %ld\n", lTimeNow, m_stFishFormData.lNextUpdateTime);

		//随机一个鱼阵
		const FormTimeConfig* pstFormConfig = stBaseCfgMgr.GetFormTimeConfigByWeight();
		if (!pstFormConfig)
		{
			LOGERROR("Failed to get formtime config!\n");
			return -1;
		}

		m_stFishFormData.iFormTypeID = pstFormConfig->ID;
		m_stFishFormData.lFormEndTime = lTimeNow + (pstFormConfig->iPauseTime * 1000 + 5 * 1000);

		//正常出鱼暂停
		for (unsigned i = 0; i<m_vOutFishRule.size(); ++i)
		{
			m_vOutFishRule[i].lOutTime += (pstFormConfig->iPauseTime * 1000 + 5 * 1000);
		}
	}

	if (!m_stFishFormData.bIsInForm && m_stFishFormData.lNextUpdateTime <= lTimeNow)
	{
		//鱼阵开始
		m_stFishFormData.bIsInForm = true;

		//清空鱼池
		m_vFishData.clear();

		//设置鱼阵出鱼信息
		const std::vector<TrajectoryConfig>* pstTrajectoryConfig = stBaseCfgMgr.GetTrajectoryConfigByType(m_stFishFormData.iFormTypeID);
		if (!pstTrajectoryConfig)
		{
			LOGERROR("Failed to get trajectory config, form type id %d\n", m_stFishFormData.iFormTypeID);
			return -2;
		}

		m_stFishFormData.vFishOutData.clear();
		m_stFishFormData.vFormFreezeInfo.clear();

		FormFishOutData stOutData;
		for (unsigned i = 0; i<(*pstTrajectoryConfig).size(); ++i)
		{
			stOutData.iFishID = (*pstTrajectoryConfig)[i].iFishID;

			const FishConfig* pstFishConfig = stBaseCfgMgr.GetFishConfig(stOutData.iFishID);
			if (!pstFishConfig)
			{
				LOGERROR("Failed to get fish config, fish id %d\n", stOutData.iFishID);
				return -3;
			}

			stOutData.iOutID = (*pstTrajectoryConfig)[i].ID;
			stOutData.iFishSeqID = pstFishConfig->Sequence_i;
			stOutData.iTraceID = (*pstTrajectoryConfig)[i].iTraceID;
			stOutData.lBeginTime = lTimeNow + (*pstTrajectoryConfig)[i].iStartTime;
			stOutData.iRemainNum = (*pstTrajectoryConfig)[i].iFishNumMax;
			stOutData.iInterval = (*pstTrajectoryConfig)[i].iInterval;
			stOutData.iType = pstFishConfig->Type_i;

			memset(stOutData.szAliveFish, 0, sizeof(stOutData.szAliveFish));
			for (int j = 0; j < stOutData.iRemainNum / 64; ++j)
			{
				stOutData.szAliveFish[j] = ~0x0;
			}
			stOutData.szAliveFish[stOutData.iRemainNum / 64] = ((ulong8)0x01 << (stOutData.iRemainNum % 64)) - 1;

			//鱼的倍数
			if (pstFishConfig->Multiple_i_min == pstFishConfig->Multiple_i_max)
			{
				stOutData.iMultiple = pstFishConfig->Multiple_i_min;
			}
			else
			{
				stOutData.iMultiple = pstFishConfig->Multiple_i_min + rand() % (pstFishConfig->Multiple_i_max - pstFishConfig->Multiple_i_min);
			}

			m_stFishFormData.vFishOutData.push_back(stOutData);
		}

		//推送鱼阵结束时间
		SendFishFormTime(NULL, false);

		//推送鱼阵详细信息
		SendFishFormInfo(NULL);
	}

	if (m_stFishFormData.bIsInForm && m_stFishFormData.lFormEndTime > lTimeNow)
	{
		//鱼阵进行中
	}
	else if (m_stFishFormData.bIsInForm && m_stFishFormData.lFormEndTime <= lTimeNow)
	{
		//鱼阵结束
		InitFishFormRule(true);

		//清空鱼池
		//m_vFishData.clear();
	}

	return 0;
}

//更新玩家信息
int CFishpondObj::UpdateFishUserInfo(long lTimeNow)
{
	int iTimeNow = lTimeNow / 1000;
	for (unsigned i = 0; i < m_vSeatUserData.size(); ++i)
	{
		CGameRoleObj* pstRoleObj = CUnitUtility::GetRoleByUin(m_vSeatUserData[i].uiUin);
		if (!pstRoleObj)
		{
			continue;
		}

		//todo jasonxiong 临时去掉，后续要恢复回来
		/*
		if ((lTimeNow / 1000 - m_vSeatUserData[i].iActiveTime) >= 5 * 60)
		{
			//已经超过5分钟不活跃，强制踢掉玩家
			ExitFishpond(*pstRoleObj, true);
		}
		*/

		if (!m_vSeatUserData[i].bAutoShoot)
		{
			//未开自动发炮
			continue;
		}

		if (pstRoleObj->GetMonthEndTime() < iTimeNow)
		{
			//月卡已过期.取消自动发炮
			m_vSeatUserData[i].bAutoShoot = false;
		}
	}

	return T_SERVER_SUCCESS;
}

//鱼池中加鱼
int CFishpondObj::AddNewFish(int iFishSeqID, int iFishID, long lOutTime, int iTraceID)
{
	//是否鱼池已满
	if (m_vFishData.size() >= MAX_FISH_NUM)
	{
		return 0;
	}

	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();
	const FishConfig* pstFishConfig = stBaseCfgMgr.GetFishConfig(iFishSeqID, iFishID);
	if (!pstFishConfig)
	{
		//找不到鱼的配置
		return T_ZONE_INVALID_CFG;
	}

	//出鱼
	FishData stFishInfo;
	stFishInfo.uUniqueID = CFishUtility::GetFishUniqID();
	stFishInfo.iFishID = iFishID;
	stFishInfo.lBornTime = lOutTime;
	stFishInfo.iFishSeqID = iFishSeqID;
	stFishInfo.iType = pstFishConfig->Type_i;
	stFishInfo.iLimitType = pstFishConfig->iLimitType;

	if (stFishInfo.iType == FISH_TYPE_SMALLGROUP)
	{
		//小鱼组
		const SmallFishConfig* pstSmallConfig = stBaseCfgMgr.GetSmallFishConfig(stFishInfo.iFishID);
		if (!pstSmallConfig)
		{
			LOGERROR("Failed to get small fish config, fish id %d\n", stFishInfo.iFishID);
			return T_ZONE_INVALID_CFG;
		}

		stFishInfo.cIndex = (0x01 << pstSmallConfig->vTrackIDs.size()) - 1;
		
		//设置轨迹为第一个鱼的轨迹
		if (pstSmallConfig->vTrackIDs.size() > 0)
		{
			iTraceID = pstSmallConfig->vTrackIDs[0];
		}
	}
	else if (stFishInfo.iType == FISH_TYPE_WARHEAD)
	{
		//是弹头鱼,停止出鱼
		m_bStopOutFish = true;
	}

	if (pstFishConfig->Multiple_i_min == pstFishConfig->Multiple_i_max)
	{
		stFishInfo.iMultiple = pstFishConfig->Multiple_i_min;
	}
	else
	{
		stFishInfo.iMultiple = pstFishConfig->Multiple_i_min + rand() % (pstFishConfig->Multiple_i_max - pstFishConfig->Multiple_i_min);
	}

	if (iTraceID == 0)
	{
		//计算轨迹
		std::vector<int> vAllTraceIDs;
		std::vector<int> vOneTypeTraceIDs;
		for (unsigned j = 0; j < pstFishConfig->vTraceType_i.size(); ++j)
		{
			stBaseCfgMgr.GetTraceIDsByType(pstFishConfig->vTraceType_i[j], vOneTypeTraceIDs);
			vAllTraceIDs.insert(vAllTraceIDs.end(), vOneTypeTraceIDs.begin(), vOneTypeTraceIDs.end());
		}

		if (vAllTraceIDs.size() == 0)
		{
			//找不到合法的轨迹
			return -2;
		}

		//随机鱼的轨迹
		iTraceID = vAllTraceIDs[rand() % vAllTraceIDs.size()];
	}

	stFishInfo.iTraceID = iTraceID;

	//获取轨迹配置
	const TraceConfig* pstTraceConfig = stBaseCfgMgr.GetTraceConfigByID(stFishInfo.iTraceID);
	if (!pstTraceConfig)
	{
		//找不到配置
		return -3;
	}

	//设置死亡时间
	stFishInfo.lDeadTime = stFishInfo.lBornTime + pstTraceConfig->vPoints[pstTraceConfig->vPoints.size() - 1].iTime;

	//查找轨迹最近出现的时间
	long lTraceLastTime = GetLastTraceBornTime(stFishInfo.iTraceID);
	long lIntervalTime = stFishInfo.lBornTime - lTraceLastTime;
	if (lIntervalTime < 5 * 1000)
	{
		stFishInfo.lBornTime += 5 * 1000;
		stFishInfo.lDeadTime += 5 * 1000;
	}

	//打印出鱼日志
	//LOGDEBUG("NewFish Info, table %u, fish id %d, seq id %d, trace id %d, born time %ld\n", m_uTableID, stFishInfo.iFishID,
	//	stFishInfo.iFishID, stFishInfo.iTraceID, stFishInfo.lBornTime);

	//增加到鱼列表中
	m_vFishData.push_back(stFishInfo);

	return 0;
}

long CFishpondObj::GetLastTraceBornTime(int iTraceID)
{
	long lLastTraceTime = 0;
	for (unsigned i = 0; i < m_vFishData.size(); ++i)
	{
		if ((m_vFishData[i].iTraceID == iTraceID) && (lLastTraceTime < m_vFishData[i].lBornTime))
		{
			lLastTraceTime = m_vFishData[i].lBornTime;
		}
	}

	return lLastTraceTime;
}

//是否对应类型的房间
bool CFishpondObj::IsRoomType(int iRoomType)
{
	return (m_pstRoomConfig->iRoomTypeID & iRoomType);
}

//是否对应房间模式
bool CFishpondObj::IsRoomPattern(int iRoomPattern)
{
	return (m_pstRoomConfig->iRoomPattern == iRoomPattern);
}

//玩家坐下
int CFishpondObj::PlayerSitDown(CGameRoleObj& stRoleObj)
{
	SeatUserData stUserData;
	stUserData.uiUin = stRoleObj.GetUin();

	for (int iSeat = 0; iSeat < m_pstRoomConfig->iPlayerNum; ++iSeat)
	{
		bool bHasPlayer = false;
		for (unsigned i = 0; i < m_vSeatUserData.size(); ++i)
		{
			if (iSeat == m_vSeatUserData[i].iSeat)
			{
				//该位置已经有人
				bHasPlayer = true;
				break;
			}
		}

		if (!bHasPlayer)
		{
			//该位置空闲
			stUserData.iSeat = iSeat;
			stUserData.iActiveTime = CTimeUtility::GetNowTime();
			m_vSeatUserData.push_back(stUserData);

			return T_SERVER_SUCCESS;
		}
	}

	return T_ZONE_PARA_ERROR;
}

//删除玩家所有子弹
void CFishpondObj::ClearUserBullets(int iSeat)
{
	unsigned iLastIndex = m_vBulletData.size();
	for (unsigned i = 0; i < iLastIndex;)
	{
		if (m_vBulletData[i].iSeat != iSeat)
		{
			++i;
			continue;
		}

		m_vBulletData[i] = m_vBulletData[iLastIndex - 1];
		--iLastIndex;
	}

	m_vBulletData.resize(iLastIndex);

	return;
}

//获取玩家座位信息
SeatUserData* CFishpondObj::GetSeatUserByUin(unsigned uiUin)
{
	for (unsigned i = 0; i < m_vSeatUserData.size(); ++i)
	{
		if (m_vSeatUserData[i].uiUin == uiUin)
		{
			return &m_vSeatUserData[i];
		}
	}

	return NULL;
}

//获取玩家子弹
BulletData* CFishpondObj::GetBulletData(unsigned uBulletUniqID)
{
	for (unsigned i = 0; i < m_vBulletData.size(); ++i)
	{
		if (m_vBulletData[i].uUniqueID == uBulletUniqID)
		{
			return &m_vBulletData[i];
		}
	}

	return NULL;
}

//获取鱼的数据
FishData* CFishpondObj::GetFishData(unsigned uUniqID)
{
	for (unsigned i = 0; i<m_vFishData.size(); ++i)
	{
		if (m_vFishData[i].uUniqueID == uUniqID)
		{
			return &(m_vFishData[i]);
		}
	}

	return NULL;
}

//获取鱼阵中鱼的信息
FormFishOutData* CFishpondObj::GetFormFishInfo(int iFishOutID, int iFishIndex, FishData& stFishInfo)
{
	stFishInfo.Reset();

	std::vector<FormFishOutData>& vFormOutData = m_stFishFormData.vFishOutData;
	std::vector<FreezeData>& vFreezeInfo = m_stFishFormData.vFormFreezeInfo;
	for (unsigned i = 0; i < vFormOutData.size(); ++i)
	{
		if (vFormOutData[i].iOutID != iFishOutID)
		{
			continue;
		}

		//检查Index
		if (iFishIndex < 0 || iFishIndex >= vFormOutData[i].iRemainNum)
		{
			return NULL;
		}

		//该鱼已经死亡
		if (!vFormOutData[i].IsFishAlive(iFishIndex))
		{
			return NULL;
		}

		//获取鱼
		stFishInfo.uUniqueID = iFishOutID * 1000 + (iFishIndex + 1);
		stFishInfo.iFishID = vFormOutData[i].iFishID;
		stFishInfo.iFishSeqID = vFormOutData[i].iFishSeqID;
		stFishInfo.iTraceID = vFormOutData[i].iTraceID;
		stFishInfo.lBornTime = vFormOutData[i].lBeginTime + vFormOutData[i].iInterval*iFishIndex;
		stFishInfo.lDeadTime = 0;
		stFishInfo.iMultiple = vFormOutData[i].iMultiple;
		stFishInfo.iType = vFormOutData[i].iType;

		//获取冰冻信息
		for (unsigned j = 0; j < vFreezeInfo.size(); ++j)
		{
			if (vFreezeInfo[j].uUniqID == stFishInfo.uUniqueID)
			{
				//找到
				stFishInfo.lFreezeBeginTime = vFreezeInfo[j].lFreezeBeginTime;
				stFishInfo.iFreezeContTime = vFreezeInfo[j].iFreezeContTime;
				stFishInfo.iTotalFreezeTime = vFreezeInfo[j].iTotalFreezeTime;

				break;
			}
		}

		return &vFormOutData[i];
	}

	return NULL;
}

//删除鱼
void CFishpondObj::DeleteFishData(unsigned uUniqID)
{
	for (std::vector<FishData>::iterator it = m_vFishData.begin(); it != m_vFishData.end(); ++it)
	{
		if ((*it).uUniqueID == uUniqID)
		{
			m_vFishData.erase(it);

			return;
		}
	}

	return;
}

//删除子弹
void CFishpondObj::DeleteBulletData(unsigned uUniqID)
{
	for (std::vector<BulletData>::iterator it = m_vBulletData.begin(); it != m_vBulletData.end(); ++it)
	{
		if ((*it).uUniqueID == uUniqID)
		{
			m_vBulletData.erase(it);
			return;
		}
	}

	return;
}

//检查是否有效命中
bool CFishpondObj::CheckIsValidHit(long lHitTime, BulletData& stBulletInfo, FishData& stFishInfo, int iFishIndex)
{
	//todo jasonxiong 后面再来考虑位置问题
	return true;

	/*
	//获取鱼的位置
	FISHPOS stFishPos;
	int iRatio = 0;
	if (!GetFishPosByTime(stFishInfo, lHitTime, stFishPos, iRatio))
	{
	return false;
	}

	if (stFishInfo.iType == FISH_TYPE_SMALLGROUP)
	{
	//小鱼组
	const SmallFishConfig* pstSmallConfig = stBaseCfgMgr.GetSmallFishConfig(stFishInfo.iFishID);
	if (!pstSmallConfig)
	{
	return false;
	}

	stFishPos.iX += pstSmallConfig->vTrackIDs[iFishIndex].iX;
	stFishPos.iY += pstSmallConfig->vTrackIDs[iFishIndex].iY;
	}

	//获取子弹位置
	FISHPOS stBulletPos;
	if (!GetBulletPosByTime(stBulletInfo, lHitTime, stBulletPos))
	{
	return false;
	}

	const FishConfig* pstFishConfig = stBaseCfgMgr.GetFishConfig(stFishInfo.iFishSeqID, stFishInfo.iFishID);
	if (!pstFishConfig)
	{
	//找不到配置
	return false;
	}

	int iLength = (stFishPos.iX - stBulletPos.iX)*(stFishPos.iX - stBulletPos.iX) + (stFishPos.iY - stBulletPos.iY)*(stFishPos.iY - stBulletPos.iY);
	int iValidLength = pstFishConfig->iWidth * pstFishConfig->iWidth + pstFishConfig->iHeight * pstFishConfig->iHeight;
	iValidLength = iValidLength * iRatio / 4000 + 100 * 100;	//允许100像素误差

	//判断位置是否命中
	if (iLength < iValidLength)
	{
	return true;
	}

	LOGERROR("hit fish pos %d:%d, trace %d, born time %ld\n", stFishPos.iX, stFishPos.iY,
	stFishInfo.iTraceID, stFishInfo.lBornTime);

	LOGERROR("bullet pos %d:%d, seat %d, target pos %d:%d, shoot time %ld, hit time %ld, server time %ld\n",
	stBulletPos.iX, stBulletPos.iY, stBulletInfo.iSeat, stBulletInfo.stTargetPos.iX, stBulletInfo.stTargetPos.iY, stBulletInfo.lShootTime,
	lHitTime, CTimeUtility::GetMSTime());

	return false;
	*/
}

//检查逻辑是否命中
bool CFishpondObj::CheckIsLogicHit(CGameRoleObj& stRoleObj, SeatUserData& stUserData, const GunConfig& stGunConfig, FishData& stFishInfo, int iAdjust, bool bIsForm)
{
	//计算公式： P=(1-X+红包加成概率+新手红包加成概率-服务器状态-个人状态 + 体验线加成)*Fish->Adjust/Fish->Multiple

	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();

	//获取房间算法数据
	RoomAlgorithmData& stData = FishAlgorithm::astData[m_pstRoomConfig->iAlgorithmType];

	//计算红包总加成概率
	int iRedPacketPercent = 0;
	int iNowNewRedNum = 0;
	if (m_pstRoomConfig->iAlgorithmType == 1)
	{
		//新手红包只在算法类型1生效
		iNowNewRedNum = stRoleObj.GetNowNewRedNum();
	}

	if (stUserData.uReturnID != 0 && stUserData.lUnEffectTime >= CTimeUtility::GetMSTime())
	{
		if ((stData.lTotalReturnSilver + iNowNewRedNum) >= (stGunConfig.iMultiple*stFishInfo.iMultiple))
		{
			if (iNowNewRedNum >= (stGunConfig.iMultiple*stFishInfo.iMultiple))
			{
				//新手红包能覆盖这条鱼
				iRedPacketPercent = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_RATIOBONUS) + stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_NEWADDRATE);
			}
			else
			{
				//没有新手红包
				iRedPacketPercent = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_RATIOBONUS);
			}
		}
	}
	else if (iNowNewRedNum >= (stGunConfig.iMultiple*stFishInfo.iMultiple))
	{
		//新手红包可用
		iRedPacketPercent = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_NEWADDRATE);
	}

	//计算个人状态
	int iUserX = GetUserStatus(stUserData);

	int iBasePercent = 1000;

	//获取体验线数据
	int iExpLinePercent = 0;
	const ExpLineConfig* pstConfig = NULL;
	TEXPLINEINFO* pstExpLineInfo = GetRoleExpLineInfo(stUserData.uiUin, pstConfig);
	if (pstExpLineInfo)
	{
		iExpLinePercent = pstConfig->iAddRate;
	}

	long8 lHitPercent = (iBasePercent - stData.iX + iRedPacketPercent - stData.iServerX - iUserX + iExpLinePercent) * iAdjust;
	lHitPercent = lHitPercent / (stFishInfo.iMultiple * 1000);

	//多倍鱼需要增加鱼的倍数
	if (stFishInfo.iType == FISH_TYPE_MULTIPLE)
	{
		int iMultiple = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_MULTIPLEFISH);
		lHitPercent = lHitPercent * 1000 / iMultiple;

		if (stData.lMultiFishLossNum < 0)
		{
			//上个周期服务器多倍鱼亏损，提高多倍鱼系数加成
			int iMultipleAddtion = stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_MULTIPLEFISH_ADD);
			lHitPercent = lHitPercent * 1000 / iMultipleAddtion;
		}
	}

	//打印运营日志 玩家命中概率 ID = 10004
	RoomAlgorithmData& stAlgorithmData = FishAlgorithm::astData[m_pstRoomConfig->iAlgorithmType];
	CZoneOssLog::TraceHitFish(stRoleObj.GetUin(), stRoleObj.GetChannel(), stGunConfig.iGunID, stFishInfo.iFishID, stFishInfo.uUniqueID, 
		bIsForm, stAlgorithmData.lTotalReturnSilver, stAlgorithmData.iX, iRedPacketPercent, stAlgorithmData.iServerX, iUserX, iExpLinePercent, stUserData.iReturnType);

	if (rand() % 1000 < lHitPercent)
	{
		return true;
	}

	return false;
}

//获取玩家概率状态
int CFishpondObj::GetUserStatus(SeatUserData& stUserData)
{
	return 0;

	/*
	if (stUserData.lUserSilverCost == 0)
	{
		return 0;
	}

	int iRewardRate = (int)(stUserData.lUserSilverReward * 1000 / stUserData.lUserSilverCost);
	const StatControlConfig* pstConfig = CConfigManager::Instance()->GetBaseCfgMgr().GetStatConfig(iRewardRate);
	if (!pstConfig)
	{
		//取不到配置
		LOGERROR("Failed to get user state config, reward rate %d\n", iRewardRate);
		return 0;
	}

	if (pstConfig->iStep <= 0)
	{
		return 0;
	}

	return (iRewardRate - pstConfig->iValueMin)*pstConfig->iDecrease / pstConfig->iStep;
	*/
}

bool FishCompareFunc(const FishData& stFish1, const FishData& stFish2)
{
	return stFish1.iMultiple < stFish2.iMultiple;
}

bool FormFishOutCompareFunc(const FormFishOutData& stFormOut1, const FormFishOutData& stFormOut2)
{
	return stFormOut1.iMultiple < stFormOut2.iMultiple;
}

//获取玩家体验线
TEXPLINEINFO* CFishpondObj::GetRoleExpLineInfo(unsigned uiUin, const ExpLineConfig*& pstConfig)
{
	CGameRoleObj* pstRoleObj = CUnitUtility::GetRoleByUin(uiUin);
	if (!pstRoleObj)
	{
		return NULL;
	}

	TEXPLINEINFO* pstExpLineInfo = pstRoleObj->GetExpLineInfo(m_pstRoomConfig->iAlgorithmType);
	if (!pstExpLineInfo)
	{
		return NULL;
	}

	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();

	//没有体验线
	if (pstExpLineInfo->iExpLineID == 0)
	{
		//选择一条体验线
		pstConfig = stBaseCfgMgr.GetExpLineConfig(pstExpLineInfo->iExpLineType, m_pstRoomConfig->iAlgorithmType);
		if (!pstConfig)
		{
			LOGERROR("Failed to get exp line config, uin %u, type %d, algorithm type %d\n", uiUin, pstExpLineInfo->iExpLineType,
				m_pstRoomConfig->iAlgorithmType);
			return NULL;
		}

		pstExpLineInfo->iExpLineID = pstConfig->iID;

		return pstExpLineInfo;
	}

	//已经有体验线
	pstConfig = stBaseCfgMgr.GetExpLineConfig(pstExpLineInfo->iExpLineID);
	if (!pstConfig)
	{
		LOGERROR("Failed to get exp line config, uin %u, id %d\n", uiUin, pstExpLineInfo->iExpLineID);
		return NULL;
	}

	if (pstExpLineInfo->lCostNum < (pstConfig->iBulletMax * pstConfig->iStandardNum))
	{
		//不需要切换体验线
		return pstExpLineInfo;
	}

	//需要切换体验线

	//体验线运营日志
	CZoneOssLog::TraceExpLine(*pstRoleObj, pstExpLineInfo->lIntervalWinNum, pstExpLineInfo->iExpLineID, pstExpLineInfo->iBulletNum, pstExpLineInfo->lCostNum);

	if (pstConfig->iNextLine != 0)
	{
		//切换到下一条
		pstExpLineInfo->iExpLineID = pstConfig->iNextLine;
		pstExpLineInfo->iBulletNum = 0;
		pstExpLineInfo->lCostNum = 0;
		pstExpLineInfo->lIntervalWinNum = 0;

		return pstExpLineInfo;
	}

	int iNewExpLineType = pstExpLineInfo->iExpLineType;
	if (pstExpLineInfo->lUserWinNum > 0)
	{
		iNewExpLineType = pstExpLineInfo->iExpLineType + 1;
	}
	else
	{
		iNewExpLineType = pstExpLineInfo->iExpLineType - 1;
	}

	pstConfig = stBaseCfgMgr.GetExpLineConfig(iNewExpLineType, m_pstRoomConfig->iAlgorithmType);
	if (!pstConfig)
	{
		//找不到新的，退回到老的
		iNewExpLineType = pstExpLineInfo->iExpLineType;
		pstConfig = stBaseCfgMgr.GetExpLineConfig(iNewExpLineType, m_pstRoomConfig->iAlgorithmType);
	}

	if (!pstConfig)
	{
		LOGERROR("Failed to get exp line config, type %d, algorithm type %d\n", iNewExpLineType, m_pstRoomConfig->iAlgorithmType);
		return NULL;
	}

	//设置新体验线
	pstExpLineInfo->iExpLineID = pstConfig->iID;
	pstExpLineInfo->iExpLineType = iNewExpLineType;
	pstExpLineInfo->lUserWinNum = 0;
	pstExpLineInfo->lIntervalWinNum = 0;
	pstExpLineInfo->iBulletNum = 0;
	pstExpLineInfo->lCostNum = 0;

	return pstExpLineInfo;
}

//处理电鳗和炸弹鱼逻辑
void CFishpondObj::BoomFish(SeatUserData& stUserData, FishData*& pstFishInfo, bool bIsFormFish)
{
	//倍数重新计算
	int iMultiple = 0;
	unsigned uBoomFishUniqID = pstFishInfo->uUniqueID;

	//炸弹鱼采用新逻辑，炸死BOSS鱼,鱼票鱼,弹头鱼之外的所有鱼
	std::vector<unsigned> vFishUniqIDs;
	std::vector<unsigned> vFormUniqIDs;
	std::vector<BYTE> vSmallFishs;

	std::vector<FishData> vNewFishData;
	for (unsigned i = 0; i < m_vFishData.size(); ++i)
	{
		if (m_vFishData[i].iType == FISH_TYPE_BIGBOSS ||
			m_vFishData[i].iType == FISH_TYPE_PHONETICKET ||
			m_vFishData[i].iType == FISH_TYPE_WARHEAD)
		{
			//不炸死
			vNewFishData.push_back(m_vFishData[i]);
			continue;
		}

		if (m_vFishData[i].uUniqueID == pstFishInfo->uUniqueID)
		{
			//不炸死自己
			vNewFishData.push_back(m_vFishData[i]);
			continue;
		}

		//计算炸弹鱼倍数
		iMultiple += m_vFishData[i].iMultiple;

		vFishUniqIDs.push_back(m_vFishData[i].uUniqueID);
	}

	//删除鱼
	m_vFishData = vNewFishData;

	//如果是普通鱼，stFishInfo需要重新获取
	if (!bIsFormFish)
	{
		pstFishInfo = GetFishData(uBoomFishUniqID);
	}

	pstFishInfo->iMultiple = iMultiple;
	if (pstFishInfo->iMultiple == 0)
	{
		//先写死如果没有给10倍
		pstFishInfo->iMultiple = 10;
	}

	//推送爆炸死的鱼信息
	SendBoomFishInfoNotify(stUserData, vFishUniqIDs, vSmallFishs, vFormUniqIDs);

	LOGDEBUG("BoomFish, fish uniq id %u, formfish %d, fish id %d, multiple %d, fish num %zu, formfish num %zu\n", pstFishInfo->uUniqueID,
		bIsFormFish, pstFishInfo->iFishID, pstFishInfo->iMultiple, vFishUniqIDs.size(), vFormUniqIDs.size());

	return;

	/*
	//先凑齐鱼倍数
	int iLeftMultiple = pstFishInfo->iMultiple;
	std::vector<unsigned> vFishUniqIDs;
	std::vector<unsigned> vFormUniqIDs;
	std::vector<BYTE> vSmallFishs;
	std::vector<unsigned> vDelFishIndexes;
	std::vector<FormFishOutData>& vFishOutData = m_stFishFormData.vFishOutData;
	unsigned uBoomFishUniqID = pstFishInfo->uUniqueID;

	//先排序
	std::sort(m_vFishData.begin(), m_vFishData.end(), FishCompareFunc);
	std::sort(vFishOutData.begin(), vFishOutData.end(), FormFishOutCompareFunc);

	long lTimeNow = CTimeUtility::GetMSTime();
	for (unsigned i = 0; i < m_vFishData.size(); ++i)
	{
	if (!bIsFormFish && m_vFishData[i].uUniqueID == uBoomFishUniqID)
	{
	//是炸弹鱼自己
	continue;
	}

	for (unsigned j = 0; j < vFishOutData.size(); ++j)
	{
	if (vFishOutData[j].iMultiple <= m_vFishData[i].iMultiple)
	{
	//用鱼阵中的鱼来凑
	if (iLeftMultiple <= vFishOutData[j].iMultiple*vFishOutData[j].GetLiveFishNum())
	{
	//鱼阵中的鱼够凑
	for (int iFishIndex = 0; iFishIndex < vFishOutData[j].iRemainNum; ++iFishIndex)
	{
	if (bIsFormFish && (unsigned)(vFishOutData[j].iOutID * 1000 + iFishIndex + 1) == uBoomFishUniqID)
	{
	//是炸弹鱼自己
	continue;
	}

	if (iLeftMultiple < vFishOutData[j].iMultiple)
	{
	break;
	}

	if (!vFishOutData[j].IsFishAlive(iFishIndex))
	{
	//已经死亡
	continue;
	}

	//获取鱼的信息
	FishData stFishInfo;
	FormFishOutData* pstFormOutData = GetFormFishInfo(vFishOutData[j].iOutID, iFishIndex, stFishInfo);
	if (!pstFormOutData)
	{
	//获取鱼阵的鱼失败
	continue;
	}

	//鱼是否在屏幕中
	if (!IsFishInScreen(stFishInfo, lTimeNow))
	{
	//不在屏幕中
	continue;
	}

	iLeftMultiple -= vFishOutData[j].iMultiple;
	vFormUniqIDs.push_back(vFishOutData[j].iOutID * 1000 + (iFishIndex + 1));
	vFishOutData[j].SetFishAlive(iFishIndex, false);
	}
	}
	else
	{
	//鱼阵中的鱼凑不够
	iLeftMultiple -= (vFishOutData[j].iMultiple * vFishOutData[j].GetLiveFishNum());
	for (int iFishIndex = 0; iFishIndex < vFishOutData[j].iRemainNum; ++iFishIndex)
	{
	if (!vFishOutData[j].IsFishAlive(iFishIndex))
	{
	//已经死亡
	continue;
	}

	if (bIsFormFish && (unsigned)(vFishOutData[j].iOutID * 1000 + iFishIndex + 1) == uBoomFishUniqID)
	{
	//是炸弹鱼自己
	continue;
	}

	//获取鱼的信息
	FishData stFishInfo;
	FormFishOutData* pstFormOutData = GetFormFishInfo(vFishOutData[j].iOutID, iFishIndex, stFishInfo);
	if (!pstFormOutData)
	{
	//获取鱼阵的鱼失败
	continue;
	}

	//鱼是否在屏幕中
	if (!IsFishInScreen(stFishInfo, lTimeNow))
	{
	//不在屏幕中
	continue;
	}

	vFormUniqIDs.push_back(vFishOutData[j].iOutID * 1000 + (iFishIndex + 1));
	vFishOutData[j].SetFishAlive(iFishIndex, false);
	}
	}
	}
	else
	{
	//不用鱼阵中的鱼
	break;
	}
	}

	//使用正常的鱼
	if (iLeftMultiple < m_vFishData[i].iMultiple)
	{
	//已经找不到满足条件的
	break;
	}
	else
	{
	//鱼是否在屏幕中
	if (!IsFishInScreen(m_vFishData[i], lTimeNow))
	{
	//不在屏幕中
	continue;
	}

	if (m_vFishData[i].iType == FISH_TYPE_PHONETICKET)
	{
	//点券鱼不能炸
	continue;
	}

	if (m_vFishData[i].iType != FISH_TYPE_SMALLGROUP)
	{
	//不是小鱼组
	iLeftMultiple -= m_vFishData[i].iMultiple;
	vFishUniqIDs.push_back(m_vFishData[i].uUniqueID);
	vSmallFishs.push_back(0);
	vDelFishIndexes.push_back(i);
	}
	else
	{
	//小鱼组
	vFishUniqIDs.push_back(m_vFishData[i].uUniqueID);

	BYTE ucOldIndex = m_vFishData[i].cIndex;
	for (unsigned j = 0; j < 8; ++j)
	{
	if (iLeftMultiple < m_vFishData[i].iMultiple)
	{
	//已经找不到满足条件的
	break;
	}

	//小鱼组最多有5条鱼
	if (m_vFishData[i].cIndex & (0x01 << j))
	{
	//该位置有鱼
	iLeftMultiple -= m_vFishData[i].iMultiple;
	m_vFishData[i].cIndex &= ~(0x01 << j);
	}
	}

	vSmallFishs.push_back(ucOldIndex - m_vFishData[i].cIndex);
	if (m_vFishData[i].cIndex == 0)
	{
	//没有鱼了，删除
	vDelFishIndexes.push_back(i);
	}
	}

	}
	}

	//删除鱼
	for (unsigned i = 0; i < vDelFishIndexes.size(); ++i)
	{
	m_vFishData[vDelFishIndexes[i]] = m_vFishData[m_vFishData.size() - i - 1];
	}
	m_vFishData.resize(m_vFishData.size() - vDelFishIndexes.size());

	//如果是普通鱼，stFishInfo需要重新获取
	if (!bIsFormFish)
	{
	pstFishInfo = GetFishData(uBoomFishUniqID);
	}

	//更新爆炸鱼倍数
	pstFishInfo->iMultiple -= iLeftMultiple;

	//推送爆炸死的鱼信息
	SendBoomFishInfoNotify(vFishUniqIDs, vSmallFishs, vFormUniqIDs);

	LOGDEBUG("BoomFish, fish uniq id %u, formfish %d, fish id %d, multiple %d, left multiple %d, fish num %zu, formfish num %zu\n", pstFishInfo->uUniqueID,
	bIsFormFish, pstFishInfo->iFishID, pstFishInfo->iMultiple, iLeftMultiple, vFishUniqIDs.size(), vFormUniqIDs.size());

	return;
	*/
}

//鱼是否在屏幕中
bool CFishpondObj::IsFishInScreen(FishData& stFishInfo, long lTimeNow)
{
	//FISHPOS stPos;
	//int iRatio = 0;

	//todo jasonxiong 再来开发
	return true;

	/*
	if (!GetFishPosByTime(stFishInfo, lTimeNow, stPos, iRatio))
	{
	//获取位置失败
	return false;
	}

	if (abs(stPos.iX) > CLIENT_SCREEN_WIDTH / 2 || abs(stPos.iY) > CLIENT_SCREEN_HEIGHT / 2)
	{
	//鱼在屏幕外
	return false;
	}
	*/

	return true;
}

//掉落点券
bool CFishpondObj::AddUserTicket(CGameRoleObj& stRoleObj, unsigned uFishUniqID, bool bIsFormFish, int iMultiple, int iNum, bool bIsTicketFish)
{
	if (iNum == 0)
	{
		return true;
	}

	long8 lRandTicketNum = 0;
	if (bIsTicketFish)
	{
		//点券鱼掉落
		lRandTicketNum = iNum;
	}
	else
	{
		//计算公式为: 数量 = iRewardSilver * ConfigY /(100*1000); 向下或向上取整，根据随机概率
		int iConfigY = CConfigManager::Instance()->GetBaseCfgMgr().GetGlobalConfig(GLOBAL_TYPE_PHONETICKETRATE);
		lRandTicketNum = iNum * iConfigY;

		if (lRandTicketNum % (100 * 1000) > rand() % (100 * 1000))
		{
			lRandTicketNum = lRandTicketNum / (100 * 1000) + 1;
		}
		else
		{
			lRandTicketNum = lRandTicketNum / (100 * 1000);
		}
	}

	if (lRandTicketNum == 0)
	{
		return true;
	}

	//添加点券
	if (!CResourceUtility::AddUserRes(stRoleObj, RESOURCE_TYPE_TICKET, lRandTicketNum))
	{
		return false;
	}

	//推送获得点券的消息
	SendAddTicketNotify(stRoleObj.GetUin(), uFishUniqID, bIsFormFish, lRandTicketNum);

	//打印运营日志 打鱼获得点券 ID = 10
	//todo jasonxiong 再统一来整
	//GetGameLogic()->OssFishPhoneTicketInfo(player, iMultiple, lRandTicketNum, bIsTicketFish);

	return true;
}

//更新红包信息
void CFishpondObj::UpdateRedPacketInfo(CGameRoleObj& stRoleObj, SeatUserData& stUserData)
{
	//获取房间算法数据
	RoomAlgorithmData& stData = FishAlgorithm::astData[m_pstRoomConfig->iAlgorithmType];

	if (stUserData.uCycleID != stData.uCycleID)
	{
		//玩家未进入当前结算周期
		return;
	}

	//先判断是否已经有红包
	if (stUserData.uReturnID == stData.uReturnID)
	{
		//已经有最新红包
		return;
	}

	//判断是否红包生成之前登录
	if (stRoleObj.GetLoginTime() >= stData.lLastUpdateTime)
	{
		//红包生成后登录
		return;
	}

	//尝试获取一个红包
	FishAlgorithm::GetOneRedPacket(stRoleObj, stUserData, m_pstRoomConfig->iAlgorithmType);

	return;
}

//发送鱼阵时间
void CFishpondObj::SendFishFormTime(CGameRoleObj* pstRoleObj, bool bIsBegin)
{
	//暂时没有鱼阵
	return;

	CZoneMsgHelper::GenerateMsgHead(ms_stZoneMsg, MSGID_ZONE_FORMTIME_NOTIFY);

	Zone_FormTime_Notify* pstNotify = ms_stZoneMsg.mutable_stbody()->mutable_m_stzone_formtime_notify();
	pstNotify->set_bisbegin(bIsBegin);
	pstNotify->set_ltime((bIsBegin ? m_stFishFormData.lNextUpdateTime : m_stFishFormData.lFormEndTime));

	if (pstRoleObj)
	{
		//发送给玩家
		CZoneMsgHelper::SendZoneMsgToRole(ms_stZoneMsg, pstRoleObj);
	}
	else
	{
		//发送给桌子
		SendZoneMsgToFishpond(ms_stZoneMsg);
	}
}

//推送玩家座位信息
void CFishpondObj::SendSeatUserInfo(CGameRoleObj* pstToRole, CGameRoleObj* pstSeatRole)
{
	//pstToRole		推送的目标玩家，为空则推送给所有人
	//pstSeatRole	推送的玩家信息，如果为空则推送所有座位上玩家的信息

	CZoneMsgHelper::GenerateMsgHead(ms_stZoneMsg, MSGID_ZONE_SEATUSER_NOTIFY);
	Zone_SeatUser_Notify* pstNotify = ms_stZoneMsg.mutable_stbody()->mutable_m_stzone_seatuser_notify();

	if (pstSeatRole)
	{
		//推送pstSeatRole的信息
		SeatUserData* pstUserData = GetSeatUserByUin(pstSeatRole->GetUin());
		if (!pstUserData)
		{
			return;
		}

		SEATUSERINFO* pstOneInfo = pstNotify->add_stusers();
		pstOneInfo->set_iseat(pstUserData->iSeat);
		pstOneInfo->set_uin(pstSeatRole->GetUin());
		pstOneInfo->set_igunid(pstSeatRole->GetWeaponInfo().iWeaponID);
		pstOneInfo->set_lcoins(pstSeatRole->GetResource(RESOURCE_TYPE_COIN));
		pstOneInfo->set_ltickets(pstSeatRole->GetResource(RESOURCE_TYPE_TICKET));
		pstOneInfo->set_strnickname(pstSeatRole->GetNickName());
		pstOneInfo->set_igunstyleid(pstSeatRole->GetWeaponInfo().iStyleID);
		pstOneInfo->set_strpicid(pstSeatRole->GetPicID());
		pstOneInfo->set_iviplevel(pstSeatRole->GetVIPLevel());
	}
	else
	{
		//推送桌子上所有人的信息
		for (unsigned i = 0; i < m_vSeatUserData.size(); ++i)
		{
			CGameRoleObj* pstRoleObj = CUnitUtility::GetRoleByUin(m_vSeatUserData[i].uiUin);
			if (!pstRoleObj)
			{
				//找不到玩家信息
				continue;
			}

			SEATUSERINFO* pstOneInfo = pstNotify->add_stusers();
			pstOneInfo->set_iseat(m_vSeatUserData[i].iSeat);
			pstOneInfo->set_uin(pstRoleObj->GetUin());
			pstOneInfo->set_igunid(pstRoleObj->GetWeaponInfo().iWeaponID);
			pstOneInfo->set_lcoins(pstRoleObj->GetResource(RESOURCE_TYPE_COIN));
			pstOneInfo->set_ltickets(pstRoleObj->GetResource(RESOURCE_TYPE_TICKET));
			pstOneInfo->set_strnickname(pstRoleObj->GetNickName());
			pstOneInfo->set_igunstyleid(pstRoleObj->GetWeaponInfo().iStyleID);
			pstOneInfo->set_strpicid(pstRoleObj->GetPicID());
			pstOneInfo->set_iviplevel(pstRoleObj->GetVIPLevel());
		}
	}

	if (pstToRole)
	{
		//发送给单个玩家
		CZoneMsgHelper::SendZoneMsgToRole(ms_stZoneMsg, pstToRole);
	}
	else
	{
		//发送给桌子上所有玩家
		SendZoneMsgToFishpond(ms_stZoneMsg);
	}

	return;
}

//推送鱼信息
void CFishpondObj::SendFishInfoToUser(CGameRoleObj* pstTargetRole, const std::vector<FishData>& vFishes)
{
	if (vFishes.size() == 0)
	{
		return;
	}

	CZoneMsgHelper::GenerateMsgHead(ms_stZoneMsg, MSGID_ZONE_FISHINFO_NOTIFY);
	Zone_FishInfo_Notify* pstNotify = ms_stZoneMsg.mutable_stbody()->mutable_m_stzone_fishinfo_notify();

	for (unsigned i = 0; i < vFishes.size(); ++i)
	{
		FISHINFO* pstFishInfo = pstNotify->add_stfishes();
		pstFishInfo->set_uuniqid(vFishes[i].uUniqueID);
		pstFishInfo->set_ifishid(vFishes[i].iFishID);
		pstFishInfo->set_ifishseqid(vFishes[i].iFishSeqID);
		pstFishInfo->set_itraceid(vFishes[i].iTraceID);
		pstFishInfo->set_lbortime(vFishes[i].lBornTime);
		pstFishInfo->set_ldeadtime(FishAlgorithm::astData[m_pstRoomConfig->iAlgorithmType].lTotalReturnSilver);
		pstFishInfo->set_cindex(vFishes[i].cIndex);
	}

	if (pstTargetRole)
	{
		//发送给个人
		CZoneMsgHelper::SendZoneMsgToRole(ms_stZoneMsg, pstTargetRole);
	}
	else
	{
		//发送给桌子
		SendZoneMsgToFishpond(ms_stZoneMsg);
	}

	return;
}

//推送桌子上子弹信息给玩家
void CFishpondObj::SendBulletInfoToUser(CGameRoleObj& stRoleObj)
{
	if (m_vBulletData.size() == 0)
	{
		//没有子弹
		return;
	}

	//获取玩家座位信息
	SeatUserData* pstUserData = GetSeatUserByUin(stRoleObj.GetUin());
	if (!pstUserData)
	{
		return;
	}

	CZoneMsgHelper::GenerateMsgHead(ms_stZoneMsg, MSGID_ZONE_BULLETINFO_NOTIFY);
	Zone_BulletInfo_Notify* pstNotify = ms_stZoneMsg.mutable_stbody()->mutable_m_stzone_bulletinfo_notify();

	for (unsigned i = 0; i < m_vBulletData.size(); ++i)
	{
		if (m_vBulletData[i].iSeat == pstUserData->iSeat)
		{
			//不发送自己的
			continue;
		}

		BULLETINFO* pstOneInfo = pstNotify->add_stbullets();
		pstOneInfo->set_uuniqid(m_vBulletData[i].uUniqueID);
		pstOneInfo->set_igunid(m_vBulletData[i].iGunID);
		pstOneInfo->set_iseat(m_vBulletData[i].iSeat);
		pstOneInfo->mutable_sttargetpos()->set_ix(m_vBulletData[i].stTargetPos.iX);
		pstOneInfo->mutable_sttargetpos()->set_iy(m_vBulletData[i].stTargetPos.iY);
		pstOneInfo->set_lshoottime(m_vBulletData[i].lShootTime);
		pstOneInfo->set_ufishuniqid(m_vBulletData[i].uFishUniqID);
		pstOneInfo->set_bisaimformfish(m_vBulletData[i].bIsAimFormFish);
		pstOneInfo->set_ifishindex(m_vBulletData[i].iFishIndex);
	}

	CZoneMsgHelper::SendZoneMsgToRole(ms_stZoneMsg, &stRoleObj);

	return;
}

//推送桌子信息给玩家
void CFishpondObj::SendFishpondInfoToUser(CGameRoleObj& stRoleObj)
{
	//推送桌子上所有人给玩家
	SendSeatUserInfo(&stRoleObj, NULL);

	//推送桌子上所有鱼给玩家
	SendFishInfoToUser(&stRoleObj, m_vFishData);

	//推送桌子上子弹信息给玩家
	SendBulletInfoToUser(stRoleObj);

	return;
}

//推送鱼阵详细信息
void CFishpondObj::SendFishFormInfo(CGameRoleObj* pstRoleObj)
{
	if (m_stFishFormData.vFishOutData.size() == 0)
	{
		return;
	}

	CZoneMsgHelper::GenerateMsgHead(ms_stZoneMsg, MSGID_ZONE_FORMINFO_NOTIFY);
	Zone_FormInfo_Notify* pstNotify = ms_stZoneMsg.mutable_stbody()->mutable_m_stzone_forminfo_notify();
	for (unsigned i = 0; i < m_stFishFormData.vFishOutData.size(); ++i)
	{
		FORMFISHOUTINFO* pstFormInfo = pstNotify->add_stforminfos();
		pstFormInfo->set_ioutid(m_stFishFormData.vFishOutData[i].iOutID);
		pstFormInfo->set_ifishid(m_stFishFormData.vFishOutData[i].iFishID);
		pstFormInfo->set_ifishseqid(m_stFishFormData.vFishOutData[i].iFishSeqID);
		pstFormInfo->set_itraceid(m_stFishFormData.vFishOutData[i].iTraceID);
		pstFormInfo->set_lbegintime(m_stFishFormData.vFishOutData[i].lBeginTime);
		pstFormInfo->set_iremainum(m_stFishFormData.vFishOutData[i].iRemainNum);
		pstFormInfo->set_iinterval(m_stFishFormData.vFishOutData[i].iInterval);

		for (int i = 0; i < 4; ++i)
		{
			pstFormInfo->add_szalivefish(m_stFishFormData.vFishOutData[i].szAliveFish[i]);
		}
	}

	if (pstRoleObj)
	{
		//发送给玩家
		CZoneMsgHelper::SendZoneMsgToRole(ms_stZoneMsg, pstRoleObj);
	}
	else
	{
		//发送给桌子
		SendZoneMsgToFishpond(ms_stZoneMsg);
	}

	return;
}

//推送退出椅子消息
void CFishpondObj::SendExitFishpondAll(int iSeat, bool bForceExit)
{
	CZoneMsgHelper::GenerateMsgHead(ms_stZoneMsg, MSGID_ZONE_EXITFISH_NOTIFY);

	Zone_ExitFish_Notify* pstNotify = ms_stZoneMsg.mutable_stbody()->mutable_m_stzone_exitfish_notify();
	pstNotify->set_iseat(iSeat);
	pstNotify->set_bforceexit(bForceExit);

	SendZoneMsgToFishpond(ms_stZoneMsg);

	return;
}

//推送炮台切换的通知
void CFishpondObj::SendChangeGunNotify(SeatUserData& stUserData, int iNewGunID, bool bIsStyle)
{
	CZoneMsgHelper::GenerateMsgHead(ms_stZoneMsg, MSGID_ZONE_CHANGEGUN_NOTIFY);

	Zone_ChangeGun_Notify* pstNotify = ms_stZoneMsg.mutable_stbody()->mutable_m_stzone_changegun_notify();
	pstNotify->set_iseat(stUserData.iSeat);
	pstNotify->set_inewgunid(iNewGunID);
	pstNotify->set_bisstyle(bIsStyle);

	SendZoneMsgToFishpond(ms_stZoneMsg);

	return;
}

//推送子弹消息
void CFishpondObj::SendShootBulletNotify(BulletData& stBulletInfo)
{
	CZoneMsgHelper::GenerateMsgHead(ms_stZoneMsg, MSGID_ZONE_SHOOTBULLET_NOTIFY);

	Zone_ShootBullet_Notify* pstNotify = ms_stZoneMsg.mutable_stbody()->mutable_m_stzone_shootbullet_notify();
	pstNotify->set_uuniqid(stBulletInfo.uUniqueID);
	pstNotify->set_igunid(stBulletInfo.iGunID);
	pstNotify->set_iseat(stBulletInfo.iSeat);
	pstNotify->mutable_sttargetpos()->set_ix(stBulletInfo.stTargetPos.iX);
	pstNotify->mutable_sttargetpos()->set_iy(stBulletInfo.stTargetPos.iY);
	pstNotify->set_lshoottime(stBulletInfo.lShootTime);
	pstNotify->set_ufishuniqid(stBulletInfo.uFishUniqID);
	pstNotify->set_bisaimformfish(stBulletInfo.bIsAimFormFish);
	pstNotify->set_ifishindex(stBulletInfo.iFishIndex);
	pstNotify->set_biswildbullet((stBulletInfo.iWildNum != 0));

	SendZoneMsgToFishpond(ms_stZoneMsg);

	return;
}

//推送命中鱼的消息
void CFishpondObj::SendHitFishInfoNotify(SeatUserData& stUserData, unsigned uBulletUniqID, unsigned uFishUniqID, int iRewardSilver,
	bool bIsForm, int iCost, int iFishIndex, int iMultipleFish)
{
	CZoneMsgHelper::GenerateMsgHead(ms_stZoneMsg, MSGID_ZONE_HITFISH_NOTIFY);

	Zone_HitFish_Notify* pstNotify = ms_stZoneMsg.mutable_stbody()->mutable_m_stzone_hitfish_notify();
	pstNotify->set_iseat(stUserData.iSeat);
	pstNotify->set_ubulletuniqid(uBulletUniqID);
	pstNotify->set_ufishuniqid(uFishUniqID);
	pstNotify->set_irewardcoins(iRewardSilver);
	pstNotify->set_bisform(bIsForm);
	pstNotify->set_ifishindex(iFishIndex);
	pstNotify->set_imultifish(iMultipleFish);
	pstNotify->set_ibulletlivetime(stUserData.lUnEffectTime > CTimeUtility::GetMSTime());

	SendZoneMsgToFishpond(ms_stZoneMsg);
}

//推送爆炸死的鱼信息
void CFishpondObj::SendBoomFishInfoNotify(SeatUserData& stUserData, const std::vector<unsigned>& vFishUniqIDs, const std::vector<BYTE>& vSmallFishs,
	const std::vector<unsigned>& vFormUniqIDs)
{
	if (vFishUniqIDs.empty() && vFormUniqIDs.empty())
	{
		return;
	}

	/*
	if (vFishUniqIDs.size() != vSmallFishs.size())
	{
		return;
	}
	*/

	CZoneMsgHelper::GenerateMsgHead(ms_stZoneMsg, MSGID_ZONE_BOOMFISHINFO_NOTIFY);

	Zone_BoomFishInfo_Notify* pstNotify = ms_stZoneMsg.mutable_stbody()->mutable_m_stzone_boomfishinfo_notify();
	pstNotify->set_iseat(stUserData.iSeat);

	for (unsigned i = 0; i < vFishUniqIDs.size(); ++i)
	{
		pstNotify->add_ufishuniqids(vFishUniqIDs[i]);
		//pstNotify->add_usmallfishindex(vSmallFishs[i]);
	}

	for (unsigned i = 0; i < vFormUniqIDs.size(); ++i)
	{
		pstNotify->add_uformuniqids(vFormUniqIDs[i]);
	}

	SendZoneMsgToFishpond(ms_stZoneMsg);

	return;
}

//推送获得点券的消息
void CFishpondObj::SendAddTicketNotify(unsigned uiUin, unsigned uFishUniqID, bool bIsFormFish, long8 lRandTicketNum)
{
	CZoneMsgHelper::GenerateMsgHead(ms_stZoneMsg, MSGID_ZONE_ADDTICKET_NOTIFY);

	Zone_AddTicket_Notify* pstNotify = ms_stZoneMsg.mutable_stbody()->mutable_m_stzone_addticket_notify();
	pstNotify->set_uiuin(uiUin);
	pstNotify->set_ufishuniqid(uFishUniqID);
	pstNotify->set_bisformfish(bIsFormFish);
	pstNotify->set_iaddnum(lRandTicketNum);

	SendZoneMsgToFishpond(ms_stZoneMsg);

	return;
}

//发送消息给桌子上所有人
void CFishpondObj::SendZoneMsgToFishpond(GameProtocolMsg& stMsg)
{
	TRoleObjList stRoleList;
	stRoleList.m_iRoleNumber = m_vSeatUserData.size();
	for (int i = 0; i < stRoleList.m_iRoleNumber; ++i)
	{
		stRoleList.m_apstRoleObj[i] = CUnitUtility::GetRoleByUin(m_vSeatUserData[i].uiUin);
	}

	//发送消息
	CZoneMsgHelper::SendZoneMsgToRoleList(stMsg, stRoleList);

	return;
}

//发送走马灯
void CFishpondObj::SendFishHorseLamp(CGameRoleObj& stRoleObj, int iLampID, int iParam1, int iParam2, int iParam3)
{
	std::vector<HorseLampData> vDatas;

	HorseLampData stOneData;
	stOneData.iLampID = iLampID;

	char szParam[64] = { 0 };

	SAFE_SPRINTF(szParam, sizeof(szParam) - 1, "%s", stRoleObj.GetNickName());
	stOneData.vParams.push_back(szParam);

	SAFE_SPRINTF(szParam, sizeof(szParam) - 1, "%d", iParam1);
	stOneData.vParams.push_back(szParam);

	SAFE_SPRINTF(szParam, sizeof(szParam) - 1, "%d", iParam2);
	stOneData.vParams.push_back(szParam);

	SAFE_SPRINTF(szParam, sizeof(szParam) - 1, "%d", iParam3);
	stOneData.vParams.push_back(szParam);

	vDatas.push_back(stOneData);

	CChatUtility::SendHorseLamp(vDatas);

	return;
}

//重置鱼池
void CFishpondObj::Reset()
{
	m_pPrev = NULL;
	m_pNext = NULL;

	//鱼池ID
	m_uTableID = 0;

	//鱼池所在房间ID
	m_iFishRoomID = 0;

	//鱼池配置
	m_pstRoomConfig = NULL;

	//座位信息
	m_vSeatUserData.clear();

	//鱼池出鱼规则
	m_vOutFishRule.clear();

	//鱼池中的子弹
	m_vBulletData.clear();

	//鱼阵信息
	m_stFishFormData.Reset();

	//鱼池出鱼上限信息
	m_mFishLimitData.clear();

	//鱼池中鱼的信息
	m_vFishData.clear();

	m_bStopOutFish = false;

	m_lLastTickTime = 0;

	return;
}
