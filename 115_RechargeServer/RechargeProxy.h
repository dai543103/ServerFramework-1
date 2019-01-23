#pragma once

#include "GameProtocol.hpp"

//充值基类
class CRechargeProxy
{
public:

	//充值请求处理
	virtual int OnRechargeReq(const std::string& strURI, unsigned int uiSessionFD, const char* pszCodeBuff, int iCodeLen) = 0;

	//充值返回处理
	virtual int OnRechargeResp(const GameProtocolMsg& stMsg) = 0;

protected:

	//发送到对应服务器
	int SendRechargeMsgToWorld(GameProtocolMsg& stMsg, int iWorldID);

protected:

	//发送长度
	int m_iSendLen;

	//发送缓冲区
	char m_szSendBuff[2048];
};
