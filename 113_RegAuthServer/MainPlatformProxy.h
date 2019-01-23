#pragma once

#include "PlatformProxy.h"

//游戏自建账号
class CMainPlatformProxy : public CPlatformProxy
{
public:
	
	//平台认证
	virtual int PlatformAuth(const AccountInfo& stInfo, RegAuth_PlatformAuth_Response& stResp, int iThreadIndex);
};