#pragma once

#include "GameProtocol.hpp"
#include "CommDefine.h"
#include "ExchangePublic.hpp"
#include "ExchangeProtocolEngine.hpp"
#include "ExchangeHandlerSet.hpp"

//主线程消息处理
class CExchangeMsgProcessor
{
public:
	static CExchangeMsgProcessor* Instance();
	~CExchangeMsgProcessor();

	//初始化
	int Initialize();

	//接收处理消息
	void ProcessRecvMsg(int& iNewMsgCount);

private:
	CExchangeMsgProcessor();

private:

	//实际消息长度
	int m_iCodeLen;

	//发送缓冲区
	unsigned char m_szCodeBuff[MAX_CODE_LEN];
};
