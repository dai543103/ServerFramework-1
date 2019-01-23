#include <time.h>

#include "GameProtocol.hpp"
#include "TimeUtility.hpp"
#include "Kernel/ZoneOssLog.hpp"
#include "Kernel/GameRole.hpp"
#include "Kernel/ZoneMsgHelper.hpp"

#include "StringUtility.hpp"
#include "LogAdapter.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////
//运营OSS日志接口

#define GAME_OSSLOG_CONFIG_FILE "../conf/OssLogConfig.xml"

//魔方日志参数
static const int MOFANG_FISHGAME_AREAID = 404;
static const int MOFANG_FISHGAME_GAMEID = 20171120;

using namespace ServerLib;

//玩家登录的日志
void CZoneOssLog::TraceLogin(CGameRoleObj& stRoleObj)
{
	CGameSessionObj* pstSessionObj = CModuleHelper::GetSessionManager()->FindSessionByRoleID(stRoleObj.GetRoleID());
	if(!pstSessionObj)
	{
		return;
	}

	//判断是否是当天首次登陆
	int iIsFirstLogin = 1;
	int iLastLoginTime = stRoleObj.GetLastLoginTime();
	int iLoginTime = stRoleObj.GetLoginTime();
	if (CTimeUtility::IsInSameDay(iLastLoginTime, iLoginTime))
	{
		iIsFirstLogin = 0;
	}

    //| uin | OSS_LOG_TYPE_LOGIN | time | channel | LoginTime | LoginIP | VipLv | isfirsttime | createtime 
    TRACEBILL("|%u|%d|%d|%s|%d|%s|%d|%d|%d\n", stRoleObj.GetUin(), OSS_LOG_TYPE_LOGIN, CTimeUtility::GetNowTime(), stRoleObj.GetChannel(),
              stRoleObj.GetLoginTime(), pstSessionObj->GetClientIP(), stRoleObj.GetVIPLevel(), iIsFirstLogin, stRoleObj.GetCreateTime());

	return;
}

//玩家登出的日志
void CZoneOssLog::TraceLogout(CGameRoleObj& stRoleObj)
{
	CGameSessionObj* pstSessionObj = CModuleHelper::GetSessionManager()->FindSessionByRoleID(stRoleObj.GetRoleID());
	if(!pstSessionObj)
	{
		return;
	}

	//| uin | OSS_LOG_TYPE_LOGOUT | time | channel | LogoutTime | LoginIP | LoginTime | VipLevel | OnlineTime
	TRACEBILL("|%u|%d|%d|%s|%d|%s|%d|%d|%d\n", stRoleObj.GetUin(), OSS_LOG_TYPE_LOGOUT, CTimeUtility::GetNowTime(), stRoleObj.GetChannel(),
		stRoleObj.GetLogoutTime(), pstSessionObj->GetClientIP(), stRoleObj.GetLoginTime(), stRoleObj.GetVIPLevel(),
		stRoleObj.GetOnlineTotalTime());

	return;
}

//玩家体验线日志
void CZoneOssLog::TraceExpLine(CGameRoleObj& stRoleObj, long8 iWinNum, int iExpLine, int iShootNum, long8 lCostNum)
{
	//| uin | OSS_LOG_TYPE_EXPLINE | time | channel | winnum | expline | shootnum | costnum
	TRACEBILL("|%u|%d|%d|%s|%lld|%d|%d|%lld\n", stRoleObj.GetUin(), OSS_LOG_TYPE_EXPLINE, CTimeUtility::GetNowTime(), stRoleObj.GetChannel(),
		iWinNum, iExpLine, iShootNum, lCostNum);

	return;
}

