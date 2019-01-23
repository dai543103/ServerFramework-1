#pragma once

#include <vector>
#include "GameProtocol.hpp"
#include "CommDefine.h"

//兑换记录的上限
static const unsigned int MAX_EXCHANGE_RECORD_NUM = 100;

//限量兑换
struct ExchangeLimit
{
	int iExchangeID;		//兑换ID
	int iLimitNum;			//限量剩余数量

	ExchangeLimit()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

//兑换记录
struct UserExchangeRec
{
	std::string strName;
	int iExchangeID;
	int iTime;

	UserExchangeRec()
	{
		Reset();
	}

	void Reset()
	{
		strName.c_str();
		iExchangeID = 0;
		iTime = 0;
	}
};

class CWorldExchangeLimitManager
{
public:
	static CWorldExchangeLimitManager* Instance();

	~CWorldExchangeLimitManager();

	//初始化
	void Init();

	//拉取限量信息
	void GetExchangeLimit(Zone_GetLimitNum_Response& stResp);

	//修改限量信息
	int UpdateExchangeLimit(int iExchangeID, int iAddNum);

	//增加兑换记录
	void AddExchangeRec(const ExchangeRec& stRecord);

	//拉取兑换记录
	int GetExchangeRec(int iFromIndex, int iNum, Zone_GetExchangeRec_Response& stResp);

private:

	CWorldExchangeLimitManager();

	//加载限量信息
	void LoadExchangeLimit();

	//保存限量信息
	void SaveExchangeLimit();

	//加载兑换记录
	void LoadExchangeRecord();

	//保存兑换记录
	void SaveExchangeRecord();

	//限量信息不存在，从配置读取
	void LoadLimitInfoFromConfig();

	//重置
	void Reset();

private:

	//限量兑换信息
	std::vector<ExchangeLimit> m_vWorldExchangeLimits;

	//限量兑换记录
	std::vector<UserExchangeRec> m_vUserExchangeRecs;
};
