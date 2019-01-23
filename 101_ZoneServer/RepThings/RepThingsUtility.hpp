#pragma once

#include "CommDefine.h"

//背包操作工具类
class CGameRoleObj;
class CRepThingsUtility
{
public:

    //增删物品的接口,如果iAddNum小于0表示删除
    static int AddItemNum(CGameRoleObj& stRoleObj, int iItemID, int iAddNum, int iItemChannel);

	//开宝箱
	static int OpenBoxGift(CGameRoleObj& stRoleObj, int iBoxID, int iBoxNum = 1, bool bDeleteItem = false);

	//赠送失败，返还道具给玩家
	static void OnSendGiftFailed(unsigned uiUin, unsigned uiToUin, int iItemID, int iItemNum, int iErrorNum);
};
