#pragma once

#include "UnixTime.hpp"
#include "AppDefW.hpp"
#include "HandlerFactory.hpp"
#include "FetchRoleWorldHandler.hpp"
#include "UpdateRoleWorldHandler.hpp"
#include "CreateRoleAccountHandler.hpp"
#include "DeleteRoleAccountHandler.hpp"
#include "ListRoleAccountHandler.hpp"
#include "GameMasterHandler.hpp"
#include "LogoutHandler.hpp"
#include "KickRoleWorldHandler.hpp"
#include "WorldNameHandler.hpp"
#include "ChatHandler.hpp"
#include "ListZoneHandler.hpp"
#include "OnlineStatHandler.hpp"
#include "RankInfoWorldHandler.h"
#include "MailWorldHandler.h"
#include "ExchangeWorldHandler.h"
#include "LimitLotteryWorldHandler.h"
#include "LasvegasWorldHandler.h"
#include "RechargeWorldHandler.h"

class CAppLoopW;
class CWorldProtocolEngineW
{
public:
    int Initialize(bool bResume);

public:
    // 将数据解包, 并交给Handler处理
    int OnRecvCode(char* pszMsgBuffer, const int iMsgLength, const SERVERBUSID& stFromBusID);

    // 将数据打包, 并发送到对端
    int SendWorldMsg(const GameProtocolMsg& rstWorldMsg, const SERVERBUSID& stToBusID);

private:

    int ForwardMsg(const GameProtocolMsg& stMsg);

private:
    char m_szCode[MAX_MSGBUFFER_SIZE];

    GameProtocolMsg m_stWorldMsg;

public:
    int RegisterAllHandler();

private:
    CFetchRoleWorldHandler m_stFetchRoleWorldHandler;
    CUpdateRoleWorldHandler m_stUpdateRoleWorldHandler;
    CGameMasterHandler m_stGameMasterHandler;
    CCreateRoleAccountHandler m_stCreateRoleAccountHandler;
    CDeleteRoleAccountHandler m_stDeleteRoleAccountHandler;
    CListRoleAccountHandler m_stListRoleAccountHandler;
    CWorldNameHandler m_stWorldNameHandler;
    COnlineStatHandler m_OnlineStatHandler;
    CLogoutHandler m_stLogoutHandler;
    CKickRoleWorldHandler m_stKickRoleWorldHandler;
    CChatHandler m_stChatHandler;
    CListZoneHandler m_stListZoneHandler;
	CRankInfoWorldHandler m_stRankInfoHandler;
	CMailWorldHandler m_stMailHandler;
	CExchangeWorldHandler m_stExchangeHandler;
	CLimitLotteryWorldHandler m_stLimitLotteryHandler;
	CLasvegasWorldHandler m_stLasvegasHandler;
	CRechargeWorldHandler m_stRechargeHandler;
};
