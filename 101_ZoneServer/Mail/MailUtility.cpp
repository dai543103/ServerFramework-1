
#include "GameProtocol.hpp"
#include "ErrorNumDef.hpp"
#include "LogAdapter.hpp"
#include "Kernel/UnitUtility.hpp"
#include "Kernel/ModuleHelper.hpp"
#include "Kernel/ZoneOssLog.hpp"

#include "MailUtility.h"

using namespace ServerLib;

//赠送道具邮件
int CMailUtility::SendGiftMail(CGameRoleObj& stRoleObj, unsigned uiToUin, int iItemID, int iItemNum)
{
	//读取邮件配置
	const MailConfig* pstConfig = CConfigManager::Instance()->GetBaseCfgMgr().GetMailConfig(MAIL_SEND_ITEMGIFT);
	if (!pstConfig)
	{
		LOGERROR("Failed to send item, invalid mail config %d\n", MAIL_SEND_ITEMGIFT);
		return T_ZONE_INVALID_CFG;
	}

	World_SendMail_Request stReq;

	//封装发送接收
	stReq.set_uitouin(uiToUin);
	stReq.set_uifromuin(stRoleObj.GetUin());

	//封装邮件内容
	OneMailInfo* pstMailInfo = stReq.mutable_stmailinfo();
	pstMailInfo->set_isendtime(CTimeUtility::GetNowTime());
	pstMailInfo->set_imailid(MAIL_SEND_ITEMGIFT);
	//pstMailInfo->set_strtitle(pstConfig->strTitle);
	
	//参数1发送者名字
	pstMailInfo->add_strparams(stRoleObj.GetNickName());

	//参数2发送者数字ID
	char szParam[32] = { 0 };
	SAFE_SPRINTF(szParam, sizeof(szParam)-1, "%u", stRoleObj.GetUin());
	pstMailInfo->add_strparams(szParam);

	//增加附件
	MailAppendix* pstAppendix = pstMailInfo->add_stappendixs();
	pstAppendix->set_itype(REWARD_TYPE_ITEM);
	pstAppendix->set_iid(iItemID);
	pstAppendix->set_inum(iItemNum);

	//发送邮件
	return SendPersonalMail(stReq);
}

//背包满邮件
int CMailUtility::SendRepFullMail(CGameRoleObj& stRoleObj, int iItemID, int iItemNum)
{
	//读取邮件配置
	const MailConfig* pstConfig = CConfigManager::Instance()->GetBaseCfgMgr().GetMailConfig(MAIL_SEND_REPFULL);
	if (!pstConfig)
	{
		LOGERROR("Failed to send item, invalid mail config %d\n", MAIL_SEND_REPFULL);
		return T_ZONE_INVALID_CFG;
	}

	World_SendMail_Request stReq;

	//封装发送接收
	stReq.set_uitouin(stRoleObj.GetUin());
	stReq.set_uifromuin(stRoleObj.GetUin());

	//封装邮件内容
	OneMailInfo* pstMailInfo = stReq.mutable_stmailinfo();
	pstMailInfo->set_isendtime(CTimeUtility::GetNowTime());
	pstMailInfo->set_imailid(MAIL_SEND_REPFULL);
	//pstMailInfo->set_strtitle(pstConfig->strTitle);

	//增加附件
	MailAppendix* pstAppendix = pstMailInfo->add_stappendixs();
	pstAppendix->set_itype(REWARD_TYPE_ITEM);
	pstAppendix->set_iid(iItemID);
	pstAppendix->set_inum(iItemNum);

	//发送邮件
	return SendPersonalMail(stReq);
}

//贵族礼包邮件
int CMailUtility::SendVIPGiftMail(CGameRoleObj& stRoleObj, const RewardConfig& stReward)
{
	//读取邮件配置
	const MailConfig* pstConfig = CConfigManager::Instance()->GetBaseCfgMgr().GetMailConfig(MAIL_SEND_VIPGIFT);
	if (!pstConfig)
	{
		LOGERROR("Failed to send item, invalid mail config %d\n", MAIL_SEND_VIPGIFT);
		return T_ZONE_INVALID_CFG;
	}

	World_SendMail_Request stReq;

	//封装发送接收
	stReq.set_uitouin(stRoleObj.GetUin());
	stReq.set_uifromuin(stRoleObj.GetUin());

	//封装邮件内容
	OneMailInfo* pstMailInfo = stReq.mutable_stmailinfo();
	pstMailInfo->set_isendtime(CTimeUtility::GetNowTime());
	pstMailInfo->set_imailid(MAIL_SEND_VIPGIFT);
	//pstMailInfo->set_strtitle(pstConfig->strTitle);

	//参数1 玩家贵族等级
	char szParam[32] = { 0 };
	SAFE_SPRINTF(szParam, sizeof(szParam) - 1, "%d", (stRoleObj.GetVIPLevel()-1));
	pstMailInfo->add_strparams(szParam);

	//增加附件
	MailAppendix* pstAppendix = pstMailInfo->add_stappendixs();
	pstAppendix->set_itype(stReward.iType);
	pstAppendix->set_iid(stReward.iRewardID);
	pstAppendix->set_inum(stReward.iRewardNum);

	//发送邮件
	return SendPersonalMail(stReq);
}

