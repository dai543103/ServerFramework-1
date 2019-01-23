#pragma once

#include "CommDefine.h"

//在这里添加一些宏定义
#ifdef _DEBUG_
const int APP_REGAUTHDB_MAX_SLEEP_USEC = 5 * 1000;	//线程sleep时间
#else
const int APP_REGAUTHDB_MAX_SLEEP_USEC = 5 * 1000;	//线程sleep时间
#endif

#define APP_CONFIG_FILE     "../conf/GameServer.tcm"

//连接的MYSQL数据库相关的配置文件
#define REGAUTHDBINFO_CONFIG_FILE "../conf/DBMSConf.xml"

//连接的UniqUinDB数据库相关的配置文件
#define UNIQUINDBINFO_CONFIG_FILE "../conf/DBMSConf_UniqUin.xml"

//玩家帐号数据库的表名
#define MYSQL_ACCOUNTINFO_TABLE "t_accountdata"

//玩家帐号数据表的列数，为: accountID, accountType, uin, password, worldID
#define MYSQL_ACCOUNTINFO_FIELDS   5

//生成玩家唯一UIN的数据库表名
#define MYSQL_UNIQUININFO_TABLE "t_uniquindata"

//消息缓冲区大小
const int MAX_REGAUTHDB_MSGBUFFER_SIZE = 204800;

//数据库操作缓冲区大小
const int MAX_QUERYBUFF_SIZE = 1024;
