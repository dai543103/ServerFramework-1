#pragma once

#include "GameProtocol.hpp"
#include "CommDefine.h"

typedef struct
{
	GameProtocolMsg stResponseMsg;	//响应消息
	int iNeedResponse;				//true需要回复，false不需要

	void Reset()
	{
		stResponseMsg.Clear();
		iNeedResponse = true;
	}
} SHandleResult;

// 消息处理者抽象类
class CMsgHandler
{
protected:
	CMsgHandler() {};

public:
    virtual ~CMsgHandler() {};

    // 消息处理
    virtual void OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult = NULL, TNetHead_V2* pstNetHead = NULL) = 0;

protected:

	void GenerateMsgHead(GameProtocolMsg& stMsg, unsigned int uiSessionID, unsigned int uiMsgID, unsigned int uiUin);
};
