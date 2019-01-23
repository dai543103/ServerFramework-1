
#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "ErrorNumDef.hpp"
#include "TimeUtility.hpp"
#include "Kernel/ZoneMsgHelper.hpp"
#include "Kernel/ModuleHelper.hpp"

#include "RankInfoManager.h"

using namespace ServerLib;

static GameProtocolMsg stMsg;

CRankInfoManager* CRankInfoManager::Instance()
{
	static CRankInfoManager* pInstance = NULL;
	if (!pInstance)
	{
		pInstance = new CRankInfoManager;
	}

	return pInstance;
}

CRankInfoManager::~CRankInfoManager()
{

}

CRankInfoManager::CRankInfoManager()
{
	Reset();
}

//初始化
void CRankInfoManager::Init()
{
	//拉取排行信息
	m_iLastUpdateTime = CTimeUtility::GetNowTime();

	for (int i = RANK_TYPE_INVALID + 1; i < RANK_TYPE_MAX; ++i)
	{
		GetRankInfoFromWorld(i);
	}

	return;
}

//拉取排行信息
int CRankInfoManager::GetRankInfo(unsigned uin, int iType, int iFromRank, int iNum, bool bLastRank, Zone_GetRankInfo_Response& stResp)
{
	if (iType <= RANK_TYPE_INVALID || iType >= RANK_TYPE_MAX)
	{
		LOGERROR("Failed to get rank info, invalid type %d\n", iType);
		return T_ZONE_PARA_ERROR;
	}

	if (iFromRank <= 0 || iFromRank > MAX_RANK_INFO_NUM || iNum <= 0)
	{
		LOGERROR("Failed to get rank info, from %d, num %d\n", iFromRank, iNum);
		return T_ZONE_PARA_ERROR;
	}

	RankList& stOneList = m_astRankLists[iType];
	if (bLastRank)
	{
		PackRankInfo(uin, iFromRank, iNum, stOneList.vLastRankInfos, stResp);
	}
	else
	{
		PackRankInfo(uin, iFromRank, iNum, stOneList.vRankInfos, stResp);
	}

	return T_SERVER_SUCCESS;
}

//更新排行信息
void CRankInfoManager::UpdateRoleRank(CGameRoleObj& stRoleObj)
{
	RankData stData;
	stData.uiUin = stRoleObj.GetUin();
	stData.strNickName = stRoleObj.GetNickName();
	stData.strPicID = stRoleObj.GetPicID();
	stData.iVIPLevel = stRoleObj.GetVIPLevel();

	//刷新隔天隔周积分
	stRoleObj.AddFishScore(0);

	//更新金币排行
	stData.iNum = stRoleObj.GetResource(RESOURCE_TYPE_COIN);
	UpdateRankByType(RANK_TYPE_MONEY, stData, stRoleObj.GetUpdateRank());

	//更新黄金弹头排行
	stData.iNum = stRoleObj.GetRepThingsManager().GetRepItemNum(GOLD_WARHEAD_ID);
	if (IsInRank(RANK_TYPE_WARHEAD, stData) || stData.iNum >= WARHEAD_INRANK_NUM)
	{
		UpdateRankByType(RANK_TYPE_WARHEAD, stData, stRoleObj.GetUpdateRank());
	}

	//更新日积分排行
	if (stRoleObj.GetScoreInfo().lDayScore > 0)
	{
		stData.iNum = stRoleObj.GetScoreInfo().lDayScore;
		UpdateRankByType(RANK_TYPE_DAYSCORE, stData, stRoleObj.GetUpdateRank());
	}

	//更新周积分排行
	if (stRoleObj.GetScoreInfo().lWeekScore > 0)
	{
		stData.iNum = stRoleObj.GetScoreInfo().lWeekScore;
		UpdateRankByType(RANK_TYPE_WEEKSCORE, stData, stRoleObj.GetUpdateRank());
	}

	//设置强制更新为false
	stRoleObj.SetUpdateRank(false);

	return;
}

