#pragma once

#include "Kernel/HandlerFactory.hpp"
#include "LogAdapter.hpp"
#include "GMPrivConfigManager.hpp"
#include "BaseConfigManager.hpp"
#include "ServerBusConfigManager.h"

using namespace ServerLib;

// 消息最大CD时间 2000ms
const int MAX_MSG_INTERVAL_TIME = 2000;

// 默认不限制CD
const int DEFAULT_MSG_INTERVAL = 0;

//模块开关
enum MODULE_ID
{
	MODULE_FISH_GAME = 0,		//捕鱼模块开关
	MODULE_MAX_ID,
};

class CConfigManager
{
public:
	static CConfigManager* Instance();
	~CConfigManager();

    int Initialize(bool bResumeMode);

    //GM权限配置
	CGMPrivConfigManager& GetGMPrivConfigManager();

	//获取通信BUS管理器
	ServerBusConfigManager& GetBusConfigMgr();

	//获取基础配置管理器
	BaseConfigManager& GetBaseCfgMgr();
	
	int GetMsgInterval(unsigned int uiMsgID);

	//模块是否关闭
	bool IsModuleSwitchOn(MODULE_ID eModuleID);

	//是否检查SessionKey
	bool IsSSKeyCheckEnabled();

	// 是否开启Player日志
	bool IsPlayerLogEnabled();

private:

	//加载服务器配置
	int LoadZoneConfig();

	int LoadMsgInterval();
	int LoadMsgDiableList();

    int PrintConfigMemoryStatics();

private:
	CConfigManager();

private:

    //GM工具权限配置管理器
    CGMPrivConfigManager m_stGMPrivConfigManager;

	//通信BUS配置管理器
	ServerBusConfigManager m_stBusConfigManager;

	//Excel配置管理器
	BaseConfigManager m_stBaseConfigManager;

	//功能模块开关
	int m_aiModuleSwitch[MODULE_MAX_ID];

	//是否检查SessionKey
	int m_iCheckSSKeyEnabled;
	
	//是否打印Player日志
	int m_iIsEnablePlayerLog;
};
