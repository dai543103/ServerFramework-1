#include "GameProtocol.hpp"
#include "ThreadLogManager.hpp"
#include "StringUtility.hpp"
#include "DBClientWrapper.hpp"

#include "NameDBHandlerSet.hpp"

int CNameDBHandlerSet::RegisterAllHandlers()
{
    int iRet = RegisterHandler(MSGID_ADDNEWNAME_REQUEST, &m_stAddNameHandler);
    if (iRet)
    {
        return -1;
    }
    m_stAddNameHandler.SetThreadIdx(m_iThreadIdx);

	iRet = RegisterHandler(MSGID_DELETENAME_REQUEST, &m_stDeleteNameHandler);
    if (iRet)
    {
        return -2;
    }
    m_stDeleteNameHandler.SetThreadIdx(m_iThreadIdx);

    return 0;
}

CNameDBHandlerSet::CNameDBHandlerSet() :
    m_stAddNameHandler(&m_oDBClient, &m_pQueryBuff),
    m_stDeleteNameHandler(&m_oDBClient, &m_pQueryBuff)
{
    
}

CNameDBHandlerSet::~CNameDBHandlerSet()
{

}

int CNameDBHandlerSet::Initialize(int iThreadIndex)
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
