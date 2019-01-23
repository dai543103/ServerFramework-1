#include <string.h>
#include <arpa/inet.h>

#include "GameProtocol.hpp"
#include "StringUtility.hpp"
#include "SessionKeyUtility.hpp"
#include "LogAdapter.hpp"
#include "Kernel/ModuleHelper.hpp"
#include "Kernel/ZoneObjectHelper.hpp"
#include "Kernel/HandlerFactory.hpp"
#include "Kernel/ZoneMsgHelper.hpp"
#include "Kernel/AppLoop.hpp"
#include "Kernel/HandlerHelper.hpp"
#include "Kernel/ZoneOssLog.hpp"
#include "Kernel/UnitUtility.hpp"

#include "LoginHandler.hpp"

CLoginHandler::~CLoginHandler()
{
}

int CLoginHandler::OnClientMsg()
{
	switch (m_pRequestMsg->sthead().uimsgid())
	{
	case MSGID_ZONE_LOGINSERVER_REQUEST:
		{
			OnRequestLoginServer();
			break;
		}

	default:
		{
			break;
		}
	}

	return 0;
}

int CLoginHandler::OnRequestLoginServer()
{
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    int iRet = SecurityCheck(*m_pNetHead);
    if (iRet)
    {
        LOGERROR("Security Check Failed: Uin = %u, iRet = %d\n", m_pRequestMsg->sthead().uin(), iRet);
        LoginFailed(*m_pNetHead);
        return -1;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    const Zone_LoginServer_Request& rstRequest = m_pRequestMsg->stbody().m_stzone_loginserver_request();
    iRet = LoginRole(rstRequest.stroleid(), m_pNetHead, rstRequest);
    if (iRet < 0)
    {
        CHandlerHelper::SetErrorCode(T_ZONE_LOGINSERVER_FAILED);
        LoginFailed(*m_pNetHead);
    }

    return iRet;	
}

//通知角色登录
int CLoginHandler::NotifyRoleLogin(CGameRoleObj* pstRoleObj)
{
    return 0;
}

int CLoginHandler::SecurityCheck(const TNetHead_V2& rstNetHead)
{
    unsigned int uiUin = m_pRequestMsg->sthead().uin();
    if (uiUin == 0)
    {
        CHandlerHelper::SetErrorCode(T_ZONE_PARA_ERROR);
		return -1;
    }
	
	Zone_LoginServer_Request* pstRequest = m_pRequestMsg->mutable_stbody()->mutable_m_stzone_loginserver_request();

	//检查uin
	if (uiUin != pstRequest->stroleid().uin())
	{
		CHandlerHelper::SetErrorCode(T_ZONE_PARA_ERROR);
		return -2;
	}

	//是否名字包含屏蔽字
	if (CConfigManager::Instance()->GetBaseCfgMgr().IsContainMaskWord(pstRequest->strnickname()))
	{
		//包含屏蔽字
		CHandlerHelper::SetErrorCode(T_ZONE_PARA_ERROR);
		return -5;
	}

	//检查客户端版本号
	//pstRequest->uclientversion();
	
	int iRealNameStat = REAL_STAT_NONAME;

	CGMPrivConfigManager& stGMPrivMgr = CConfigManager::Instance()->GetGMPrivConfigManager();
	if (CConfigManager::Instance()->IsSSKeyCheckEnabled() && !stGMPrivMgr.CheckIsGMIP(rstNetHead.m_uiSrcIP))
	{
		//非GM玩家，检查SessionKey
		int iRet = CheckSessionKey(pstRequest->strsessionkey(), uiUin, CModuleHelper::GetWorldID(), iRealNameStat);
		if (iRet)
		{
			CHandlerHelper::SetErrorCode(iRet);
			return -3;
		}
	}

	pstRequest->set_irealnamestat(iRealNameStat);

    //todo jasonxiong 后续根据需要增加黑白名单
    /*
    CWhiteListConfig& rWhiteListConfig = 
        WhiteListCfgMgr();
    if (rWhiteListConfig.IsInBlackList(uiUin))
    {
        TRACESVR("uin: %u is in black list, can not login\n", uiUin);
        CHandlerHelper::SetErrorCode(EQEC_UinInBlackList);
        return -2;
    }
    */
	
    return 0;
}

//校验SessionKey
int CLoginHandler::CheckSessionKey(const std::string& strSessionKey, unsigned uiUin, int nWorldID, int& iRealNameStat)
{
	//session key格式：	 uin|time|WorldID|realname_stat|pic_url
	char szOriginKey[256] = { 0 };
	int iOriginKeyLen = sizeof(szOriginKey);
	int iRet = CSessionKeyUtility::DecryptSessionKey(strSessionKey.c_str(), strSessionKey.length(), szOriginKey, iOriginKeyLen);
	if (iRet)
	{
		LOGERROR("Failed to decrypt session key, uin %u, ret %d\n", uiUin, iRet);
		return T_ZONE_PARA_ERROR;;
	}

	unsigned uKeyUin = 0;
	unsigned uKeyTime = 0;
	int iKeyWorldID = 0;
	char szPicID[128] = { 0 };

	//session key : uin|time|WorldID|realname_stat|pic_url
	sscanf(szOriginKey, "%u|%u|%d|%d|%s", &uKeyUin, &uKeyTime, &iKeyWorldID, &iRealNameStat, szPicID);

	//设置玩家头像
	m_strPicID = szPicID;

	//检查参数,session key 1分钟后失效
	//todo jasonxiong 先调成30分钟，方便调试
	if (uKeyUin != uiUin || iKeyWorldID != nWorldID || (CTimeUtility::GetNowTime() - (int)uKeyTime) > 60*30)
	{
		LOGERROR("Failed to check session key, uin %u, world %d.\n", uiUin, nWorldID);
		return T_ZONE_PARA_ERROR;
	}

	return T_SERVER_SUCCESS;
}

int CLoginHandler::LoginRole(const RoleID& stRoleID, TNetHead_V2* pNetHead, const Zone_LoginServer_Request& rstRequest)
{
    ASSERT_AND_LOG_RTN_INT(pNetHead);

	int iSessionID = (int)ntohl(pNetHead->m_uiSocketFD);

	LOGDETAIL("Login Req: Uin = %u, Session = %d\n", stRoleID.uin(), iSessionID);

	// 该Session已经存在
	CSessionManager* pSessionManager = CModuleHelper::GetSessionManager();
	CGameSessionObj* pSessionObj = pSessionManager->FindSessionByID(iSessionID);
	if (pSessionObj)
	{
		LOGERROR("Session Already Exist: ID = %d\n", iSessionID);
		return -2;
	}

	// 该RoleID的Session已经存在，且处于未登录状态
	pSessionObj = pSessionManager->FindSessionByRoleID(stRoleID);
	if (pSessionObj && pSessionObj->GetBindingRole() == NULL)
	{
		LOGERROR("Session Alreay Binding Role: ID = %d, Uin = %u\n", iSessionID, stRoleID.uin());
		return -3;
	}

    // 创建一个新的会话
    pSessionObj = pSessionManager->CreateSession(pNetHead, stRoleID);
    if(!pSessionObj)
    {
        LOGERROR("Cannot Create Session: Uin = %u, SessionID = %d\n", stRoleID.uin(), iSessionID);  
        return -2;
    }

    // 保存客户端版本号
    pSessionObj->SetClientVersion(rstRequest.uclientversion());

	//保存登录原因
	pSessionObj->SetLoginReason(rstRequest.uloginreason());

	//设置玩家头像ID
	pSessionObj->SetPictureID(m_strPicID);

	if (rstRequest.strchannel().size() != 0)
	{
		//上报玩家登录信息
		CZoneOssLog::ReportUserLogin(stRoleID.uin(), rstRequest.straccount().c_str(), rstRequest.strdeviceid().c_str(), rstRequest.strchannel().c_str(),
			rstRequest.bisnew(), pSessionObj->GetClientIP());
	}

    // 向World发送踢人请求,相同uin的都踢下去
    int iRet = KickRoleFromWorldServer(stRoleID, iSessionID, rstRequest);
    if (iRet < 0)
    {
        CModuleHelper::GetSessionManager()->DeleteSession(iSessionID);
    }

    return iRet;
}

int CLoginHandler::SendFailedResponse(const unsigned int uiResultID, const TNetHead_V2& rstNetHead)
{
	static GameProtocolMsg stMsg;

    CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_LOGINSERVER_RESPONSE);

    Zone_LoginServer_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_loginserver_response();
    pstResp->Clear();

    pstResp->set_iresult(uiResultID);

    CZoneMsgHelper::SendZoneMsgToClient(stMsg, rstNetHead);

    return 0;
}

