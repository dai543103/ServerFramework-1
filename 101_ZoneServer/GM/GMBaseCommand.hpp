#pragma once

//GM命令基类
#include "GameProtocol.hpp"
#include "Kernel/GameRole.hpp"

class IGMBaseCommand
{
public:
    virtual ~IGMBaseCommand();

    //执行GM命令
    virtual int Run(unsigned uin, const GameMaster_Request& stReq, GameMaster_Response& stResp) = 0;

protected:

	//GM操作目标玩家
	CGameRoleObj* m_pRoleObj;
};
