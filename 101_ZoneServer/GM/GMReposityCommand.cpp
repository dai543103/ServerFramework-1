
#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "ErrorNumDef.hpp"
#include "Kernel/GameRole.hpp"
#include "Kernel/UnitUtility.hpp"
#include "RepThings/RepThingsUtility.hpp"

#include "GMReposityCommand.hpp"

using namespace ServerLib;

CGMReposityCommand::CGMReposityCommand()
{
	m_pRoleObj = NULL;
}

//执行相应GM命令的功能
int CGMReposityCommand::Run(unsigned uin, const GameMaster_Request& stReq, GameMaster_Response& stResp)
{
	m_pRoleObj = CUnitUtility::GetRoleByUin(uin);
	if (!m_pRoleObj)
	{
		LOGERROR("Failed  to run reposity GM command, invalid role %u!\n", uin);
		return T_ZONE_GAMEROLE_NOT_EXIST;
	}

	//执行相应的GM操作
	switch (stReq.ioperatype())
	{
	case GM_OPERA_ADDITEM:
	{
		//修改玩家道具
		if (stReq.strparams_size() != 2)
		{
			LOGERROR("Failed to run rep gm command, invalid param, to uin %u\n", uin);
			return T_ZONE_PARA_ERROR;
		}

		int iItemID = atoi(stReq.strparams(0).c_str());
		int iItemNum = atoi(stReq.strparams(1).c_str());

		int iRet = CRepThingsUtility::AddItemNum(*m_pRoleObj, iItemID, iItemNum, ITEM_CHANNEL_GMADD);
		if (iRet)
		{
			LOGERROR("Failed to add rep item, ret %d, uin %u, item id %d, num %d\n", iRet, m_pRoleObj->GetUin(), iItemID, iItemNum);
			return iRet;
		}
	}
	break;

	case GM_OPERA_GETREPINFO:
	{
		m_pRoleObj->UpdateRepThingsToDB(*stResp.mutable_stiteminfo());
	}
	break;

	default:
	{
		return T_ZONE_PARA_ERROR;
	}
	break;
	}

	LOGDEBUG("Success to run reposty GM command, uin %u\n", m_pRoleObj->GetUin());

	return T_SERVER_SUCCESS;
}