//玩家命中鱼概率日志
void CZoneOssLog::TraceHitFish(unsigned uin, const char* szChannelID, int iGunID, int iFishID, unsigned uFishUniqueID, bool bIsForm, long8 lTotalReturnSilver,
	int iX, int iRedPacketPercent, int iServerX, int iUserX, int iExpLineX, int iReturnType)
{
	//| uin | OSS_LOG_TYPE_HITFISH | time | channel | gunid | fishid | fishuniqid | isform | returnsilver | X | redX | serverX | userX | ExpLineX | returnType
	TRACEBILL("|%u|%d|%d|%s|%d|%d|%u|%d|%lld|%d|%d|%d|%d|%d|%d\n", uin, OSS_LOG_TYPE_HITFISH, CTimeUtility::GetNowTime(), szChannelID, iGunID, iFishID, uFishUniqueID,
		bIsForm, lTotalReturnSilver, iX, iRedPacketPercent, iServerX, iUserX, iExpLineX, iReturnType);

	return;
}

//玩家打爆鱼日志（个人捕鱼）
void CZoneOssLog::TraceCatchFish(unsigned uin, const char* szChannelID, const char* szNickName, int iGunMultiple, int iFishType, int iResType, int iRoomID,
	long8 lOldResNum, long8 lNewResNum)
{
	//| uin | OSS_LOG_TYPE_HITFISH | time | channel | nickname | gunmultiple | fishtype | restype | roomid | oldNum | newNum |
	TRACEBILL("|%u|%d|%d|%s|%s|%d|%d|%d|%d|%lld|%lld\n", uin, OSS_LOG_TYPE_CATCHFISH, CTimeUtility::GetNowTime(), szChannelID, szNickName,
		iGunMultiple, iFishType, iResType, iRoomID, lOldResNum, lNewResNum);

	return;
}

//玩家打爆弹头鱼（个人捕鱼）
void CZoneOssLog::TraceCatchWarHeadFish(unsigned uin, const char* szChannelID, const char* szNickName, int iGunMultiple, int iFishType, int iItemType, int iItemID, int iRoomID,
	long8 lOldItemNum, long8 lNewItemNum)
{
	//| uin | OSS_LOG_TYPE_HITFISH | time | channel | nickname | gunmultiple | fishtype | restype | roomid | oldNum | newNum |
	TRACEBILL("|%u|%d|%d|%s|%s|%d|%d|%d|%d|%d|%lld|%lld\n", uin, OSS_LOG_TYPE_CATCHWARHEADFISH, CTimeUtility::GetNowTime(), szChannelID, szNickName, 
		iGunMultiple, iFishType, iItemType, iItemID, iRoomID, lOldItemNum, lNewItemNum);

	return;
}

//玩家充值日志
void CZoneOssLog::TraceRecharge(unsigned uin, const char* szChannelID, const char* szNickName, int iRechargeOption, int iAddCoin, long8 lOldNum, long8 lNewNum, int iTotalOnlineTime, int iVipLv)
{
	//| uin | OSS_LOG_TYPE_RECHARGE | time | channel | nickname | rechargeoption | addCoinNum | oldCoinNum | newCoinNum | iTotalOnlineTime | iVipLv
	TRACEBILL("|%u|%d|%d|%s|%s|%d|%d|%lld|%lld|%d|%d\n", uin, OSS_LOG_TYPE_RECHARGE, CTimeUtility::GetNowTime(), szChannelID, szNickName, iRechargeOption,
		iAddCoin, lOldNum, lNewNum, iTotalOnlineTime, iVipLv);

	return;
}

//玩家VIP抽奖日志， 海盗宝藏
void CZoneOssLog::TraceVipLottery(unsigned uin, const char* szChannelID, const char* szNickName, int iCostNum, int iRewardResType, int iRewardID, long8 lOldNum, long8 lNewNum)
{
	//| uin | OSS_TYPE_VIPLOTTERY | time | channel | nickname | costnum | RewardType | RewardID | lOldNum | lNewNum |
	TRACEBILL("|%u|%d|%d|%s|%s|%d|%d|%d|%lld|%lld\n", uin, OSS_LOG_TYPE_VIPLOTTERY, CTimeUtility::GetNowTime(), szChannelID, szNickName, iCostNum,
		iRewardResType, iRewardID, lOldNum, lNewNum);

	return;
}