int CLoginHandler::SendSuccessfulResponse(CGameRoleObj* pLoginRoleObj)
{
	static GameProtocolMsg stMsg;

    CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_LOGINSERVER_RESPONSE);
    
    Zone_LoginServer_Response* pstLoginResp = stMsg.mutable_stbody()->mutable_m_stzone_loginserver_response();
    pstLoginResp->set_iresult(T_SERVER_SUCCESS);
	pstLoginResp->set_uin(pLoginRoleObj->GetUin());
    pstLoginResp->set_iworldid(CModuleHelper::GetWorldID());
    pstLoginResp->set_izoneid(CModuleHelper::GetZoneID());

    //返回玩家身上的基本信息
    BASEDBINFO* pstBaseInfo = pstLoginResp->mutable_stlogininfo()->mutable_stbaseinfo();
    pLoginRoleObj->UpdateBaseInfoToDB(*pstBaseInfo);

    //返回玩家的背包信息
    ITEMDBINFO* pstItemInfo = pstLoginResp->mutable_stlogininfo()->mutable_stiteminfo();
    pLoginRoleObj->UpdateRepThingsToDB(*pstItemInfo);

	//返回玩家的任务信息
	QUESTDBINFO* pstQuestInfo = pstLoginResp->mutable_stlogininfo()->mutable_stquestinfo();
	pLoginRoleObj->UpdateQuestToDB(*pstQuestInfo);

	//返回玩家的好友信息

	//返回玩家的邮件信息
	MAILDBINFO* pstMailInfo = pstLoginResp->mutable_stlogininfo()->mutable_stmailinfo();
	pLoginRoleObj->UpdateMailToDB(*pstMailInfo);

	//RESERVED2
	RESERVED2DBINFO* pstReserved2Info = pstLoginResp->mutable_stlogininfo()->mutable_streserved2();
	
	//玩家兑换信息
	EXCHANGEDBINFO* pstExchangeInfo = pstReserved2Info->mutable_stexchangeinfo();
	pLoginRoleObj->UpdateExchangeToDB(*pstExchangeInfo);

	//玩家充值信息
	RECHARGEDBINFO* pstRechargeInfo = pstReserved2Info->mutable_strechargeinfo();
	pLoginRoleObj->UpdateRechargeToDB(*pstRechargeInfo);

    CZoneMsgHelper::SendZoneMsgToRole(stMsg, pLoginRoleObj);

    return 0;
}

