#include <math.h>

#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "LogFile.hpp"
#include "AppLoopW.hpp"
#include "HandlerFactory.hpp"
#include "MsgStatistic.hpp"
#include "WorldMsgHelper.hpp"
#include "ServerBusManager.h"

#include "WorldProtocolEngineW.hpp"

using namespace ServerLib;

int CWorldProtocolEngineW::Initialize(bool bResume)
{
    m_stWorldMsg.Clear();

    return 0;
}

int CWorldProtocolEngineW::OnRecvCode(char* pszMsgBuffer, const int iMsgLength, const SERVERBUSID& stFromBusID)
{
    ASSERT_AND_LOG_RTN_INT(pszMsgBuffer);

    int iRet = -1;
    timeval tvtmp = {0, 0};

    //解码消息
    char* pRealBuff = pszMsgBuffer + sizeof(unsigned short);
    int iBuffLen = iMsgLength - sizeof(unsigned short);

	m_stWorldMsg.Clear();
    if(!m_stWorldMsg.ParseFromArray(pRealBuff, iBuffLen))
    {
        LOGERROR("Fail to decode proto msg from buff!\n");
        return -1;
    }

    unsigned int uiMsgID = m_stWorldMsg.sthead().uimsgid();

    //根据MsgID获取消息处理器Handler
    IHandler* pHandler = CHandlerFactory::GetHandler(uiMsgID);
    if (!pHandler)
    {
		//找不到处理器，直接转发
        return ForwardMsg(m_stWorldMsg);
    }

	if (uiMsgID != MSGID_WORLD_GETRANKINFO_REQUEST && uiMsgID != MSGID_WORLD_UPDATERANK_REQUEST)
	{
		//拉取排名和更新排名不打印
		LOGDEBUG("Recv msg from %d, id %d, length %d.\n", stFromBusID.usServerID, uiMsgID, iMsgLength);
	}

    iRet = pHandler->OnClientMsg(&m_stWorldMsg);
    if(iRet < 0)
    {
        return -3;
    }

    MsgStatisticSingleton::Instance()->AddMsgStat(uiMsgID, iRet, 0, tvtmp, ESMT_FROM_SERVER);

    return 0;
}

int CWorldProtocolEngineW::ForwardMsg(const GameProtocolMsg& stMsg)
{
    switch (stMsg.sthead().uimsgid())
    {
	case MSGID_WORLD_WRITELOG_REQUEST:
	{
		return CWorldMsgHelper::SendWorldMsgToLogServer(stMsg);
	}
	break;
	
	case MSGID_WORLD_GETCARDNO_REQUEST:
	{
		//发送给兑换服务
		return CWorldMsgHelper::SendWorldMsgToExchange(stMsg);
	}
	break;

	case MSGID_WORLD_GETCARDNO_RESPONSE:
	{
		//发送给Zone服务
		int iZoneID = stMsg.stbody().m_stworld_getcardno_response().ifromzoneid();
		return CWorldMsgHelper::SendWorldMsgToWGS(stMsg, iZoneID);
	}
	break;

	default:
	{
		LOGERROR("Unknown msg id %d..........................\n", m_stWorldMsg.sthead().uimsgid());
		return -1;
	}
	break;
    }

    return 0;
}

int CWorldProtocolEngineW::SendWorldMsg(const GameProtocolMsg& rstWorldMsg, const SERVERBUSID& stToBusID)
{
    int iBuffLen = 0;
    char* pBuff = NULL;
    unsigned short ushTotalLength = 0;
    if(stToBusID.usServerID == GAME_SERVER_ROLEDB ||
		stToBusID.usServerID == GAME_SERVER_NAMEDB)
    {
        //2字节长度
        ushTotalLength = rstWorldMsg.ByteSize()+ sizeof(unsigned short) + sizeof(unsigned int);
        m_szCode[0] = ushTotalLength / 256;
        m_szCode[1] = ushTotalLength % 256;

        //如果是DBServer加4字节uin
        *((unsigned*)(&m_szCode[sizeof(unsigned short)])) = rstWorldMsg.sthead().uin();

        //数据
        iBuffLen = sizeof(m_szCode) - sizeof(unsigned short) - sizeof(unsigned int);
        pBuff = &m_szCode[sizeof(unsigned short) + sizeof(unsigned int)];
    }
    else
    {
        //2字节长度
        ushTotalLength = rstWorldMsg.ByteSize()+ sizeof(unsigned short);
        m_szCode[0] = ushTotalLength / 256;
        m_szCode[1] = ushTotalLength % 256;

        //数据
        iBuffLen = sizeof(m_szCode) - sizeof(unsigned short);
        pBuff = &m_szCode[sizeof(unsigned short)];
    }

    bool bRet = rstWorldMsg.SerializeToArray(pBuff, iBuffLen);
    if(!bRet)
    {
        LOGERROR("fail to parse world proto msg to array!\n");
        return -1;
    }

	return CServerBusManager::Instance()->SendOneMsg(m_szCode, ushTotalLength, stToBusID);
}

