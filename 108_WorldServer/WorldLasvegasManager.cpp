#include <algorithm>
#include <fstream>

#include "json/json.h"
#include "GameProtocol.hpp"
#include "ErrorNumDef.hpp"
#include "LogAdapter.hpp"
#include "TimeUtility.hpp"
#include "Random.hpp"
#include "WorldMsgHelper.hpp"
#include "ConfigManager.hpp"

#include "WorldLasvegasManager.h"

static const char* WORLD_LASVEGAS_RECORD_FILE = "../conf/LasvegasRecord.dat";

static GameProtocolMsg stMsg;

CWorldLasvegasManager* CWorldLasvegasManager::Instance()
{
	static CWorldLasvegasManager* pInstance = NULL;
	if (!pInstance)
	{
		pInstance = new CWorldLasvegasManager;
	}

	return pInstance;
}

CWorldLasvegasManager::CWorldLasvegasManager()
{
	Reset();
}

CWorldLasvegasManager::~CWorldLasvegasManager()
{
	Reset();
}

//初始化
void CWorldLasvegasManager::Init()
{
	//重置数据
	Reset();

	//加载历史记录
	LoadLasvegasInfo();
}

//更新数字下注信息
void CWorldLasvegasManager::UpdateBetNumber(int iNumber, int iBetCoins)
{
	if (m_iStepType != LASVEGAS_STEP_BET)
	{
		//不是下注阶段
		return;
	}

	//增加下注金额
	for (unsigned i = 0; i < m_vBetDatas.size(); ++i)
	{
		if (m_vBetDatas[i].iNumber == iNumber)
		{
			m_vBetDatas[i].iBetCoins += iBetCoins;
			return;
		}
	}

	//没找到，新增一个
	LasvegasBetData stOneData;
	stOneData.iNumber = iNumber;
	stOneData.iBetCoins = iBetCoins;
	m_vBetDatas.push_back(stOneData);

	m_bSendUpdate = true;

	return;
}

//更新中奖信息
void CWorldLasvegasManager::UpdatePrizeInfo(const World_UpdatePrizeInfo_Request& stReq)
{
	//最新插入到最前
	LotteryPrizeData stOneData;
	
	for (int i = 0; i < stReq.stprizes_size(); ++i)
	{
		stOneData.strNickName = stReq.stprizes(i).strname();
		stOneData.iNumber = stReq.stprizes(i).inumber();
		stOneData.iRewardCoins = stReq.stprizes(i).irewardcoins();

		m_vPrizeDatas.insert(m_vPrizeDatas.begin(), stOneData);
	}

	//是否超过上限
	if (m_vPrizeDatas.size() > (unsigned)MAX_LASVEGAS_RECORD_NUM)
	{
		//删除尾部的
		m_vPrizeDatas.erase(m_vPrizeDatas.begin()+MAX_LASVEGAS_RECORD_NUM, m_vPrizeDatas.end());
	}

	//保存到文件
	SaveLasvegasInfo();

	m_bSendUpdate = true;

	return;
}

//定时器
void CWorldLasvegasManager::OnTick(int iTimeNow)
{
	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();

	//处理转盘逻辑
	switch (m_iStepType)
	{
	case LASVEGAS_STEP_INVALID:
	{
		//直接进入下注阶段
		m_iStepType = LASVEGAS_STEP_BET;
		m_iStepEndTime = iTimeNow+stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_LASVEGAS_BETTIME);

		//推送给客户端
		SendUpdateLasvegasNotify(iTimeNow);
	}
	break;

	case LASVEGAS_STEP_BET:
	{
		//下注阶段
		if (m_iStepEndTime <= iTimeNow)
		{
			//进入开奖阶段
			m_iStepType = LASVEGAS_STEP_LOTTERY;
			m_iStepEndTime = iTimeNow + stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_LASVEGAS_LOTTERYTIME);

			//清除下注信息
			m_vBetDatas.clear();

			//开奖
			const LasvegasConfig* pstConfig = stBaseCfgMgr.GetLasvegasConfig();
			if (pstConfig)
			{
				m_vLotteryIDs.push_back(pstConfig->iID);
				if (m_vLotteryIDs.size() > (unsigned)MAX_LASVEGAS_LOTTERY_NUM)
				{
					//删除第一个
					m_vLotteryIDs.erase(m_vLotteryIDs.begin());
				}

				//保存到文件
				SaveLasvegasInfo();
			}

			//推送给客户端消息
			SendUpdateLasvegasNotify(iTimeNow);
		}
	}
	break;

	case LASVEGAS_STEP_LOTTERY:
	{
		//开奖阶段
		if (m_iStepEndTime <= iTimeNow)
		{
			//开奖阶段结束，进入下一轮下注
			m_iStepType = LASVEGAS_STEP_BET;
			m_iStepEndTime = iTimeNow + stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_LASVEGAS_BETTIME);

			//推送给客户端
			SendUpdateLasvegasNotify(iTimeNow);
		}
	}
	break;

	default:
		break;
	}

	//是否推送转盘信息,2s更新一次
	if (m_bSendUpdate && (m_iLastUpdateTime + 2) <= iTimeNow)
	{
		SendUpdateLasvegasNotify(iTimeNow);
	}

	return;
}

