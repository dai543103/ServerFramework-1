#pragma once

//运营日志类

#include "GameProtocol.hpp"

enum RegAuthOssType
{
	REGAUTH_OSS_INVALID		= 0,	//非法
	REGAUTH_OSS_REGAUTH		= 1,	//注册认证
};

class CRegAuthOssLog
{
public:

	//玩家认证注册
	static void PlayerRegAuth(const AccountInfo& stInfo, unsigned uin, bool bIsRegister, bool bSuccess);
};