//玩家赠送日志
void CZoneOssLog::TracePresent(unsigned uinGive, unsigned uinRecepte, int iItemID, int iItemNum)
{
	//| uingive | OSS_LOG_TYPE_PRESENT | time | uinrecepte | itemid | itemnum
	TRACEBILL("|%u|%d|%d||%u||%d|%d\n", uinGive, OSS_LOG_TYPE_PRESENT, CTimeUtility::GetNowTime(),
		uinRecepte, iItemID, iItemNum);

	return;
}

//玩家使用弹头日志
void CZoneOssLog::TraceUseWarHead(unsigned uin, const char* szChannelID, const char* szNickName, int iWarHeadID, int iAddCoinNum, int iRoomID, long8 lOldWarHeadNum,
	long8 lNewWarHeadNum, long8 lOldCoinNum, long8 lNewCoinNum)
{
	//| uin | OSS_LOG_TYPE_USEWARHEAD | time | channel | nickname | ItemID | AddCoin | OldlWarHead | newWarHead | oldCoin | newCoin | roomid |
	TRACEBILL("|%u|%d|%d|%s|%s|%d|%d|%lld|%lld|%lld|%lld|%d\n", uin, OSS_LOG_TYPE_USEWARHEAD, CTimeUtility::GetNowTime(), szChannelID, szNickName, iWarHeadID,
		iAddCoinNum, lOldWarHeadNum, lNewWarHeadNum, lOldCoinNum, lNewCoinNum, iRoomID);

	return;
}

//玩家捕鱼王中王活动日志
void CZoneOssLog::TracePointRace(unsigned uin, const char* szNickName, long8 lBounsPoints, int iRewardItemType, int iRewardID, long8 lRewardItemNum)
{
	//| uin | OSS_LOG_TYPE_POINTRACE | time | nickname | BounsPoints | RewardType | RewardID | ItemNum |
	TRACEBILL("|%u|%d|%d|%s|%lld|%d|%d|%lld\n", uin, OSS_LOG_TYPE_POINTRACE, CTimeUtility::GetNowTime(), szNickName, lBounsPoints, 
		iRewardItemType, iRewardID, lRewardItemNum);

	return;
}

//玩家限量抽奖日志
void CZoneOssLog::TraceLimitLottery(unsigned uin, const char* szChannelID, const char* szNickName, int iLimitLotteryType, int iLimitLotteryID, int iRewardResType, int iRewardID, long8 lOldNum, long8 lNewNum)
{
	//| uin | OSS_LOG_TYPE_RECHARGELOTTERY | time | channel | nickname | LotteryType | LotteryID | RewardType | RewardID | oldNum | NewNum
	TRACEBILL("|%u|%d|%d|%s|%s|%d|%d|%d|%d|%lld|%lld\n", uin, OSS_LOG_TYPE_RECHARGELOTTERY, CTimeUtility::GetNowTime(), szChannelID, szNickName,
		iLimitLotteryType, iLimitLotteryID, iRewardResType, iRewardID, lOldNum, lNewNum);

	return;
}

//玩家兑换日志
void CZoneOssLog::TraceExchange(unsigned uin, const char* szChannelID, const char* szNickName, int iExchangeID, long8 lCostTicketNum, long8 lOldNum, long8 lNewNum, int iRewardType, int iRewardID, long8 lOldRewardNnum, long8 lNewRewardNum)
{
	//| uin | OSS_LOG_TYPE_EXCHANGE | time | channel | nickname | ExchangeId | CostNum | oldticketnum | newticketnum | RewardType | RewardID | lOldRewardNum | lNewRewardNum
	TRACEBILL("|%u|%d|%d|%s|%s|%d|%d|%lld|%lld|%d|%d|%lld|%lld\n", uin, OSS_LOG_TYPE_EXCHANGE, CTimeUtility::GetNowTime(), szChannelID, szNickName, iExchangeID, lCostTicketNum,
		lOldNum, lNewNum, iRewardType, iRewardID, lOldRewardNnum, lNewRewardNum);

	return;
}

