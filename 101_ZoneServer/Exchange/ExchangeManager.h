#pragma once

//玩家兑换管理器

#include <string>
#include <vector>

#include "CommDefine.h"
#include "GameProtocol.hpp"
#include "Kernel/ConfigManager.hpp"

//兑换弹头ID
static const int EXCHANGE_WARHEAD_ID = 3;
//兑换5元 10元ID
static const int EXCHANGE_FIVEBILL_ID = 50003;
static const int EXCHANGE_TENBILL_ID = 50004;

enum PersonLimitType
{
	PERSONLIMIT_TYPE_INVALID = 0,	//非法的初始化类型
	PERSONLIMIT_TYPE_FIVEBILL = 1,	//初始化5元话费信息
	PERSONLIMIT_TYPE_TENBILL = 2,	//初始化10元话费信息
};

//个人兑换信息
struct UserExchangeData
{
	std::string strName;	//兑换名字
	std::string strPhone;	//兑换电话
	std::string strMailNum;	//兑换邮编
	std::string strAddress;	//兑换地址
	std::string strRemarks;	//玩家备注
	std::string strQQNum;	//玩家QQ号
	int iLastSetTime;		//上次设置时间
	unsigned int auPersonLimit[2];	//玩家个人限制

	UserExchangeData()
	{
		Reset();
	}

	void Reset()
	{
		strName.clear();
		strPhone.clear();
		strMailNum.clear();
		strAddress.clear();
		strRemarks.clear();
		strQQNum.clear();

		iLastSetTime = 0;
		memset(auPersonLimit, 0, sizeof(auPersonLimit));
	}
};

//个人订单信息
struct OrderData
{
	int iExchangeID;		//兑换的ID
	std::string strOrderID;	//订单号
	std::string strPhone;	//兑换手机号
	int iExchangeTime;		//兑换时间
	int iStat;				//订单状态

	OrderData()
	{
		Reset();
	}

	void Reset()
	{
		iExchangeID = 0;
		strOrderID.clear();
		strPhone.clear();
		iExchangeTime = 0;
		iStat = 0;
	}
};

class CGameRoleObj;
class CExchangeManager
{
public:
	CExchangeManager();
	~CExchangeManager();

public:

	//初始化
	int Initialize();

	void SetOwner(unsigned int uin);
	CGameRoleObj* GetOwner();

	//设置兑换信息
	int SetUserExchangeInfo(const ExchangeUser& stUserInfo, Zone_SetExchange_Response& stResp);

	//玩家兑换
	int ExchangeItem(const ExchangeConfig& stConfig, ExchangeOrder& stOrderInfo);

	//扣除兑换消耗
	int DoExchangeCost(const ExchangeConfig& stConfig, bool bReturn = false);

	//用户信息是否已设置
	bool IsUserInfoSet();

	//初始化个人限量信息
	unsigned GetPersonLimitInfo(int iLimitType);

	//是否允许个人限量兑换
	bool CheckIsLimit(int iLimitType);

	//兑换后处理个人兑换限量
	void SetPersonLimit(int iLimitType, unsigned iNum);

	//发送拉取卡密的请求
	static int SendGetCardNoRequest(unsigned uin, const ExchangeConfig& stConfig);

	//兑换信息数据库操作函数
	void UpdateExchangeToDB(EXCHANGEDBINFO& stExchangeInfo);
	void InitExchangeFromDB(const EXCHANGEDBINFO& stExchangeInfo);

private:

	//获取订单号
	std::string GetOrderID(int iTimeNow);

private:
	
	//玩家uin
	unsigned m_uiUin;

	//兑换信息
	UserExchangeData m_stExchangeData;

	//兑换订单信息
	std::vector<OrderData> m_vOrderData;
};
