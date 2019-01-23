
#include "GameProtocol.hpp"
#include "Kernel/GameRole.hpp"
#include "Kernel/ZoneObjectHelper.hpp"
#include "Kernel/ZoneMsgHelper.hpp"
#include "Kernel/ModuleHelper.hpp"
#include "Kernel/ConfigManager.hpp"

#include "Chat/ChatHandler.hpp"

static GameProtocolMsg stMsg;

CChatHandler::~CChatHandler(void)
{

}

CChatHandler::CChatHandler()
{

}

int CChatHandler::OnClientMsg()
{
	switch (m_pRequestMsg->sthead().uimsgid())
	{
	case MSGID_ZONE_CHAT_REQUEST:
	{
		OnRequestChat();
	}
	break;

	case MSGID_WORLD_CHAT_NOTIFY:
	{
		OnNotifyChat();
	}
	break;

	case MSGID_ZONE_HORSELAMP_NOTIFY:
	{
		OnNotifyHorseLamp();
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

int CChatHandler::OnRequestChat()
{
	int iRet = SecurityCheck();
	if (iRet < 0)
	{
		LOGERROR("Security Check Failed: Uin = %u, iRet = %d\n", m_pRequestMsg->sthead().uin(), iRet);

		SendFailedResponse(MSGID_ZONE_CHAT_RESPONSE, T_ZONE_SECURITY_CHECK_FAILED, *m_pNetHead);

		return -1;
	}

	unsigned int uiUin = m_pRoleObj->GetUin();
	CGameRoleObj* pstRoleObj = GameTypeK32<CGameRoleObj>::GetByKey(uiUin);
	ASSERT_AND_LOG_RTN_INT(pstRoleObj);

	//玩家聊天操作的处理
	const Zone_Chat_Request& rstRequest = m_pRequestMsg->stbody().m_stzone_chat_request();
	SendChatMsg(*pstRoleObj, rstRequest.ichannel(), rstRequest.strmessage().c_str());

	//处理聊天消息成功，返回
	SendSuccessfulResponse();

	return T_SERVER_SUCCESS;
}

int CChatHandler::OnNotifyChat()
{
	//广播给本线的所有用户
	CZoneMsgHelper::SendZoneMsgToZoneAll(*m_pRequestMsg);

	return T_SERVER_SUCCESS;
}

//世界走马灯的推送
int CChatHandler::OnNotifyHorseLamp()
{
	//广播给本线的所有用户
	CZoneMsgHelper::SendZoneMsgToZoneAll(*m_pRequestMsg);

	return T_SERVER_SUCCESS;
}

//发送聊天信息
int CChatHandler::SendChatMsg(CGameRoleObj& stRoleObj, int iChannel, const char* pMsg)
{
	if (!pMsg)
	{
		return T_ZONE_PARA_ERROR;
	}

	static GameProtocolMsg stChatNotify;
	CZoneMsgHelper::GenerateMsgHead(stChatNotify, MSGID_WORLD_CHAT_NOTIFY);

	World_Chat_Notify* pstNotify = stChatNotify.mutable_stbody()->mutable_m_stworld_chat_notify();

	switch (iChannel)
	{
	case CHAT_CHANNEL_WORLD:
	{
		pstNotify->set_ichannel((ChatChannelType)iChannel);
		pstNotify->mutable_stroleid()->CopyFrom(stRoleObj.GetRoleID());
		pstNotify->set_sznickname(stRoleObj.GetNickName());
		pstNotify->set_izoneid(CModuleHelper::GetZoneID());
		pstNotify->set_szmessage(pMsg);

		//先广播给本线的玩家
		CZoneMsgHelper::SendZoneMsgToZoneAll(stChatNotify);

		//转发给世界服务器
		CZoneMsgHelper::SendZoneMsgToWorld(stChatNotify);
	}
	break;

	case CHAT_CHANNEL_SYSTEM:
	{
		//系统消息
		pstNotify->set_ichannel((ChatChannelType)iChannel);
		pstNotify->set_szmessage(pMsg);

		//发送给客户端
		CZoneMsgHelper::SendZoneMsgToRole(stChatNotify, &stRoleObj);
	}
	break;

	case CHAT_CHANNEL_PRIVATE:
	{
		//玩家私聊，暂时不开发
		;
	}
	break;

	default:
	{
		LOGERROR("Failed to do chat, invalid channel %d, uin %u\n", iChannel, stRoleObj.GetUin());
		return T_ZONE_PARA_ERROR;
	}
	break;
	}

	return T_SERVER_SUCCESS;
}

//发送失败的回复
int CChatHandler::SendFailedResponse(int iMsgID, int iResultID, const TNetHead_V2& rstNetHead)
{
	CZoneMsgHelper::GenerateMsgHead(stMsg, iMsgID);

	stMsg.mutable_stbody()->mutable_m_stzone_chat_response()->set_iresult(iResultID);

	CZoneMsgHelper::SendZoneMsgToClient(stMsg, rstNetHead);

	return 0;
}

//发送成功的回复
int CChatHandler::SendSuccessfulResponse()
{
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_CHAT_RESPONSE);

	CGameRoleObj* pRoleObj = m_pSession->GetBindingRole();
	ASSERT_AND_LOG_RTN_INT(pRoleObj);

	stMsg.mutable_stbody()->mutable_m_stzone_chat_response()->set_iresult(T_SERVER_SUCCESS);

	CZoneMsgHelper::SendZoneMsgToRole(stMsg, pRoleObj);

	return 0;
}
