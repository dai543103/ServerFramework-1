#pragma once

#include <vector>

#include "GameProtocol.hpp"
#include "CommDefine.h"
#include "Kernel/GameRole.hpp"

//玩家充值记录
struct PayRecordData
{
	int iTime;			//充值时间
	int iRechargeID;	//充值ID

	PayRecordData()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

class CRechargeManager
{
public:
	CRechargeManager();
	~CRechargeManager();

public:
	//初始化
	int Initialize();

	void SetOwner(unsigned int uin);
	CGameRoleObj* GetOwner();

	//玩家充值
	int UserRecharge(int iRechargeID, int iTime, const std::string& strOrderID);

	//玩家拉取记录
	void GetPayRecord(Zone_GetPayRecord_Response& stResp);

	//玩家领取充值礼包
	int GetPayGift(int iGiftType);

	//更新充值到DB
	void UpdateRechargeToDB(RECHARGEDBINFO& stInfo);

	//从DB加载充值
	void InitRechargeFromDB(const RECHARGEDBINFO& stInfo);
private:

	//增加玩家充值记录
	void AddRechargeRecord(int iRechargeID, int iTime);

	//增加VIP经验
	void AddVipExp(int iAddExp);

	void Reset();

private:
	//玩家uin
	unsigned m_uiUin;

	//玩家首冲礼包状态
	int m_iFirstRewardStat;

	//玩家已充值金币项ID
	std::vector<int> m_vRechargeIDs;

	//玩家充值记录
	std::vector<PayRecordData> m_vRecords;
};
