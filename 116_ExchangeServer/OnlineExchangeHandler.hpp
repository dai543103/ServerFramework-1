#pragma once

#include <string>

#include "MsgHandler.hpp"

//多线程处理Handler
class COnlineExchangeHandler : public CMsgHandler
{
public:
	COnlineExchangeHandler();

	//第三个参数传空
	virtual void OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult = NULL, TNetHead_V2* pstNetHead = NULL);

	//设置线程index
	void SetThreadIdx(int iThreadIndex);

private:

	//在线兑换
	int OnRequestOnlineExchange(SHandleResult& stHandleResult);

	//封装话费充值参数
	std::string PackBillRequestParams(const OnlineExchange& stInfo);

	//处理在线兑换请求
	int ProcessOnlineExchange(unsigned uin, const std::string& strAPI, const std::string& strParams);

	//获取SP订单号
	const std::string GetSpOrderID();

private:

	// 待处理的消息
	GameProtocolMsg* m_pstRequestMsg;

	//线程idnex
	int m_iThreadIndex;
};