//玩家道具使用日志
void CZoneOssLog::TraceUseItem(unsigned uin, const char* szChannelID, const char* szNickName, int iItemID, long8 lOldNum, long8 lNewNum, int iRewardID, long8 lOldRewardNum, long8 lNewRewardNum)
{
	//| uin | OSS_LOG_TYPE_USEITEM | time |channel | nickname | ItemID | oldnum | newnum | rewardID | rewardoldnum | rewardnewnum |
	TRACEBILL("|%u|%d|%d|%s|%s|%d|%lld|%lld|%d|%lld|%lld\n", uin, OSS_LOG_TYPE_USEITEM, CTimeUtility::GetNowTime(), szChannelID, szNickName,
		iItemID, lOldNum, lNewNum, iRewardID, lOldRewardNum, lNewRewardNum);

	return;
}

//玩家完成任务日志
void CZoneOssLog::TraceQuest(unsigned uin, const char* szChannelID, const char * szNickName, int iQuestID, int iQuestType,int iRewardType, int iRewardID, long8 lOldNum, long8 lNewNum)
{
	//| uin | OSS_LOG_TYPE_QUEST | time | channel | nickname | questid | questtype | rewardType | rewardid | oldnum | newnum |
	TRACEBILL("|%u|%d|%d|%s|%s|%d|%d|%d|%d|%lld|%lld\n", uin, OSS_LOG_TYPE_QUEST, CTimeUtility::GetNowTime(), szChannelID, szNickName,
		iQuestID, iQuestType, iRewardType, iRewardID, lOldNum, lNewNum);

	return;
}

//玩家新手起航礼包日志
void CZoneOssLog::TraceLoginReward(unsigned uin, const char* szChannelID, const char * szNickName, int iLoginRewardID, int iday, long8 lOldNum, long8 lNewNum)
{
	//| uin | OSS_LOG_TYPE_LOGINREWARD | time | channel | nickname | RewardId | day | oldnum | newnum |
	TRACEBILL("|%u|%d|%d|%s|%s|%d|%d|%lld|%lld\n", uin, OSS_LOG_TYPE_LOGINREWARD, CTimeUtility::GetNowTime(), szChannelID, szNickName,
		iLoginRewardID, iday, lOldNum, lNewNum);

	return;
}

//玩家拉斯维加斯转盘日志
void CZoneOssLog::TraceLasvegas(unsigned uin, const char* szChannelID, const char * szNickName, int iBetType, int iBetNum, int iLotteryNum, int iAddCoin, long8 lOldNum, long8 lNewNum)
{
	//| uin | OSS_LOG_TYPE_LASVEGAS | time | channel | nickanme | bettype | betnum | lotterynum | addcoin | oldcoin | newCoin |
	TRACEBILL("|%u|%d|%d|%s|%s|%d|%d|%d|%d|%lld|%lld\n", uin, OSS_LOG_TYPE_LASVEGAS, CTimeUtility::GetNowTime(), szChannelID, szNickName,
		iBetType, iBetNum, iLotteryNum, iAddCoin, lOldNum, lNewNum);

	return;
}

//玩家拉斯维加斯转盘下注日志
void CZoneOssLog::TraceLasvegasBet(unsigned uin, const char* szChannelID, const char * szNickName, int iBetType, int iBetNum)
{
	//| uin | OSS_LOG_TYPE_PLAYERBET | time | channel | nickname | bettype | betnum |
	TRACEBILL("|%u|%d|%d|%s|%s|%d|%d\n", uin, OSS_LOG_TYPE_PLAYERBET, CTimeUtility::GetNowTime(), szChannelID, szNickName, iBetType, iBetNum);

	return;
}

