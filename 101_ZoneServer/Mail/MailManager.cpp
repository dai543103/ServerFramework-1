
#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "ErrorNumDef.hpp"
#include "Random.hpp"
#include "Kernel/ConfigManager.hpp"
#include "Kernel/GameRole.hpp"
#include "Kernel/UnitUtility.hpp"
#include "Kernel/ModuleHelper.hpp"
#include "Kernel/ZoneOssLog.hpp"
#include "Reward/RewardUtility.h"
#include "RepThings/RepThingsUtility.hpp"
#include "MailUtility.h"

#include "MailManager.h"

using namespace ServerLib;

//服务器系统邮件唯一ID
unsigned CMailManager::m_uServerSystemMailUniqID = 0;

CMailManager::CMailManager()
{
	m_uiUin = 0;

	m_uRoleSystemUniqID = 0;
	m_uPersonalUniqID = 0;

	m_vMails.clear();
}

CMailManager::~CMailManager()
{
	m_uiUin = 0;

	m_uRoleSystemUniqID = 0;
	m_uPersonalUniqID = 0;

	m_vMails.clear();
}

//初始化
int CMailManager::Initialize()
{
	return 0;
}

void CMailManager::SetOwner(unsigned int uin)
{
	m_uiUin = uin;
}

CGameRoleObj* CMailManager::GetOwner()
{
	return CUnitUtility::GetRoleByUin(m_uiUin);
}

//新增邮件
void CMailManager::AddNewMail(const OneMailInfo& stMailInfo, bool bSendNotify)
{
	std::vector<unsigned> vDelUniqIDs;
	if (m_vMails.size() >= (unsigned)MAX_ROLE_MAIL_NUM)
	{
		//邮箱已满，先删除老的
		for (unsigned i = 0; i < (m_vMails.size() - MAX_ROLE_MAIL_NUM + 1); ++i)
		{
			vDelUniqIDs.push_back(m_vMails[i].uUniqID);
		}

		m_vMails.erase(m_vMails.begin(), m_vMails.begin()+(m_vMails.size()-MAX_ROLE_MAIL_NUM+1));
	}

	//增加新邮件
	MailData stOneMail;
	stOneMail.uUniqID = ++m_uPersonalUniqID;
	stOneMail.iMailID = stMailInfo.imailid();
	stOneMail.iSendTime = stMailInfo.isendtime();
	stOneMail.iMailStat = MAIL_STAT_NOTREAD;
	stOneMail.strTitle = stMailInfo.strtitle();

	for (int i = 0; i < stMailInfo.strparams_size(); ++i)
	{
		stOneMail.vParams.push_back(stMailInfo.strparams(i));
	}

	RewardConfig stOneReward;
	for (int i = 0; i < stMailInfo.stappendixs_size(); ++i)
	{
		stOneReward.iType = stMailInfo.stappendixs(i).itype();
		stOneReward.iRewardID = stMailInfo.stappendixs(i).iid();
		stOneReward.iRewardNum = stMailInfo.stappendixs(i).inum();

		stOneMail.vAppendixes.push_back(stOneReward);
	}

	m_vMails.push_back(stOneMail);

	//打印运营日志 玩家收到邮件
	CGameRoleObj *stRoleObj = GetOwner();

	//判断是否有附件
	if (stOneMail.vAppendixes.size() == 0)
	{
		//没有附件
		CZoneOssLog::TraceMail(stRoleObj->GetUin(), stRoleObj->GetChannel(), stRoleObj->GetNickName(), stOneMail.uUniqID, stOneMail.strTitle.c_str(), 0, 0, 0, stOneMail.iMailID, stOneMail.iSendTime);
	}
	else
	{
		//有附件
		for (unsigned i = 0; i < stOneMail.vAppendixes.size(); ++i)
		{
			CZoneOssLog::TraceMail(stRoleObj->GetUin(), stRoleObj->GetChannel(), stRoleObj->GetNickName(), stOneMail.uUniqID, stOneMail.strTitle.c_str(),
				stOneMail.vAppendixes[i].iType, stOneMail.vAppendixes[i].iRewardID, stOneMail.vAppendixes[i].iRewardNum, stOneMail.iMailID, stOneMail.iSendTime);
		}
	}

	if (bSendNotify)
	{
		//推送消息
		SendAddNewMailNotify(stOneMail, vDelUniqIDs);
	}

	return;
}

