
#include <algorithm>
#include <fstream>

#include "json/json.h"
#include "GameProtocol.hpp"
#include "ErrorNumDef.hpp"
#include "LogAdapter.hpp"
#include "TimeUtility.hpp"
#include "WorldMsgHelper.hpp"
#include "ConfigManager.hpp"
#include "WorldMailManager.hpp"

#include "WorldRankInfoManager.h"

using namespace ServerLib;

const static char* WORLD_RANK_INFO_FILE = "../conf/RankInfo.dat";

CWorldRankInfoManager* CWorldRankInfoManager::Instance()
{
	static CWorldRankInfoManager* pInstance = NULL;
	if (!pInstance)
	{
		pInstance = new CWorldRankInfoManager;
	}

	return pInstance;
}

CWorldRankInfoManager::CWorldRankInfoManager()
{
	Reset();
}

CWorldRankInfoManager::~CWorldRankInfoManager()
{

}

//初始化RankInfo
void CWorldRankInfoManager::Init()
{
	Reset();

	//加载排行信息
	LoadRankInfo();
}

//拉取排行信息,FromRank从1开始
int CWorldRankInfoManager::GetWorldRank(int iRankType, unsigned uVersionID, World_GetRankInfo_Response& stResp)
{
	//判断类型
	if (iRankType <= RANK_TYPE_INVALID || iRankType >= RANK_TYPE_MAX)
	{
		LOGERROR("Failed to get rank info, invalid type %d\n", iRankType);
		return T_WORLD_PARA_ERROR;
	}

	RankList& stOneList = m_astRankLists[iRankType];

	//数据是否有更新
	if (uVersionID >= stOneList.uVersionID)
	{
		stResp.set_itype(iRankType);
		stResp.set_iversionid(stOneList.uVersionID);

		return T_SERVER_SUCCESS;
	}

	//封装返回
	stResp.set_ilastupdate(stOneList.iLastUpdateTime);
	stResp.set_itype(iRankType);
	stResp.set_iversionid(stOneList.uVersionID);

	//上个周期
	for (unsigned i = 0; i < stOneList.vLastRankInfos.size(); ++i)
	{
		RankInfo* pstOneInfo = stResp.add_stlastranks();
		pstOneInfo->set_uin(stOneList.vLastRankInfos[i].uiUin);
		pstOneInfo->set_sznickname(stOneList.vLastRankInfos[i].strNickName);
		pstOneInfo->set_strpicid(stOneList.vLastRankInfos[i].strPicID);
		pstOneInfo->set_iviplevel(stOneList.vLastRankInfos[i].iVIPLevel);
		pstOneInfo->set_inum(stOneList.vLastRankInfos[i].iNum);
		pstOneInfo->set_strsign(stOneList.vLastRankInfos[i].strSign);
	}

	//本周期
	for (unsigned i = 0; i < stOneList.vRankInfos.size(); ++i)
	{
		RankInfo* pstOneInfo = stResp.add_stranks();
		pstOneInfo->set_uin(stOneList.vRankInfos[i].uiUin);
		pstOneInfo->set_sznickname(stOneList.vRankInfos[i].strNickName);
		pstOneInfo->set_strpicid(stOneList.vRankInfos[i].strPicID);
		pstOneInfo->set_iviplevel(stOneList.vRankInfos[i].iVIPLevel);
		pstOneInfo->set_inum(stOneList.vRankInfos[i].iNum);
		pstOneInfo->set_strsign(stOneList.vRankInfos[i].strSign);
	}

	return T_SERVER_SUCCESS;
}

bool CompareRank(const RankData& stRank1, const RankData& stRank2)
{
	return stRank1.iNum > stRank2.iNum;
}

