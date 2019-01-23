#pragma once

#include "CommDefine.h"

//在这里添加一些宏定义
#ifdef _DEBUG_
const int APP_LOGSERVER_MAX_SLEEP_USEC = 5 * 1000;  //线程sleep时间
#else
const int APP_LOGSERVER_MAX_SLEEP_USEC = 5 * 1000;	//线程sleep时间
#endif

#define APP_CONFIG_FILE     "../conf/GameServer.tcm"

//连接的MYSQL数据库相关的配置文件
#define MOFANGDBINFO_CONFIG_FILE "../conf/DBMSConf.xml"

//日志数据库的登录表
#define MYSQL_MOFANGLOGIN_TABLE "t_mofang_login"

//日志数据库的充值表
#define MYSQL_MOFANGPAY_TABLE "t_mofang_pay"

//日志数据库的玩家结算表
#define MYSQL_TALLYINFO_TABLE "t_tally_info"

const int MAX_LOGSERVER_MSGBUFFER_SIZE = 204800;

//数据库操作缓冲区大小
const int MAX_QUERYBUFF_SIZE = 2048;