//操作邮件
int CMailManager::OperateMail(int iOperaType, unsigned uUniqID)
{
	if (iOperaType <= MAIL_OPERA_INVALID || iOperaType >= MAIL_OPERA_MAX)
	{
		LOGERROR("Failed to opera mail, invalid opera type %d, uin %u\n", iOperaType, m_uiUin);
		return T_ZONE_PARA_ERROR;
	}

	int iMailIndex = -1;
	if (uUniqID != 0)
	{
		//获取邮件信息
		iMailIndex = GetMailIndexByUniqID(uUniqID);
		if (iMailIndex < 0)
		{
			LOGERROR("Failed to opera mail, invalid mail uniqid %u, uin %u\n", uUniqID, m_uiUin);
			return T_ZONE_PARA_ERROR;
		}
	}

	int iRet = T_SERVER_SUCCESS;

	//操作邮件
	switch (iOperaType)
	{
	case MAIL_OPERA_READ:
	{
		//阅读邮件
		m_vMails[iMailIndex].iMailStat = MAIL_STAT_READ;
	}
	break;

	case MAIL_OPERA_RECIEVE:
	{
		//收取附件
		if (m_vMails[iMailIndex].iMailStat == MAIL_STAT_GETAPPENDIX || m_vMails[iMailIndex].vAppendixes.size() == 0)
		{
			LOGERROR("Failed to get mail appendix, no items, uin %u, mail uniq id %u\n", m_uiUin, m_vMails[iMailIndex].uUniqID);
			return T_ZONE_PARA_ERROR;
		}

		iRet = GetMailAppendix(m_vMails[iMailIndex]);
		if (iRet)
		{
			LOGERROR("Failed to get mail appendix, mail id %d, ret %d, uin %u\n", m_vMails[iMailIndex].iMailID, iRet, m_uiUin);
			return iRet;
		}

		//标记为已领取
		m_vMails[iMailIndex].iMailStat = MAIL_STAT_GETAPPENDIX;
	}
	break;

	case MAIL_OPERA_DELETE:
	{
		//删除邮件
		m_vMails.erase(m_vMails.begin()+iMailIndex);
	}
	break;

	case MAIL_OPERA_ONERECIEVE:
	{
		//一键收取附件
		for (unsigned i = 0; i < m_vMails.size(); ++i)
		{
			if (m_vMails[i].iMailStat == MAIL_STAT_GETAPPENDIX || m_vMails[i].vAppendixes.size() == 0)
			{
				//没有附件
				continue;
			}

			//领取附件
			iRet = GetMailAppendix(m_vMails[i]);
			if (iRet)
			{
				LOGERROR("Failed to get mail appendix, mail id %d, ret %d, uin %u\n", m_vMails[i].iMailID, iRet, m_uiUin);
				return iRet;
			}

			//标记为已领取
			m_vMails[i].iMailStat = MAIL_STAT_GETAPPENDIX;
		}
	}
	break;

	case MAIL_OPERA_ONEDELETE:
	{
		//一键删除，不删除带附件的
		std::vector<MailData> vMails;
		for (unsigned i = 0; i < m_vMails.size(); ++i)
		{
			if (m_vMails[i].iMailID == MAIL_SEND_CARDNO)
			{
				//卡密邮件不能一键删除
				continue;
			}

			if (m_vMails[i].iMailStat != MAIL_STAT_GETAPPENDIX && m_vMails[i].vAppendixes.size() != 0)
			{
				//还有附件未领取
				vMails.push_back(m_vMails[i]);
			}
		}

		m_vMails = vMails;
	}
	break;

	default:
		break;
	}

	return T_SERVER_SUCCESS;
}

//服务器系统邮件唯一ID
unsigned CMailManager::GetServerMailUniqID()
{
	return m_uServerSystemMailUniqID;
}

void CMailManager::SetServerMailUniqID(unsigned uUniqID)
{
	m_uServerSystemMailUniqID = uUniqID;
}

//玩家系统邮件唯一ID
unsigned CMailManager::GetRoleSystemUniqID()
{
	return m_uRoleSystemUniqID;
}