//更新排行信息
int CWorldRankInfoManager::UpdateWorldRank(int iRankType, const RankInfo& stRankInfo)
{
	//判断类型
	if (iRankType <= RANK_TYPE_INVALID || iRankType >= RANK_TYPE_MAX)
	{
		LOGERROR("Failed to update rank info, invalid rank type %d\n", iRankType);
		return T_WORLD_PARA_ERROR;
	}

	int iTimeNow = CTimeUtility::GetNowTime();
	RankList& stOneList = m_astRankLists[iRankType];

	//是否需要更新日排行
	if ((iRankType == RANK_TYPE_DAYSCORE) && !CTimeUtility::IsInSameDay(iTimeNow, m_astRankLists[RANK_TYPE_DAYSCORE].iLastUpdateTime))
	{
		//发放日榜奖励
		SendRankReward(RANK_TYPE_DAYSCORE, iTimeNow);

		RankList& stOneList = m_astRankLists[RANK_TYPE_DAYSCORE];
		stOneList.vLastRankInfos.assign(stOneList.vRankInfos.begin(), stOneList.vRankInfos.end());
		stOneList.vRankInfos.clear();

		++stOneList.uVersionID;
		stOneList.iLastUpdateTime = iTimeNow;
	}

	//是否需要更新周排行
	if ((iRankType == RANK_TYPE_WEEKSCORE) && !CTimeUtility::IsInSameWeek(iTimeNow, m_astRankLists[RANK_TYPE_WEEKSCORE].iLastUpdateTime))
	{
		//发放周榜奖励
		SendRankReward(RANK_TYPE_WEEKSCORE, iTimeNow);

		RankList& stOneList = m_astRankLists[RANK_TYPE_WEEKSCORE];
		stOneList.vLastRankInfos.assign(stOneList.vRankInfos.begin(), stOneList.vRankInfos.end());
		stOneList.vRankInfos.clear();

		++stOneList.uVersionID;
		stOneList.iLastUpdateTime = iTimeNow;
	}

	//先查找是否在排名中
	int iIndex = -1;
	for (unsigned i = 0; i < stOneList.vRankInfos.size(); ++i)
	{
		if (stOneList.vRankInfos[i].uiUin == stRankInfo.uin())
		{
			iIndex = i;
			break;
		}
	}

	if (iIndex < 0)
	{
		//不在排名中
		if (stOneList.vRankInfos.size() < (unsigned)MAX_RANK_INFO_NUM)
		{
			//未满,插入到最后
			RankData stData;
			SetRankData(stData, stRankInfo);
			stOneList.vRankInfos.push_back(stData);
		}
		else
		{
			//已满，确认是否能进入排名
			if (stOneList.vRankInfos[MAX_RANK_INFO_NUM - 1].iNum >= stRankInfo.inum())
			{
				//不能进入排名，直接返回
				if (stOneList.iLastUpdateTime == iTimeNow)
				{
					//保存到文件
					SaveRankInfo();
				}

				return T_SERVER_SUCCESS;
			}
			else
			{
				//替换最后一名
				SetRankData(stOneList.vRankInfos[MAX_RANK_INFO_NUM - 1], stRankInfo);
			}
		}
	}
	else
	{
		//已经在排名中,更新信息

		//如果是弹头排行并且不能入榜
		if (iRankType == RANK_TYPE_WARHEAD && stRankInfo.inum() < WARHEAD_INRANK_NUM)
		{
			//从排行榜中删除
			stOneList.vRankInfos.erase(stOneList.vRankInfos.begin()+iIndex);
		}
		else
		{
			SetRankData(stOneList.vRankInfos[iIndex], stRankInfo);
		}
	}

	//重新排序
	std::stable_sort(stOneList.vRankInfos.begin(), stOneList.vRankInfos.end(), CompareRank);

	//修改版本号
	++stOneList.uVersionID;
	stOneList.iLastUpdateTime = iTimeNow;

	//保存Rank信息
	SaveRankInfo();

	return T_SERVER_SUCCESS;
}

//定时器
void CWorldRankInfoManager::OnTick(int iTimeNow)
{
	//是否保存到文件
	bool bSaveRankInfo = false;

	if (!CTimeUtility::IsInSameDay(iTimeNow, m_astRankLists[RANK_TYPE_DAYSCORE].iLastUpdateTime))
	{
		//发放日榜奖励
		SendRankReward(RANK_TYPE_DAYSCORE, iTimeNow);

		RankList& stOneList = m_astRankLists[RANK_TYPE_DAYSCORE];
		stOneList.vLastRankInfos.assign(stOneList.vRankInfos.begin(), stOneList.vRankInfos.end());
		stOneList.vRankInfos.clear();

		++stOneList.uVersionID;
		stOneList.iLastUpdateTime = iTimeNow;
		bSaveRankInfo = true;
	}

	//是否发放周榜奖励
	if (!CTimeUtility::IsInSameWeek(iTimeNow, m_astRankLists[RANK_TYPE_WEEKSCORE].iLastUpdateTime))
	{
		//发放周榜奖励
		SendRankReward(RANK_TYPE_WEEKSCORE, iTimeNow);

		RankList& stOneList = m_astRankLists[RANK_TYPE_WEEKSCORE];
		stOneList.vLastRankInfos.assign(stOneList.vRankInfos.begin(), stOneList.vRankInfos.end());
		stOneList.vRankInfos.clear();

		//修改版本号
		++stOneList.uVersionID;
		stOneList.iLastUpdateTime = iTimeNow;
		bSaveRankInfo = true;
	}

	if (bSaveRankInfo)
	{
		//保存到文件
		SaveRankInfo();
	}

	return;
}

