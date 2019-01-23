#pragma once

//日志目标平台代理类

#include <string>

#include "DBClientWrapper.hpp"

class LogTargetProxy
{
public:

	//写日志
	virtual int WriteLog(DBClientWrapper* pDataBase, char* pQueryBuff, int iThreadIndex, const std::string& strLogData) = 0;
};
