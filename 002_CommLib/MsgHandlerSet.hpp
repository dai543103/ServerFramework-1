#pragma once

#include <string.h>

#include "MsgHandler.hpp"

enum enMsgType
{
	EKMT_CLIENT = 1,    // 客户端消息
	EKMT_SERVER = 2,    // 服务器消息
};

struct TMsgHandler
{
	CMsgHandler* m_pHandler;       // 消息处理函数
	enMsgType m_enMsgType;   // 消息类型
};

const unsigned int MAX_MSG_HANDLER_NUMBER = 16384;

// 消息处理者管理器
class CMsgHandlerSet
{
public:
	virtual ~CMsgHandlerSet() {}

	// 初始化消息处理者集合中的各个消息处理者，返回值为0表示成功，其他表示失败
	virtual int Initialize(int iThreadIndex = -1) = 0;

	// 根据消息id返回该消息的处理者
	CMsgHandler* GetHandler(unsigned int uiMsgID, enMsgType enMsgType = EKMT_SERVER);

protected:
	CMsgHandlerSet();

	// 根据消息id注册它的处理者，返回值为0表示成功，其他表示失败
	int RegisterHandler(unsigned int uiMsgID, CMsgHandler* pHandler, enMsgType enMsgType = EKMT_SERVER);

protected:

	// 用数组表示的消息处理者集合
	TMsgHandler m_apHandler[MAX_MSG_HANDLER_NUMBER];
};
