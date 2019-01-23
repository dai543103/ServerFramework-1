#pragma once

#include "GameProtocol.hpp"

class CPlatformProxy
{
public:

	//平台认证
	virtual int PlatformAuth(const AccountInfo& stInfo, RegAuth_PlatformAuth_Response& stResp, int iThreadIndex) = 0;
};