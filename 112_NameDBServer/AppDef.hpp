#pragma once

#include "CommDefine.h"

//在这里添加一些宏定义
#ifdef _DEBUG_
const int APP_NAMEDB_MAX_SLEEP_USEC = 5 * 1000;  //线程sleep时间
#else
const int APP_NAMEDB_MAX_SLEEP_USEC = 5 * 1000;	//线程sleep时间
#endif

#define APP_CONFIG_FILE     "../conf/GameServer.tcm"

//连接的MYSQL数据库相关的配置文件
#define NAMEDBINFO_CONFIG_FILE "../conf/DBMSConf.xml"

//名字数据库的表名
#define MYSQL_NAMEINFO_TABLE "t_nameinfo"

//名字数据库的列数定义: nickname, nametype, nameid
#define MYSQL_NAMEINFO_FIELDS   3

const int MAX_NAMEDB_MSGBUFFER_SIZE = 204800;

//数据库操作缓冲区大小
const int MAX_QUERYBUFF_SIZE = 1024;
