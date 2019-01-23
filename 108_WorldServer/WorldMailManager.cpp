
#include <algorithm>
#include <fstream>

#include "json/json.h"
#include "GameProtocol.hpp"
#include "ErrorNumDef.hpp"
#include "LogAdapter.hpp"
#include "TimeUtility.hpp"
#include "WorldMsgHelper.hpp"
#include "ConfigManager.hpp"
#include "WorldRoleStatus.hpp"
#include "WorldObjectAllocatorW.hpp"

#include "WorldMailManager.hpp"

using namespace ServerLib;

const static char* WORLD_SYSTEM_MAIL_FILE = "../conf/SystemMail.dat";

CWorldMailManager* CWorldMailManager::Instance()
{
	static CWorldMailManager* pInstance = NULL;
	if (!pInstance)
	{
		pInstance = new CWorldMailManager;
	}

	return pInstance;
}

CWorldMailManager::CWorldMailManager()
{
	Reset();
}

CWorldMailManager::~CWorldMailManager()
{

}

//初始化Mail
void CWorldMailManager::Init()
{
	Reset();

	//加载系统邮件
	LoadMailInfo();
}

//发送GM邮件
int CWorldMailManager::SendGMMail(const GameMaster_Request& stReq)
{
	//参数至少2个，且为3*X+2的格式
	if (stReq.ioperatype() != GM_OPERA_SENDMAIL || stReq.strparams_size() < 2 || (stReq.strparams_size()-2)%3!=0)
	{
		return T_ZONE_PARA_ERROR;
	}

	World_SendMail_Request stSendMailReq;
	stSendMailReq.set_uitouin(stReq.utouin());
	OneMailInfo* pstMailInfo = stSendMailReq.mutable_stmailinfo();
	pstMailInfo->set_isendtime(CTimeUtility::GetNowTime());

	//参数1 Title 标题
	pstMailInfo->set_strtitle(stReq.strparams(0));

	//参数2 Content 内容
	pstMailInfo->add_strparams(stReq.strparams(1));

	//后续参数如果有为附件
	for (int i = 2; i < stReq.strparams_size(); i+=3)
	{
		if ((i + 3) > stReq.strparams_size())
		{
			//附件参数有误
			break;
		}

		if (pstMailInfo->stappendixs_size() >= MAX_APPENDIX_NUM)
		{
			//超过附件上限
			break;
		}

		MailAppendix* pstAppendix = pstMailInfo->add_stappendixs();
		pstAppendix->set_itype(atoi(stReq.strparams(i).c_str()));
		pstAppendix->set_iid(atoi(stReq.strparams(i+1).c_str()));
		pstAppendix->set_inum(atoi(stReq.strparams(i+2).c_str()));
	}

	if (pstMailInfo->stappendixs_size() == 0)
	{
		//GM普通邮件
		pstMailInfo->set_imailid(MAIL_SEND_GMNORMAL);
	}
	else
	{
		//GM附件邮件
		pstMailInfo->set_imailid(MAIL_SEND_GMAPPENDIX);
	}

	if (stReq.utouin() == 0)
	{
		//发送系统邮件
		SendSystemMail(stReq, stSendMailReq.stmailinfo());
	}
	else
	{
		//发送个人邮件
		int iRet = CWorldMailManager::SendPersonalMail(stSendMailReq);
		if (iRet)
		{
			LOGERROR("Failes to GM send mail, ret %d, from %u, to %u, mail id %d\n", iRet, stReq.ufromuin(), stReq.utouin(), pstMailInfo->imailid());
			return iRet;
		}
	}

	LOGDEBUG("Success to GM send mail, from %u, to %u, mail id %d\n", stReq.ufromuin(), stReq.utouin(), pstMailInfo->imailid());

	return T_SERVER_SUCCESS;
}

