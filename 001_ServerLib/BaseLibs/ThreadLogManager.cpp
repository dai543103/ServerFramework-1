#include <stdlib.h>
#include "StringUtility.hpp"

#include "ThreadLogManager.hpp"

using namespace ServerLib;

CThreadLogManager* CThreadLogManager::Instance()
{
	static CThreadLogManager* pInstance = NULL;
	if (!pInstance)
	{
		pInstance = new CThreadLogManager;
	}

	return pInstance;
}

CThreadLogManager::CThreadLogManager()
{
	m_iLogNum = 0;
}

CThreadLogManager::~CThreadLogManager()
{
	m_iLogNum = 0;
}

int CThreadLogManager::Initialize(const int iLogNum, const TLogConfig& rLogConfig)
{
	if (iLogNum <= 0)
	{
		return -1;
	}

	m_iLogNum = iLogNum;
	char szBaseName[sizeof(rLogConfig.m_szBaseName)];
	for (int i = 0; i < m_iLogNum; i++)
	{
		TLogConfig stLogConfig = rLogConfig;
		SAFE_STRCPY(szBaseName, stLogConfig.m_szBaseName, sizeof(stLogConfig.m_szBaseName));
		//加上线程的index到日志名字的末尾
		SAFE_SPRINTF(stLogConfig.m_szBaseName, sizeof(stLogConfig.m_szBaseName), "%s%d", szBaseName, i);

		m_astLogAdapter[i].ReloadLogConfig(stLogConfig);
	}

	return 0;
}

CServerLogAdapter* CThreadLogManager::GetLogAdapter(const int iThreadIdx)
{
	if (iThreadIdx < 0 || iThreadIdx >= m_iLogNum)
	{
		return NULL;
	}

	return &m_astLogAdapter[iThreadIdx];
}
