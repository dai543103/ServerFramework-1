
#include "GameProtocol.hpp"
#include "ThreadLogManager.hpp"
#include "StringUtility.hpp"
#include "AppDef.hpp"
#include "DBClientWrapper.hpp"

#include "RoleDBHandlerSet.hpp"

int CRoleDBHandlerSet::RegisterAllHandlers()
{
	int iRet = RegisterHandler(MSGID_WORLD_FETCHROLE_REQUEST, &m_FetchRoleHandler);
    if (iRet)
    {
        return -1;
    }
    m_FetchRoleHandler.SetThreadIdx(m_iThreadIdx);

	iRet = RegisterHandler(MSGID_WORLD_UPDATEROLE_REQUEST, &m_UpdateRoleHandler);
    if (iRet)
    {
        return -2;
    }
    m_UpdateRoleHandler.SetThreadIdx(m_iThreadIdx);

	iRet = RegisterHandler(MSGID_WORLD_CREATEROLE_REQUEST, &m_CreateRoleHandler);
    if (iRet)
    {
        return -3;
    }
    m_CreateRoleHandler.SetThreadIdx(m_iThreadIdx);

	iRet = RegisterHandler(MSGID_ACCOUNT_LISTROLE_REQUEST, &m_ListRoleHandler);
    if (iRet)
    {
        return -4;
    }
    m_ListRoleHandler.SetThreadIdx(m_iThreadIdx);

	iRet = RegisterHandler(MSGID_WORLD_USERRECHARGE_REQUEST, &m_RechargeHandler);
	if (iRet)
	{
		return -5;
	}
	m_RechargeHandler.SetThreadIdx(m_iThreadIdx);

	iRet = RegisterHandler(MSGID_WORLD_GAMEMASTER_REQUEST, &m_GameMasterHandler);
	if (iRet)
	{
		return -6;
	}
	m_GameMasterHandler.SetThreadIdx(m_iThreadIdx);

	iRet = RegisterHandler(MSGID_WORLD_SENDMAIL_REQUEST, &m_MailHandler);
	if (iRet)
	{
		return -7;
	}
	m_MailHandler.SetThreadIdx(m_iThreadIdx);

	iRet = RegisterHandler(MSGID_WORLD_GETUSERINFO_REQUEST, &m_RechargeHandler);
	if (iRet)
	{
		return -8;
	}
	m_RechargeHandler.SetThreadIdx(m_iThreadIdx);

    return 0;
}

CRoleDBHandlerSet::CRoleDBHandlerSet() :
	m_FetchRoleHandler(&m_oDBClient, &m_pQueryBuff),
	m_UpdateRoleHandler(&m_oDBClient, &m_pQueryBuff),
	m_CreateRoleHandler(&m_oDBClient, &m_pQueryBuff),
	m_ListRoleHandler(&m_oDBClient, &m_pQueryBuff),
	m_RechargeHandler(&m_oDBClient, &m_pQueryBuff),
	m_GameMasterHandler(&m_oDBClient, &m_pQueryBuff),
	m_MailHandler(&m_oDBClient, &m_pQueryBuff)
{
    
}

CRoleDBHandlerSet::~CRoleDBHandlerSet()
{

}

int CRoleDBHandlerSet::Initialize(int iThreadIndex)
{
	m_iThreadIdx = iThreadIndex;

	//注册Handler
	int iRet = RegisterAllHandlers();
	if (iRet)
	{
		return -1;
	}
	
	//初始化mysql
	m_oDBClient.Init(false);

	//分配缓冲区
	m_pQueryBuff = new char[MAX_QUERYBUFF_SIZE];

    return 0;
}