void CMailManager::SetRoleSystemMailUniqID(unsigned uUniqID)
{
	m_uRoleSystemUniqID = uUniqID;
}

//定时器
void CMailManager::OnTick(int iTimeNow)
{
	if (m_uServerSystemMailUniqID == 0 || m_uRoleSystemUniqID != m_uServerSystemMailUniqID)
	{
		//拉取系统邮件
		CMailUtility::GetSystemMail(*GetOwner(), MAIL_REASON_NORMAL, m_uRoleSystemUniqID);
	}

	return;
}

//更新邮件到DB
void CMailManager::UpdateMailToDB(MAILDBINFO& stMailDBInfo)
{
	//系统邮件唯一ID
	stMailDBInfo.set_usystemmaxid(m_uRoleSystemUniqID);

	//个人邮件唯一ID
	stMailDBInfo.set_upersonalmaxid(m_uPersonalUniqID);

	//邮件详细信息
	for (unsigned i = 0; i < m_vMails.size(); ++i)
	{
		OneMailInfo* pstMailInfo = stMailDBInfo.add_stmails();
		pstMailInfo->set_uuniqid(m_vMails[i].uUniqID);
		pstMailInfo->set_imailid(m_vMails[i].iMailID);
		pstMailInfo->set_isendtime(m_vMails[i].iSendTime);
		pstMailInfo->set_imailstat(m_vMails[i].iMailStat);
		pstMailInfo->set_strtitle(m_vMails[i].strTitle);

		for (unsigned j = 0; j < m_vMails[i].vParams.size(); ++j)
		{
			pstMailInfo->add_strparams(m_vMails[i].vParams[j]);
		}

		for (unsigned j = 0; j < m_vMails[i].vAppendixes.size(); ++j)
		{
			MailAppendix* pstOneAppendix = pstMailInfo->add_stappendixs();
			pstOneAppendix->set_itype(m_vMails[i].vAppendixes[j].iType);
			pstOneAppendix->set_iid(m_vMails[i].vAppendixes[j].iRewardID);
			pstOneAppendix->set_inum(m_vMails[i].vAppendixes[j].iRewardNum);
		}
	}

	return;
}

//从DB初始化邮件
void CMailManager::InitMailFromDB(const MAILDBINFO& stMailDBInfo)
{
	//系统邮件唯一ID
	m_uRoleSystemUniqID = stMailDBInfo.usystemmaxid();

	//个人邮件唯一ID
	m_uPersonalUniqID = stMailDBInfo.upersonalmaxid();

	//邮件详细信息
	m_vMails.clear();
	MailData stOneMail;
	for (int i = 0; i < stMailDBInfo.stmails_size(); ++i)
	{
		stOneMail.Reset();

		stOneMail.uUniqID = stMailDBInfo.stmails(i).uuniqid();
		stOneMail.iMailID = stMailDBInfo.stmails(i).imailid();
		stOneMail.iSendTime = stMailDBInfo.stmails(i).isendtime();
		stOneMail.iMailStat = stMailDBInfo.stmails(i).imailstat();
		stOneMail.strTitle = stMailDBInfo.stmails(i).strtitle();

		for (int j = 0; j < stMailDBInfo.stmails(i).strparams_size(); ++j)
		{
			stOneMail.vParams.push_back(stMailDBInfo.stmails(i).strparams(j));
		}

		RewardConfig stOneReward;
		for (int j = 0; j < stMailDBInfo.stmails(i).stappendixs_size(); ++j)
		{
			stOneReward.iType = stMailDBInfo.stmails(i).stappendixs(j).itype();
			stOneReward.iRewardID = stMailDBInfo.stmails(i).stappendixs(j).iid();
			stOneReward.iRewardNum = stMailDBInfo.stmails(i).stappendixs(j).inum();

			stOneMail.vAppendixes.push_back(stOneReward);
		}

		m_vMails.push_back(stOneMail);
	}

	return;
}

//获取邮件位置，不存在返回-1
int CMailManager::GetMailIndexByUniqID(unsigned uUniqID)
{
	for (unsigned i = 0; i < m_vMails.size(); ++i)
	{
		if (m_vMails[i].uUniqID == uUniqID)
		{
			return i;
		}
	}

	return -1;
}

