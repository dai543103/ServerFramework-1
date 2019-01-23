#pragma once

//充值工具类
class CGameRoleObj;
class CRechargeUtility
{
public:

	//增加月卡时间,iAddTime是天数
	static void AddMonthEndTime(CGameRoleObj& stRoleObj, int iAddTime);
};
