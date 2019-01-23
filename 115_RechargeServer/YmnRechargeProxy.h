#pragma once

#include <string>
#include <map>

#include "RechargeProxy.h"

class CYmnRechargeProxy : public CRechargeProxy
{
public:

	//充值请求处理
	virtual int OnRechargeReq(const std::string& strURI, unsigned int uiSessionFD, const char* pszCodeBuff, int iCodeLen);

	//充值返回处理
	virtual int OnRechargeResp(const GameProtocolMsg& stMsg);

private:

	//获取充值请求字段
	bool GetRechargeToken(const char* pszCodeBuff, const char* pszKey, std::string& strValue);

	//发送返回
	int SendRechargeResponseToClient(unsigned int uiSessionFD, bool bSuccess);

private:

	//HTTP请求参数
	std::map<std::string, std::string> m_mReqParams;
};
