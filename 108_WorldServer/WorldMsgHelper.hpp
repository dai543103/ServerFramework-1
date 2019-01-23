#pragma once

#include "GameProtocol.hpp"
#include "AppDefW.hpp"

class CWorldMsgHelper
{
public:
    static int GenerateMsgHead(GameProtocolMsg& stMsg,
                               unsigned int uiSessionFD, unsigned int msgID, unsigned int uiUin);
    static int SendWorldMsgToRoleDB(const GameProtocolMsg& stMsg);
    static int SendWorldMsgToWGS(const GameProtocolMsg& stMsg, int iZoneID);
	static int SendWorldMsgToAllZone(const GameProtocolMsg& stMsg, int iExcludeZoneID = 0);
    static int SendWorldMsgToAccount(const GameProtocolMsg& stMsg);
    static int SendWorldMsgToMailDB(const GameProtocolMsg& stMsg);
    static int SendWorldMsgToNameDB(const GameProtocolMsg& stMsg);
    static int SendWorldMsgToCluster(const GameProtocolMsg& stMsg);
	static int SendWorldMsgToRecharge(const GameProtocolMsg& stMsg);
	static int SendWorldMsgToExchange(const GameProtocolMsg& stMsg);
	static int SendWorldMsgToLogServer(const GameProtocolMsg& stMsg);
};