//设置排行信息
int CRankInfoManager::SetRankListInfo(const World_GetRankInfo_Response& stResp)
{
	if (stResp.itype() <= RANK_TYPE_INVALID || stResp.itype() >= RANK_TYPE_MAX)
	{
		return T_ZONE_PARA_ERROR;
	}

	RankList& stOneList = m_astRankLists[stResp.itype()];

	if (stResp.iversionid() <= stOneList.uVersionID)
	{
		//已经是最新数据
		return T_SERVER_SUCCESS;
	}

	//更新排行数据
	stOneList.uVersionID = stResp.iversionid();
	stOneList.iLastUpdateTime = stResp.ilastupdate();
	
	RankData stOneData;

	//上周期数据
	stOneList.vLastRankInfos.clear();
	for (int i = 0; i < stResp.stlastranks_size(); ++i)
	{
		stOneData.uiUin = stResp.stlastranks(i).uin();
		stOneData.strNickName = stResp.stlastranks(i).sznickname();
		stOneData.strPicID = stResp.stlastranks(i).strpicid();
		stOneData.iVIPLevel = stResp.stlastranks(i).iviplevel();
		stOneData.iNum = stResp.stlastranks(i).inum();
		stOneData.strSign = stResp.stlastranks(i).strsign();

		stOneList.vLastRankInfos.push_back(stOneData);
	}

	//本周期数据
	stOneList.vRankInfos.clear();
	for (int i = 0; i < stResp.stranks_size(); ++i)
	{
		stOneData.uiUin = stResp.stranks(i).uin();
		stOneData.strNickName = stResp.stranks(i).sznickname();
		stOneData.strPicID = stResp.stranks(i).strpicid();
		stOneData.iVIPLevel = stResp.stranks(i).iviplevel();
		stOneData.iNum = stResp.stranks(i).inum();
		stOneData.strSign = stResp.stranks(i).strsign();

		stOneList.vRankInfos.push_back(stOneData);
	}

	return T_SERVER_SUCCESS;
}

//定时器
void CRankInfoManager::OnTick()
{
	int iTimeNow = CTimeUtility::GetNowTime();

	//20s更新一次
	if ((iTimeNow - m_iLastUpdateTime) < 20)
	{
		return;
	}

	m_iLastUpdateTime = iTimeNow;

	for (int i = RANK_TYPE_INVALID + 1; i < RANK_TYPE_MAX; ++i)
	{
		GetRankInfoFromWorld(i);
	}

	return;
}

//更新排行信息
void CRankInfoManager::UpdateRankByType(int iType, const RankData& stData, bool bUpdateRank)
{
	//校验参数
	if (iType <= RANK_TYPE_INVALID || iType >= RANK_TYPE_MAX)
	{
		return;
	}

	RankList& stOneList = m_astRankLists[iType];

	//是否在排名中
	bool bIsInRank = false;
	std::vector<RankData>::iterator it = std::find(stOneList.vRankInfos.begin(), stOneList.vRankInfos.end(), stData);
	if (it != stOneList.vRankInfos.end())
	{
		bIsInRank = true;

		if (it->iNum == stData.iNum && !bUpdateRank)
		{
			//在排名中并且不需要强制更新
			return;
		}
	}

	//不在排行中并且不能进入排行
	if(!bIsInRank && stOneList.vRankInfos.size() >= (int)MAX_RANK_INFO_NUM && 
		stOneList.vRankInfos[MAX_RANK_INFO_NUM - 1].iNum >= stData.iNum && 
		CTimeUtility::IsInSameDay(stOneList.iLastUpdateTime, CTimeUtility::GetNowTime()))
	{
		return;
	}

	//请求World更新排行
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_WORLD_UPDATERANK_REQUEST);
	World_UpdateRank_Request* pstReq = stMsg.mutable_stbody()->mutable_m_stworld_updaterank_request();
	pstReq->set_ifromzoneid(CModuleHelper::GetZoneID());

	UpdateRankInfo* pstRankInfo = pstReq->add_stupdateranks();
	pstRankInfo->set_iranktype(iType);
	pstRankInfo->mutable_strank()->set_uin(stData.uiUin);
	pstRankInfo->mutable_strank()->set_sznickname(stData.strNickName);
	pstRankInfo->mutable_strank()->set_strpicid(stData.strPicID);
	pstRankInfo->mutable_strank()->set_iviplevel(stData.iVIPLevel);
	pstRankInfo->mutable_strank()->set_inum(stData.iNum);
	pstRankInfo->mutable_strank()->set_strsign(stData.strSign);

	CZoneMsgHelper::SendZoneMsgToWorld(stMsg);

	return;
}

