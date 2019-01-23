#pragma once

#include "GameProtocol.hpp"
#include "CommDefine.h"
#include "RegAuthPublic.hpp"
#include "RegAuthProtocolEngine.hpp"
#include "RegAuthHandlerSet.hpp"

//主线程消息处理
class CRegAuthMsgProcessor
{
public:
	static CRegAuthMsgProcessor* Instance();
	~CRegAuthMsgProcessor();

	//初始化
	int Initialize();

	//接收处理消息
	void ProcessRecvMsg(int& iNewMsgCount);

	//发送到Gateway
	int SendRegAuthToLotus(TNetHead_V2* pNetHead, const GameProtocolMsg& stMsg);
	int SendRegAuthToLotus(unsigned uiSessionFd, const GameProtocolMsg& stMsg);

	//发送到RegAuthDB
	int SendRegAuthToDB(const GameProtocolMsg& stMsg);

	//发送到Platform
	int SendRegAuthToPlatform(const GameProtocolMsg& stMsg);

private:
	CRegAuthMsgProcessor();

	//处理单个消息
	int ProcessOneMsg(int iMsgPeer, unsigned uiInstanceID = 1);

private:

	//实际消息长度
	int m_iCodeLen;

	//发送缓冲区
	unsigned char m_szCodeBuff[MAX_CODE_LEN];

	//协议解析引擎
	CRegAuthProtocolEngine m_stProtocolEngine;

	//协议Handler
	CRegAuthHandlerSet m_stHandlerSet;
};
