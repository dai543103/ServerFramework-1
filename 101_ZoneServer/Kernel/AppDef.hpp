#ifndef __APP_DEF_HPP__
#define __APP_DEF_HPP__

#include "CommDefine.h"

//在这里添加一些宏定义
#ifdef _DEBUG_
	const int SERVER_PRESTOP_TIME = 0;
#else
	const int SERVER_PRESTOP_TIME = 20;
#endif

extern EGameServerStatus g_enServerStatus;

#define APP_CONFIG_FILE "../conf/GameServer.tcm"

#define GM_PRIV_CONFIG_FILE "../conf/GMPrivConfig.xml"

#endif
