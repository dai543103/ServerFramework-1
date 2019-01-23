#include <assert.h>
#include <string.h>

#include "ProtoDataUtility.hpp"
#include "GameProtocol.hpp"
#include "Kernel/ModuleHelper.hpp"
#include "Kernel/ZoneObjectHelper.hpp"
#include "Kernel/HandlerFactory.hpp"
#include "ErrorNumDef.hpp"
#include "Kernel/UpdateRoleInfoHandler.hpp"
#include "Kernel/ZoneMsgHelper.hpp"
#include "Kernel/UnitUtility.hpp"
#include "Login/LoginHandler.hpp"
#include "Kernel/HandlerHelper.hpp"
#include "CommDefine.h"
#include "Login/LogoutHandler.hpp"
#include "Resource/ResourceUtility.h"
#include "RepThings/RepThingsUtility.hpp"

#include "FetchRoleWorldHandler.hpp"

using namespace ServerLib;

CFetchRoleWorldHandler::~CFetchRoleWorldHandler(void)
{
}

int CFetchRoleWorldHandler::OnClientMsg()
{
    int iRet = -1;
    switch (m_pRequestMsg->sthead().uimsgid())
    {
    case MSGID_WORLD_FETCHROLE_RESPONSE:
        {
            iRet = OnFetchRole();
            break;
        }

    default:
        {
            break;
        }
    }

    return iRet;
}

int CFetchRoleWorldHandler::OnFetchRole()
{
    const World_FetchRole_Response& rstFetchResp = m_pRequestMsg->stbody().m_stworld_fetchrole_response();
    unsigned int uiUin = rstFetchResp.stroleid().uin();

    // 获取会话
    CSessionManager* pSessionManager = CModuleHelper::GetSessionManager();
    m_pSession = pSessionManager->FindSessionByRoleID(rstFetchResp.stroleid());
    if(!m_pSession)
    {
        TRACESVR("Cannot Find Session: Uin = %u\n", uiUin);
        return -2;
    }

    // Session 已经被使用,login时检查
    if (rstFetchResp.bislogin() && m_pSession->GetBindingRole() != NULL)
    {
        TRACESVR("Session Already Binding Role: Uin = %u, Session = %d\n", uiUin, m_pSession->GetID());
        CHandlerHelper::SetErrorCode(T_ZONE_SESSION_EXISTS_ERR);
        CLoginHandler::LoginFailed(m_pSession->GetNetHead());
        return -3;
    }

    // 获取角色信息失败
    if (rstFetchResp.iresult() != 0)
    {
        TRACESVR("FetchRole Failed: Uin = %u, ResultID = %d\n", uiUin, rstFetchResp.iresult());
        CHandlerHelper::SetErrorCode(T_ZONE_SESSION_EXISTS_ERR);
        CLoginHandler::LoginFailed(m_pSession->GetNetHead());
        pSessionManager->DeleteSession(m_pSession->GetID());
        return -4;
    }

    // 检查uin是否一致
    if (uiUin != m_pSession->GetRoleID().uin())
    {
        CHandlerHelper::SetErrorCode(T_ZONE_SESSION_EXISTS_ERR);
        CLoginHandler::LoginFailed(m_pSession->GetNetHead());
        pSessionManager->DeleteSession(m_pSession->GetID());

        // 清除World缓存
        CLogoutHandler::NotifyLogoutToWorld(rstFetchResp.stroleid());

        TRACESVR("Invalid Session: Uin = %u\n", uiUin);

        return -5;
    }

    TRACESVR("FetchRole OK: Uin = %u\n", uiUin);

    // 登录角色
    int iRet = LoginRole();
    if (iRet < 0)
    {
        CHandlerHelper::SetErrorCode(T_ZONE_SESSION_EXISTS_ERR);
        CLoginHandler::LoginFailed(m_pSession->GetNetHead());
        pSessionManager->DeleteSession(m_pSession->GetID());

        // 清除World缓存
        CLogoutHandler::NotifyLogoutToWorld(rstFetchResp.stroleid());

        TRACESVR("LoginRole Failed: iRet = %d\n", iRet);

        return -6;
    }

    TRACESVR("LoginRole OK: Uin = %u\n", uiUin);

    return 0;
}

