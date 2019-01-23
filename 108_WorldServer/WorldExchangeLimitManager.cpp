#include <algorithm>
#include <fstream>

#include "json/json.h"
#include "GameProtocol.hpp"
#include "ErrorNumDef.hpp"
#include "LogAdapter.hpp"
#include "TimeUtility.hpp"
#include "WorldMsgHelper.hpp"
#include "ConfigManager.hpp"

#include "WorldExchangeLimitManager.h"

using namespace ServerLib;

const static char* WORLD_EXCHANGE_LIMIT_FILE = "../conf/ExchangeLimit.dat";
const static char* WORLD_EXCHANGE_RECORD_FILE = "../conf/ExchangeRecord.dat";

CWorldExchangeLimitManager* CWorldExchangeLimitManager::Instance()
{
	static CWorldExchangeLimitManager* pInstance = NULL;
	if (!pInstance)
	{
		pInstance = new CWorldExchangeLimitManager;
	}

	return pInstance;
}

CWorldExchangeLimitManager::CWorldExchangeLimitManager()
{
	Reset();
}

CWorldExchangeLimitManager::~CWorldExchangeLimitManager()
{
	Reset();
}

//初始化
void CWorldExchangeLimitManager::Init()
{
	Reset();

	//加载限量信息
	LoadExchangeLimit();

	//加载兑换记录
	LoadExchangeRecord();

	return;
}

//拉取限量信息
void CWorldExchangeLimitManager::GetExchangeLimit(Zone_GetLimitNum_Response& stResp)
{
	for (unsigned i = 0; i < m_vWorldExchangeLimits.size(); ++i)
	{
		LimitInfo* pstOneInfo = stResp.add_stlimits();
		pstOneInfo->set_iexchangeid(m_vWorldExchangeLimits[i].iExchangeID);
		pstOneInfo->set_inum(m_vWorldExchangeLimits[i].iLimitNum);
	}

	return;
}

//修改限量信息
int CWorldExchangeLimitManager::UpdateExchangeLimit(int iExchangeID, int iAddNum)
{
	//先查找兑换信息
	for (unsigned i = 0; i < m_vWorldExchangeLimits.size(); ++i)
	{
		if (m_vWorldExchangeLimits[i].iExchangeID != iExchangeID)
		{
			continue;
		}

		if ((m_vWorldExchangeLimits[i].iLimitNum + iAddNum) < 0)
		{
			//不能修改
			return T_WORLD_PARA_ERROR;
		}

		m_vWorldExchangeLimits[i].iLimitNum += iAddNum;

		//保存到文件
		SaveExchangeLimit();

		return T_SERVER_SUCCESS;
	}

	//找不到则添加
	if (iAddNum < 0)
	{
		return T_WORLD_PARA_ERROR;
	}

	ExchangeLimit stLimitInfo;
	stLimitInfo.iExchangeID = iExchangeID;
	stLimitInfo.iLimitNum = iAddNum;

	m_vWorldExchangeLimits.push_back(stLimitInfo);

	//保存到文件
	SaveExchangeLimit();

	return T_SERVER_SUCCESS;
}

//增加兑换记录
void CWorldExchangeLimitManager::AddExchangeRec(const ExchangeRec& stRecord)
{
	UserExchangeRec stExchangeRec;
	stExchangeRec.strName = stRecord.strname();
	stExchangeRec.iExchangeID = stRecord.iexchangeid();
	stExchangeRec.iTime = stRecord.itime();

	//加在最前面
	m_vUserExchangeRecs.insert(m_vUserExchangeRecs.begin(), stExchangeRec);

	
	if (m_vUserExchangeRecs.size() > MAX_EXCHANGE_RECORD_NUM)
	{
		m_vUserExchangeRecs.resize(MAX_EXCHANGE_RECORD_NUM);
	}

	//保存到文件
	SaveExchangeRecord();

	return;
}

//拉取兑换记录
int CWorldExchangeLimitManager::GetExchangeRec(int iFromIndex, int iNum, Zone_GetExchangeRec_Response& stResp)
{
	if (iFromIndex <= 0 || iNum <= 0)
	{
		return T_WORLD_PARA_ERROR;
	}

	for (int i = (iFromIndex-1); i < (iFromIndex-1 + iNum) && i < (int)m_vUserExchangeRecs.size(); ++i)
	{
		ExchangeRec* pstOneRec = stResp.add_strecords();
		pstOneRec->set_strname(m_vUserExchangeRecs[i].strName);
		pstOneRec->set_iexchangeid(m_vUserExchangeRecs[i].iExchangeID);
		pstOneRec->set_itime(m_vUserExchangeRecs[i].iTime);
	}

	return T_SERVER_SUCCESS;
}