//拉取排行榜信息
void CRankInfoManager::GetRankInfoFromWorld(int iType)
{
	if (iType <= RANK_TYPE_INVALID || iType >= RANK_TYPE_MAX)
	{
		return;
	}

	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_WORLD_GETRANKINFO_REQUEST);
	World_GetRankInfo_Request* pstReq = stMsg.mutable_stbody()->mutable_m_stworld_getrankinfo_request();
	pstReq->set_ifromzoneid(CModuleHelper::GetZoneID());
	pstReq->set_itype(iType);
	pstReq->set_iversionid(m_astRankLists[iType].uVersionID);

	CZoneMsgHelper::SendZoneMsgToWorld(stMsg);
}

//打包排行信息
void CRankInfoManager::PackRankInfo(unsigned uin, int iFromRank, int iNum, const std::vector<RankData>& vRankDatas, Zone_GetRankInfo_Response& stResp)
{
	if (vRankDatas.size() == 0)
	{
		//没有数据
		return;
	}

	for (int i = 0; i < iNum; ++i)
	{
		if ((iFromRank + i) >(int)vRankDatas.size())
		{
			break;
		}

		//封装数据
		RankInfo* pstOneInfo = stResp.add_stranks();
		pstOneInfo->set_uin(vRankDatas[iFromRank + i - 1].uiUin);
		pstOneInfo->set_sznickname(vRankDatas[iFromRank + i - 1].strNickName);
		pstOneInfo->set_strpicid(vRankDatas[iFromRank + i - 1].strPicID);
		pstOneInfo->set_iviplevel(vRankDatas[iFromRank + i - 1].iVIPLevel);
		pstOneInfo->set_inum(vRankDatas[iFromRank + i - 1].iNum);
		pstOneInfo->set_strsign(vRankDatas[iFromRank + i - 1].strSign);
		pstOneInfo->set_iranknum(iFromRank + i);
	}

	//获取自己的排名
	for (unsigned i = 0; i < vRankDatas.size(); ++i)
	{
		if (vRankDatas[i].uiUin == uin)
		{
			//在排行中
			stResp.set_imyrank(i+1);
			break;
		}
	}

	return;
}

//是否在排行中
bool CRankInfoManager::IsInRank(int iType, const RankData& stData)
{
	if (iType <= RANK_TYPE_INVALID || iType >= RANK_TYPE_MAX)
	{
		return false;
	}

	for (unsigned i = 0; i < m_astRankLists[iType].vRankInfos.size(); ++i)
	{
		if (m_astRankLists[iType].vRankInfos[i].uiUin == stData.uiUin)
		{
			//在排行榜中
			return true;
		}
	}

	return false;
}

void CRankInfoManager::Reset()
{
	m_iLastUpdateTime = 0;

	for (int i = RANK_TYPE_INVALID; i < RANK_TYPE_MAX; ++i)
	{
		m_astRankLists[i].uVersionID = 0;
		m_astRankLists[i].iLastUpdateTime = 0;
		m_astRankLists[i].vRankInfos.clear();
		m_astRankLists[i].vLastRankInfos.clear();
	}
}
