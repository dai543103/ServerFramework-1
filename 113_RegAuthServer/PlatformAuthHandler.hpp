#pragma once

#include "MsgHandler.hpp"
#include "PlatformProxy.h"

//多线程处理Handler

class CPlatformAuthHandler : public CMsgHandler
{
public:
	CPlatformAuthHandler();

	//第三个参数传空
	virtual void OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult = NULL, TNetHead_V2* pstNetHead = NULL);

	//设置线程index
	void SetThreadIdx(int iThreadIndex);

private:

	//平台认证
	int OnRequestPlatformAuth(SHandleResult& stHandleResult);

	//获取平台信息
	CPlatformProxy* GetPlatformProxy(int iType);

private:

	// 待处理的消息
	GameProtocolMsg* m_pstRequestMsg;

	//认证平台信息
	CPlatformProxy* m_apPlatformProxy[LOGIN_PLATFORM_MAX];

	//线程idnex
	int m_iThreadIndex;
};
