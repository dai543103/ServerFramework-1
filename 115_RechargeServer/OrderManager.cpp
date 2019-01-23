
#include <fstream>

#include "json/json.h"
#include "LogAdapter.hpp"

#include "OrderManager.h"

using namespace ServerLib;

const static char* RECHARGE_ORDER_INFO_FILE = "../conf/RechargeOrder.dat";

COrderManager* COrderManager::Instance()
{
	static COrderManager* pInstance = NULL;
	if (!pInstance)
	{
		pInstance = new COrderManager;
	}

	return pInstance;
}

COrderManager::~COrderManager()
{

}


COrderManager::COrderManager()
{
	Reset();
}

//初始化
void COrderManager::Init()
{
	Reset();

	//加载订单详细信息
	LoadOrderInfo();

	return;
}

//增加订单
void COrderManager::AddOrder(const OrderData& stOrderData, bool bAddAll)
{
	if (bAddAll)
	{
		//增加详细信息
		m_vOrderDatas.push_back(stOrderData);

		//保存到文件
		SaveOrderInfo();
	}
	else
	{
		//只增加订单号
		m_setOrders.insert(stOrderData.strOrder);
	}

	return;
}

//删除订单号
void COrderManager::DeleteOrderID(const std::string& strOrder)
{
	//只删除订单号
	m_setOrders.erase(strOrder);

	return;
}

//订单号是否存在
bool COrderManager::IsOrderIDExist(const std::string& strOrder)
{
	return m_setOrders.count(strOrder);
}

//定时器
void COrderManager::OnTick(int iTimeNow)
{
	unsigned i = 0;
	for (; i < m_vOrderDatas.size(); ++i)
	{
		//最多保存3小时的记录
		if (m_vOrderDatas[i].iTime >= (iTimeNow - 3 * 60 * 60))
		{
			break;
		}
	}

	if (i > 0)
	{
		//有删除的记录
		m_vOrderDatas.erase(m_vOrderDatas.begin(), m_vOrderDatas.begin()+i);

		//保存到文件
		SaveOrderInfo();
	}

	return;
}

//保存到文件
void COrderManager::SaveOrderInfo()
{
	//只保存订单详细信息

	//保存订单信息到文件
	std::ofstream os;
	os.open(RECHARGE_ORDER_INFO_FILE, std::ios::binary);
	if (os.fail())
	{
		//打开文件失败
		LOGERROR("Failed to open recharge order file %s\n", RECHARGE_ORDER_INFO_FILE);
		return;
	}

	//打包数据成json串
	Json::Value jValue;

	Json::Value jOneOrder;
	for (unsigned i = 0; i < m_vOrderDatas.size(); ++i)
	{
		jOneOrder.clear();

		jOneOrder["order"] = m_vOrderDatas[i].strOrder;
		jOneOrder["time"] = m_vOrderDatas[i].iTime;
		jOneOrder["uin"] = m_vOrderDatas[i].uiUin;
		jOneOrder["rechargeid"] = m_vOrderDatas[i].iRechargeID;

		jValue.append(jOneOrder);
	}

	os << jValue.toStyledString();
	os.close();

	return;

	
}

//从文件中加载
void COrderManager::LoadOrderInfo()
{
	//加载订单详细信息
	//从文件中读取Rank信息
	std::ifstream is;
	is.open(RECHARGE_ORDER_INFO_FILE, std::ios::binary);
	if (is.fail())
	{
		//打开文件失败
		LOGERROR("Failed to open recharge order file %s\n", RECHARGE_ORDER_INFO_FILE);
		return;
	}

	Json::Reader jReader;
	Json::Value jValue;
	if (!jReader.parse(is, jValue))
	{
		//解析json文件失败
		is.close();
		LOGERROR("Failed to parse rank info file %s\n", RECHARGE_ORDER_INFO_FILE);
		return;
	}

	m_vOrderDatas.clear();
	m_setOrders.clear();

	OrderData stOneOrder;
	for (unsigned i = 0; i < jValue.size(); ++i)
	{
		stOneOrder.Reset();

		stOneOrder.strOrder = jValue[i]["order"].asString();
		stOneOrder.iTime = jValue[i]["time"].asInt();
		stOneOrder.uiUin = jValue[i]["uin"].asUInt();
		stOneOrder.iRechargeID = jValue[i]["rechargeid"].asInt();

		m_setOrders.insert(stOneOrder.strOrder);
		m_vOrderDatas.push_back(stOneOrder);
	}

	is.close();

	return;
}

//重置
void COrderManager::Reset()
{
	m_setOrders.clear();
	m_vOrderDatas.clear();
}
