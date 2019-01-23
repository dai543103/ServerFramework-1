
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "GameProtocol.hpp"
#include "Kernel/GameRole.hpp"
#include "Kernel/ZoneObjectHelper.hpp"
#include "Kernel/UnitUtility.hpp"
#include "Kernel/ZoneMsgHelper.hpp"
#include "Kernel/ModuleHelper.hpp"
#include "Kernel/ConfigManager.hpp"
#include "GMReposityCommand.hpp"
#include "GMResourceCommand.hpp"
#include "GMHorseLampCommand.hpp"

#include "GameMasterHandler.hpp"

static GameProtocolMsg stMsg;

CGameMasterHandler::~CGameMasterHandler(void)
{

}

CGameMasterHandler::CGameMasterHandler()
{
	//注册GMCommand
	RegisterGMCommand();
}

int CGameMasterHandler::OnClientMsg()
{
	switch (m_pRequestMsg->sthead().uimsgid())
	{
	case MSGID_ZONE_GAMEMASTER_REQUEST:
	{
		//管理员GM请求
		OnRequestGM();
	}
	break;

	case MSGID_WORLD_GAMEMASTER_REQUEST:
	{
		//World转发GM
		OnRequestWorldGM();
	}
	break;

	case MSGID_WORLD_GAMEMASTER_RESPONSE:
	{
		//World转发GM返回
		OnResponseWorldGM();
	}
	break;

	default:
	{
		LOGERROR("Failed to handler request msg, invalid msgid: %u\n", m_pRequestMsg->sthead().uimsgid());
		return -1;
	}
	break;
	}

	return 0;
}

int CGameMasterHandler::OnRequestGM()
{
    //校验参数的有效性
    int iRet = SecurityCheck();
    if(iRet)
    {
        LOGERROR("Failed to check request param, uin %u, ret %d\n", m_pRequestMsg->sthead().uin(), iRet);
        return iRet;
    }

	unsigned uin = m_pRoleObj->GetUin();

	//返回消息
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_GAMEMASTER_RESPONSE);
	GameMaster_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_gamemaster_response();

	//检查GM权限
	if (!CheckIsGMUser())
	{
		LOGERROR("Failed to run gm command, invalid gmuser %u\n", uin);

		pstResp->set_iresult(T_ZONE_INVALID_GMUSER);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return T_ZONE_INVALID_GMUSER;
	}

	//检查参数
	const GameMaster_Request& stReq = m_pRequestMsg->stbody().m_stzone_gamemaster_request();
	if (stReq.ioperatype() <= GM_OPERA_INVALID || stReq.ioperatype() >= GM_OPERA_MAX)
	{
		LOGERROR("Failed to run gm command, invalid opera type %d, gm user %u\n", stReq.ioperatype(), uin);

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return T_ZONE_PARA_ERROR;
	}

	if (!m_apGMCommand[stReq.ioperatype()])
	{
		LOGERROR("Failed to get gm handler, opera type %d, gm user %u\n", stReq.ioperatype(), uin);
		
		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return T_ZONE_PARA_ERROR;
	}

	if (stReq.ioperatype() == GM_OPERA_HORSELAMP)
	{
		//走马灯不需要转发World
		iRet = m_apGMCommand[stReq.ioperatype()]->Run(uin, stReq, *pstResp);
		if (iRet)
		{
			LOGERROR("Failed to run gm command, ret %d, opera type %d, gm user %u\n", iRet, stReq.ioperatype(), uin);

			pstResp->set_iresult(iRet);
			CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
			return iRet;
		}

		pstResp->set_iresult(T_SERVER_SUCCESS);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return T_SERVER_SUCCESS;
	}

	//转发GM请求到World
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_WORLD_GAMEMASTER_REQUEST);
	stMsg.mutable_sthead()->set_uin(uin);
	
	GameMaster_Request* pstReq = stMsg.mutable_stbody()->mutable_m_stworld_gamemaster_request();
	pstReq->CopyFrom(stReq);
	pstReq->set_ufromuin(uin);
	pstReq->set_ifromzoneid(CModuleHelper::GetZoneID());

	//发送到World
	CZoneMsgHelper::SendZoneMsgToWorld(stMsg);

    return T_SERVER_SUCCESS;
}