//玩家活跃度宝箱日志
void CZoneOssLog::TraceLiveness(unsigned uin, const char* szChannelID, const char * szNickName, int iBoxID, int iRewardType, int iRewardID, long8 lOldNum, long8 lNewNum)
{
	//| uin | OSS_LOG_TYPE_LIVENESS | channel | time | nickname | BoxID | RewardType | RewardID | oldnum | newnum |
	TRACEBILL("|%u|%d|%d|%s|%s|%d|%d|%d|%lld|%lld\n", uin, OSS_LOG_TYPE_LIVENESS, CTimeUtility::GetNowTime(), szChannelID, szNickName,
		iBoxID, iRewardType, iRewardID, lOldNum, lNewNum);

	return;
}

//玩家邮箱日志
void CZoneOssLog::TraceMail(unsigned uin, const char* szChannelID, const char* szNickName, unsigned uUinqID, const char* szTitle, int iRewardType, int iRewardID, int iRewardNum, int iMailID, int iSendTime)
{
	//| uin | OSS_LOG_TYPE_MAIL | time | channel | nickname | MailUniqID | title | RewardType | RewardID | RewardNum  | mailid | sendtime
	TRACEBILL("|%u|%d|%d|%s|%s|%u|%s|%d|%d|%d|%d|%d\n", uin, OSS_LOG_TYPE_MAIL, CTimeUtility::GetNowTime(), szChannelID, szNickName, uUinqID,
		szTitle, iRewardType, iRewardID, iRewardNum, iMailID, iSendTime);

	return;
}

//玩家救济金领取日志
void CZoneOssLog::TraceAlms(unsigned uin, const char* szChannelID, const char * szNickName, int iReciveTimes, int iAddCoin, long8 lOldNum, long8 lNewNum)
{
	//| uin | OSS_LOG_TYPE_ALMS | time | channel | nickname | revicetime | addnum | oldnum | newnum |
	TRACEBILL("|%u|%d|%d|%s|%s|%d|%d|%lld|%lld\n", uin, OSS_LOG_TYPE_ALMS, CTimeUtility::GetNowTime(), szChannelID, szNickName,
		iReciveTimes, iAddCoin, lOldNum, lNewNum);
	
	return;
}

//玩家技能使用日志
void CZoneOssLog::TraceUseSkill(unsigned uin, const char* szChannelID, const char * szNickName, int iSkillID, int iRoomID, long8 lOldNum, long8 lNewNum)
{
	//| uin | OSS_LOG_TYPE_USESKILL | channel | time | nickname | skillid | roomid | lOldnum | lNewNum |
	TRACEBILL("|%u|%d|%d|%s|%s|%d|%d|%lld|%lld\n", uin, OSS_LOG_TYPE_USESKILL, CTimeUtility::GetNowTime(), szChannelID, szNickName,
		iSkillID, iRoomID, lOldNum, lNewNum);

	return;
}

//玩家VIP登录领取日志
void CZoneOssLog::TraceVipReward(unsigned uin, const char* szChannelID, const char * szNickName, int iVipLv, long8 lOldNum, long8 lNewNum)
{
	//| uin | OSS_LOG_TYPE_VIPREWARD | time | channel | nickname | viplv | oldnum | newnum |
	TRACEBILL("|%u|%d|%d|%s|%s|%d|%lld|%lld\n", uin, OSS_LOG_TYPE_VIPREWARD, CTimeUtility::GetNowTime(), szChannelID, szNickName,
		iVipLv, lOldNum, lNewNum);

	return;
}

//玩家发射子弹
void CZoneOssLog::TraceShootBullet(unsigned uin, const char* szChannelID, const char* szNickName, int iConsume, int iRoomID)
{
	//| uin | OSS_LOG_TYPE_SHOOTBULLET | time | channel | nickname | GunMultiple |
	TRACEBILL("|%u|%d|%d|%s|%s|%d|%d\n", uin, OSS_LOG_TYPE_SHOOTBULET, CTimeUtility::GetNowTime(), szChannelID, szNickName,
		iConsume, iRoomID);

	return;
}

