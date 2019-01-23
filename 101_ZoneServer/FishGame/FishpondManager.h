#pragma once

#include <map>

#include "FishpondObj.h"

class CFishpondManager
{
public:
	static CFishpondManager* Instance();
	~CFishpondManager();

public:

	//创建鱼池
	CFishpondObj* CreateFishpond(int iFishRoomID);

	//销毁鱼池
	void DelFishpond(CFishpondObj& stFishpondObj);

	//重新加载鱼池配置
	void ReloadFishConfig();

private:
	CFishpondManager();

	//获取空闲鱼池
	CFishpondObj* NewFromFishpondList(const FishRoomConfig& stConfig);

	//增加鱼池到空闲链表
	void AddToFishpondList(const FishRoomConfig& stConfig, CFishpondObj& stFishpondObj);

private:
	
	//空闲鱼池链表
	std::map<int, CFishpondObj*> m_mpFreeHead;
};
