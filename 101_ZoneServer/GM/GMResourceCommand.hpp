#pragma once

#include "GMBaseCommand.hpp"

class CGameRoleObj;
class CGMResourceCommand : public IGMBaseCommand
{
public:
	CGMResourceCommand();
	~CGMResourceCommand() { };

public:
	//÷¥––œ‡”¶GM√¸¡Ó
	virtual int Run(unsigned uin, const GameMaster_Request& stReq, GameMaster_Response& stResp);
};