//发送个人邮件
int CWorldMailManager::SendPersonalMail(const World_SendMail_Request& stReq)
{
	static GameProtocolMsg stMailMsg;
	CWorldMsgHelper::GenerateMsgHead(stMailMsg, 0, MSGID_WORLD_SENDMAIL_REQUEST, stReq.uitouin());
	stMailMsg.mutable_stbody()->mutable_m_stworld_sendmail_request()->CopyFrom(stReq);

	CWorldRoleStatusWObj* pstRoleStatObj = WorldTypeK32<CWorldRoleStatusWObj>::GetByUin(stReq.uitouin());
	if (pstRoleStatObj)
	{
		//目标玩家在线，发送在线邮件
		return CWorldMsgHelper::SendWorldMsgToWGS(stMailMsg, pstRoleStatObj->GetZoneID());
	}
	else
	{
		//目标玩家不在线，发送离线邮件
		return CWorldMsgHelper::SendWorldMsgToRoleDB(stMailMsg);
	}

	return T_SERVER_SUCCESS;
}

//拉取系统邮件
void CWorldMailManager::GetSystemMail(unsigned uin, int iReason, unsigned uUniqID, int iVIPLevel, const std::string& strChannel, World_GetSystemMail_Response& stResp)
{
	stResp.set_usystemuniqid(m_uSystemUniqID);

	if (iReason==MAIL_REASON_NORMAL && uUniqID >= m_uSystemUniqID)
	{
		//普通拉取并且没有新邮件
		return;
	}

	int iTimeNow = CTimeUtility::GetNowTime();
	for (unsigned i = 0; i < m_vSystemMail.size(); ++i)
	{
		SystemMailData& stOne = m_vSystemMail[i];

		//是否过期
		if (m_vSystemMail[i].iEndTime <= iTimeNow)
		{
			//邮件已过期
			continue;
		}

		switch (iReason)
		{
		case MAIL_REASON_NORMAL:
		{
			//普通拉取

			//VIP是否满足
			if ((stOne.iVIPMin != 0 || stOne.iVIPMax != 0) &&
				(stOne.iVIPMin>iVIPLevel || stOne.iVIPMax<iVIPLevel))
			{
				continue;
			}

			//渠道是否满足
			if (stOne.vChannelIDs.size() != 0 && std::find(stOne.vChannelIDs.begin(), stOne.vChannelIDs.end(), strChannel) == stOne.vChannelIDs.end())
			{
				continue;
			}

			//是否已经收取
			if (stOne.stMail.uUniqID <= uUniqID)
			{
				continue;
			}
		}
		break;

		case MAIL_REASON_VIPLV:
		{
			//VIP等级改变拉取
			//只检查VIP是否满足
			if (stOne.iVIPMin == 0 && stOne.iVIPMax == 0)
			{
				//不拉取没有VIP等级限制的
				continue;
			}

			if (stOne.iVIPMin != iVIPLevel)
			{
				//VIP不满足
				continue;
			}
		}
		break;

		default:
			break;
		}

		OneMailInfo* pstMailInfo = stResp.add_stmails();
		pstMailInfo->set_imailid(stOne.stMail.iMailID);
		pstMailInfo->set_uuniqid(stOne.stMail.uUniqID);
		pstMailInfo->set_isendtime(stOne.stMail.iSendTime);
		pstMailInfo->set_strtitle(stOne.stMail.strTitle);

		for (unsigned j = 0; j < stOne.stMail.vParams.size(); ++j)
		{
			pstMailInfo->add_strparams(stOne.stMail.vParams[j]);
		}

		for (unsigned j = 0; j < stOne.stMail.vAppendixes.size(); ++j)
		{
			MailAppendix* pstAppendix = pstMailInfo->add_stappendixs();
			pstAppendix->set_itype(stOne.stMail.vAppendixes[j].iType);
			pstAppendix->set_iid(stOne.stMail.vAppendixes[j].iRewardID);
			pstAppendix->set_inum(stOne.stMail.vAppendixes[j].iRewardNum);
		}
	}

	return;
}

