#pragma once

#include "LogTargetProxy.hpp"

class CMofangLogProxy : public LogTargetProxy
{
public:

	//写魔方日志
	virtual int WriteLog(DBClientWrapper* pDataBase, char* pQueryBuff, int iThreadIndex, const std::string& strLogData);

private:

	//生成数据库操作串
	int GenerateQueryString(char* pQueryBuff, int& iLength, int iThreadIndex, const std::string& strLogData);

	//往数据库写入日志
	int WriteMofangLog(DBClientWrapper* pDataBase, char* pQueryBuff, int iLength, int iThreadIndex);
};
