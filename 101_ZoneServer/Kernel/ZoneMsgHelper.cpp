#include <iconv.h>
#include <errno.h>
#include <arpa/inet.h>

#include "Kernel/ZoneMsgHelper.hpp"
#include "Kernel/ModuleHelper.hpp"
#include "Kernel/ZoneObjectHelper.hpp"
#include "LogAdapter.hpp"
#include "Kernel/HandlerHelper.hpp"
#include "StringUtility.hpp"
#include "Kernel/UnitUtility.hpp"

using namespace ServerLib;

// 发送消息给本线所有用户
int CZoneMsgHelper::SendZoneMsgToZoneAll(GameProtocolMsg& rstGameMsg)
{
	if (GameTypeK32<CGameRoleObj>::GetUsedObjNumber() == 0)
	{
		return 0;
	}

	unsigned int auiSocketFD[MAX_SOCKETFD_NUMBER];
	CModuleHelper::GetZoneProtocolEngine()->SendZoneMsgListToMultiLotus(
		rstGameMsg, auiSocketFD, SEND_ALL_SESSION);

    return 0;
}

// 发送消息给本线所有用户, 除了pMe
int CZoneMsgHelper::SendZoneMsgToZoneAllExcludeMe(GameProtocolMsg& rstZoneMsg, CGameRoleObj* pMe)
{
	ASSERT_AND_LOG_RTN_INT(pMe);

	unsigned int auiSocketFD[MAX_SOCKETFD_NUMBER];
	auiSocketFD[0] = pMe->GetSession()->GetNetHead().m_uiSocketFD;

	CModuleHelper::GetZoneProtocolEngine()->SendZoneMsgListToMultiLotus(
		rstZoneMsg, auiSocketFD, SEND_ALL_SESSION_BUTONE);

    return 0;
}

//添加到发送列表
void CZoneMsgHelper::AddRoleToList(TRoleObjList& rstRoleList, CGameRoleObj* pRoleObj)
{
	ASSERT_AND_LOG_RTN_VOID(rstRoleList.m_iRoleNumber < MAX_ROLE_OBJECT_NUMBER_IN_ZONE);

	rstRoleList.m_apstRoleObj[rstRoleList.m_iRoleNumber] = pRoleObj;
	++rstRoleList.m_iRoleNumber;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CZoneMsgHelper::GenerateMsgHead(GameProtocolMsg& rstGameMsg, const unsigned int uiMsgID)
{
    rstGameMsg.Clear();

    // 初始化头部
    GameCSMsgHead* pstMsgHead = rstGameMsg.mutable_sthead();
    pstMsgHead->set_uimsgid((ProtocolMsgID)uiMsgID);
}

int CZoneMsgHelper::SendZoneMsgToClient(GameProtocolMsg& rstZoneMsg, 
                                          const TNetHead_V2& rstNetHead)
{
    CSessionManager* pSessionManager = CModuleHelper::GetSessionManager();
    ASSERT_AND_LOG_RTN_INT(pSessionManager);

    int iSessionID = (int)ntohl(rstNetHead.m_uiSocketFD);
    CGameSessionObj* pSession = pSessionManager->FindSessionByID(iSessionID);
    if (!pSession)
    {
        LOGDEBUG("Cannot Find Session: SessionID = %d\n", iSessionID);
        return -1;
    }

	// 未上线, 先不下发消息
	// logout response是游戏里最后一个回包，玩家状态已经被设置成离线了，所以特殊处理
	CGameRoleObj* pRoleObj = pSession->GetBindingRole();
	if (pRoleObj
		&& (!pRoleObj->IsOnline()) 
		&& (rstZoneMsg.sthead().uimsgid() != MSGID_LOGOUTSERVER_RESPONSE)
        && (rstZoneMsg.sthead().uimsgid() != MSGID_ZONE_LOGOUT_NOTIFY))
	{
		LOGERROR("Not online: %u, %d\n", pRoleObj->GetUin(), rstZoneMsg.sthead().uimsgid());
		return -2;
	}

    int iRet = CModuleHelper::GetZoneProtocolEngine()->SendZoneMsg(rstZoneMsg, rstNetHead, GAME_SERVER_LOTUS);
    
    return iRet;
}

int CZoneMsgHelper::SendZoneMsgToWorld(const GameProtocolMsg& rstZoneMsg)
{
    static TNetHead_V2 stDummyHead;

    int iRet = CModuleHelper::GetZoneProtocolEngine()->SendZoneMsg((GameProtocolMsg&)rstZoneMsg, stDummyHead, GAME_SERVER_WORLD);

    return iRet;
}

int CZoneMsgHelper::SendZoneMsgToRole(GameProtocolMsg& rstGameMsg, CGameRoleObj *pRoleObj)
{
    if(!pRoleObj)
    {
        return 0;
    }

    CGameSessionObj* pSession = pRoleObj->GetSession();
    if (!pSession)
    {
		LOGERROR("No session: %u\n", pRoleObj->GetUin());
        return -1;
    }

    // 设置Uin
    rstGameMsg.mutable_sthead()->set_uin(pRoleObj->GetUin());

    // 发送数据
    int iRet = SendZoneMsgToClient(rstGameMsg, pSession->GetNetHead());

    return iRet;
}

#define ENABLE_UINLIST

// 发送消息给很多人
int CZoneMsgHelper::SendZoneMsgToRoleList(GameProtocolMsg& rstZoneMsg, const TRoleObjList& rstRoleList)
{
#ifndef ENABLE_UINLIST

    for (int i = 0; i < rstRoleList.m_iRoleNumber; i++)
    {
        SendZoneMsgToRole(rstZoneMsg, rstRoleList.m_apstRoleObj[i]);
    }
    
#else

    unsigned int auiSocketFD[MAX_SOCKETFD_NUMBER];
    unsigned int uiSocketNumber = 0;

    // 分批发出去
    for (int i = 0; i < rstRoleList.m_iRoleNumber; i++)
    {
		if (!rstRoleList.m_apstRoleObj[i]->IsOnline())
		{
			continue;
		}

        CGameSessionObj* pSessionObj = rstRoleList.m_apstRoleObj[i]->GetSession();
        if (!pSessionObj)
        {
            continue;
        }

        auiSocketFD[uiSocketNumber] = pSessionObj->GetNetHead().m_uiSocketFD;
        uiSocketNumber++;

        if (uiSocketNumber == (unsigned int)MAX_SOCKETFD_NUMBER)
        {
            CModuleHelper::GetZoneProtocolEngine()->SendZoneMsgListToMultiLotus(
                rstZoneMsg, auiSocketFD, uiSocketNumber);

            uiSocketNumber = 0;
        }
    }

    // 最后一部分
    if (uiSocketNumber > 0)
    {
        if (uiSocketNumber > 1)
        {
            CModuleHelper::GetZoneProtocolEngine()->SendZoneMsgListToMultiLotus(
                rstZoneMsg, auiSocketFD, uiSocketNumber);
            return 0;
        }

        // 一个人, 按原来的格式发
		CGameSessionObj* pSessionObj = CModuleHelper::GetSessionManager()->FindSessionByID(ntohl(auiSocketFD[0]));
		if (pSessionObj)
		{
			SendZoneMsgToClient(rstZoneMsg, pSessionObj->GetNetHead());
		}
    }
#endif

    return 0;
}

