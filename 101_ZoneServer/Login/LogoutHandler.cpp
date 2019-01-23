#include <arpa/inet.h>

#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "Kernel/ModuleHelper.hpp"
#include "Kernel/GameSession.hpp"
#include "Kernel/GameRole.hpp"
#include "Kernel/ZoneObjectHelper.hpp"
#include "Kernel/ZoneMsgHelper.hpp"
#include "Kernel/HandlerHelper.hpp"
#include "Kernel/UpdateRoleInfoHandler.hpp"
#include "Kernel/HandlerHelper.hpp"
#include "Kernel/ZoneOssLog.hpp"
#include "Kernel/HandlerFactory.hpp"
#include "Kernel/UnitUtility.hpp"
#include "FishGame/FishpondObj.h"
#include "FishGame/FishpondManager.h"
#include "FishGame/FishUtility.h"
#include "Lottery/LasvegasManager.h"
#include "KickRoleWorldHandler.hpp"

#include "LogoutHandler.hpp"

CLogoutHandler::~CLogoutHandler()
{

}

int CLogoutHandler::OnClientMsg()
{
    switch (m_pRequestMsg->sthead().uimsgid())
    {
    case MSGID_LOGOUTSERVER_REQUEST:
        {
            OnRequestLogout();
            break;
        }

    default:
        {
            break;
        }
    }

    return 0;
}

int CLogoutHandler::OnRequestLogout()
{
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    int iRet = SecurityCheck();
    if (iRet < 0)
    {
		LOGERROR("Securit check fail: %d\n", iRet);
        return -1;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
	if (CUnitUtility::IsUnitStatusSet(&(m_pRoleObj->GetUnitInfo()), EGUS_LOGOUT))
	{
		LOGERROR("Role in logout: %u\n", m_pRoleObj->GetRoleID().uin());
		return -2;
	}

	//设置下线原因
    iRet = LogoutRole(m_pRoleObj, LOGOUT_REASON_NORMAL);

    return iRet;
}

int CLogoutHandler::LogoutRole(CGameRoleObj* pRoleObj, int iReason)
{
    ASSERT_AND_LOG_RTN_INT(pRoleObj);

    CGameSessionObj* pSessionObj = pRoleObj->GetSession();
	ASSERT_AND_LOG_RTN_INT(pSessionObj);

    unsigned int uiUin = pRoleObj->GetRoleID().uin();

    LOGDETAIL("Role log out: Uin = %u, Session = %d, Reason = %d .................\n", uiUin, pSessionObj->GetID(), iReason);

	CUnitUtility::SetUnitStatus(&(pRoleObj->GetUnitInfo()), EGUS_LOGOUT);
    time_t stNow = time(NULL);
    pRoleObj->SetLogoutTime(stNow);
	pRoleObj->SetLastLoginTime(pRoleObj->GetLoginTime());
	pRoleObj->SetLogoutReason(iReason);

    // 1. 登出准备, 停止角色的所有行为, 更新角色的数据
    LogoutPrepare(pRoleObj);

	//向World推送Logout Notify
	static GameProtocolMsg stMsg;
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_LOGOUT_NOTIFY);
	Zone_Logout_Notify* pstNotify = stMsg.mutable_stbody()->mutable_m_stzone_logout_notify();
	pstNotify->mutable_stroleid()->CopyFrom(pRoleObj->GetRoleID());
	CZoneMsgHelper::SendZoneMsgToWorld(stMsg);

    //同步玩家角色数据到数据库
    int iRet = CUpdateRoleInfo_Handler::UpdateRoleInfo(pRoleObj, 1);
    if(iRet)
    {
		LOGERROR("Failed to update role info before logout, uin %u\n", uiUin);
		return iRet;
    }

    return 0;
}

// 登出准备工作, 停止玩家的所有行为
int CLogoutHandler::LogoutPrepare(CGameRoleObj* pRoleObj)
{
    ASSERT_AND_LOG_RTN_INT(pRoleObj);

    CGameSessionObj* pSessionObj = pRoleObj->GetSession();
    ASSERT_AND_LOG_RTN_INT(pSessionObj);

    // 记录流水
    CZoneOssLog::TraceLogout(*pRoleObj);

    unsigned int uiUin = pRoleObj->GetRoleID().uin();

	// 激发登出事件
	CGameEventManager::NotifyLogout(*pRoleObj);

	if (pRoleObj->GetTableID() != 0)
	{
		//退出鱼池
		CFishpondObj* pstFishpondObj = CFishUtility::GetFishpondByID(pRoleObj->GetTableID());
		pstFishpondObj->ExitFishpond(*pRoleObj, false);
		CFishpondManager::Instance()->DelFishpond(*pstFishpondObj);
	}

	//退出转盘
	CLasvegasManager::Instance()->EnterLasvegas(*pRoleObj, false);

    LOGDETAIL("LogoutPrepare:  Uin = %u, Sessoion = %d\n", uiUin, pSessionObj->GetID());

    return 0;
}