int CWorldProtocolEngineW::RegisterAllHandler()
{
    // RoleDB
    CHandlerFactory::RegisterHandler(MSGID_WORLD_FETCHROLE_RESPONSE, &m_stFetchRoleWorldHandler);
    CHandlerFactory::RegisterHandler(MSGID_WORLD_FETCHROLE_REQUEST, &m_stFetchRoleWorldHandler);
	CHandlerFactory::RegisterHandler(MSGID_WORLD_CREATEROLE_RESPONSE, &m_stFetchRoleWorldHandler);

    CHandlerFactory::RegisterHandler(MSGID_WORLD_UPDATEROLE_RESPONSE, &m_stUpdateRoleWorldHandler);
    CHandlerFactory::RegisterHandler(MSGID_WORLD_UPDATEROLE_REQUEST, &m_stUpdateRoleWorldHandler);

	//GM
    CHandlerFactory::RegisterHandler(MSGID_WORLD_GAMEMASTER_REQUEST, &m_stGameMasterHandler);
    CHandlerFactory::RegisterHandler(MSGID_WORLD_GAMEMASTER_RESPONSE, &m_stGameMasterHandler);

    // Account
    //CHandlerFactory::RegisterHandler(MSGID_ACCOUNT_LISTROLE_REQUEST, &m_stListRoleAccountHandler);
    //CHandlerFactory::RegisterHandler(MSGID_ACCOUNT_LISTROLE_RESPONSE, &m_stListRoleAccountHandler);

    //CHandlerFactory::RegisterHandler(MSGID_ACCOUNT_CREATEROLE_REQUEST, &m_stCreateRoleAccountHandler);
    //CHandlerFactory::RegisterHandler(MSGID_WORLD_CREATEROLE_RESPONSE, &m_stCreateRoleAccountHandler);

    CHandlerFactory::RegisterHandler(MSGID_ACCOUNT_DELETEROLE_REQUEST, &m_stDeleteRoleAccountHandler);
    CHandlerFactory::RegisterHandler(MSGID_ACCOUNT_DELETEROLE_RESPONSE, &m_stDeleteRoleAccountHandler);

    // NameDB
    CHandlerFactory::RegisterHandler(MSGID_ADDNEWNAME_REQUEST, &m_stWorldNameHandler);
    CHandlerFactory::RegisterHandler(MSGID_ADDNEWNAME_RESPONSE, &m_stWorldNameHandler);
    CHandlerFactory::RegisterHandler(MSGID_DELETENAME_REQUEST, &m_stWorldNameHandler);

    // LoginOut
    CHandlerFactory::RegisterHandler(MSGID_ZONE_LOGOUT_NOTIFY, &m_stLogoutHandler);
    CHandlerFactory::RegisterHandler(MSGID_WORLD_KICKROLE_REQUEST, &m_stKickRoleWorldHandler);
    CHandlerFactory::RegisterHandler(MSGID_WORLD_KICKROLE_RESPONSE, &m_stKickRoleWorldHandler);

    //CHandlerFactory::RegisterHandler(MSGID_KICKROLE_CLUSTER_REQUEST, &m_stKickRoleWorldHandler);
    //CHandlerFactory::RegisterHandler(MSGID_KICKROLE_CLUSTER_RESPONSE, &m_stKickRoleWorldHandler);

    // Chat
    CHandlerFactory::RegisterHandler(MSGID_WORLD_CHAT_NOTIFY, &m_stChatHandler);
	CHandlerFactory::RegisterHandler(MSGID_ZONE_HORSELAMP_NOTIFY, &m_stChatHandler);

    //拉取Zone列表
    CHandlerFactory::RegisterHandler(MSGID_ACCOUNT_LISTZONE_REQUEST, &m_stListZoneHandler);

	//排行榜
	CHandlerFactory::RegisterHandler(MSGID_WORLD_GETRANKINFO_REQUEST, &m_stRankInfoHandler);
	CHandlerFactory::RegisterHandler(MSGID_WORLD_UPDATERANK_REQUEST, &m_stRankInfoHandler);
	
	//玩家邮件
	CHandlerFactory::RegisterHandler(MSGID_WORLD_SENDMAIL_REQUEST, &m_stMailHandler);
	CHandlerFactory::RegisterHandler(MSGID_WORLD_SENDMAIL_RESPONSE, &m_stMailHandler);
	CHandlerFactory::RegisterHandler(MSGID_WORLD_GETSYSTEMMAIL_REQUEST, &m_stMailHandler);

	//限量兑换
	CHandlerFactory::RegisterHandler(MSGID_WORLD_ADDLIMITNUM_REQUEST, &m_stExchangeHandler);
	CHandlerFactory::RegisterHandler(MSGID_ZONE_GETLIMITNUM_REQUEST, &m_stExchangeHandler);
	CHandlerFactory::RegisterHandler(MSGID_WORLD_ADDEXCREC_REQUEST, &m_stExchangeHandler);
	CHandlerFactory::RegisterHandler(MSGID_ZONE_GETEXCHANGEREC_REQUEST, &m_stExchangeHandler);

	//在线兑换
	CHandlerFactory::RegisterHandler(MSGID_WORLD_ONLINEEXCHANGE_REQUEST, &m_stExchangeHandler);
	CHandlerFactory::RegisterHandler(MSGID_WORLD_ONLINEEXCHANGE_RESPONSE, &m_stExchangeHandler);

	//充值抽奖
	CHandlerFactory::RegisterHandler(MSGID_ZONE_LIMITLOTTERY_REQUEST, &m_stLimitLotteryHandler);
	CHandlerFactory::RegisterHandler(MSGID_ZONE_PAYLOTTERYRECORD_REQUEST, &m_stLimitLotteryHandler);

	//大转盘
	CHandlerFactory::RegisterHandler(MSGID_WORLD_UPDATEBETINFO_REQUEST, &m_stLasvegasHandler);
	CHandlerFactory::RegisterHandler(MSGID_WORLD_UPDATEPRIZEINFO_REQUEST, &m_stLasvegasHandler);

	//充值
	CHandlerFactory::RegisterHandler(MSGID_WORLD_USERRECHARGE_REQUEST, &m_stRechargeHandler);
	CHandlerFactory::RegisterHandler(MSGID_WORLD_USERRECHARGE_RESPONSE, &m_stRechargeHandler);
	CHandlerFactory::RegisterHandler(MSGID_ZONE_GETPAYORDER_REQUEST, &m_stRechargeHandler);
	CHandlerFactory::RegisterHandler(MSGID_WORLD_GETUSERINFO_REQUEST, &m_stRechargeHandler);
	CHandlerFactory::RegisterHandler(MSGID_WORLD_GETUSERINFO_RESPONSE, &m_stRechargeHandler);

    // Friend
    /*
    CHandlerFactory::RegisterHandler(MSGID_FRIEND_FETCHGAMEFRIEND_REQUEST, &m_stFriendMsgHandler);
    CHandlerFactory::RegisterHandler(MSGID_FRIEND_FETCHGAMEFRIEND_RESPONSE, &m_stFriendMsgHandler);
    CHandlerFactory::RegisterHandler(MSGID_FRIEND_ADD_REQUEST, &m_stFriendMsgHandler);
    CHandlerFactory::RegisterHandler(MSGID_FRIEND_ADD_RESPONSE, &m_stFriendMsgHandler);
    CHandlerFactory::RegisterHandler(MSGID_FRIEND_ADD_NOTIFY, &m_stFriendMsgHandler);
    CHandlerFactory::RegisterHandler(MSGID_FRIEND_ROLEINFO_REQUEST, &m_stFriendMsgHandler);
    CHandlerFactory::RegisterHandler(MSGID_FRIEND_ROLEINFO_RESPONSE, &m_stFriendMsgHandler);
    CHandlerFactory::RegisterHandler(MSGID_FRIEND_UPDATEGAMEFRIEND_REQUEST, &m_stFriendMsgHandler);
    CHandlerFactory::RegisterHandler(MSGID_FRIEND_LEVELCHANGE_NOTIFY, &m_stFriendMsgHandler);
    CHandlerFactory::RegisterHandler(MSGID_FRIEND_LOGINOUT_NOTIFY, &m_stFriendMsgHandler);
    CHandlerFactory::RegisterHandler(MSGID_SOCIAL_QUERY_REQUEST, &m_stFriendMsgHandler);
    CHandlerFactory::RegisterHandler(MSGID_SOCIAL_QUERY_RESPONSE, &m_stFriendMsgHandler);
    CHandlerFactory::RegisterHandler(MSGID_SOCIAL_CHECK_REQUEST, &m_stFriendMsgHandler);
    CHandlerFactory::RegisterHandler(MSGID_SOCIAL_CHECK_RESPONSE, &m_stFriendMsgHandler);
    CHandlerFactory::RegisterHandler(MSGID_FRIEND_SEARCH_REQUEST, &m_stFriendMsgHandler);
    CHandlerFactory::RegisterHandler(MSGID_FRIEND_CONTACT_REQUEST, &m_stFriendMsgHandler);
    CHandlerFactory::RegisterHandler(MSGID_CLOSERFRIENDUPDATE_NOTIFY, &m_stFriendMsgHandler);
    */

    // Online
    CHandlerFactory::RegisterHandler(MSGID_ZONE_ONLINEROLENUM_REQUEST, &m_OnlineStatHandler);
    //CHandlerFactory::RegisterHandler(MSGID_WORLD_ONLINESTAT_RESPONSE, &m_OnlineStatHandler);

    // Msg
    //CHandlerFactory::RegisterHandler(MSGID_PROMPTMSG_NOTIFY, &m_stBroadcastMsgHandler);
    //CHandlerFactory::RegisterHandler(MSGID_GAMEMASTER_REQUEST, &m_stBroadcastMsgHandler);
    //CHandlerFactory::RegisterHandler(MSGID_BOSSSTATUSCHANGED_NOTIFY, &m_stBroadcastMsgHandler);
    //CHandlerFactory::RegisterHandler(MSGID_PINSTANCEDRAWLUCKY_NOTIFY, &m_stBroadcastMsgHandler);

    return 0;
}
