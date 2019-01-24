#include <unistd.h>
#include "GameProtocol.hpp"
#include "StringUtility.hpp"
#include "ThreadLogManager.hpp"

#include "AppThread.hpp"

void* ThreadProc(void *pvArgs)
{
	if (!pvArgs)
	{
		return NULL;
	}

	CAppThread *pThread = (CAppThread *)pvArgs;
	pThread->Run();

	return NULL;
}

int CAppThread::Initialize(int iThreadIdx, CProtocolEngine* pstProtocolEngine, CMsgHandlerSet* pstHandlerSet)
{
	if (!pstProtocolEngine || !pstHandlerSet)
	{
		return -1;
	}

	m_iThreadIdx = iThreadIdx;
	m_pstProtocolEngine = pstProtocolEngine;
	m_pstHandlerSet = pstHandlerSet;

	//初始化CodeQueue
	int iRet = InitCodeQueue(iThreadIdx);
	if (iRet)
	{
		return -3;
	}

	iRet = m_pstHandlerSet->Initialize(iThreadIdx);
	if (iRet)
	{
		return -4;
	}

	//创建线程
	iRet = CreateThread();
	if (iRet)
	{
		return -5;
	}

	return 0;
}

int CAppThread::CreateThread()
{
	int iRet = pthread_attr_init(&m_stAttr);
	if (iRet)
	{
		TRACE_THREAD(m_iThreadIdx, "Error: pthread_attr_init fail. rt:%d\n", iRet);
		return -1;
	}

	iRet = pthread_attr_setscope(&m_stAttr, PTHREAD_SCOPE_SYSTEM);
	if (iRet)
	{
		TRACE_THREAD(m_iThreadIdx, "Error: pthread_attr_setscope fail. rt:%d\n", iRet);
		return -3;
	}

	iRet = pthread_attr_setdetachstate(&m_stAttr, PTHREAD_CREATE_JOINABLE);
	if (iRet)
	{
		TRACE_THREAD(m_iThreadIdx, "Error: pthread_attr_setdetachstate fail. rt:%d\n", iRet);
		return -5;
	}

	iRet = pthread_create(&m_hTrd, &m_stAttr, ThreadProc, (void *)this);
	if (iRet)
	{
		TRACE_THREAD(m_iThreadIdx, "Error: pthread_create fail. rt:%d\n", iRet);
		return -7;
	}

	return 0;
}

int CAppThread::InitCodeQueue(const int iThreadIdx)
{
	static const int CODEQUEUESIZE = 16777216;

	//输入
	m_pInputQueue = new CCodeQueue(CODEQUEUESIZE);
	if (!m_pInputQueue)
	{
		TRACE_THREAD(m_iThreadIdx, "Error: init input queue fail. thread idx:%d\n", m_iThreadIdx);
		return -1;
	}

	m_pInputQueue->Initialize(false);

	//输出
	m_pOutputQueue = new CCodeQueue(CODEQUEUESIZE);
	if (!m_pOutputQueue)
	{
		TRACE_THREAD(m_iThreadIdx, "Error: init output queue fail. thread idx:%d\n", m_iThreadIdx);
		return -3;
	}

	m_pOutputQueue->Initialize(false);

	return 0;
}

int CAppThread::Run()
{
	int iRet = 0;
	int iCodeLen = 0;
	SHandleResult stHandleResult;

	while (true)
	{
		while (true)
		{
			iRet = ReceiveMsg(iCodeLen);
			if (iRet)
			{
				//没有消息
				break;
			}

			iRet = m_pstProtocolEngine->Decode((unsigned char*)m_szCodeBuf, iCodeLen, m_stGameMsg, m_iThreadIdx);
			if (iRet)
			{
				break;
			}

			DEBUG_THREAD(m_iThreadIdx, "Receive code OK, Uin = %u, Msg = %d\n",
				m_stGameMsg.sthead().uin(),
				m_stGameMsg.sthead().uimsgid());

			stHandleResult.Reset();
			iRet = ProcessMsg(stHandleResult);
			if (iRet)
			{
				break;
			}

			// 不需要回复
			if (!stHandleResult.iNeedResponse)
			{
				continue;
			}

			iRet = EncodeAndSendMsg(stHandleResult);
			if (iRet != 0)
			{
				TRACE_THREAD(m_iThreadIdx, "Error: encode and push fail. rt:%d\n", iRet);
			}
		}

		usleep(APP_THREAD_MAX_SLEEP_USEC);
	}

	return 0;
}

int CAppThread::ReceiveMsg(int& riCodeLength)
{
	int iRet = 0;
	int iMaxLength = MAX_MSGBUFFER_SIZE - 1;
	iRet = m_pInputQueue->PopOneCode(m_szCodeBuf, iMaxLength, riCodeLength);

	if (iRet < 0 || riCodeLength <= (int)sizeof(int))
	{
		return -1;
	}

	DEBUG_THREAD(m_iThreadIdx, "Debug: receive code. len:%d\n", riCodeLength);
	return 0;
}


int CAppThread::ProcessMsg(SHandleResult& stMsgHandleResult)
{
	//处理消息
	CMsgHandler* pHandler = m_pstHandlerSet->GetHandler(m_stGameMsg.sthead().uimsgid());
	if (!pHandler)
	{
		TRACE_THREAD(m_iThreadIdx, "Failed to find a message handler, msg id: %u\n", m_stGameMsg.sthead().uimsgid());
		return -1;
	}

	stMsgHandleResult.iNeedResponse = true; // 默认需要回复
	pHandler->OnClientMsg(m_stGameMsg, &stMsgHandleResult);

	return 0;
}

int CAppThread::EncodeAndSendMsg(SHandleResult& stHandleResult)
{
	int iBufLen = sizeof(m_szCodeBuf);
	int iCodeLen = 0;
	int iRet = 0;

	// 编码本地数据为网络数据
	iRet = m_pstProtocolEngine->Encode(stHandleResult.stResponseMsg, (unsigned char*)m_szCodeBuf, iBufLen, iCodeLen, m_iThreadIdx);
	if (iRet != 0)
	{
		TRACE_THREAD(m_iThreadIdx, "Error: encode fail. rt:%d\n", iRet);
		return -1;
	}

	// 压入队列给主线程
	iRet = m_pOutputQueue->PushOneCode((const unsigned char*)m_szCodeBuf, iCodeLen);
	if (iRet != 0)
	{
		TRACE_THREAD(m_iThreadIdx, "Error: push code fail. rt:%d\n", iRet);
		return -3;
	}

	DEBUG_THREAD(m_iThreadIdx, "Push code into main thread. MsgID = %d, Uin = %u, len:%d, idx:%d\n",
		stHandleResult.stResponseMsg.sthead().uimsgid(),
		stHandleResult.stResponseMsg.sthead().uin(),
		iCodeLen, m_iThreadIdx);

	return 0;
}

int CAppThread::PushCode(const unsigned char* pMsg, int iCodeLength)
{
	if (!pMsg)
	{
		return -1;
	}

	return m_pInputQueue->PushOneCode(pMsg, iCodeLength);
}


int CAppThread::PopCode(unsigned char* pMsg, int iMaxLength, int& riLength)
{
	if (!pMsg)
	{
		return -1;
	}

	return m_pOutputQueue->PopOneCode(pMsg, iMaxLength, riLength);
}
