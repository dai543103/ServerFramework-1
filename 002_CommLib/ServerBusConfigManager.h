#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include "CommDefine.h"
#include "StringUtility.hpp"

using namespace ServerLib;

//服务器通信BUS配置管理器

//通信BUSID
// 默认instance从1开始
// ZoneID为0, 表示属于整个world的服务器
struct SERVERBUSID
{
	unsigned short usWorldID;
	unsigned short usServerID;
	unsigned short usInstanceID;
	unsigned short usZoneID;

	SERVERBUSID()
	{
		Reset();
	}

	void Reset()
	{
		usWorldID = 0;
		usServerID = 0;
		usInstanceID = 1;
		usZoneID = 0;
	}
};

// 生成的ServerID: world:16.zone:16.function:16.instance:16
// 默认instance从1开始
// ZoneID为0, 表示属于整个world的服务器
inline uint64_t GenerateBusID(short iWorldID, EGameServerID enServerID, short iInstance = 1, short iZoneID = 0)
{
	uint64_t ullBusID = ((uint64_t)iWorldID) << 48;
	ullBusID += ((uint64_t)enServerID) << 32;
	ullBusID += ((uint64_t)iInstance) << 16;
	ullBusID += iZoneID;

	return ullBusID;
}

// 生成的ServerID: world:16.zone:16.function:16.instance:16
// 默认instance从1开始
// ZoneID为0, 表示属于整个world的服务器
inline std::string GenerateBusString(short iWorldID, EGameServerID enServerID, short iInstance = 1, short iZoneID = 0)
{
	char szServerBusID[SERVER_BUSID_LEN] = { 0 };
	SAFE_SPRINTF(szServerBusID, sizeof(szServerBusID)-1, "%hu.%hu.%hu.%hu", iWorldID, enServerID, iInstance, iZoneID);

	return std::string(szServerBusID);
}

//根据BusString获取BusID
inline uint64_t GetBusID(const std::string& strBusString)
{
	//计算通信BusID
	unsigned short usWorldID = 0;
	unsigned short usServerID = 0;
	unsigned short usInstanceID = 0;
	unsigned short usZoneID = 0;
	sscanf(strBusString.c_str(), "%hu.%hu.%hu.%hu", &usWorldID, &usServerID, &usInstanceID, &usZoneID);

	return GenerateBusID(usWorldID, (EGameServerID)usServerID, usInstanceID, usZoneID);
}

//根据BusID获取BusString
inline std::string GetBusString(uint64_t ullBusID)
{
	//计算通信BusID
	unsigned short usWorldID = ullBusID>>48;
	unsigned short usServerID = ullBusID >> 32;
	unsigned short usInstanceID = ullBusID >> 16;
	unsigned short usZoneID = ullBusID;

	return GenerateBusString(usWorldID, (EGameServerID)usServerID, usInstanceID, usZoneID);
}

//根据BusID获取BUSID结构体
inline void GetBusStructID(uint64_t ullBusID, SERVERBUSID& stBusID)
{
	//计算通信BusID
	stBusID.usWorldID = ullBusID >> 48;
	stBusID.usServerID = ullBusID >> 32;
	stBusID.usInstanceID = ullBusID >> 16;
	stBusID.usZoneID = ullBusID;

	return;
}

//服务器通信BUS配置
//从 conf/GameServer.tcm 文件中读取服务器通信BUS的配置
struct ServerBusConfig
{
	bool bIsServerEnd;							//ZMQ Server端还是Client端
	uint64_t ullServerBusID;					//ZMQ 通信BUSID
	char szBusAddr[MAX_SERVER_BUS_ADDR_LEN];    //ZMQ通信通道的实际地址，格式为 ip:port

	ServerBusConfig()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

class ServerBusConfigManager
{
public:
	ServerBusConfigManager();
	~ServerBusConfigManager();

	void Reset();

	//加载通信BUS的配置
	int LoadServerBusConfig();

	//获取通信Bus配置,格式为 XX.XX.XX.XX
	//ServerBusID: world:16.zone:16.function:16.instance:16
	const ServerBusConfig* GetServerBus(const char* szServerBusID);

	//获取所有通信BUS的配置
	const std::vector<ServerBusConfig>& GetAllServerBus();

private:
	//内网通信BUS配置
	std::vector<ServerBusConfig> m_vServerBusConfig;
};