//玩家获得红包
void CZoneOssLog::TraceGetRedPacket(unsigned uin, const char* szNickName, int iRedPacketType)
{
	//| uin | OSS_LOG_TYPE_GETREDPACKET | time | channel | nickname | RedPacketType
	TRACEBILL("|%u|%d|%d||%s|%d\n", uin, OSS_LOG_TYPE_GETREDPACKET, CTimeUtility::GetNowTime(), szNickName, iRedPacketType);

	return;
}

//玩家周期结算日志
void CZoneOssLog::TraceCycleProfit(unsigned uin, const char* szChannelID, const char* szNickName, int iLossNum, long8 lNewUsedNum, long8 lCoinNum, long8 lTicketNum)
{
	//| uin | OSS_LOG_TYPE_CYCLEPROFIT | time | channel | nickname | LossNum | newUsedNum | coinnum | ticketnum
	TRACEBILL("|%u|%d|%d|%s|%s|%d|%lld|%lld|%lld\n", uin, OSS_LOG_TYPE_CYCLEPROFIT, CTimeUtility::GetNowTime(), szChannelID, szNickName, iLossNum, lNewUsedNum, lCoinNum, lTicketNum);

	return;
}

//服务器周期结算
void CZoneOssLog::TraceServerCycleProfit(int iAlgorithmType, long8 lRewardSilver, long8 lCostSilver, long8 lServerWinNum, 
	long8 lTicketFishDropNum, int iPump, long8 lPlayingNum, int iAverageGunMultiple, long8 lTotalReturnSilver, long8 lTicketReturnNum,
	long8 lNextReturnTime, int iReturnPlayerNum, long8 lUsedReturnSilver)
{
	//| none | OSS_LOG_TYPE_SERVERCYCLEPROFIT | time | none | algorithm | Reward | cost | WinNum | TicketNum | pump | usernum | averageGunMultiple | returnsilver | ticketreturnsilver | NextRedPacketTime | ReturnPlayerNum | UsedReturnSilver
	TRACEBILL("||%d|%d||%d|%lld|%lld|%lld|%lld|%d|%lld|%d|%lld|%lld|%lld|%d|%lld\n", OSS_LOG_TYPE_SERVERCYCLEPROFIT, CTimeUtility::GetNowTime(), 
		iAlgorithmType, lRewardSilver, lCostSilver, lServerWinNum, lTicketFishDropNum, iPump, lPlayingNum, iAverageGunMultiple, lTotalReturnSilver, 
		lTicketReturnNum, lNextReturnTime, iReturnPlayerNum, lUsedReturnSilver);

	return;
}

//记录弹头鱼流水
void CZoneOssLog::TraceShootCostByFishType(unsigned uin, const char* szChannelID, const char* szNickName, int iRoomID, int iBulletCost, int iFishType)
{
	//| uin | OSS_LOG_TYPE_FISHCOSTBYTYPE | time | Channel | nickname | roomid | Gunmultiple | fishid |
	TRACEBILL("|%u|%d|%d|%s|%s|%d|%d|%d\n", uin, OSS_LOG_TYPE_FISHCOSTBYTYPE, CTimeUtility::GetNowTime(), szChannelID,
		szNickName, iRoomID, iBulletCost, iFishType);

	return;
}

//记录完成新手引导
void CZoneOssLog::TraceFinNewGuide(unsigned uin, const char* szChannelID, const char* szNickName, int iGuideID)
{
	// | uin | OSS_LOG_TYPE_FINNEWGUIDE | time | Channel | nickname | iGuideID
	TRACEBILL("|%u|%d|%d|%s|%s|%d\n", uin, OSS_LOG_TYPE_FINNEWGUIDE, CTimeUtility::GetNowTime(), szChannelID, szNickName, iGuideID);

	return;
}

