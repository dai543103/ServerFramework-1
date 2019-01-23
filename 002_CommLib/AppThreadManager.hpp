#pragma once

#include "CommDefine.h"
#include "AppThread.hpp"

class CAppThreadManager
{
public:
	static CAppThreadManager* Instance();
	~CAppThreadManager();

	//初始化线程,pstProtocolEngine和pstHandlerSet线程独享，不能多个线程共用
	int InitOneThread(CProtocolEngine* pstProtocolEngine, CMsgHandlerSet* pstHandlerSet);

	//写线程输入队列
	int PushCode(unsigned int uHashValue, const unsigned char* pMsg, int iCodeLength);

	//轮询写线程输入队列
	int PollingPushCode(const unsigned char* pMsg, int iCodeLength);

	//读线程输出队列
	int PopCode(const int iThreadIdx, unsigned char* pMsg, int iMaxLength, int& riLength);

	CAppThread* GetThread(int iThreadIdx);

	//获取实际线程数量
	int GetThreadNum();

private:
	CAppThreadManager();

private:

	//实际线程数
	int m_iThreadNum;

	CAppThread m_aThreads[MAX_APP_THREAD_NUM];
};
