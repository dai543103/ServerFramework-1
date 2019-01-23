#include <string.h>

#include "ModuleHelper.hpp"

#include "WorldMsgHelper.hpp"

int CWorldMsgHelper::GenerateMsgHead(GameProtocolMsg& stMsg,
                                     unsigned int uiSessionFD, unsigned int msgID, unsigned int uiUin)
{
    GameCSMsgHead* pMsgHead = stMsg.mutable_sthead();
    pMsgHead->set_uimsgid((ProtocolMsgID)msgID);
    pMsgHead->set_uin(uiUin);
    pMsgHead->set_uisessionfd(uiSessionFD);

	stMsg.mutable_stbody()->Clear();

    return 0;
}

int CWorldMsgHelper::SendWorldMsgToRoleDB(const GameProtocolMsg& stMsg)
{
	SERVERBUSID stToBusID;
	stToBusID.usWorldID = CModuleHelper::GetWorldID();
	stToBusID.usServerID = GAME_SERVER_ROLEDB;
    int iRet = CModuleHelper::GetWorldProtocolEngine()->SendWorldMsg(stMsg, stToBusID);
    if (iRet < 0)
    {
        LOGERROR("Send Failed: ret = %d\n", iRet);
    }

    return iRet;
}

int CWorldMsgHelper::SendWorldMsgToWGS(const GameProtocolMsg& stMsg, int iZoneID)
{
    if ((!CModuleHelper::GetZoneTick()->IsZoneActive(iZoneID)))
    {
        //LOGERROR("iZoneID:%d is dead, not send\n", iZoneID);
        return 0;
    }

	SERVERBUSID stToBusID;
	stToBusID.usWorldID = CModuleHelper::GetWorldID();
	stToBusID.usServerID = GAME_SERVER_ZONE;
	stToBusID.usZoneID = iZoneID;
    int iRet = CModuleHelper::GetWorldProtocolEngine()->SendWorldMsg(stMsg, stToBusID);
    if (iRet < 0)
    {
        LOGERROR("Send Failed: ret = %d\n", iRet);
    }

    return iRet;
}

int CWorldMsgHelper::SendWorldMsgToAllZone(const GameProtocolMsg& stMsg, int iExcludeZoneID)
{
	for (int iZoneID = 0; iZoneID < MAX_ZONE_PER_WORLD; ++iZoneID)
	{
		if (iZoneID == iExcludeZoneID)
		{
			//不发送
			continue;
		}
		
		SendWorldMsgToWGS(stMsg, iZoneID);
	}

	return 0;
}

int CWorldMsgHelper::SendWorldMsgToAccount(const GameProtocolMsg& stMsg)
{
	return 0;

	//单角色去掉Account服务器
	/*
    int iRet = CModuleHelper::GetWorldProtocolEngine()->SendWorldMsg(rstWorldMsg, GAME_SERVER_ACCOUNT);
    if (iRet < 0)
    {
        LOGERROR("Failed to send World msg, ret = %d\n", iRet);
    }

    return iRet;
	*/
}

int CWorldMsgHelper::SendWorldMsgToMailDB(const GameProtocolMsg& stMsg)
{
	return 0;

	/*
    int iRet = CModuleHelper::GetWorldProtocolEngine()->SendWorldMsg(rstWorldMsg, GAME_SERVER_MAILDB);
    if (iRet < 0)
    {
        LOGERROR("Send Failed: ret = %d\n", iRet);
    }

    return iRet;
	*/
}

int CWorldMsgHelper::SendWorldMsgToNameDB(const GameProtocolMsg& stMsg)
{
	SERVERBUSID stToBusID;
	stToBusID.usWorldID = CModuleHelper::GetWorldID();
	stToBusID.usServerID = GAME_SERVER_NAMEDB;
    int iRet = CModuleHelper::GetWorldProtocolEngine()->SendWorldMsg(stMsg, stToBusID);
    if (iRet < 0)
    {
        LOGERROR("Send Failed: ret = %d\n", iRet);
    }

    return iRet;
}

int CWorldMsgHelper::SendWorldMsgToCluster(const GameProtocolMsg& stMsg)
{
    //去掉Cluster
    /*
    int iRet = CModuleHelper::GetWorldProtocolEngine()->SendWorldMsg(rstWorldMsg, GAME_SERVER_CLUSTER);
    if (iRet < 0)
    {
        TRACESVR("Send Failed: ret = %d\n", iRet);
    }

    return iRet;
    */

    return 0;
}

int CWorldMsgHelper::SendWorldMsgToRecharge(const GameProtocolMsg& stMsg)
{
	SERVERBUSID stToBusID;
	stToBusID.usWorldID = CModuleHelper::GetWorldID();
	stToBusID.usServerID = GAME_SERVER_RECHARGE;
	int iRet = CModuleHelper::GetWorldProtocolEngine()->SendWorldMsg(stMsg, stToBusID);
	if (iRet < 0)
	{
		LOGERROR("Send Failed: ret = %d\n", iRet);
	}

	return iRet;
}

int CWorldMsgHelper::SendWorldMsgToExchange(const GameProtocolMsg& stMsg)
{
	SERVERBUSID stToBusID;
	stToBusID.usWorldID = CModuleHelper::GetWorldID();
	stToBusID.usServerID = GAME_SERVER_EXCHANGE;
	int iRet = CModuleHelper::GetWorldProtocolEngine()->SendWorldMsg(stMsg, stToBusID);
	if (iRet < 0)
	{
		LOGERROR("Send Failed: ret = %d\n", iRet);
	}

	return iRet;
}

int CWorldMsgHelper::SendWorldMsgToLogServer(const GameProtocolMsg& stMsg)
{
	SERVERBUSID stToBusID;
	stToBusID.usWorldID = CModuleHelper::GetWorldID();
	stToBusID.usServerID = GAME_SERVER_LOGSERVER;
	int iRet = CModuleHelper::GetWorldProtocolEngine()->SendWorldMsg(stMsg, stToBusID);
	if (iRet < 0)
	{
		LOGERROR("Send Failed: ret = %d\n", iRet);
	}

	return iRet;
}
