#pragma once

#include "Kernel/GameRole.hpp"

//VIP工具类
class CVipUtility
{
public:
	
	//更新救济金信息
	static void UpdateAlmsInfo(CGameRoleObj& stRoleObj, int iAlmsNum, int iNextAlmsTime);

	//触发救济金更新
	static int TriggerAlmsUpdate(CGameRoleObj& stRoleObj);
};
