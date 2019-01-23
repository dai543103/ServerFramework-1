#pragma once

#include "LogTargetProxy.hpp"

//TapDB日志代理类

class CTapDBLogProxy : public LogTargetProxy
{
public:
	
	//写日志
	virtual int WriteLog(DBClientWrapper* pDataBase, char* pQueryBuff, int iThreadIndex, const std::string& strLogData);
};