//玩家从邮件领取奖励
void CZoneOssLog::TraceGetRewardFromMail(unsigned uin, const char* szChannelID, const char* szNickName, int iRewardID, int iRewardNum, int iOldNum, int iNewNum)
{
	// | uin | OSS_LOG_TYPE_GETREWARDFROMMAIL | time | Channel | nickname | RewardID | RewardNum | oldnum | newnum
	TRACEBILL("|%u|%d|%d|%s|%s|%d|%d|%d|%d\n", uin, OSS_LOG_TYPE_GETREWARDFROMMAIL, CTimeUtility::GetNowTime(), szChannelID,
		szNickName, iRewardID, iRewardNum, iOldNum, iNewNum);

	return;
}

//记录玩家实物兑换
void CZoneOssLog::TraceEntityExchange(unsigned uin, int iExchangeID, const std::string& strOrderID, int iTime, const std::string& strName, const std::string& strPhone,
	const std::string& strMailNum, const std::string& strAddress, const std::string& strRemarks, const std::string& strQQNum)
{
	//| uin | OSS_LOG_TYPE_ENTITYEXCHANGE | time | exchangeid | orderid | name | phone | mailnum | address | remarks | qqnum
	TRACEBILL("|%u|%d|%d|%d|%s|%s|%s|%s|%s|%s|%s\n", uin, OSS_LOG_TYPE_ENTITYEXCHANGE, iTime, iExchangeID, strOrderID.c_str(), strName.c_str(), strPhone.c_str(), 
		strMailNum.c_str(), strAddress.c_str(), strRemarks.c_str(), strQQNum.c_str());

	return;
}

//玩家使用话费道具兑换话费
void CZoneOssLog::TraceExchangeBill(unsigned uin, const char* szChannelID, const char* szNickName, int iItemID, int iItemNum, int iExchangeNum,
	const char* szPhoneNum, int iOldNum, int iNewNum)
{
	// | uin |  | time | Channel | nickname | ItemID | ItemNum | ExchangeNum | PhoneNum | oldnum | newnum
	TRACEBILL("|%u|%d|%d|%s|%s|%d|%d|%d|%s|%d|%d\n", uin, OSS_LOG_TYPE_EXCHANGEBILL, CTimeUtility::GetNowTime(), szChannelID,
		szNickName, iItemID, iItemNum, iExchangeNum, szPhoneNum, iOldNum, iNewNum);

	return;
}

//登录上报日志
void CZoneOssLog::ReportUserLogin(unsigned uin, const char* szAccount, const char* szDeviceID, const char* szChannelID, bool bIsNew, const char* szClientIP)
{
	char szReportBuff[256] = { 0 };

	//日志格式: REPORT_LOG_MOFANGLOGIN | areaid,gameid,numid,username,deviceid,channelid,is_new,clientip, clienttype(2)
	SAFE_SPRINTF(szReportBuff, sizeof(szReportBuff)-1, "%d|%d,%d,%u,'%s','%s','%s',%d,'%s',%d", REPORT_LOG_MOFANGLOGIN, MOFANG_FISHGAME_AREAID, MOFANG_FISHGAME_GAMEID,
		uin, szAccount, szDeviceID, szChannelID, bIsNew, szClientIP, 2);

	static GameProtocolMsg stMsg;
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_WORLD_WRITELOG_REQUEST);

	World_WriteLog_Request* pstReq = stMsg.mutable_stbody()->mutable_m_stworld_writelog_request();
	pstReq->set_ilogtargettype(LOG_TARGET_MOFANG);
	pstReq->set_strlogdata(szReportBuff);

	CZoneMsgHelper::SendZoneMsgToWorld(stMsg);

	return;
}

