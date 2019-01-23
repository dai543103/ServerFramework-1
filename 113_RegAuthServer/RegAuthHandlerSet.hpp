#pragma once

#include "MsgHandlerSet.hpp"
#include "RegisterAccountHandler.hpp"
#include "AuthAccountHandler.hpp"
#include "RegAuthDBAddHandler.hpp"
#include "RegAuthDBFetchHandler.hpp"
#include "ClientClosedHandler.hpp"
#include "PlatformAuthHandler.hpp"

using namespace ServerLib;

// RegAuth应用中的消息处理者管理器
class CRegAuthHandlerSet : public CMsgHandlerSet
{
public:
	CRegAuthHandlerSet();

	virtual ~CRegAuthHandlerSet();

	//初始化消息处理管理器
	virtual int Initialize(int iThreadIndex = -1);

private:
	int RegisterAllHandlers();

private:

	//主线程消息
	CRegisterAccountHandler m_stRegisterAccountHandler;
	CRegAuthDBAddHandler m_stRegAuthDBAddHandler;
	CRegAuthDBFetchHandler m_stRegAuthDBFetchHandler;
	CAuthAccountHandler m_stAuthAccountHandler;
	CClientClosedHandler m_stClientClosedHandler;

	//平台认证线程消息
	CPlatformAuthHandler m_stPlatformAuthHandler;

private:

	//如果多线程，为线程index
	int m_iThreadIndex;
};