//加载Lasvegas信息
void CWorldLasvegasManager::LoadLasvegasInfo()
{
	//从文件中读取转盘信息
	std::ifstream is;
	is.open(WORLD_LASVEGAS_RECORD_FILE, std::ios::binary);
	if (is.fail())
	{
		//打开文件失败
		LOGERROR("Failed to open lasvegas record file %s\n", WORLD_LASVEGAS_RECORD_FILE);
		return;
	}

	Json::Reader jReader;
	Json::Value jValue;
	if (!jReader.parse(is, jValue))
	{
		//解析json文件失败
		is.close();
		LOGERROR("Failed to parse lasvegas record file %s\n", WORLD_LASVEGAS_RECORD_FILE);
		return;
	}

	m_vLotteryIDs.clear();
	m_vPrizeDatas.clear();

	//开奖记录
	for (unsigned i = 0; i < jValue["lottery"].size(); ++i)
	{
		m_vLotteryIDs.push_back(jValue["lottery"][i].asInt());
	}

	//中奖纪录
	LotteryPrizeData stData;
	for (unsigned i = 0; i < jValue["prize"].size(); ++i)
	{
		stData.strNickName = jValue["prize"][i]["name"].asString();
		stData.iNumber = jValue["prize"][i]["num"].asInt();
		stData.iRewardCoins = jValue["prize"][i]["coins"].asInt();

		m_vPrizeDatas.push_back(stData);
	}

	is.close();

	return;
}

//保存Lasvegas信息
void CWorldLasvegasManager::SaveLasvegasInfo()
{
	//保存转盘信息到文件
	std::ofstream os;
	os.open(WORLD_LASVEGAS_RECORD_FILE, std::ios::binary);
	if (os.fail())
	{
		//打开文件失败
		LOGERROR("Failed to open lasvegas record file %s\n", WORLD_LASVEGAS_RECORD_FILE);
		return;
	}

	//打包数据成json串
	Json::Value jValue;
	Json::Value jOneInfo;

	//开奖记录
	for (unsigned i = 0; i < m_vLotteryIDs.size(); ++i)
	{
		jOneInfo.append(m_vLotteryIDs[i]);
	}
	jValue["lottery"] = jOneInfo;

	//中奖纪录
	Json::Value jOneRewardInfo;
	for (unsigned i = 0; i < m_vPrizeDatas.size(); ++i)
	{
		jOneRewardInfo.clear();
		jOneRewardInfo["name"] = m_vPrizeDatas[i].strNickName;
		jOneRewardInfo["num"] = m_vPrizeDatas[i].iNumber;
		jOneRewardInfo["coins"] = m_vPrizeDatas[i].iRewardCoins;

		jValue["prize"].append(jOneRewardInfo);
	}

	os << jValue.toStyledString();
	os.close();

	return;
}

//推送大转盘信息
void CWorldLasvegasManager::SendUpdateLasvegasNotify(int iTimeNow)
{
	CWorldMsgHelper::GenerateMsgHead(stMsg, 0, MSGID_WORLD_UPDATELASVEGAS_NOTIFY, 0);

	World_UpdateLasvegas_Notify* pstNotify = stMsg.mutable_stbody()->mutable_m_stworld_updatelasvegas_notify();
	pstNotify->mutable_stinfo()->set_isteptype(m_iStepType);
	pstNotify->mutable_stinfo()->set_istependtime(m_iStepEndTime);
	
	//开奖信息
	for (unsigned i = 0; i < m_vLotteryIDs.size(); ++i)
	{
		pstNotify->mutable_stinfo()->add_ilotteryids(m_vLotteryIDs[i]);
	}

	//中奖信息
	for (unsigned i = 0; i < m_vPrizeDatas.size(); ++i)
	{
		PrizeInfo* pstOneInfo = pstNotify->mutable_stinfo()->add_stprizes();
		pstOneInfo->set_strname(m_vPrizeDatas[i].strNickName);
		pstOneInfo->set_inumber(m_vPrizeDatas[i].iNumber);
		pstOneInfo->set_irewardcoins(m_vPrizeDatas[i].iRewardCoins);
	}

	//下注信息
	for (unsigned i = 0; i < m_vBetDatas.size(); ++i)
	{
		NumberBetInfo* pstOneInfo = pstNotify->mutable_stinfo()->add_stbets();
		pstOneInfo->set_inumber(m_vBetDatas[i].iNumber);
		pstOneInfo->set_lbetcoins(m_vBetDatas[i].iBetCoins);
	}

	CWorldMsgHelper::SendWorldMsgToAllZone(stMsg);

	m_iLastUpdateTime = iTimeNow;
	m_bSendUpdate = false;

	return;
}

//重置
void CWorldLasvegasManager::Reset()
{
	m_iStepType = 0;
	m_iStepEndTime = 0;
	
	m_bSendUpdate = false;
	m_iLastUpdateTime = 0;

	m_vLotteryIDs.clear();
	m_vPrizeDatas.clear();
	m_vBetDatas.clear();
}