// 消息通知和数据删除
int CLogoutHandler::LogoutAction(CGameRoleObj* pRoleObj)
{
    ASSERT_AND_LOG_RTN_INT(pRoleObj);

    CGameSessionObj* pSessionObj = pRoleObj->GetSession();
    ASSERT_AND_LOG_RTN_INT(pSessionObj);

    // 返回成功回复
	if(pRoleObj->GetKicker().stkicker().ifromzoneid() <= 0)
	{
		//只有主动登出时才发送resp
		SendSuccessfulResponse(pRoleObj);
	}

	// 通知视野内玩家
    NotifyRoleLogout(pRoleObj);

    // 断开会话
    CModuleHelper::GetSessionManager()->DeleteSession(pSessionObj->GetID());

	// 通知World，改变玩家状态
    NotifyLogoutToWorld(pRoleObj->GetRoleID());

    unsigned int uiUin = pRoleObj->GetRoleID().uin();

    // 通知World踢人成功
    World_KickRole_Request& rstKicker = pRoleObj->GetKicker();
    if (rstKicker.stkicker().ifromzoneid() > 0)
    {
		CKickRoleWorldHandler::SendSuccessfullResponse(rstKicker);
    }

    //删除玩家角色
    CUnitUtility::DeleteUnit(&(pRoleObj->GetUnitInfo()));

    pRoleObj = NULL;

    LOGDETAIL("Logout Success: Uin = %u\n", uiUin);

    return 0;
}

int CLogoutHandler::SecurityCheck()
{
	if (m_pRequestMsg->stbody().ByteSize() == 0)
	{
		CSessionManager* pSessionManager = CModuleHelper::GetSessionManager();

		int iSessionID = (int)ntohl(m_pNetHead->m_uiSocketFD);
		m_pSession = pSessionManager->FindSessionByID(iSessionID);
		if (!m_pSession)
		{
			LOGERROR("No session: ID = %d\n", iSessionID);
			return -1;
		}

		m_pRoleObj = m_pSession->GetBindingRole();
		if (!m_pRoleObj)
		{
			LOGERROR("Connect Closed: Session = %d\n", iSessionID);
			CModuleHelper::GetSessionManager()->DeleteSession(m_pSession->GetID());
			return -3;
		}

		LOGDEBUG("Connect Closed: Uin = %u， Session = %d\n", m_pRoleObj->GetRoleID().uin(), iSessionID);

		return 0;
	}

	int iRet = IHandler::SecurityCheck();
	if (iRet < 0)
	{
		LOGERROR("IHandler::SecurityCheck failed, iRet:%d\n", iRet);
		return iRet;
	}

    return 0;
}

int CLogoutHandler::NotifyLogoutToWorld(const RoleID& stRoleID)
{
	static GameProtocolMsg stMsg;

    CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_LOGOUT_NOTIFY);

	stMsg.mutable_stbody()->mutable_m_stzone_logout_notify()->mutable_stroleid()->CopyFrom(stRoleID);

	return CZoneMsgHelper::SendZoneMsgToWorld(stMsg);
}

void CLogoutHandler::NotifyRoleLogout(CGameRoleObj* pstRoleObj)
{
	ASSERT_AND_LOG_RTN_VOID(pstRoleObj);

	static GameProtocolMsg stMsg;

	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_LOGOUT_NOTIFY);
	stMsg.mutable_stbody()->mutable_m_stzone_logout_notify()->mutable_stroleid()->CopyFrom(pstRoleObj->GetRoleID());

	//发送给自己
	CZoneMsgHelper::SendZoneMsgToRole(stMsg, pstRoleObj);

	return;
}

int CLogoutHandler::SendSuccessfulResponse(CGameRoleObj* pRoleObj)
{
    ASSERT_AND_LOG_RTN_INT(pRoleObj);
	 
	static GameProtocolMsg stMsg;

    CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_LOGOUTSERVER_RESPONSE);
	LogoutServer_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stlogoutserver_response();
	pstResp->set_iresult(T_SERVER_SUCCESS);
	pstResp->set_ireason(pRoleObj->GetLogoutReason());
	pstResp->set_irealnamestat(pRoleObj->GetRealNameStat());

    CZoneMsgHelper::SendZoneMsgToRole(stMsg, pRoleObj);

    return 0;
}

