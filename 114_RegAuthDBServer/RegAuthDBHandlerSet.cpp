
#include "GameProtocol.hpp"
#include "DBClientWrapper.hpp"

#include "RegAuthDBHandlerSet.hpp"

int CRegAuthDBHandlerSet::RegisterAllHandlers()
{
	int iRet = RegisterHandler(MSGID_REGAUTHDB_ADDACCOUNT_REQUEST, &m_stAddRegAuthHandler);
    if (iRet)
    {
        return -1;
    }
    m_stAddRegAuthHandler.SetThreadIdx(m_iThreadIdx);

	iRet = RegisterHandler(MSGID_REGAUTHDB_DELETE_REQUEST, &m_stDeleteRegAuthHandler);
    if (iRet)
    {
        return -2;
    }
    m_stDeleteRegAuthHandler.SetThreadIdx(m_iThreadIdx);

	iRet = RegisterHandler(MSGID_REGAUTHDB_FETCH_REQUEST, &m_stFetchRegAuthHandler);
    if(iRet)
    {
        return -3;
    }
    m_stFetchRegAuthHandler.SetThreadIdx(m_iThreadIdx);

	iRet = RegisterHandler(MSGID_REGAUTH_AUTHACCOUNT_REQUEST, &m_stAuthAccountHandler);
	if (iRet)
	{
		return -5;
	}
	m_stAuthAccountHandler.SetThreadIdx(m_iThreadIdx);

    return 0;
}

CRegAuthDBHandlerSet::CRegAuthDBHandlerSet() :
    m_stAddRegAuthHandler(&m_oDBClient, &m_pQueryBuff),
    m_stDeleteRegAuthHandler(&m_oDBClient, &m_pQueryBuff),
    m_stFetchRegAuthHandler(&m_oDBClient, &m_pQueryBuff),
	m_stAuthAccountHandler(&m_oDBClient, &m_pQueryBuff)
{
    
}

CRegAuthDBHandlerSet::~CRegAuthDBHandlerSet()
{

}

int CRegAuthDBHandlerSet::Initialize(int iThreadIndex)
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