//发送系统邮件
void CWorldMailManager::SendSystemMail(const GameMaster_Request& stReq, const OneMailInfo& stMailInfo)
{
	SystemMailData stSystemMail;

	//VIP等级限制
	stSystemMail.iVIPMin = stReq.ivipmin();
	stSystemMail.iVIPMax = stReq.ivipmax();
	
	//邮件过期时间
	stSystemMail.iEndTime = stReq.imailendtime();

	//渠道限制
	for (int i = 0; i < stReq.strchannelids_size(); ++i)
	{
		stSystemMail.vChannelIDs.push_back(stReq.strchannelids(i));
	}

	stSystemMail.stMail.uUniqID = ++m_uSystemUniqID;
	stSystemMail.stMail.iMailID = stMailInfo.imailid();
	stSystemMail.stMail.iSendTime = stMailInfo.isendtime();
	stSystemMail.stMail.iMailStat = MAIL_STAT_NOTREAD;
	stSystemMail.stMail.strTitle = stMailInfo.strtitle();

	for (int i = 0; i < stMailInfo.strparams_size(); ++i)
	{
		stSystemMail.stMail.vParams.push_back(stMailInfo.strparams(i));
	}

	RewardConfig stOneAppendix;
	for (int i = 0; i < stMailInfo.stappendixs_size(); ++i)
	{
		stOneAppendix.iType = stMailInfo.stappendixs(i).itype();
		stOneAppendix.iRewardID = stMailInfo.stappendixs(i).iid();
		stOneAppendix.iRewardNum = stMailInfo.stappendixs(i).inum();

		stSystemMail.stMail.vAppendixes.push_back(stOneAppendix);
	}

	m_vSystemMail.push_back(stSystemMail);

	SendSystemMailIDNotify();

	//保存到文件
	SaveMailInfo();

	return;
}

//推送
void CWorldMailManager::SendSystemMailIDNotify()
{
	static GameProtocolMsg stMsg;
	CWorldMsgHelper::GenerateMsgHead(stMsg, 0, MSGID_WORLD_SYSTEMMAILID_NOTIFY, 0);

	World_SystemMailID_Notify* pstNotify = stMsg.mutable_stbody()->mutable_m_stworld_systemmailid_notify();
	pstNotify->set_usystemuniqid(m_uSystemUniqID);

	CWorldMsgHelper::SendWorldMsgToAllZone(stMsg);

	return;
}

//加载Mail信息
void CWorldMailManager::LoadMailInfo()
{
	//从文件中读取Rank信息
	std::ifstream is;
	is.open(WORLD_SYSTEM_MAIL_FILE, std::ios::binary);
	if (is.fail())
	{
		//打开文件失败
		return;
	}

	Json::Reader jReader;
	Json::Value jValue;
	if (!jReader.parse(is, jValue))
	{
		//解析json文件失败
		is.close();
		LOGERROR("Failed to parse system mail info file %s\n", WORLD_SYSTEM_MAIL_FILE);
		return;
	}

	//唯一ID
	m_uSystemUniqID = jValue["uniqid"].asUInt();

	m_vSystemMail.clear();
	
	SystemMailData stOneData;
	RewardConfig stAppendix;
	for (unsigned i = 0; i < jValue["mail"].size(); ++i)
	{
		stOneData.Reset();

		//VIP限制
		stOneData.iVIPMin = jValue["mail"][i]["vipmin"].asInt();
		stOneData.iVIPMin = jValue["mail"][i]["vipmax"].asInt();

		//邮件过期时间
		stOneData.iEndTime = jValue["mail"][i]["endtime"].asInt();

		//渠道限制
		for (unsigned j = 0; j < jValue["mail"][i]["channel"].size(); ++j)
		{
			stOneData.vChannelIDs.push_back(jValue["mail"][i]["channel"][j].asString());
		}

		//邮件内容
		stOneData.stMail.iMailID = jValue["mail"][i]["mailinfo"]["id"].asInt();
		stOneData.stMail.uUniqID = jValue["mail"][i]["mailinfo"]["uniqid"].asUInt();
		stOneData.stMail.iSendTime = jValue["mail"][i]["mailinfo"]["time"].asUInt();
		stOneData.stMail.strTitle = jValue["mail"][i]["mailinfo"]["title"].asString();

		for (unsigned j = 0; j < jValue["mail"][i]["mailinfo"]["param"].size(); ++j)
		{
			stOneData.stMail.vParams.push_back(jValue["mail"][i]["mailinfo"]["param"][j].asString());
		}

		for (unsigned j = 0; j < jValue["mail"][i]["mailinfo"]["appendix"].size(); ++j)
		{
			stAppendix.iType = jValue["mail"][i]["mailinfo"]["appendix"][j]["type"].asInt();
			stAppendix.iRewardID = jValue["mail"][i]["mailinfo"]["appendix"][j]["id"].asInt();
			stAppendix.iRewardNum = jValue["mail"][i]["mailinfo"]["appendix"][j]["num"].asInt();

			stOneData.stMail.vAppendixes.push_back(stAppendix);
		}

		m_vSystemMail.push_back(stOneData);
	}

	is.close();

	return;
}

