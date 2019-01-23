#pragma once

#include "GMBaseCommand.hpp"

class CGameRoleObj;
class CGMHorseLampCommand : public IGMBaseCommand
{
public:
	CGMHorseLampCommand();
	~CGMHorseLampCommand() { };

public:
	//执行相应的GM命令的功能
	virtual int Run(unsigned uin, const GameMaster_Request& stReq, GameMaster_Response& stResp);

	//发送GM走马灯
	void SendGMHorseLamp(int iLampID, const std::string& strHorseLamp);
};
