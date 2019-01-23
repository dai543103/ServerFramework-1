
#include "GameProtocol.hpp"
#include "CommonDef.hpp"

#include "RegAuthHandlerSet.hpp"

using namespace ServerLib;

int CRegAuthHandlerSet::RegisterAllHandlers()
{
    //注册创建平台帐号的Handler
    int iRet = RegisterHandler(MSGID_REGAUTH_REGACCOUNT_REQUEST, &m_stRegisterAccountHandler, EKMT_CLIENT);
    if (iRet)
    {
        return -1;
    }

	iRet = RegisterHandler(MSGID_REGAUTHDB_ADDACCOUNT_RESPONSE, &m_stRegAuthDBAddHandler, EKMT_CLIENT);
    if(iRet)
    {
        return -2;
    }

	iRet = RegisterHandler(MSGID_REGAUTHDB_FETCH_RESPONSE, &m_stRegAuthDBFetchHandler, EKMT_CLIENT);
    if(iRet)
    {
        return -3;
    }

    //注册认证平台帐号的Handler
	iRet = RegisterHandler(MSGID_REGAUTH_AUTHACCOUNT_REQUEST, &m_stAuthAccountHandler, EKMT_CLIENT);
    if(iRet)
    {
        return -4;
    }

	iRet = RegisterHandler(MSGID_LOGOUTSERVER_REQUEST, &m_stClientClosedHandler, EKMT_CLIENT);
    if (iRet)
    {
        return -7;
    }

	iRet = RegisterHandler(MSGID_LOGOUTSERVER_RESPONSE, &m_stClientClosedHandler, EKMT_CLIENT);
    if (iRet)
    {
        return -8;
    }

	//平台认证返回Handler
	iRet = RegisterHandler(MSGID_REGAUTH_PLATFORMAUTH_RESPONSE, &m_stAuthAccountHandler);
	if (iRet)
	{
		return -9;
	}

	//RegAuthDB认证返回Handler
	iRet = RegisterHandler(MSGID_REGAUTH_AUTHACCOUNT_RESPONSE, &m_stAuthAccountHandler);
	if (iRet)
	{
		return -10;
	}

	//多线程Handler
	iRet = RegisterHandler(MSGID_REGAUTH_PLATFORMAUTH_REQUEST, &m_stPlatformAuthHandler);
	if (iRet)
	{
		return -11;
	}
	m_stPlatformAuthHandler.SetThreadIdx(m_iThreadIndex);

    return 0;
}

CRegAuthHandlerSet::CRegAuthHandlerSet()
{

}

CRegAuthHandlerSet::~CRegAuthHandlerSet()
{
}

int CRegAuthHandlerSet::Initialize(int iThreadIndex)
{
	m_iThreadIndex = iThreadIndex;

	int iRet = RegisterAllHandlers();
	if (iRet)
	{
		return iRet;
	}

	return 0;
}

