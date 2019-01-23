#pragma once

#include "PlatformProxy.h"

//有猫腻账号
class CYmnPlatformProxy : public CPlatformProxy
{
public:
	//构造函数
	CYmnPlatformProxy();

	//平台认证
	virtual int PlatformAuth(const AccountInfo& stInfo, RegAuth_PlatformAuth_Response& stResp, int iThreadIndex);

private:

	//认证账号
	int AuthYmnAccount(const AccountInfo& stInfo, RegAuth_PlatformAuth_Response& stResp, int iThreadIndex);

	//拉取实名认证信息
	void GetRealNameStat(const AccountInfo& stInfo, RegAuth_PlatformAuth_Response& stResp, int iThreadIndex);
};