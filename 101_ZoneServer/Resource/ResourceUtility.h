#pragma once

class CGameRoleObj;

//资源工具类
class CResourceUtility
{
public:

	//增加玩家资源，iAddNum为负表示减少
	static bool AddUserRes(CGameRoleObj& stRoleObj, int iResType, long8 lAddNum);
};
