#ifndef __ZONE_OSS_LOG_HPP__
#define __ZONE_OSS_LOG_HPP__

///////////////////////////////////////////////////////////////////////////////////// 
#include "LogAdapter.hpp"
#include "CommDefine.h"
#include "Kernel/ModuleHelper.hpp"

using namespace ServerLib;

class CGameRoleObj;

//OSS运营系统日志工具类

//输出运营日志的类型
enum TOssLogType
{
    OSS_LOG_TYPE_INVALID			= 0,				//非法的日志操作类型
    OSS_LOG_TYPE_LOGIN				= 10001,			//玩家登录帐号
    OSS_LOG_TYPE_LOGOUT				= 10002,			//玩家登出帐号
	OSS_LOG_TYPE_EXPLINE			= 10003,			//玩家体验线
	OSS_LOG_TYPE_HITFISH			= 10004,			//玩家命中鱼
	OSS_LOG_TYPE_CATCHFISH			= 10005,			//玩家打爆鱼
	OSS_LOG_TYPE_CATCHWARHEADFISH	= 10006,			//玩家打爆弹头鱼
	OSS_LOG_TYPE_RECHARGE			= 10007,			//玩家充值记录
	OSS_LOG_TYPE_VIPLOTTERY			= 10008,			//玩家VIP抽奖 海盗宝藏
	OSS_LOG_TYPE_PRESENT			= 10009,			//玩家赠送
	OSS_LOG_TYPE_USEWARHEAD			= 10010,			//玩家使用弹头
	OSS_LOG_TYPE_POINTRACE			= 10011,			//捕鱼王中王积分赛活动
	OSS_LOG_TYPE_RECHARGELOTTERY	= 10012,			//充值抽奖
	OSS_LOG_TYPE_EXCHANGE			= 10013,			//兑换日志
	OSS_LOG_TYPE_USEITEM			= 10014,			//道具使用日志
	OSS_LOG_TYPE_QUEST				= 10015,			//任务日志
	OSS_LOG_TYPE_LOGINREWARD		= 10016,			//登录奖励日志
	OSS_LOG_TYPE_LASVEGAS			= 10017,			//拉斯维加斯玩家赢钱
	OSS_LOG_TYPE_LIVENESS			= 10018,			//活跃度宝箱日志
	OSS_LOG_TYPE_MAIL				= 10019,			//邮箱日志
	OSS_LOG_TYPE_ALMS				= 10020,			//救纪金日志
	OSS_LOG_TYPE_USESKILL			= 10021,			//技能使用日志
	OSS_LOG_TYPE_VIPREWARD			= 10022,			//VIP金币赠送日志
	OSS_LOG_TYPE_SHOOTBULET			= 10023,			//玩家发射子弹
	OSS_LOG_TYPE_GETREDPACKET		= 10024,			//玩家获得红包
	OSS_LOG_TYPE_CYCLEPROFIT		= 10025,			//玩家周期结算
	OSS_LOG_TYPE_SERVERCYCLEPROFIT	= 10026,			//服务器周期盈利结算日志
	OSS_LOG_TYPE_PLAYERBET			= 10027,			//玩家拉斯维加斯转盘下注日志
	OSS_LOG_TYPE_FISHCOSTBYTYPE		= 10028,			//各类鱼子弹消耗
	OSS_LOG_TYPE_FINNEWGUIDE		= 10029,			//玩家完成新手任务
	OSS_LOG_TYPE_GETREWARDFROMMAIL	= 10030,			//玩家从邮件领取奖励
	OSS_LOG_TYPE_EXCHANGEBILL		= 10031,			//玩家使用话费道具兑换话费
	OSS_LOG_TYPE_ENTITYEXCHANGE		= 10032,			//玩家实物兑换
};

class CZoneOssLog
{
public:
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    //Zone日志打印接口

