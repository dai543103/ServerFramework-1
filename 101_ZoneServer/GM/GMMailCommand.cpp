
#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "ErrorNumDef.hpp"
#include "Kernel/GameRole.hpp"
#include "Kernel/UnitUtility.hpp"
#include "Mail/MailUtility.h"

#include "GMMailCommand.hpp"

using namespace ServerLib;

CGMMailCommand::CGMMailCommand()
{
	m_pRoleObj = NULL;
}

//执行相应GM命令的功能
int CGMMailCommand::Run(unsigned uin, const GameMaster_Request& stReq, GameMaster_Response& stResp)
{
	return T_SERVER_SUCCESS;
}
