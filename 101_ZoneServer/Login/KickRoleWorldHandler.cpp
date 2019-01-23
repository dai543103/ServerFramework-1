
#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "Kernel/HandlerHelper.hpp"
#include "Kernel/ZoneMsgHelper.hpp"
#include "Kernel/AppLoop.hpp"
#include "Kernel/UnitUtility.hpp"
#include "LogoutHandler.hpp"

#include "KickRoleWorldHandler.hpp"

CKickRoleWorldHandler::~CKickRoleWorldHandler()
{
}

int CKickRoleWorldHandler::OnClientMsg()
{
    switch (m_pRequestMsg->sthead().uimsgid())
    {
    case MSGID_WORLD_KICKROLE_REQUEST:
        {
            OnRequestKickRole();
            break;
        }

    case MSGID_WORLD_KICKROLE_RESPONSE:
        {
            OnResponseKickRole();
            break;
        }

    default:
        {
            break;
        }
    }

    return 0;
}

// 收到World的被踢下线通知
int CKickRoleWorldHandler::OnRequestKickRole()
{
	const World_KickRole_Request& rstReq = m_pRequestMsg->stbody().m_stworld_kickrole_request();
	const unsigned int uiKickedUin = rstReq.ukickeduin();

    LOGDEBUG("Recv KickRole Req: Uin = %u\n", uiKickedUin);

    CGameRoleObj* pRoleObj = CUnitUtility::GetRoleByUin(uiKickedUin);
    // 找不到玩家, 返回成功
    if (!pRoleObj)
    {
        LOGERROR("KickRole OK: Uin = %u\n", uiKickedUin);

        SendSuccessfullResponse(rstReq);
        return 0;
    }

	//设置踢人者
	pRoleObj->SetKicker(rstReq);

    // 将玩家踢下线 
    int iRet = CLogoutHandler::LogoutRole(pRoleObj, LOGOUT_REASON_KICKED);
    if (iRet < 0)
    {
        LOGERROR("KickRole Failed: Uin = %u\n", uiKickedUin);
        
        SendFailedResponse(rstReq);
        return iRet;
    }
     
    pRoleObj->SetKicker(rstReq);

    return iRet;
}


// 收到World的踢人下线回复
int CKickRoleWorldHandler::OnResponseKickRole()
{
	const World_KickRole_Response& stResp = m_pRequestMsg->stbody().m_stworld_kickrole_response();

	const KickerInfo& stKicker = stResp.stkicker();
	unsigned int uiUin = stKicker.stroleid().uin();

	LOGDEBUG("Recv KickRole Resp: Uin = %u, iRet = %d\n", uiUin, stResp.iresult());

	if (stKicker.ifromworldid() != CModuleHelper::GetWorldID()
		|| stKicker.ifromzoneid() != CModuleHelper::GetZoneID())
	{
		LOGERROR("Not From Mine!\n");
		return -1;
	}

    // 找到Session
    CGameSessionObj *pSession = CModuleHelper::GetSessionManager()->FindSessionByID(stKicker.isessionid());
    if (!pSession)
    {
		LOGERROR("Cannot find Session: ID = %d\n", stKicker.isessionid());
        return -1;
    }

	// 确认Session未登录
	if (pSession->GetBindingRole() != NULL)
	{
		LOGERROR("Session Already Binding Role!\n");
		return -2;
	}

    if (stResp.iresult() != T_SERVER_SUCCESS)
    {
		if (stResp.bislogin())
		{
            //如果是登录
			CLoginHandler::LoginFailed(pSession->GetNetHead());
			CModuleHelper::GetSessionManager()->DeleteSession(pSession->GetID());
		}

		LOGERROR("Kick fail: %u, %d\n", uiUin, stResp.iresult());
        return -3;
    }

    // 如果踢人成功, 则再次确认Zone没有数据
    CGameRoleObj* pRoleObj = CUnitUtility::GetRoleByUin(uiUin);
    if (pRoleObj)
    {
		if (stResp.bislogin())
		{
			CLoginHandler::LoginFailed(pSession->GetNetHead());
			CModuleHelper::GetSessionManager()->DeleteSession(pSession->GetID());
		}

		LOGERROR("Role Already Login!\n");
		return -4;
	}

	//如果是登录则重新拉取数据
	if(stResp.bislogin())
	{
		int iRet = FetchRoleFromWorldServer(pSession->GetRoleID(), stKicker, 0);
		if (iRet < 0)
		{
			CLoginHandler::LoginFailed(pSession->GetNetHead());
			CModuleHelper::GetSessionManager()->DeleteSession(pSession->GetID());
			return -5;
		}
	}

	LOGDEBUG("Recv KickRole Ok: Uin = %u\n", uiUin);

    return 0;
}

int CKickRoleWorldHandler::FetchRoleFromWorldServer(const RoleID& stRoleID, const KickerInfo& stKicker, char cEnterType)
{
	static GameProtocolMsg stMsg;

    CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_WORLD_FETCHROLE_REQUEST);
	stMsg.mutable_sthead()->set_uin(stRoleID.uin());

	World_FetchRole_Request* pstFetchRequest = stMsg.mutable_stbody()->mutable_m_stworld_fetchrole_request();

	pstFetchRequest->mutable_stroleid()->CopyFrom(stRoleID);
	pstFetchRequest->set_bislogin(true);
	pstFetchRequest->set_ireqid(CModuleHelper::GetZoneID());
	pstFetchRequest->mutable_stkicker()->CopyFrom(stKicker);

    LOGDETAIL("Send FetchRole Request: Uin = %u\n", stRoleID.uin());

    int iRet = CZoneMsgHelper::SendZoneMsgToWorld(stMsg);

    return iRet;
}

// 返回成功回复
int CKickRoleWorldHandler::SendSuccessfullResponse(const World_KickRole_Request& rstKicker)
{
	static GameProtocolMsg stMsg;

    CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_WORLD_KICKROLE_RESPONSE);
    World_KickRole_Response* pstResponse = stMsg.mutable_stbody()->mutable_m_stworld_kickrole_response();
	pstResponse->set_bislogin(rstKicker.bislogin());
	pstResponse->set_ukickeduin(rstKicker.ukickeduin());
	pstResponse->mutable_stkicker()->CopyFrom(rstKicker.stkicker());
	pstResponse->set_iresult(T_SERVER_SUCCESS);
	
    CZoneMsgHelper::SendZoneMsgToWorld(stMsg);

    return 0;
}

// 返回失败回复
int CKickRoleWorldHandler::SendFailedResponse(const World_KickRole_Request& rstKicker)
{
	static GameProtocolMsg stMsg;

    CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_WORLD_KICKROLE_RESPONSE);
    World_KickRole_Response* pstResponse = stMsg.mutable_stbody()->mutable_m_stworld_kickrole_response();
	pstResponse->set_bislogin(rstKicker.bislogin());
	pstResponse->set_ukickeduin(rstKicker.ukickeduin());
	pstResponse->mutable_stkicker()->CopyFrom(rstKicker.stkicker());
	pstResponse->set_iresult(T_ZONE_LOGINSERVER_FAILED);

    CZoneMsgHelper::SendZoneMsgToWorld(stMsg);

    return 0;
}
