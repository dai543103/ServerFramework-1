#include <assert.h>
#include <string.h>

#include "GameProtocol.hpp"
#include "ModuleHelper.hpp"
#include "WorldMsgHelper.hpp"
#include "LogAdapter.hpp"
#include "ErrorNumDef.hpp"
#include "WorldMailManager.hpp"

#include "MailWorldHandler.h"

static GameProtocolMsg stMsg;

CMailWorldHandler::~CMailWorldHandler()
{

}

int CMailWorldHandler::OnClientMsg(GameProtocolMsg* pMsg)
{
	ASSERT_AND_LOG_RTN_INT(pMsg);

	m_pRequestMsg = (GameProtocolMsg*)pMsg;

	switch (m_pRequestMsg->sthead().uimsgid())
	{
	case MSGID_WORLD_SENDMAIL_REQUEST:
	{
		//发送邮件的请求
		OnRequestSendMail();
	}
	break;

	case MSGID_WORLD_SENDMAIL_RESPONSE:
	{
		//发送邮件的返回
		OnResponseSendMail();
	}
	break;

	case MSGID_WORLD_GETSYSTEMMAIL_REQUEST:
	{
		//拉取系统邮件的请求
		OnRequestGetSystemMail();
	}
	break;

	default:
		break;
	}

	return 0;
}

//发送邮件的请求
int CMailWorldHandler::OnRequestSendMail()
{
	LOGDEBUG("OnRequestSendMail\n");

	const World_SendMail_Request& stReq = m_pRequestMsg->stbody().m_stworld_sendmail_request();

	CWorldRoleStatusWObj* pstRoleStatObj = WorldTypeK32<CWorldRoleStatusWObj>::GetByUin(stReq.uifromuin());
	if (!pstRoleStatObj)
	{
		return -1;
	}

	//返回消息
	CWorldMsgHelper::GenerateMsgHead(stMsg, 0, MSGID_WORLD_SENDMAIL_RESPONSE, stReq.uifromuin());
	World_SendMail_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stworld_sendmail_response();
	pstResp->set_uifromuin(stReq.uifromuin());
	pstResp->set_uitouin(stReq.uitouin());
	pstResp->mutable_stmailinfo()->CopyFrom(stReq.stmailinfo());

	//发送邮件
	int iRet = CWorldMailManager::SendPersonalMail(stReq);
	if (iRet)
	{
		LOGERROR("Failed to send mail, ret %d, mail id %d, to uin %u, from uin %u\n", iRet, stReq.stmailinfo().imailid(), stReq.uitouin(), stReq.uifromuin());

		pstResp->set_iresult(iRet);
		CWorldMsgHelper::SendWorldMsgToWGS(stMsg, pstRoleStatObj->GetZoneID());
		return iRet;
	}

	//成功在其他地方返回

	return T_SERVER_SUCCESS;;
}

//发送邮件的返回
int CMailWorldHandler::OnResponseSendMail()
{
	LOGDEBUG("OnResponseSendMail\n");

	const World_SendMail_Response& stResp = m_pRequestMsg->stbody().m_stworld_sendmail_response();
	if (stResp.iresult() != 0 && stResp.stmailinfo().imailid() != MAIL_SEND_ITEMGIFT && stResp.itrytimes() < 5)
	{
		//非赠送邮件发送失败，最多重试5次
		World_SendMail_Request stReq;
		stReq.set_uifromuin(stResp.uifromuin());
		stReq.set_uitouin(stResp.uitouin());
		stReq.set_itrytimes(stResp.itrytimes());
		stReq.mutable_stmailinfo()->CopyFrom(stResp.stmailinfo());

		int iRet = CWorldMailManager::SendPersonalMail(stReq);
		if (iRet == 0)
		{
			//发送成功，直接返回
			return 0;
		}
	}

	if (stResp.iresult())
	{
		//发送邮件失败,打印日志
		LOGERROR("Failed to send mail, from uin %u, to uin %u, mail id %d, ret %d\n", stResp.uifromuin(), stResp.uitouin(), stResp.stmailinfo().imailid(), stResp.iresult());
	}

	if (stResp.uifromuin() != 0)
	{
		//不是系统邮件
		CWorldRoleStatusWObj* pstRoleStatObj = WorldTypeK32<CWorldRoleStatusWObj>::GetByUin(stResp.uifromuin());
		if (pstRoleStatObj)
		{
			//发送给邮件发送者
			CWorldMsgHelper::SendWorldMsgToWGS(*m_pRequestMsg, pstRoleStatObj->GetZoneID());
		}
	}

	return T_SERVER_SUCCESS;
}

//拉取系统邮件请求
int CMailWorldHandler::OnRequestGetSystemMail()
{
	LOGDEBUG("OnRequestGetSystemMail\n");

	const World_GetSystemMail_Request& stReq = m_pRequestMsg->stbody().m_stworld_getsystemmail_request();

	//返回消息
	CWorldMsgHelper::GenerateMsgHead(stMsg, 0, MSGID_WORLD_GETSYSTEMMAIL_RESPONSE, 0);
	World_GetSystemMail_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stworld_getsystemmail_response();
	pstResp->set_uin(stReq.uin());

	CWorldMailManager::Instance()->GetSystemMail(stReq.uin(), stReq.ireason(), stReq.urolesystemuniqid(), stReq.iviplevel(), stReq.strchannel(), *pstResp);

	//发送返回
	CWorldMsgHelper::SendWorldMsgToWGS(stMsg, stReq.ifromzoneid());

	return T_SERVER_SUCCESS;
}

