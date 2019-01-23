#pragma once

#include <pthread.h>

#include "GameProtocol.hpp"
#include "CodeQueue.hpp"
#include "ProtocolEngine.hpp"
#include "MsgHandlerSet.hpp"

using namespace ServerLib;

//在这里添加一些宏定义
#ifdef _DEBUG_
const int APP_THREAD_MAX_SLEEP_USEC = 5 * 1000;    //线程sleep时间
#else
const int APP_THREAD_MAX_SLEEP_USEC = 5 * 1000;	//线程sleep时间
#endif

static const int MAX_MSGBUFFER_SIZE = 204800;		//缓冲区长度

//App线程
class CAppThread
{
public:
    int Initialize(int iThreadIdx, CProtocolEngine* pstProtocolEngine, CMsgHandlerSet* pstHandlerSet);

	int Run();

	int GetIdx() const { return m_iThreadIdx; }

private:

	int InitCodeQueue(const int iThreadIdx);

	//接收消息
	int ReceiveMsg(int& riCodeLength);
	
	//处理消息
	int ProcessMsg(SHandleResult& stMsgHandleResult);

	//压入返回消息
	int EncodeAndSendMsg(SHandleResult& stMsgHandleResult);

public:
	//压入返回
	int PushCode(const unsigned char* pMsg, int iCodeLength);

	//接收消息
	int PopCode(unsigned char* pMsg, int iMaxLength, int& riLength);

private:
	int CreateThread();

private:
    CProtocolEngine* m_pstProtocolEngine;
    CMsgHandlerSet* m_pstHandlerSet;

	CCodeQueue* m_pInputQueue;	//输入消息队列，来自内部服务器，由主线程分发
	CCodeQueue* m_pOutputQueue; //输出消息队列
	int m_iThreadIdx;			//线程index

	pthread_t m_hTrd;
	pthread_attr_t m_stAttr;

    GameProtocolMsg m_stGameMsg;
    unsigned char m_szCodeBuf[MAX_MSGBUFFER_SIZE];
};
