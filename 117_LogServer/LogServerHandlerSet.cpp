#include "GameProtocol.hpp"
#include "ThreadLogManager.hpp"
#include "StringUtility.hpp"
#include "DBClientWrapper.hpp"

#include "LogServerHandlerSet.hpp"

int CLogServerHandlerSet::RegisterAllHandlers()
{
    int iRet = RegisterHandler(MSGID_WORLD_WRITELOG_REQUEST, &m_stWriteLogHandler);
    if (iRet)
    {
        return -1;
    }
	m_stWriteLogHandler.SetThreadIdx(m_iThreadIdx);

    return 0;
}

CLogServerHandlerSet::CLogServerHandlerSet() :
    m_stWriteLogHandler(&m_oDBClient, &m_pQueryBuff)
{
    
}

CLogServerHandlerSet::~CLogServerHandlerSet()
{

}

int CLogServerHandlerSet::Initialize(int iThreadIndex)
{
	m_iThreadIdx = iThreadIndex;

	//注册Handler
	int iRet = RegisterAllHandlers();
	if (iRet)
	{
		return -1;
	}

	//初始化DB
	m_oDBClient.Init(true);

	//DB操作缓冲区
	m_pQueryBuff = new char[MAX_QUERYBUFF_SIZE];

    return 0;
}
