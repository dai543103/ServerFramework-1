#pragma once

#include "CommDefine.h"

//在这里添加一些宏定义
#ifdef _DEBUG_
const int APP_ROLEDB_MAX_SLEEP_USEC = 5000;	//线程sleep时间
#else
const int APP_ROLEDB_MAX_SLEEP_USEC = 5000;	//线程sleep时间
#endif

#define APP_CONFIG_FILE     "../conf/GameServer.tcm"

//连接的MYSQL数据库相关的配置文件
#define ROLEDBINFO_CONFIG_FILE "../conf/DBMSConf.xml"

//玩家角色数据表的表名
#define MYSQL_USERINFO_TABLE "t_userdata"

//玩家角色数据表的列数,详细字段参见建表语句
#define MYSQL_USERINFO_FIELDS   9

//数据字段类型
enum RoleDBDataType
{
	ROLEDB_DATA_INVALID = 0,	//非法的字段
	ROLEDB_DATA_BASEINFO = 1,	//基础信息
	ROLEDB_DATA_ITEMINFO = 2,	//道具信息
	ROLEDB_DATA_QUESTINFO = 3,	//任务信息
	ROLEDB_DATA_MAILINFO = 4,	//邮件信息
	ROLEDB_DATA_OFFLINE = 5,	//离线信息
	ROLEDB_DATA_RESERVED2 = 6,	//保留信息
	ROLEDB_DATA_MAX,			//字段数量最大，程序用
};

//缓冲区大小
const int MAX_ROLEDB_MSGBUFFER_SIZE = 2048000;

//查询缓冲区大小
const int MAX_QUERYBUFF_SIZE = 20480;