//月卡礼包邮件
int CMailUtility::SendMonthCardMail(CGameRoleObj& stRoleObj, int iMonthBoxID, int iBoxNum)
{
	//读取邮件配置
	const MailConfig* pstConfig = CConfigManager::Instance()->GetBaseCfgMgr().GetMailConfig(MAIL_SEND_MONTHCARD);
	if (!pstConfig)
	{
		LOGERROR("Failed to send item, invalid mail config %d\n", MAIL_SEND_MONTHCARD);
		return T_ZONE_INVALID_CFG;
	}

	World_SendMail_Request stReq;

	//封装发送接收
	stReq.set_uitouin(stRoleObj.GetUin());
	stReq.set_uifromuin(stRoleObj.GetUin());

	//封装邮件内容
	OneMailInfo* pstMailInfo = stReq.mutable_stmailinfo();
	pstMailInfo->set_isendtime(CTimeUtility::GetNowTime());
	pstMailInfo->set_imailid(MAIL_SEND_MONTHCARD);
	//pstMailInfo->set_strtitle(pstConfig->strTitle);

	//增加附件
	MailAppendix* pstAppendix = pstMailInfo->add_stappendixs();
	pstAppendix->set_itype(REWARD_TYPE_ITEM);
	pstAppendix->set_iid(iMonthBoxID);
	pstAppendix->set_inum(iBoxNum);

	//发送邮件,不推送通知
	return SendPersonalMail(stReq, false);
}

//兑换卡密邮件
int CMailUtility::SendCardNoMail(CGameRoleObj& stRoleObj, int iExchangeID, const std::string& strCardNo, const std::string& strCardPwd)
{
	//读取邮件配置
	const MailConfig* pstConfig = CConfigManager::Instance()->GetBaseCfgMgr().GetMailConfig(MAIL_SEND_CARDNO);
	if (!pstConfig)
	{
		LOGERROR("Failed to send card number mail, invalid mail config %d\n", MAIL_SEND_CARDNO);
		return T_ZONE_INVALID_CFG;
	}

	World_SendMail_Request stReq;

	//封装发送接收
	stReq.set_uitouin(stRoleObj.GetUin());
	stReq.set_uifromuin(stRoleObj.GetUin());

	//封装邮件内容
	OneMailInfo* pstMailInfo = stReq.mutable_stmailinfo();
	pstMailInfo->set_isendtime(CTimeUtility::GetNowTime());
	pstMailInfo->set_imailid(MAIL_SEND_CARDNO);
	//pstMailInfo->set_strtitle(pstConfig->strTitle);

	//参数1 玩家兑换ID
	char szParam[32] = { 0 };
	SAFE_SPRINTF(szParam, sizeof(szParam) - 1, "%d", iExchangeID);
	pstMailInfo->add_strparams(szParam);

	//参数2是卡密卡号
	pstMailInfo->add_strparams(strCardNo);
	
	//参数3是卡密密码
	pstMailInfo->add_strparams(strCardPwd);

	//发送邮件,推送通知
	return SendPersonalMail(stReq);
}

//系统邮件
int CMailUtility::SendSystemMail(CGameRoleObj& stRoleObj, const World_GetSystemMail_Response& stResp)
{
	World_SendMail_Request stReq;

	//封装发送接收
	stReq.set_uitouin(stRoleObj.GetUin());
	stReq.set_uifromuin(0);

	int iRet = T_SERVER_SUCCESS;
	for (int i = 0; i < stResp.stmails_size(); ++i)
	{
		//封装邮件内容
		stReq.mutable_stmailinfo()->CopyFrom(stResp.stmails(i));

		//发送邮件
		iRet = SendPersonalMail(stReq);
		if (iRet)
		{
			LOGERROR("Failed to send system mail, ret %d, uin %u, mail id %d\n", iRet, stRoleObj.GetUin(), stResp.stmails(i).imailid());
			return iRet;
		}
	}

	stRoleObj.GetMailManager().SetRoleSystemMailUniqID(stResp.usystemuniqid());

	return T_SERVER_SUCCESS;
}

//发送邮件
int CMailUtility::SendPersonalMail(const World_SendMail_Request& stReq, bool bSendNotify)
{
	static GameProtocolMsg stMsg;

	//是否在本Zone
	CGameRoleObj* pstRoleObj = CUnitUtility::GetRoleByUin(stReq.uitouin());
	if (pstRoleObj)
	{
		//在本Zone
		pstRoleObj->GetMailManager().AddNewMail(stReq.stmailinfo(), bSendNotify);
	}
	else
	{
		//不在本Zone,转发给World
		CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_WORLD_SENDMAIL_REQUEST);
		
		World_SendMail_Request* pstReq = stMsg.mutable_stbody()->mutable_m_stworld_sendmail_request();
		pstReq->CopyFrom(stReq);

		return CZoneMsgHelper::SendZoneMsgToWorld(stMsg);
	}

	return T_SERVER_SUCCESS;
}

//拉取系统邮件
void CMailUtility::GetSystemMail(CGameRoleObj& stRoleObj, int iReason, unsigned uRoleSystemUniqID)
{
	static GameProtocolMsg stSystemMailMsg;

	//拉取系统邮件
	CZoneMsgHelper::GenerateMsgHead(stSystemMailMsg, MSGID_WORLD_GETSYSTEMMAIL_REQUEST);

	World_GetSystemMail_Request* pstReq = stSystemMailMsg.mutable_stbody()->mutable_m_stworld_getsystemmail_request();
	pstReq->set_uin(stRoleObj.GetUin());
	pstReq->set_iviplevel(stRoleObj.GetVIPLevel());
	pstReq->set_strchannel(stRoleObj.GetChannel());
	pstReq->set_urolesystemuniqid(uRoleSystemUniqID);
	pstReq->set_ifromzoneid(CModuleHelper::GetZoneID());
	pstReq->set_ireason(iReason);

	CZoneMsgHelper::SendZoneMsgToWorld(stSystemMailMsg);

	return;
}