//设置RankData
void CWorldRankInfoManager::SetRankData(RankData& stData, const RankInfo& stInfo)
{
	stData.uiUin = stInfo.uin();
	stData.strNickName = stInfo.sznickname();
	stData.strPicID = stInfo.strpicid();
	stData.iVIPLevel = stInfo.iviplevel();
	stData.iNum = stInfo.inum();
	stData.strSign = stInfo.strsign();
}

//发放周榜奖励
void CWorldRankInfoManager::SendRankReward(int iRankType, int iTimeNow)
{
	//暂时去掉榜单奖励
	return;

	if (iRankType != RANK_TYPE_DAYSCORE && iRankType != RANK_TYPE_WEEKSCORE)
	{
		return;
	}

	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();

	//读取邮件配置
	int iMailID = (iRankType == RANK_TYPE_DAYSCORE) ? MAIL_SEND_SCOREDAY : MAIL_SEND_SCOREWEEK;

	const MailConfig* pstMailConfig = stBaseCfgMgr.GetMailConfig(iMailID);
	if (!pstMailConfig)
	{
		LOGERROR("Failed to get mail config, mail id %d\n", iMailID);
		return;
	}

	RankList& stOneList = m_astRankLists[iRankType];
	const RankRewardConfig* pstRankConfig = NULL;
	const RewardConfig* pstRewardConfig = NULL;
	World_SendMail_Request stReq;
	char szParam[32] = { 0 };
	int iRet = T_SERVER_SUCCESS;
	for (unsigned i = 0; i < stOneList.vRankInfos.size(); ++i)
	{
		pstRankConfig = CConfigManager::Instance()->GetBaseCfgMgr().GetRankRewardConfig(i+1);
		if (!pstRankConfig)
		{
			//此处不需要打印日志
			//LOGERROR("Failed to get rank reward config, rank %u\n", (i+1));
			continue;
		}

		pstRewardConfig = (iRankType == RANK_TYPE_DAYSCORE) ? &pstRankConfig->stDayReward : &pstRankConfig->stWeekReward;

		stReq.Clear();
		stReq.set_uifromuin(0);
		stReq.set_uitouin(stOneList.vRankInfos[i].uiUin);

		OneMailInfo* pstOneInfo = stReq.mutable_stmailinfo();
		pstOneInfo->set_isendtime(iTimeNow);
		pstOneInfo->set_imailid(iMailID);
		//pstOneInfo->set_strtitle(pstMailConfig->strTitle);

		//参数1是排名
		SAFE_SPRINTF(szParam, sizeof(szParam)-1, "%u", (i+1));
		pstOneInfo->add_strparams(szParam);

		//增加附件
		MailAppendix* pstAppendix = pstOneInfo->add_stappendixs();
		pstAppendix->set_itype(pstRewardConfig->iType);
		pstAppendix->set_iid(pstRewardConfig->iRewardID);
		pstAppendix->set_inum(pstRewardConfig->iRewardNum);

		iRet = CWorldMailManager::SendPersonalMail(stReq);
		if (iRet)
		{
			//发送邮件失败
			LOGERROR("Failed to send rank gift mail, ret %d, rank type %d, uin %u, rank %u\n", iRet, iRankType, stReq.uitouin(), (i+1));
			continue;
		}
	}

	return;
}

