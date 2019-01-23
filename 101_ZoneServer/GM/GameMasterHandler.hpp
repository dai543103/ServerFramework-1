#pragma once

#include <string>

#include "GameProtocol.hpp"
#include "ErrorNumDef.hpp"
#include "CommDefine.h"
#include "Kernel/Handler.hpp"
#include "GMHorseLampCommand.hpp"
#include "GMReposityCommand.hpp"
#include "GMResourceCommand.hpp"
#include "GMMailCommand.hpp"

class CGameRoleObj;
class CGameMasterHandler: public IHandler
{
public:
    virtual ~CGameMasterHandler(void);
	CGameMasterHandler();

    int OnClientMsg();

private:

	//管理员GM请求
	int OnRequestGM();

	//World转发GM
	int OnRequestWorldGM();

	//World转发GM返回
	int OnResponseWorldGM();

    //检查是否是GM用户
    int CheckIsGMUser();

	//注册GMCommand
	void RegisterGMCommand();

private:

	//GM Command
	IGMBaseCommand* m_apGMCommand[GM_OPERA_MAX];

	//GM Handler
	CGMHorseLampCommand m_stHorseLampHandler;
	CGMReposityCommand m_stReposityHandler;
	CGMResourceCommand m_stResouceHandler;
	CGMMailCommand m_stMailHandler;
};
