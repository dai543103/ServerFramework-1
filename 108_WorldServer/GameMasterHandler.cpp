#include <assert.h>
#include <string.h>

#include "ModuleHelper.hpp"
#include "WorldMsgHelper.hpp"
#include "LogAdapter.hpp"
#include "ErrorNumDef.hpp"
#include "WorldMailManager.hpp"

#include "GameMasterHandler.hpp"

static GameProtocolMsg stMsg;

CGameMasterHandler::~CGameMasterHandler()
{

}

int CGameMasterHandler::OnClientMsg(GameProtocolMsg* pMsg)
{
	ASSERT_AND_LOG_RTN_INT(pMsg);

	m_pRequestMsg = (GameProtocolMsg*)pMsg;

	switch (m_pRequestMsg->sthead().uimsgid())
	{
	case MSGID_WORLD_GAMEMASTER_REQUEST:
	{
		//World转发GM请求
		OnRequestGameMaster();
	}
	break;

	case MSGID_WORLD_GAMEMASTER_RESPONSE:
	{
		//World收到GM返回
		OnResponseGameMaster();
	}
	break;

	default:
	{
		;
	}
	break;
	}

	return 0;
}

int CGameMasterHandler::OnRequestGameMaster()
{
	LOGDEBUG("OnRequestGameMaster\n");

	//获取请求
	GameMaster_Request* pstReq = m_pRequestMsg->mutable_stbody()->mutable_m_stworld_gamemaster_request();

	//返回消息
	CWorldMsgHelper::GenerateMsgHead(stMsg, 0, MSGID_WORLD_GAMEMASTER_RESPONSE, 0);
	GameMaster_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stworld_gamemaster_response();
	pstResp->set_ioperatype(pstReq->ioperatype());
	pstResp->set_ufromuin(pstReq->ufromuin());
	pstResp->set_utouin(pstReq->utouin());

	int iRet = 0;
	if (pstReq->ioperatype() == GM_OPERA_SENDMAIL)
	{
		//GM发送邮件
		iRet = CWorldMailManager::Instance()->SendGMMail(*pstReq);
		
		pstResp->set_iresult(iRet);
		CWorldMsgHelper::SendWorldMsgToWGS(stMsg, pstReq->ifromzoneid());
		return T_SERVER_SUCCESS;
	}

	//获取操作的玩家
	CWorldRoleStatusWObj* pstRoleStatObj = WorldTypeK32<CWorldRoleStatusWObj>::GetByUin(pstReq->utouin());
	if (pstRoleStatObj)
	{
		//玩家在线，转发Zone
		iRet = CWorldMsgHelper::SendWorldMsgToWGS(*m_pRequestMsg, pstRoleStatObj->GetZoneID());
	}
	else
	{
		//这边需要替换stHead中的uin为操作目标的uin
		//同时将GM操作人的uin保存到uFromUin中
		pstReq->set_ufromuin(m_pRequestMsg->sthead().uin());
		m_pRequestMsg->mutable_sthead()->set_uin(pstReq->utouin());

		//玩家不在线，转发RoleDB
		iRet = CWorldMsgHelper::SendWorldMsgToRoleDB(*m_pRequestMsg);
	}

	if (iRet)
	{
		LOGERROR("Failed to send gm request, ret %d, target uin %u\n", iRet, pstReq->utouin());
		return iRet;
	}

	return 0;
}

int CGameMasterHandler::OnResponseGameMaster()
{
	//直接转发给GMUser
	CWorldRoleStatusWObj* pstRoleStatObj = WorldTypeK32<CWorldRoleStatusWObj>::GetByUin(m_pRequestMsg->sthead().uin());
	if (pstRoleStatObj)
	{
		CWorldMsgHelper::SendWorldMsgToWGS(*m_pRequestMsg, pstRoleStatObj->GetZoneID());
	}

	return 0;
}