int CFetchRoleWorldHandler::LoginRole()
{
    const World_FetchRole_Response& rstFetchResp = m_pRequestMsg->stbody().m_stworld_fetchrole_response();
    unsigned int uiUin = rstFetchResp.stroleid().uin();

    // 创建角色对象
    m_pRoleObj = (CGameRoleObj*)CUnitUtility::CreateUnit(uiUin);
    if(!m_pRoleObj)
    {
        TRACESVR("Create RoleObj Failed.\n");
        return -1;
    }

    // 初始化角色数据系统
    int iRet = m_pRoleObj->InitRole(rstFetchResp.stroleid());
    if (iRet < 0)
    {
        TRACESVR("Init Role Failed: iRet = %d\n", iRet);

        CUnitUtility::DeleteUnit(&m_pRoleObj->GetUnitInfo());

        return -2;
    }

    // 将会话和角色绑定
    m_pSession->SetBindingRole(m_pRoleObj);
    m_pRoleObj->SetSessionID(m_pSession->GetID());

    // 初始化角色数据
    iRet = InitRoleData();
    if (iRet < 0)
    {
        CUnitUtility::DeleteUnit(&m_pRoleObj->GetUnitInfo());
        TRACESVR("Init RoleData Failed: iRet = %d\n", iRet);

        return -3;
    }

    // 通知登录成功
    CLoginHandler::LoginOK(uiUin);

    // 登陆后初始化
    iRet = InitRoleAfterLogin(*m_pRoleObj, rstFetchResp);
	if (iRet)
	{
		CUnitUtility::DeleteUnit(&m_pRoleObj->GetUnitInfo());
		TRACESVR("InitRoleAfterLogin Failed: iRet = %d\n", iRet);
		
		return -4;
	}

    return 0;
}

// 初始化角色数据
int CFetchRoleWorldHandler::InitRoleData()
{
    const World_FetchRole_Response& rstFetchResp = m_pRequestMsg->stbody().m_stworld_fetchrole_response();
    unsigned int uiUin = rstFetchResp.stroleid().uin();

    CGameSessionObj *pSession = m_pRoleObj->GetSession();
    if (!pSession)
    {
        TRACESVR("No session: %u\n", uiUin);
        return -20;
    }

    //1.初始化玩家的基本信息
    BASEDBINFO stBaseInfo;
    if(!DecodeProtoData(rstFetchResp.stuserinfo().strbaseinfo(), stBaseInfo))
    {
        TRACESVR("Failed to decode base proto data, uin %u\n", uiUin);
        return -21;
    }
    m_pRoleObj->InitBaseInfoFromDB(stBaseInfo, rstFetchResp.stkicker());

    //2.初始化玩家的任务信息
    QUESTDBINFO stQuestInfo;
    if(!DecodeProtoData(rstFetchResp.stuserinfo().strquestinfo(), stQuestInfo))
    {
        TRACESVR("Failed to decode quest proto data, uin %u\n", uiUin);
        return -22;
    }
	m_pRoleObj->InitQuestFromDB(stQuestInfo);
    
    //3.初始化玩家的物品信息
    ITEMDBINFO stItemInfo;
    if(!DecodeProtoData(rstFetchResp.stuserinfo().striteminfo(), stItemInfo))
    {
        TRACESVR("Failed to decode item proto data, uin %u\n", uiUin);
        return -23;
    }
    m_pRoleObj->InitRepThingsFromDB(stItemInfo);

    //4.初始化玩家的好友信息
    FRIENDDBINFO stFriendInfo;
    if(!DecodeProtoData(rstFetchResp.stuserinfo().strfriendinfo(), stFriendInfo))
    {
        TRACESVR("Failed to decode friend proto data, uin %u\n", uiUin);
        return -26;
    }

	//5.初始化玩家的邮件信息
	MAILDBINFO stMailInfo;
	if (!DecodeProtoData(rstFetchResp.stuserinfo().strmailinfo(), stMailInfo))
	{
		TRACESVR("Failed to decode mail proto data, uin %u\n", uiUin);
		return -27;
	}
	m_pRoleObj->InitMailFromDB(stMailInfo);

    //6.玩家的Reserved1字段,离线数据单独逻辑处理

    //7.初始化玩家的Reserved2字段
    RESERVED2DBINFO stReserved2Info;
    if(!DecodeProtoData(rstFetchResp.stuserinfo().strreserved2(), stReserved2Info))
    {
        TRACESVR("Failed to decode reserved2 proto data, uin %u\n", uiUin);
        return -29;
    }

	//玩家兑换信息
	m_pRoleObj->InitExchangeFromDB(stReserved2Info.stexchangeinfo());

	//玩家充值信息
	m_pRoleObj->InitRechargeFromDB(stReserved2Info.strechargeinfo());

    // GM标志
    //todo jasonxiong 先注释掉所有GM相关的功能，后续需要时再进行开发
    /*
    m_pRoleObj->GetRoleInfo().m_stBaseProfile.m_cGMType = rstDBRoleInfo.fGM;
    CWhiteListConfig& rWhiteListConfig = WhiteListCfgMgr();
    bool bGM = rWhiteListConfig.IsInGMList(m_pRoleObj->GetRoleID().m_uiUin);
    if (bGM)
    {
        CUnitUtility::SetUnitStatus(&(m_pRoleObj->GetRoleInfo().m_stUnitInfo), EUS_ISGM);
    }
    else
    {
        CUnitUtility::ClearUnitStatus(&(m_pRoleObj->GetRoleInfo().m_stUnitInfo), EUS_ISGM);
    } 
    */ 

    // 各种时间
    m_pRoleObj->SetLoginCount(stBaseInfo.ilogincount()+1);

    m_pRoleObj->SetOnline();      

	//更新玩家结算信息
	m_pRoleObj->UpdateTallyInfo();

	if (m_pRoleObj->GetLoginCount() == 1)
	{
		//新账号第一次登录,设置实时输赢
		m_pRoleObj->GetTallyInfo().lUserWinNum = m_pRoleObj->GetUserWinNum();
		m_pRoleObj->SetUserWinNum(0);
	}

    return 0;
}