//保存Mail信息
void CWorldMailManager::SaveMailInfo()
{
	//保存Rank信息到文件
	std::ofstream os;
	os.open(WORLD_SYSTEM_MAIL_FILE, std::ios::binary);
	if (os.fail())
	{
		//打开文件失败
		LOGERROR("Failed to open system mail info file %s\n", WORLD_SYSTEM_MAIL_FILE);
		return;
	}

	//打包数据成json串
	Json::Value jValue;

	//唯一ID
	jValue["uniqid"] = m_uSystemUniqID;

	Json::Value jOneSystemMail;
	std::vector<SystemMailData> vSystemMail;
	int iTimeNow = CTimeUtility::GetNowTime();
	for (unsigned i = 0; i < m_vSystemMail.size(); ++i)
	{
		if (m_vSystemMail[i].iEndTime <= iTimeNow)
		{
			//剔除过期的
			continue;
		}

		vSystemMail.push_back(m_vSystemMail[i]);

		jOneSystemMail.clear();

		//VIP等级限制
		jOneSystemMail["vipmin"] = m_vSystemMail[i].iVIPMin;
		jOneSystemMail["vipmax"] = m_vSystemMail[i].iVIPMax;

		//邮件过期时间
		jOneSystemMail["endtime"] = m_vSystemMail[i].iEndTime;

		//渠道限制
		for (unsigned j = 0; j < m_vSystemMail[i].vChannelIDs.size(); ++j)
		{
			jOneSystemMail["channel"].append(m_vSystemMail[i].vChannelIDs[j]);
		}

		//邮件内容
		jOneSystemMail["mailinfo"]["id"] = m_vSystemMail[i].stMail.iMailID;
		jOneSystemMail["mailinfo"]["uniqid"] = m_vSystemMail[i].stMail.uUniqID;
		jOneSystemMail["mailinfo"]["time"] = m_vSystemMail[i].stMail.iSendTime;
		jOneSystemMail["mailinfo"]["title"] = m_vSystemMail[i].stMail.strTitle;

		for (unsigned j = 0; j < m_vSystemMail[i].stMail.vParams.size(); ++j)
		{
			jOneSystemMail["mailinfo"]["param"].append(m_vSystemMail[i].stMail.vParams[j]);
		}
		
		for (unsigned j = 0; j < m_vSystemMail[i].stMail.vAppendixes.size(); ++j)
		{
			Json::Value jOneAppendix;

			jOneAppendix["type"] = m_vSystemMail[i].stMail.vAppendixes[j].iType;
			jOneAppendix["id"] = m_vSystemMail[i].stMail.vAppendixes[j].iRewardID;
			jOneAppendix["num"] = m_vSystemMail[i].stMail.vAppendixes[j].iRewardNum;

			jOneSystemMail["mailinfo"]["appendix"].append(jOneAppendix);
		}

		jValue["mail"].append(jOneSystemMail);
	}

	m_vSystemMail = vSystemMail;

	os << jValue.toStyledString();
	os.close();

	return;
}

//重置
void CWorldMailManager::Reset()
{
	m_uSystemUniqID = 1;	//默认从1开始
	m_vSystemMail.clear();
}
