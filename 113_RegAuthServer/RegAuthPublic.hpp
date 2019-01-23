#pragma once

#define APP_CONFIG_FILE     "../conf/GameServer.tcm"

static const int MAX_CODE_LEN = 10240;

static const int MAX_FD_NUMBER = 1000000;

//在这里添加一些宏定义
#ifdef _DEBUG_
const int APP_RegAuth_MAX_SLEEP_USEC = 10;	//线程sleep时间
#else
const int APP_RegAuth_MAX_SLEEP_USEC = 10;	//线程sleep时间
#endif

//平台认证线程池线程数量
static const int PLATFORM_AUTH_THREAD_NUM = 8;
