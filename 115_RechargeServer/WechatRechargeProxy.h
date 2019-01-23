#pragma once

#include <string>
#include <map>

#include "RechargeProxy.h"

//微信充值错误码
enum WechatRechargeErrorNo
{
	WECHAT_RECHARGE_SUCCESS		= 0,		//充值成功
	WECHAT_RECHARGE_SERVER_ERROR= 10001,	//服务内部错误
	WECHAT_RECHARGE_SIGN_ERROR	= 10002,	//非法的签名
	WETCHAT_RECHARGE_TYPE_ERROR	= 10003,	//非法的通知类型
	WECHAT_RECHARGE_PAY_FAILED	= 10004,	//非法的支付状态
	WECHAT_RECHARGE_ORDER_STATUS= 10005,	//非法的订单状态
	WECHAT_RECHARGE_INVALID_NUM	= 10006,	//非法的购买数量
};

class CWechatRechargeProxy : public CRechargeProxy
{
public:

	//充值请求处理
	virtual int OnRechargeReq(const std::string& strURI, unsigned int uiSessionFD, const char* pszCodeBuff, int iCodeLen);

	//充值返回处理
	virtual int OnRechargeResp(const GameProtocolMsg& stMsg);

	//获取服务状态
	void GetServiceStatus(unsigned int uiSessionFD);

	//获取服务接口
	void GetServiceSchema(unsigned int uiSessionFD);

private:

	//获取充值请求字段
	bool GetRechargeToken(const char* pszCodeBuff, const char* pszKey, std::string& strValue);

	//发送充值返回
	int SendWechatRechargeResponse(unsigned int uiSessionFD, int iErrorNum);

	//发送拉取信息返回
	int SendGetUserInfoResponse(unsigned int uiSessionFD, const std::string& strUserID, unsigned uin, const std::string& strNickName);

	//拉取玩家信息
	int GetRechargeUserInfo(unsigned int uiSessionFD, const char* pReqBody);

	//微信公众号充值
	int WechatRecharge(unsigned int uiSessionFD, bool bIsURLEncoded);

private:

	//HTTP请求参数
	std::map<std::string, std::string> m_mReqParams;
};