//充值上报日志
void CZoneOssLog::ReportUserPay(unsigned uin, const char* szAccount, const char* szDeviceID, const char* szChannelID, const char* szOrderID, int iRMB, int iItemNum, const char* szClientIP)
{
	char szReportBuff[256] = { 0 };

	//日志格式: REPORT_LOG_MOFANGPAY | areaid,gameid,numid,username,deviceid,channelid,orderid,amount,virtualcurrency,clientip, clienttype(2)
	SAFE_SPRINTF(szReportBuff, sizeof(szReportBuff) - 1, "%d|%d,%d,%u,'%s','%s','%s','%s',%d,%d,'%s',%d", REPORT_LOG_MOFANGPAY, MOFANG_FISHGAME_AREAID, MOFANG_FISHGAME_GAMEID,
		uin, szAccount, szDeviceID, szChannelID, szOrderID, iRMB, iItemNum, szClientIP, 2);

	static GameProtocolMsg stMsg;
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_WORLD_WRITELOG_REQUEST);

	World_WriteLog_Request* pstReq = stMsg.mutable_stbody()->mutable_m_stworld_writelog_request();
	pstReq->set_ilogtargettype(LOG_TARGET_MOFANG);
	pstReq->set_strlogdata(szReportBuff);

	CZoneMsgHelper::SendZoneMsgToWorld(stMsg);

	return;
}

//周期结算上报日志
void CZoneOssLog::ReportTallyData(CGameRoleObj& stRoleObj)
{
	char szReportBuff[512] = { 0 };
	TROLETALLYINFO& stTallyInfo = stRoleObj.GetTallyInfo();

	if (!stTallyInfo.bNeedLog)
	{
		//不需要上报日志
		return;
	}

	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();
	CRepThingsManager& stRepThingsMgr = stRoleObj.GetRepThingsManager();

	//日志格式: REPORT_LOG_TALLYINFO|numid,deviceid,channelid,coin,ticket,cash,bronze,silver,gold,bindbronze,bindsilver,bindgold,addcoin,
	//addticket,addcash,addwin,totalwin
	SAFE_SPRINTF(szReportBuff, sizeof(szReportBuff) - 1, "%d|%u,'%s','%s',%lld,%lld,%lld,%d,%d,%d,%d,%d,%d,%lld,%lld,%lld,%lld,%lld",
		REPORT_LOG_TALLYINFO, stRoleObj.GetUin(), stRoleObj.GetDeviceID(), stRoleObj.GetChannel(),
		stRoleObj.GetResource(RESOURCE_TYPE_COIN), stRoleObj.GetResource(RESOURCE_TYPE_TICKET), stRoleObj.GetResource(RESOURCE_TYPE_DIAMOND),
		stRepThingsMgr.GetRepItemNum(stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_NORMALBRONZE)), stRepThingsMgr.GetRepItemNum(stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_NORMALSILVER)),
		stRepThingsMgr.GetRepItemNum(stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_NORMALGOLD)), stRepThingsMgr.GetRepItemNum(stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_BINDBRONZE)),
		stRepThingsMgr.GetRepItemNum(stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_BINDSILVER)), stRepThingsMgr.GetRepItemNum(stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_BINDGOLD)),
		stRoleObj.GetResource(RESOURCE_TYPE_COIN)- stTallyInfo.alResource[RESOURCE_TYPE_COIN], stRoleObj.GetResource(RESOURCE_TYPE_TICKET) - stTallyInfo.alResource[RESOURCE_TYPE_TICKET],
		stRoleObj.GetResource(RESOURCE_TYPE_DIAMOND) - stTallyInfo.alResource[RESOURCE_TYPE_DIAMOND], stTallyInfo.lUserWinNum, stRoleObj.GetUserWinNum());

	static GameProtocolMsg stMsg;
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_WORLD_WRITELOG_REQUEST);

	World_WriteLog_Request* pstReq = stMsg.mutable_stbody()->mutable_m_stworld_writelog_request();
	pstReq->set_ilogtargettype(LOG_TARGET_MOFANG);
	pstReq->set_strlogdata(szReportBuff);

	CZoneMsgHelper::SendZoneMsgToWorld(stMsg);

	return;
}
