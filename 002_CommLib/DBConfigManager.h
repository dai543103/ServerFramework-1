#pragma once

//读取DB配置信息

#include <vector>
#include <string.h>

struct OneDBInfo
{
	int iDBMajorVersion;    //使用的数据库的主版本号
	int iDBMinVersion;      //使用的数据库的子版本号
	char szDBEngine[64];    //使用的DB引擎的名字
	char szDBHost[64];      //连接数据库的HOST
	char szUserName[64];    //连接数据库的用户名
	char szUserPasswd[64];  //连接数据库的密码
	char szDBName[64];      //连接的数据库的DB表名

	OneDBInfo()
	{
		Reset();
	};

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

class DBConfigManager
{
public:
	DBConfigManager();
	~DBConfigManager();

	int LoadDBConfig(const char* pszFilePath);

	const OneDBInfo* GetDBConfigByIndex(int iDBIndex);

private:

	std::vector<OneDBInfo> m_vDBConfigInfos;
};
