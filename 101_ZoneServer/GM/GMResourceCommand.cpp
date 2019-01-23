
#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "ErrorNumDef.hpp"
#include "Kernel/GameRole.hpp"
#include "Kernel/UnitUtility.hpp"
#include "Resource/ResourceUtility.h"

#include "GMResourceCommand.hpp"

using namespace ServerLib;

CGMResourceCommand::CGMResourceCommand()
{
	m_pRoleObj = NULL;
}

//执行相应GM命令功能
int CGMResourceCommand::Run(unsigned uin, const GameMaster_Request& stReq, GameMaster_Response& stResp)
{
	m_pRoleObj = CUnitUtility::GetRoleByUin(uin);
	if (!m_pRoleObj)
	{
		LOGERROR("Failed  to run resource gm command, invalid role %d!\n", uin);
		return T_ZONE_GAMEROLE_NOT_EXIST;
	}

	//执行相应的GM操作
	int iRet = T_SERVER_SUCCESS;
	switch (stReq.ioperatype())
	{
	case GM_OPERA_ADDRES:
	{
		//修改玩家资源
		if (stReq.strparams_size() != 2)
		{
			LOGERROR("Failed to check resource gm param, to uin %u\n", uin);
			return T_ZONE_PARA_ERROR;
		}

		int iResType = atoi(stReq.strparams(0).c_str());
		int iResNum = atoi(stReq.strparams(1).c_str());

		iRet = CResourceUtility::AddUserRes(*m_pRoleObj, iResType, iResNum);
		if (!iRet)
		{
			LOGERROR("Failed to add resource, ret %d, uin %u, res type %d, num %d\n", iRet, uin, iResType, iResNum);
			return iRet;
		}
	}
	break;

	case GM_OPERA_RECHARGE:
	{
		//充值请求
		if (stReq.strparams_size() != 2)
		{
			LOGERROR("Failed to check recharge gm param, to uin %u\n", uin);
			return T_ZONE_PARA_ERROR;
		}

		int iRechargeID = atoi(stReq.strparams(0).c_str());
		int iTime = atoi(stReq.strparams(1).c_str());

		iRet = m_pRoleObj->GetRechargeManager().UserRecharge(iRechargeID, iTime, "GM");
		if (iRet)
		{
			LOGERROR("Failed to recharge user, ret %d, uin %u, recharge id %d\n", iRet, uin, iRechargeID);
			return iRet;
		}
	}
	break;

	case GM_OPERA_GETBASEINFO:
	{
		m_pRoleObj->UpdateBaseInfoToDB(*stResp.mutable_stbaseinfo());
	}
	break;

	default:
	{
		return T_ZONE_PARA_ERROR;
	}
	break;
	}

	LOGDEBUG("Success to run resource GM command, uin %u\n", m_pRoleObj->GetUin());

	return T_SERVER_SUCCESS;
}
