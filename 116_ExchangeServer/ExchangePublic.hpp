#pragma once

#define APP_CONFIG_FILE     "../conf/GameServer.tcm"

//连接的MYSQL数据库相关的配置文件
#define CARDNODBINFO_CONFIG_FILE "../conf/DBMSConf.xml"

//卡密的数据库表
#define MYSQL_CARDNO_TABLE "t_cardno_info"

static const int MAX_CODE_LEN = 10240;

static const int MAX_FD_NUMBER = 1000000;

//工作线程数量
static const int WORK_THREAD_NUM = 8;

//数据库操作缓冲区大小
const int MAX_QUERYBUFF_SIZE = 2048;
