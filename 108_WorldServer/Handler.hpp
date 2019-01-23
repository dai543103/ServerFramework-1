#pragma once

#include "GameProtocol.hpp"
class IHandler
{
public:
	virtual ~IHandler();
	virtual int OnClientMsg(GameProtocolMsg* pMsg) = 0;

};