// 登录成功处理
int CLoginHandler::LoginOK(unsigned int uiUin, bool bNeedResponse)
{
    CGameRoleObj* pLoginRoleObj =  GameTypeK32<CGameRoleObj>::GetByKey(uiUin);
    ASSERT_AND_LOG_RTN_INT(pLoginRoleObj);

    //登录事件
    CGameEventManager::NotifyLogin(*pLoginRoleObj);

	if (bNeedResponse)
	{
		// 登录成功
		SendSuccessfulResponse(pLoginRoleObj);
	}

    // 通知其他用户
    NotifyRoleLogin(pLoginRoleObj);

    LOGDEBUG("Uin: %u\n", uiUin);

    // 记录流水
    CZoneOssLog::TraceLogin(*pLoginRoleObj);

    return 0;
}

// 登录失败处理
int CLoginHandler::LoginFailed(const TNetHead_V2& rstNetHead)
{ 
	unsigned int uiResultID = CHandlerHelper::GetErrorCode();
	SendFailedResponse(uiResultID, rstNetHead);

    return 0;
}

//通过World将相同uin的已经登录的号踢下线
int CLoginHandler::KickRoleFromWorldServer(const RoleID& stRoleID, int iFromSessionID, const Zone_LoginServer_Request& rstRequest)
{
	static GameProtocolMsg stMsg;

    CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_WORLD_KICKROLE_REQUEST);

    World_KickRole_Request* pstReq = stMsg.mutable_stbody()->mutable_m_stworld_kickrole_request();
	pstReq->set_bislogin(iFromSessionID>0);
	pstReq->set_ukickeduin(stRoleID.uin());

	KickerInfo* pstKicker = pstReq->mutable_stkicker();
	pstKicker->set_ifromworldid(CModuleHelper::GetWorldID());
	pstKicker->set_ifromzoneid(CModuleHelper::GetZoneID());
	pstKicker->set_isessionid(iFromSessionID);
	pstKicker->mutable_stroleid()->CopyFrom(stRoleID);
	pstKicker->set_strnickname(rstRequest.strnickname());
	pstKicker->set_strchannel(rstRequest.strchannel());
	pstKicker->set_irealnamestat(rstRequest.irealnamestat());
	pstKicker->set_straccount(rstRequest.straccount());
	pstKicker->set_strdeviceid(rstRequest.strdeviceid());

    LOGDEBUG("Send KickRole Request: Uin = %u\n", stRoleID.uin());

    int iRet = CZoneMsgHelper::SendZoneMsgToWorld(stMsg);

    return iRet;
}
