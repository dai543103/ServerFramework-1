#pragma once

#include "GameProtocol.hpp"
#include "BaseConfigManager.hpp"
#include "Kernel/GameRole.hpp"

//邮件工具类
class CMailUtility
{
public:

	//赠送道具邮件
	static int SendGiftMail(CGameRoleObj& stRoleObj, unsigned uiToUin, int iItemID, int iItemNum);

	//背包满邮件
	static int SendRepFullMail(CGameRoleObj& stRoleObj, int iItemID, int iItemNum);

	//贵族礼包邮件
	static int SendVIPGiftMail(CGameRoleObj& stRoleObj, const RewardConfig& stReward);

	//月卡礼包邮件
	static int SendMonthCardMail(CGameRoleObj& stRoleObj, int iMonthBoxID, int iBoxNum);

	//兑换卡密邮件
	static int SendCardNoMail(CGameRoleObj& stRoleObj, int iExchangeID, const std::string& strCardNo, const std::string& strCardPwd);

	//系统邮件
	static int SendSystemMail(CGameRoleObj& stRoleObj, const World_GetSystemMail_Response& stResp);

	//发送邮件
	static int SendPersonalMail(const World_SendMail_Request& stReq, bool bSendNotify=true);

	//拉取系统邮件
	static void GetSystemMail(CGameRoleObj& stRoleObj, int iReason, unsigned uRoleSystemUniqID);
};
