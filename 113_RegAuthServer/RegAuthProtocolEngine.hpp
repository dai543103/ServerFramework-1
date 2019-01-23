#pragma once

#include "GameProtocol.hpp"
#include "ProtocolEngine.hpp"

class CRegAuthProtocolEngine : public CProtocolEngine
{
public:
	//消息解码
	virtual int Decode(const unsigned char* pszCodeBuff, int iCodeLen, GameProtocolMsg& stMsg, int iThreadIndex = -1,
		const TNetHead_V2* pNetHead = NULL, EGameServerID enMsgPeer = GAME_SERVER_UNDEFINE);

	//消息编码
	virtual int Encode(const GameProtocolMsg& stMsg, unsigned char* pszCodeBuff, int iBuffLen, int& iCodeLen, int iThreadIndex = -1,
		const TNetHead_V2* pNetHead = NULL, EGameServerID enMsgPeer = GAME_SERVER_UNDEFINE);
};