//加载限量信息
void CWorldExchangeLimitManager::LoadExchangeLimit()
{
	//从文件中读取Rank信息
	std::ifstream is;
	is.open(WORLD_EXCHANGE_LIMIT_FILE, std::ios::binary);
	if (is.fail())
	{
		//限量信息不存在，从配置读取
		LoadLimitInfoFromConfig();

		return;
	}

	Json::Reader jReader;
	Json::Value jValue;
	if (!jReader.parse(is, jValue))
	{
		//解析json文件失败
		is.close();
		LOGERROR("Failed to parse exchange limit file %s\n", WORLD_EXCHANGE_LIMIT_FILE);
		return;
	}

	ExchangeLimit stOneInfo;
	for (unsigned i = 0; i < jValue.size(); ++i)
	{
		stOneInfo.Reset();
		stOneInfo.iExchangeID = jValue[i]["id"].asInt();
		stOneInfo.iLimitNum = jValue[i]["num"].asInt();

		m_vWorldExchangeLimits.push_back(stOneInfo);
	}

	is.close();

	return;
}

//保存限量信息
void CWorldExchangeLimitManager::SaveExchangeLimit()
{
	//保存Rank信息到文件
	std::ofstream os;
	os.open(WORLD_EXCHANGE_LIMIT_FILE, std::ios::binary);
	if (os.fail())
	{
		//打开文件失败
		LOGERROR("Failed to open exchange limit file %s\n", WORLD_EXCHANGE_LIMIT_FILE);
		return;
	}

	//打包数据成json串
	Json::Value jValue;
	Json::Value jOneInfo;

	for(unsigned i=0; i<m_vWorldExchangeLimits.size(); ++i)
	{
		jOneInfo.clear();
		jOneInfo["id"] = m_vWorldExchangeLimits[i].iExchangeID;
		jOneInfo["num"] = m_vWorldExchangeLimits[i].iLimitNum;

		jValue.append(jOneInfo);
	}

	os << jValue.toStyledString();
	os.close();

	return;
}

//加载兑换记录
void CWorldExchangeLimitManager::LoadExchangeRecord()
{
	//从文件中读取记录信息
	std::ifstream is;
	is.open(WORLD_EXCHANGE_RECORD_FILE, std::ios::binary);
	if (is.fail())
	{
		return;
	}

	Json::Reader jReader;
	Json::Value jValue;
	if (!jReader.parse(is, jValue))
	{
		//解析json文件失败
		is.close();
		LOGERROR("Failed to parse exchange record file %s\n", WORLD_EXCHANGE_RECORD_FILE);
		return;
	}

	UserExchangeRec stOneRec;
	for (unsigned i = 0; i < jValue.size(); ++i)
	{
		stOneRec.Reset();
		stOneRec.iExchangeID = jValue[i]["id"].asInt();
		stOneRec.strName = jValue[i]["name"].asString();
		stOneRec.iTime = jValue[i]["time"].asInt();

		m_vUserExchangeRecs.push_back(stOneRec);
	}

	is.close();

	return;
}

//保存兑换记录
void CWorldExchangeLimitManager::SaveExchangeRecord()
{
	//保存Rank信息到文件
	std::ofstream os;
	os.open(WORLD_EXCHANGE_RECORD_FILE, std::ios::binary);
	if (os.fail())
	{
		//打开文件失败
		LOGERROR("Failed to open exchange record file %s\n", WORLD_EXCHANGE_RECORD_FILE);
		return;
	}

	//打包数据成json串
	Json::Value jValue;
	Json::Value jOneInfo;

	for (unsigned i = 0; i<m_vUserExchangeRecs.size(); ++i)
	{
		jOneInfo.clear();
		jOneInfo["id"] = m_vUserExchangeRecs[i].iExchangeID;
		jOneInfo["name"] = m_vUserExchangeRecs[i].strName;
		jOneInfo["time"] = m_vUserExchangeRecs[i].iTime;

		jValue.append(jOneInfo);
	}

	os << jValue.toStyledString();
	os.close();

	return;
}

//限量信息不存在，从配置读取
void CWorldExchangeLimitManager::LoadLimitInfoFromConfig()
{
	//获取限量配置
	std::vector<ExchangeConfig> vConfigs;
	CConfigManager::Instance()->GetBaseCfgMgr().GetExchangeConfig(true, vConfigs);

	//增加限量信息
	ExchangeLimit stOneLimit;
	for (unsigned i = 0; i < vConfigs.size(); ++i)
	{
		stOneLimit.Reset();
		stOneLimit.iExchangeID = vConfigs[i].iID;
		stOneLimit.iLimitNum = vConfigs[i].iLimitNum;

		m_vWorldExchangeLimits.push_back(stOneLimit);
	}

	return;
}

//重置
void CWorldExchangeLimitManager::Reset()
{
	m_vWorldExchangeLimits.clear();
	m_vUserExchangeRecs.clear();
}
