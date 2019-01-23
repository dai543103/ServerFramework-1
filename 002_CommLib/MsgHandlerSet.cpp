
#include "GameProtocol.hpp"
#include "MsgHandlerSet.hpp"

CMsgHandlerSet::CMsgHandlerSet()
{
	memset(m_apHandler, 0, sizeof(m_apHandler));
}

CMsgHandler* CMsgHandlerSet::GetHandler(const unsigned int uiMsgID, enMsgType enMsgType)
{
	if (uiMsgID >= MAX_MSG_HANDLER_NUMBER)
	{
		return NULL;
	}

	// 客户端消息只能调用客户端Handler, 服务器消息可以直接调用客户端Handler
	if (enMsgType == m_apHandler[uiMsgID].m_enMsgType || EKMT_SERVER == enMsgType)
	{
		return m_apHandler[uiMsgID].m_pHandler;
	}

	return NULL;
}

int CMsgHandlerSet::RegisterHandler(const unsigned int uiMsgID, CMsgHandler* pHandler, enMsgType enMsgType)
{
	if (!pHandler)
	{
		return -1;
	}

	if (uiMsgID >= MAX_MSG_HANDLER_NUMBER)
	{
		return -2;
	}

	// 防止重复注册
	if (m_apHandler[uiMsgID].m_pHandler)
	{
		return -3;
	}

	m_apHandler[uiMsgID].m_pHandler = (CMsgHandler*)pHandler;
	m_apHandler[uiMsgID].m_enMsgType = enMsgType;

	return 0;
}
