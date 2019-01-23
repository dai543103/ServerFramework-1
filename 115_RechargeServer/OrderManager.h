#pragma once

#include <vector>
#include <set>
#include <string>

//订单信息
struct OrderData
{
	std::string strOrder;	//订单号
	int iTime;				//处理完成的时间
	unsigned uiUin;			//玩家uin
	int iRechargeID;		//充值金额

	OrderData()
	{
		Reset();
	}

	void Reset()
	{
		strOrder.clear();
		iTime = 0;
		uiUin = 0;
		iRechargeID = 0;
	}
};

class COrderManager
{
public:

	static COrderManager* Instance();
	~COrderManager();

	//初始化
	void Init();

	//增加订单
	void AddOrder(const OrderData& stOrderData, bool bAddAll);

	//删除订单号
	void DeleteOrderID(const std::string& strOrder);

	//订单号是否存在
	bool IsOrderIDExist(const std::string& strOrder);

	//定时器
	void OnTick(int iTimeNow);

private:
	COrderManager();

	//保存到文件
	void SaveOrderInfo();

	//从文件中加载
	void LoadOrderInfo();

	//重置
	void Reset();

private:

	//订单号集合
	std::set<std::string> m_setOrders;

	//订单号列表，时间顺序排列
	std::vector<OrderData> m_vOrderDatas;
};
