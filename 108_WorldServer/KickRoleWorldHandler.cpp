#include "ModuleHelper.hpp"
#include "WorldMsgHelper.hpp"
#include "LogAdapter.hpp"
#include "WorldRoleStatus.hpp"
#include "ErrorNumDef.hpp"

#include "KickRoleWorldHandler.hpp"

CKickRoleWorldHandler::~CKickRoleWorldHandler(void)
{
}

GameProtocolMsg CKickRoleWorldHandler::ms_stGameMsg;
int CKickRoleWorldHandler::OnClientMsg(GameProtocolMsg* pMsg)
{
	ASSERT_AND_LOG_RTN_INT(pMsg);

	m_pRequestMsg = (GameProtocolMsg*)pMsg;

	switch (m_pRequestMsg->sthead().uimsgid())
	{
	case MSGID_WORLD_KICKROLE_REQUEST:
	{
		OnZoneRequestKickRole();
		break;
	}

	case MSGID_WORLD_KICKROLE_RESPONSE:
	{
		OnZoneResponseKickRole();
		break;
	}

	default:
	{
		break;
	}
	}

	return 0;
}

// 处理来自Zone的踢人请求
int CKickRoleWorldHandler::OnZoneRequestKickRole()
{
	const World_KickRole_Request& rstReq = m_pRequestMsg->stbody().m_stworld_kickrole_request();

	int iRet = 0;

	// 判断本world的登录情况
	LOGDEBUG("Recv Zone KickRole Req: From Uin = %u, Kicked Uin = %u, From ZoneID = %d\n", rstReq.stkicker().stroleid().uin(), rstReq.ukickeduin(), rstReq.stkicker().ifromzoneid());

	CWorldRoleStatusWObj* pWorldRoleStatusWObj = WorldTypeK32<CWorldRoleStatusWObj>::GetByUin(rstReq.ukickeduin());

	//如果就在该world登录, 向Zone发送踢人请求
	if (pWorldRoleStatusWObj != NULL)
	{
		int iZoneID = pWorldRoleStatusWObj->GetZoneID();
		if (CModuleHelper::GetZoneTick()->IsZoneActive(iZoneID))
		{
			//转发给Zone踢人
			CWorldMsgHelper::SendWorldMsgToWGS(*m_pRequestMsg, iZoneID);
			LOGDEBUG("Send zone KickRole Req: iRet:%d, Kicked Uin = %u, zoneid:%d\n", iRet, rstReq.ukickeduin(), iZoneID);
			return 0;
		}
		else
		{
			// 直接删除该uin的信息，重新登录
			WorldTypeK32<CWorldRoleStatusWObj>::DeleteByUin(rstReq.stkicker().stroleid().uin());
		}
	}

	// 向Zone发送回复
	CWorldMsgHelper::GenerateMsgHead(ms_stGameMsg, 0, MSGID_WORLD_KICKROLE_RESPONSE, 0);
	World_KickRole_Response* pstKickResp = ms_stGameMsg.mutable_stbody()->mutable_m_stworld_kickrole_response();
	pstKickResp->set_bislogin(rstReq.bislogin());
	pstKickResp->set_ukickeduin(rstReq.ukickeduin());
	pstKickResp->mutable_stkicker()->CopyFrom(rstReq.stkicker());

	pstKickResp->set_iresult(T_SERVER_SUCCESS);

	iRet = CWorldMsgHelper::SendWorldMsgToWGS(ms_stGameMsg, rstReq.stkicker().ifromzoneid());

	LOGDEBUG("Send Zone KickRole Resp: Uin = %u, iRet = %d\n", rstReq.stkicker().stroleid().uin(), iRet);

	return 0;
}

// 处理来自Zone的被踢回复
int CKickRoleWorldHandler::OnZoneResponseKickRole()
{
	const World_KickRole_Response& rstResp = m_pRequestMsg->stbody().m_stworld_kickrole_response();
	unsigned int uin = rstResp.ukickeduin();
	int iRet = -1;

	LOGDEBUG("Recv Zone KickRole Resp: Uin = %u, result = %d\n", uin, rstResp.iresult());

	// 如果踢人成功, 则再次确认World没有数据. 如果有, 则强制删除缓存数据.
	if (rstResp.iresult() == T_SERVER_SUCCESS)
	{
		CWorldRoleStatusWObj* pWorldRoleStatusWObj = WorldTypeK32<CWorldRoleStatusWObj>::GetByUin(rstResp.ukickeduin());
		if (pWorldRoleStatusWObj)
		{
			WorldTypeK32<CWorldRoleStatusWObj>::DeleteByUin(uin);
		}
	}

	//登录的zone和踢人的zone属于一个world
	if (rstResp.stkicker().ifromworldid() == CModuleHelper::GetWorldID())
	{
		// 向Zone发送回复
		iRet = CWorldMsgHelper::SendWorldMsgToWGS(*m_pRequestMsg, rstResp.stkicker().ifromzoneid());

		LOGDEBUG("Send Zone KickRole Resp: Uin = %u, iRet = %d\n", uin, iRet);

	}
	//换world登录，不回包了
	else
	{

	}

	return iRet;
}