    //记录登录相关日志
    static void TraceLogin(CGameRoleObj& stRoleObj);
    static void TraceLogout(CGameRoleObj& stRoleObj);

	//玩家体验线日志
	static void TraceExpLine(CGameRoleObj& stRoleObj, long8 iWinNum, int iExpLine, int iShootNum, long8 lCostNum);

	//玩家命中鱼概率日志
	static void TraceHitFish(unsigned uin, const char* szChannelID, int iGunID, int iFishID, unsigned uFishUniqueID, bool bIsForm,
		long8 lTotalReturnSilver, int iX, int iRedPacketPercent, int iServerX, int iUserX, int iExpLineX, int iReturnType);

	//玩家打爆鱼日志（个人捕鱼）
	static void TraceCatchFish(unsigned uin, const char* szChannelID, const char* szNickName, int iGunMultiple, int iFishType, int iResType, int iRoomID,
		long8 lOldResNum, long8 lNewResNum);

	//玩家打爆弹头鱼（个人捕鱼）
	static void TraceCatchWarHeadFish(unsigned uin, const char* szChannelID, const char* szNickName, int iGunMultiple, int iFishType, int iItemType, int iItemID, int iRoomID,
		long8 lOldItemNum, long8 lNewItemNum);

	//玩家充值日志
	static void TraceRecharge(unsigned uin, const char* szChannelID, const char* szNickName, int iRechargeOption, int iAddCoin, long8 lOldNum, long8 lNewNum, int iTotalOnlineTime, int iVipLv);

	//玩家VIP抽奖，海盗宝藏日志
	static void TraceVipLottery(unsigned uin, const char* szChannelID, const char* szNickName, int iCostNum, int iRewardResType, int iRewardID, long8 lOldNum, long8 lNewNum);

	//玩家赠送日志
	static void TracePresent(unsigned uinGive, unsigned uinRecepte, int iItemID, int iItemNum);

	//玩家弹头使用日志
	static void TraceUseWarHead(unsigned uin, const char* szChannelID, const char* szNickName, int iWarHeadID, int iAddCoinNum, int iRoomID, long8 lOldWarHeadNum,
		long8 lNewWarHeadNum, long8 lOldCoinNum, long8 lNewCoinNum);

	//玩家捕鱼王中王活动日志
	static void TracePointRace(unsigned uin, const char* szNickName, long8 lBounsPoints, int iRewardItemType, int iRewardID, long8 iRewardItemNum);

	//玩家限量抽奖日志
	static void TraceLimitLottery(unsigned uin, const char* szChannelID, const char* szNickName, int iLimitLotteryType, int iLimitLotteryID, int iRewardType, int iRewardID, long8 lOldNum, long8 lNewNum);

	//玩家兑换日志
	static void TraceExchange(unsigned uin, const char* szChannelID, const char* szNickName, int iExchangeOption, long8 lCostTicketNum, long8 lOldNum, long8 lNewNum, int iRewardType, int iRewardID, long8 lOldRewardNnum, long8 lNewRewardNum);

	//玩家道具使用日志
	static void TraceUseItem(unsigned uin, const char* szChannelID, const char* szNickName, int iItemID, long8 lOldNum, long8 lNewNum, int iRewardID, long8 lOldRewardNum, long8 lNewRewardNum);

	//玩家完成任务日志
	static void TraceQuest(unsigned uin, const char* szChannelID, const char* szNickName, int iQuestID, int iQuestType, int iRewardType, int iRewardID, long8 lOldNum, long8 lNewNum);

	//玩家新手起航礼包日志
	static void TraceLoginReward(unsigned uin, const char* szChannelID, const char* szNickName, int iLoginRewardID, int iday, long8 lOldNum, long8 lNewNum);

	//玩家拉斯维加斯转盘 赢钱 日志
	static void TraceLasvegas(unsigned uin, const char* szChannelID, const char* szNickName, int iBetType, int iBetNum, int iLotteryNum, int iAddCoin, long8 lOldNum, long8 lNewNum);