//加载Rank信息
void CWorldRankInfoManager::LoadRankInfo()
{
	//从文件中读取Rank信息
	std::ifstream is;
	is.open(WORLD_RANK_INFO_FILE, std::ios::binary);
	if (is.fail())
	{
		//打开文件失败
		LOGERROR("Failed to open rank info file %s\n", WORLD_RANK_INFO_FILE);
		return;
	}

	Json::Reader jReader;
	Json::Value jValue;
	if (!jReader.parse(is, jValue))
	{
		//解析json文件失败
		is.close();
		LOGERROR("Failed to parse rank info file %s\n", WORLD_RANK_INFO_FILE);
		return;
	}

	for (unsigned i = 0; i < jValue.size(); ++i)
	{
		RankList& stOneList = m_astRankLists[i];
		stOneList.Reset();

		stOneList.uVersionID = 1;
		stOneList.iLastUpdateTime = jValue[i]["lastupdate"].asInt();

		RankData stOneData;
		if (!jValue[i]["lastrank"].isNull())
		{
			for (unsigned j = 0; j < jValue[i]["lastrank"].size(); ++j)
			{
				stOneData.uiUin = jValue[i]["lastrank"][j]["uin"].asUInt();
				stOneData.strNickName = jValue[i]["lastrank"][j]["name"].asString();
				stOneData.strPicID = jValue[i]["lastrank"][j]["pic"].asString();
				stOneData.iVIPLevel = jValue[i]["lastrank"][j]["vip"].asInt();
				stOneData.iNum = (long8)jValue[i]["lastrank"][j]["num"].asDouble();
				stOneData.strSign = jValue[i]["lastrank"][j]["sign"].asString();

				stOneList.vLastRankInfos.push_back(stOneData);
			}
		}

		for (unsigned j = 0; j < jValue[i]["rank"].size(); ++j)
		{
			stOneData.uiUin = jValue[i]["rank"][j]["uin"].asUInt();
			stOneData.strNickName = jValue[i]["rank"][j]["name"].asString();
			stOneData.strPicID = jValue[i]["rank"][j]["pic"].asString();
			stOneData.iVIPLevel = jValue[i]["rank"][j]["vip"].asInt();
			stOneData.iNum = (long8)jValue[i]["rank"][j]["num"].asDouble();
			stOneData.strSign = jValue[i]["rank"][j]["sign"].asString();

			stOneList.vRankInfos.push_back(stOneData);
		}
	}

	is.close();

	return;
}

//保存Rank信息
void CWorldRankInfoManager::SaveRankInfo()
{
	//保存Rank信息到文件
	std::ofstream os;
	os.open(WORLD_RANK_INFO_FILE, std::ios::binary);
	if (os.fail())
	{
		//打开文件失败
		LOGERROR("Failed to open rank info file %s\n", WORLD_RANK_INFO_FILE);
		return;
	}

	//打包数据成json串
	Json::Value jValue;

	Json::Value jOneList;
	for (int i = 0; i < RANK_TYPE_MAX; ++i)
	{
		jOneList.clear();

		RankList& stOneList = m_astRankLists[i];

		if (stOneList.vRankInfos.size() != 0)
		{
			jOneList["lastupdate"] = stOneList.iLastUpdateTime;

			Json::Value jOneRankInfo;

			//上周期排名
			for (unsigned j = 0; j < stOneList.vLastRankInfos.size(); ++j)
			{
				jOneRankInfo.clear();
				jOneRankInfo["uin"] = stOneList.vLastRankInfos[j].uiUin;
				jOneRankInfo["name"] = stOneList.vLastRankInfos[j].strNickName;
				jOneRankInfo["pic"] = stOneList.vLastRankInfos[j].strPicID;
				jOneRankInfo["vip"] = stOneList.vLastRankInfos[j].iVIPLevel;
				jOneRankInfo["num"] = (double)stOneList.vLastRankInfos[j].iNum;
				jOneRankInfo["sign"] = stOneList.vLastRankInfos[j].strSign;

				jOneList["lastrank"].append(jOneRankInfo);
			}

			//本周期排名
			for (unsigned j = 0; j < stOneList.vRankInfos.size(); ++j)
			{
				jOneRankInfo.clear();
				jOneRankInfo["uin"] = stOneList.vRankInfos[j].uiUin;
				jOneRankInfo["name"] = stOneList.vRankInfos[j].strNickName;
				jOneRankInfo["pic"] = stOneList.vRankInfos[j].strPicID;
				jOneRankInfo["vip"] = stOneList.vRankInfos[j].iVIPLevel;
				jOneRankInfo["num"] = (double)stOneList.vRankInfos[j].iNum;
				jOneRankInfo["sign"] = stOneList.vRankInfos[j].strSign;

				jOneList["rank"].append(jOneRankInfo);
			}
		}

		jValue.append(jOneList);
	}

	os << jValue.toStyledString();
	os.close();

	return;
}

//重置
void CWorldRankInfoManager::Reset()
{
	for (int i = 0; i < RANK_TYPE_MAX; ++i)
	{
		m_astRankLists[i].Reset();
	}
}
