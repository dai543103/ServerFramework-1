#pragma once

#include "GMBaseCommand.hpp"

class CGameRoleObj;
class CGMMailCommand : public IGMBaseCommand
{
public:
	CGMMailCommand();
	~CGMMailCommand() { };

public:
	//执行相应GM命令的功能
	virtual int Run(unsigned uin, const GameMaster_Request& stReq, GameMaster_Response& stResp);
};