//推送新邮件消息
void CMailManager::SendAddNewMailNotify(const MailData& stOneMail, const std::vector<unsigned>& vDelUniqIDs)
{
	static GameProtocolMsg stMsg;
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_NEWMAILINFO_NOTIFY);
	Zone_NewMailInfo_Notify* pstNotify = stMsg.mutable_stbody()->mutable_m_stzone_newmailinfo_notify();

	for (unsigned i = 0; i < vDelUniqIDs.size(); ++i)
	{
		pstNotify->add_udeluniqids(vDelUniqIDs[i]);
	}

	OneMailInfo* pstMailInfo = pstNotify->mutable_stmailinfo();
	pstMailInfo->set_uuniqid(stOneMail.uUniqID);
	pstMailInfo->set_imailid(stOneMail.iMailID);
	pstMailInfo->set_isendtime(stOneMail.iSendTime);
	pstMailInfo->set_imailstat(stOneMail.iMailStat);
	pstMailInfo->set_strtitle(stOneMail.strTitle);

	for (unsigned i = 0; i < stOneMail.vParams.size(); ++i)
	{
		pstMailInfo->add_strparams(stOneMail.vParams[i]);
	}

	for (unsigned i = 0; i < stOneMail.vAppendixes.size(); ++i)
	{
		MailAppendix* pstOneAppendix = pstMailInfo->add_stappendixs();
		pstOneAppendix->set_itype(stOneMail.vAppendixes[i].iType);
		pstOneAppendix->set_iid(stOneMail.vAppendixes[i].iRewardID);
		pstOneAppendix->set_inum(stOneMail.vAppendixes[i].iRewardNum);
	}

	CZoneMsgHelper::SendZoneMsgToRole(stMsg, GetOwner());
	
	return;
}

//收取邮件附件
int CMailManager::GetMailAppendix(MailData& stOneMail)
{
	//拼装附件
	int iAppendixNum = stOneMail.vAppendixes.size();
	RewardConfig astRewards[MAX_APPENDIX_NUM];
	for (int i = 0; i < iAppendixNum; ++i)
	{
		if (i >= MAX_APPENDIX_NUM)
		{
			break;
		}

		astRewards[i].iType = stOneMail.vAppendixes[i].iType;
		astRewards[i].iRewardID = stOneMail.vAppendixes[i].iRewardID;
		astRewards[i].iRewardNum = stOneMail.vAppendixes[i].iRewardNum;
	}
	
	//领取附件奖励
	int iRet = T_SERVER_SUCCESS;
	if (stOneMail.iMailID == MAIL_SEND_MONTHCARD)
	{
		//月卡礼包，开宝箱
		iRet = CRepThingsUtility::OpenBoxGift(*GetOwner(), astRewards[0].iRewardID);
		if (iRet)
		{
			LOGERROR("Failed to get month card gift, ret %d, uin %u\n", iRet, m_uiUin);
			return iRet;
		}
	}
	else
	{
		//其他邮件直接领取
		int iItemChannel = (stOneMail.iMailID == MAIL_SEND_ITEMGIFT) ? ITEM_CHANNEL_ROLEMAIL : ITEM_CHANNEL_SYSMAIL;	//赠送是个人邮件
		iRet = CRewardUtility::GetReward(*GetOwner(), 1, astRewards, iAppendixNum, iItemChannel);
		if (iRet)
		{
			LOGERROR("Failed to get mail appendix, ret %d, uin %u, mail id %d\n", iRet, m_uiUin, stOneMail.iMailID);
			return iRet;
		}

		//打印运营日志  玩家从邮件领取奖励
		for (int i = 0; i < iAppendixNum; ++i)
		{
			//获取玩家道具数量
			int iNewNum = GetOwner()->GetRepThingsManager().GetRepItemNum(astRewards[i].iRewardID);

			CZoneOssLog::TraceGetRewardFromMail(GetOwner()->GetUin(), GetOwner()->GetChannel(), GetOwner()->GetNickName(),
				astRewards[i].iRewardID, astRewards[i].iRewardNum, iNewNum - astRewards[i].iRewardNum, iNewNum);
		}
	}

	return T_SERVER_SUCCESS;
}
