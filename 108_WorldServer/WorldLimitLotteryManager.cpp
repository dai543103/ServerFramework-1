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

#include "WorldLimitLotteryManager.h"

static const char* WORLD_LOTTERY_RECORD_FILE = "../conf/LotteryRecord.dat";

CWorldLimitLotteryManager* CWorldLimitLotteryManager::Instance()
{
	static CWorldLimitLotteryManager* pInstance = NULL;
	if (!pInstance)
	{
		pInstance = new CWorldLimitLotteryManager;
	}

	return pInstance;
}

CWorldLimitLotteryManager::CWorldLimitLotteryManager()
{
	Reset();
}

CWorldLimitLotteryManager::~CWorldLimitLotteryManager()
{
	Reset();
}

//初始化
void CWorldLimitLotteryManager::Init()
{
	//初始化限量信息
	ResetLotteryInfo(true);

	//加载历史记录
	LoadRechargeLotteryRecord();
}

//限量抽奖
void CWorldLimitLotteryManager::LimitLottery(int iLimitType, const std::string& strNickName, bool bIsTenTimes, std::vector<int>& vLotteryIDs)
{
	//更新限量信息
	ResetLotteryInfo(false);

	vLotteryIDs.clear();

	//处理抽奖逻辑
	int iLotteryID = 0;
	int iLotteryTimes = bIsTenTimes ? 10 : 1;
	for (int i = 0; i < iLotteryTimes; ++i)
	{
		iLotteryID = LotteryOneTime(iLimitType);
		if (iLotteryID != 0)
		{
			vLotteryIDs.push_back(iLotteryID);
		}
	}

	if (iLimitType == LIMIT_LOTTERY_RECHARGE)
	{
		//增加充值抽奖记录
		AddLotteryRecord(strNickName, vLotteryIDs);
	}

	return;
}

//拉取充值记录
void CWorldLimitLotteryManager::GetLotteryRecord(int iFrom, int iNum, Zone_PayLotteryRecord_Response& stResp)
{
	int iTotalNum = m_vLotteryRecord.size();
	for (int i = 0; i < iNum; ++i)
	{
		if ((iFrom + i) <= 0 || (iFrom + i) > iTotalNum)
		{
			break;
		}

		//增加记录
		PayLotteryRecord* pstOneRecord = stResp.add_strecords();
		pstOneRecord->set_strnickname(m_vLotteryRecord[iFrom+i-1].strNickName);
		pstOneRecord->set_ilotteryid(m_vLotteryRecord[iFrom + i - 1].iLotteryID);
		pstOneRecord->set_iindex(iFrom+i);
	}

	return;
}

//加载抽奖记录
void CWorldLimitLotteryManager::LoadRechargeLotteryRecord()
{
	//从文件中读取Rank信息
	std::ifstream is;
	is.open(WORLD_LOTTERY_RECORD_FILE, std::ios::binary);
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
		LOGERROR("Failed to parse lottery record file %s\n", WORLD_LOTTERY_RECORD_FILE);
		return;
	}

	m_vLotteryRecord.clear();

	RechargeLotteryRecord stOneRecord;
	for (unsigned i = 0; i < jValue.size(); ++i)
	{
		stOneRecord.strNickName = jValue[i]["name"].asString();
		stOneRecord.iLotteryID = jValue[i]["id"].asInt();

		m_vLotteryRecord.push_back(stOneRecord);
	}

	is.close();

	return;
}

//保存抽奖记录
void CWorldLimitLotteryManager::SaveRechargeLotterRecord()
{
	//保存Rank信息到文件
	std::ofstream os;
	os.open(WORLD_LOTTERY_RECORD_FILE, std::ios::binary);
	if (os.fail())
	{
		//打开文件失败
		LOGERROR("Failed to open lottery record file %s\n", WORLD_LOTTERY_RECORD_FILE);
		return;
	}

	//打包数据成json串
	Json::Value jValue;
	Json::Value jOneInfo;

	for (unsigned i = 0; i<m_vLotteryRecord.size(); ++i)
	{
		jOneInfo.clear();
		jOneInfo["name"] = m_vLotteryRecord[i].strNickName;
		jOneInfo["id"] = m_vLotteryRecord[i].iLotteryID;

		jValue.append(jOneInfo);
	}

	os << jValue.toStyledString();
	os.close();

	return;
}