// 登录后初始化
int CFetchRoleWorldHandler::InitRoleAfterLogin(CGameRoleObj& stRoleObj, const World_FetchRole_Response& stResp)
{
	unsigned uiUin = stResp.stroleid().uin();

	//推送服务时间同步
	CUnitUtility::SendSyncTimeNotify(&stRoleObj, CTimeUtility::GetMSTime());

	//处理离线数据
	RESERVED1DBINFO stReserved1Info;
	if (!DecodeProtoData(stResp.stuserinfo().strreserved1(), stReserved1Info))
	{
		TRACESVR("Failed to decode reserved1 proto data, uin %u\n", uiUin);
		return -1;
	}

	int iRet = T_SERVER_SUCCESS;

	//处理离线充值
	const PAYOFFLINEDBINFO& stPayInfo = stReserved1Info.stpayinfo();
	CRechargeManager& stRechargeMgr = stRoleObj.GetRechargeManager();
	for (int i = 0; i < stPayInfo.strecords_size(); ++i)
	{
		const RechargeRecord& stOneRecharge = stPayInfo.strecords(i);
		iRet = stRechargeMgr.UserRecharge(stOneRecharge.irechargeid(), stOneRecharge.itime(), stOneRecharge.strorderid());
		if (iRet)
		{
			//只打印错误日志
			LOGERROR("Failed to do offline recharge, recharge id %d, time %d, uin %u\n", stOneRecharge.irechargeid(), stOneRecharge.itime(), uiUin);
		}
	}

	//处理离线增加资源
	const RESOFFLINEDBINFO& stResInfo = stReserved1Info.stresinfo();
	for (int i = 0; i < stResInfo.staddres_size(); ++i)
	{
		const AddResInfo& stOneAddRes = stResInfo.staddres(i);
		if (!CResourceUtility::AddUserRes(stRoleObj, stOneAddRes.irestype(), stOneAddRes.iaddnum()))
		{
			LOGERROR("Failed to offline add res type %d, num %d, uin %u\n", stOneAddRes.irestype(), stOneAddRes.iaddnum(), uiUin);
		}
	}

	//处理离线道具
	const ItemSlotInfo& stItemInfo = stReserved1Info.stiteminfo();
	for (int i = 0; i < stItemInfo.stslots_size(); ++i)
	{
		const OneSlotInfo& stOneItem = stItemInfo.stslots(i);
		iRet = CRepThingsUtility::AddItemNum(stRoleObj, stOneItem.iitemid(), stOneItem.iitemnum(), ITEM_CHANNEL_GMADD);
		if (iRet)
		{
			LOGERROR("Failed to offline add item, id %d, num %d, uin %u\n", stOneItem.iitemid(), stOneItem.iitemnum(), uiUin);
		}
	}

	//处理离线邮件
	const MAILOFFLINEDBINFO& stMailInfo = stReserved1Info.stmailinfo();
	for (int i = 0; i < stMailInfo.stmails_size(); ++i)
	{
		//增加邮件
		stRoleObj.GetMailManager().AddNewMail(stMailInfo.stmails(i));
	}

    return 0;
}