//World转发GM
int CGameMasterHandler::OnRequestWorldGM()
{
	//返回消息
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_WORLD_GAMEMASTER_RESPONSE);
	stMsg.mutable_sthead()->set_uin(m_pRequestMsg->sthead().uin());
	GameMaster_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stworld_gamemaster_response();

	//检查参数
	const GameMaster_Request& stReq = m_pRequestMsg->stbody().m_stworld_gamemaster_request();
	pstResp->set_ioperatype(stReq.ioperatype());
	pstResp->set_ufromuin(stReq.ufromuin());
	pstResp->set_utouin(stReq.utouin());

	unsigned uGMUin = m_pRequestMsg->sthead().uin();
	if (stReq.ioperatype() <= GM_OPERA_INVALID || stReq.ioperatype() >= GM_OPERA_MAX)
	{
		LOGERROR("Failed to run gm command, invalid opera type %d, gm user %u\n", stReq.ioperatype(), uGMUin);

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToWorld(stMsg);
		return T_ZONE_PARA_ERROR;
	}

	if (!m_apGMCommand[stReq.ioperatype()])
	{
		LOGERROR("Failed to get gm handler, opera type %d, gm user %u\n", stReq.ioperatype(), uGMUin);

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToWorld(stMsg);
		return T_ZONE_PARA_ERROR;
	}

	//对个人的操作
	int iRet = m_apGMCommand[stReq.ioperatype()]->Run(stReq.utouin(), stReq, *pstResp);
	if (iRet)
	{
		LOGERROR("Failed to run gm command, ret %d, opera type %d, gm user %u\n", iRet, stReq.ioperatype(), uGMUin);

		pstResp->set_iresult(iRet);
		CZoneMsgHelper::SendZoneMsgToWorld(stMsg);
		return iRet;
	}

	pstResp->set_iresult(T_SERVER_SUCCESS);
	CZoneMsgHelper::SendZoneMsgToWorld(stMsg);

	return T_SERVER_SUCCESS;
}

//World转发GM返回
int CGameMasterHandler::OnResponseWorldGM()
{
	//获取World请求
	const GameMaster_Response& stResp = m_pRequestMsg->stbody().m_stworld_gamemaster_response();

	//获取GMUser
	m_pRoleObj = CUnitUtility::GetRoleByUin(stResp.ufromuin());
	if (!m_pRoleObj)
	{
		return 0;
	}

	//获取返回消息
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_GAMEMASTER_RESPONSE);
	GameMaster_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_gamemaster_response();
	pstResp->CopyFrom(stResp);

	//发送返回
	CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);

	return 0;
}

//检查是否是GM用户
int CGameMasterHandler::CheckIsGMUser()
{
    //首先检查是否GM玩家的uin
    CGMPrivConfigManager& rstGMPrivConfigMgr = CConfigManager::Instance()->GetGMPrivConfigManager();
    if(rstGMPrivConfigMgr.CheckIsGMUin(m_pRoleObj->GetUin()))
    {
        return true;
    }

    unsigned int uClientIP = inet_network(m_pSession->GetClientIP());
    if(rstGMPrivConfigMgr.CheckIsGMIP(uClientIP))
    {
        return true;
    }

    return false;
}

//注册GMCommand
void CGameMasterHandler::RegisterGMCommand()
{
	//背包道具相关
	m_apGMCommand[GM_OPERA_ADDITEM] = &m_stReposityHandler;
	m_apGMCommand[GM_OPERA_GETREPINFO] = &m_stReposityHandler;

	//玩家资源相关
	m_apGMCommand[GM_OPERA_ADDRES] = &m_stResouceHandler;
	m_apGMCommand[GM_OPERA_RECHARGE] = &m_stResouceHandler;
	m_apGMCommand[GM_OPERA_GETBASEINFO] = &m_stResouceHandler;

	//走马灯相关
	m_apGMCommand[GM_OPERA_HORSELAMP] = &m_stHorseLampHandler;

	//GM邮件相关
	m_apGMCommand[GM_OPERA_SENDMAIL] = &m_stMailHandler;

	return;
}