//重置重置抽奖信息
void CWorldLimitLotteryManager::ResetLotteryInfo(bool bIsInit)
{
	LimitLotteryInfo stOneInfo;
	int iTimeNow = CTimeUtility::GetNowTime();

	for (int i = LIMIT_LOTTERY_INVALID + 1; i < LIMIT_LOTTERY_MAX; ++i)
	{
		const std::vector<LimitLotteryConfig>* pvConfigs = CConfigManager::Instance()->GetBaseCfgMgr().GetLimitLotteryConfig(i);
		if (!pvConfigs)
		{
			continue;
		}

		if (bIsInit)
		{
			m_avLotteryInfo[i].clear();

			//初始化
			for (unsigned j = 0; j < pvConfigs->size(); ++j)
			{
				if ((*pvConfigs)[j].iLimitType == LIMIT_TYPE_CLOSED)
				{
					//该选项未开放
					continue;
				}

				stOneInfo.Reset();
				stOneInfo.iLotteryID = (*pvConfigs)[j].iID;
				stOneInfo.iWeight = (*pvConfigs)[j].iWeight;
				stOneInfo.iLimitType = (*pvConfigs)[j].iLimitType;
				stOneInfo.iConfigDayLimit = (*pvConfigs)[j].iDayNum;
				stOneInfo.iDailyLimit = (*pvConfigs)[j].iDayNum;
				stOneInfo.iTotalLimit = (*pvConfigs)[j].iTotalNum;

				m_avLotteryInfo[i].push_back(stOneInfo);
			}
		}
		else
		{
			//不是初始化，重置每日限量
			if (CTimeUtility::IsInSameDay(iTimeNow, m_iLastUpdateTime))
			{
				//不是隔天，不更新
				return;
			}

			for (unsigned j = 0; j < m_avLotteryInfo[i].size(); ++j)
			{
				switch (m_avLotteryInfo[i][j].iLimitType)
				{
				case LIMIT_TYPE_DAYLIMIT:
				{
					//每日限制
					m_avLotteryInfo[i][j].iDailyLimit = m_avLotteryInfo[i][j].iConfigDayLimit;
				}
				break;

				case LIMIT_TYPE_LIMITALL:
				{
					//都有限制
					if (m_avLotteryInfo[i][j].iConfigDayLimit < m_avLotteryInfo[i][j].iTotalLimit)
					{
						m_avLotteryInfo[i][j].iDailyLimit = m_avLotteryInfo[i][j].iConfigDayLimit;
					}
					else
					{
						m_avLotteryInfo[i][j].iDailyLimit = m_avLotteryInfo[i][j].iTotalLimit;
					}
				}
				break;

				default:
				{

				}
				break;
				}
				
			}
		}
	}

	m_iLastUpdateTime = iTimeNow;

	return;
}