	//玩家拉斯维加斯下注日志
	static void TraceLasvegasBet(unsigned uin, const char* szChannelID, const char* szNickName, int iBetType, int iBetNum);

	//玩家活跃度宝箱日志
	static void TraceLiveness(unsigned uin, const char* szChannelID, const char* szNickName, int iBoxID, int iRewardType, int iRewardID, long8 lOldNum, long8 lNewNum);

	//玩家邮箱日志
	static void TraceMail(unsigned uin, const char* szChannelID, const char* szNickName, unsigned uUinqID, const char* szTitle, int iRewardType, int iRewardID, int iRewardNum, int iMailID, int iSendTime);

	//玩家救济金领取日志
	static void TraceAlms(unsigned uin, const char* szChannelID, const char* szNickName, int iReciveTimes, int iAddCoin, long8 lOldNum, long8 lNewNum);

	//玩家技能使用日志
	static void TraceUseSkill(unsigned uin, const char* szChannelID, const char* szNickName, int iSkillID, int iRoomID, long8 lOldNum, long8 lNewNum);

	//玩家VIP登录领取日志
	static void TraceVipReward(unsigned uin, const char* szChannelID, const char* szNickName, int iVipLv, long8 lOldNum, long8 lNewNum);

	//玩家发射子弹
	static void TraceShootBullet(unsigned uin, const char* szChannelID, const char* szNickName, int iConsume, int iRoomID);

	//玩家获得红包
	static void TraceGetRedPacket(unsigned uin, const char* szNickName, int iRedPacketType);

	//玩家周期结算日志
	static void TraceCycleProfit(unsigned uin, const char* szChannelID, const char* szNickName, int iLossNum, long8 lNewUsedNum, long8 lCoinNum, long8 lTicketNum);

	//服务器周期结算日志
	static void TraceServerCycleProfit(int iAlgorithmType, long8 lRewardSilver, long8 lCostSilver, long8 lServerWinNum,
		long8 lTicketFishDropNum, int iPump, long8 lPlayingNum, int iAverageGunMultiple, long8 lTotalReturnSilver, long8 lTicketReturnNum,
		long8 lNextReturnTime, int iReturnPlayerNum, long8 lUsedReturnSilver);

	//记录弹头鱼流水
	static void TraceShootCostByFishType(unsigned uin, const char* szChannelID, const char* szNickName, int iRoomID, int iGunMultiple, int iFishType);

	//记录完成新手引导
	static void TraceFinNewGuide(unsigned uin, const char* szChannelID, const char* szNickName, int iGuideID);

	//记录玩家从邮件领取奖励
	static void TraceGetRewardFromMail(unsigned uin, const char* szChannelID, const char* szNickName, int iReward, int iRewardNum, int iOldNum, int iNewNum);

	//记录玩家实物兑换
	static void TraceEntityExchange(unsigned uin, int iExchangeID, const std::string& strOrderID, int iTime, const std::string& strName, const std::string& strPhone, 
		const std::string& strMailNum, const std::string& strAddress, const std::string& strRemarks, const std::string& strQQNum);

	//记录玩家使用话费道具兑换话费
	static void TraceExchangeBill(unsigned uin, const char* szChannelID, const char* szNickName, int iItemID, int iItemNum, int iExchangeNum, const char* szPhoneNum, int iOldNum, int iNewNum);

	//登录上报日志
	static void ReportUserLogin(unsigned uin, const char* szAccount, const char* szDeviceID, const char* szChannelID, bool bIsNew, const char* szClientIP);

	//充值上报日志
	static void ReportUserPay(unsigned uin, const char* szAccount, const char* szDeviceID, const char* szChannelID, const char* szOrderID, int iRMB, int iItemNum, const char* szClientIP);

	//周期结算上报日志
	static void ReportTallyData(CGameRoleObj& stRoleObj);
};

#endif