//一次抽奖，返回抽奖结果
int CWorldLimitLotteryManager::LotteryOneTime(int iLimitType)
{
	if (iLimitType <= LIMIT_LOTTERY_INVALID || iLimitType >= LIMIT_LOTTERY_MAX)
	{
		//非法的类型
		return 0;
	}

	//开始抽奖
	int iRandMax = 0;
	for (unsigned i = 0; i < m_avLotteryInfo[iLimitType].size(); ++i)
	{
		switch (m_avLotteryInfo[iLimitType][i].iLimitType)
		{
		case LIMIT_TYPE_DAYLIMIT:
		case LIMIT_TYPE_LIMITALL:
		{
			//每日限制
			if (m_avLotteryInfo[iLimitType][i].iDailyLimit == 0)
			{
				continue;
			}
		}
		break;

		case LIMIT_TYPE_TOTALLIMIT:
		{
			//总限制
			if (m_avLotteryInfo[iLimitType][i].iTotalLimit == 0)
			{
				continue;
			}
		}
		break;

		case LIMIT_TYPE_UNLIMIT:
		{
			//不限量
		}
		break;

		default:
		{
			//非法的类型
			continue;
		}
		break;
		}

		iRandMax += m_avLotteryInfo[iLimitType][i].iWeight;
	}

	int iRandNum = CRandomCalculator::GetRandomNumberInRange(iRandMax);
	LimitLotteryInfo* pstInfo = NULL;
	for (unsigned i = 0; i < m_avLotteryInfo[iLimitType].size(); ++i)
	{
		switch (m_avLotteryInfo[iLimitType][i].iLimitType)
		{
		case LIMIT_TYPE_DAYLIMIT:
		case LIMIT_TYPE_LIMITALL:
		{
			//每日限制
			if (m_avLotteryInfo[iLimitType][i].iDailyLimit == 0)
			{
				continue;
			}
		}
		break;

		case LIMIT_TYPE_TOTALLIMIT:
		{
			//总限制
			if (m_avLotteryInfo[iLimitType][i].iTotalLimit == 0)
			{
				continue;
			}
		}
		break;

		case LIMIT_TYPE_UNLIMIT:
		{
			//不限量
		}
		break;

		default:
		{
			//非法的类型
			continue;
		}
		break;
		}

		if (iRandNum >= m_avLotteryInfo[iLimitType][i].iWeight)
		{
			iRandNum -= m_avLotteryInfo[iLimitType][i].iWeight;
			continue;
		}

		//找到该信息
		pstInfo = &m_avLotteryInfo[iLimitType][i];
		break;
	}

	if (!pstInfo)
	{
		//找不到奖项
		return 0;
	}

	//扣除限量
	switch (pstInfo->iLimitType)
	{
	case LIMIT_TYPE_DAYLIMIT:
	{
		//每日限制
		--pstInfo->iDailyLimit;
	}
	break;

	case LIMIT_TYPE_TOTALLIMIT:
	{
		//总限制
		--pstInfo->iTotalLimit;
	}
	break;

	case LIMIT_TYPE_LIMITALL:
	{
		//都有限制
		--pstInfo->iDailyLimit;
		--pstInfo->iTotalLimit;
	}
	break;

	case LIMIT_TYPE_UNLIMIT:
	{
		//不限量
	}
	break;

	default:
	{

	}
	break;
	}

	return pstInfo->iLotteryID;
}

//增加抽奖记录
void CWorldLimitLotteryManager::AddLotteryRecord(const std::string& strNickName, const std::vector<int>& vLotteryIDs)
{
	if (vLotteryIDs.size() == 0)
	{
		return;
	}

	RechargeLotteryRecord stOneRecord;
	stOneRecord.strNickName = strNickName;
	for (unsigned i = 0; i < vLotteryIDs.size(); ++i)
	{
		stOneRecord.iLotteryID = vLotteryIDs[i];

		m_vLotteryRecord.insert(m_vLotteryRecord.begin(), stOneRecord);
	}

	if (m_vLotteryRecord.size() > (unsigned)MAX_RECHARGE_LOTTERY_RECORD)
	{
		//删除最后面的
		m_vLotteryRecord.erase(m_vLotteryRecord.begin()+ MAX_RECHARGE_LOTTERY_RECORD, m_vLotteryRecord.end());
	}

	//保存到文件
	SaveRechargeLotterRecord();

	return;
}

//重置
void CWorldLimitLotteryManager::Reset()
{
	for (int i = LIMIT_LOTTERY_INVALID + 1; i < LIMIT_LOTTERY_MAX; ++i)
	{
		m_avLotteryInfo[i].clear();
	}

	m_vLotteryRecord.clear();
}
