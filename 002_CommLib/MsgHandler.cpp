
#include "GameProtocol.hpp"

#include "MsgHandler.hpp"

void CMsgHandler::GenerateMsgHead(GameProtocolMsg& stMsg, unsigned int uiSessionID, unsigned int uiMsgID, unsigned int uiUin)
{
    GameCSMsgHead* pstMsgHead = stMsg.mutable_sthead();
    pstMsgHead->set_uimsgid((ProtocolMsgID)uiMsgID);
    pstMsgHead->set_uin(uiUin);
    pstMsgHead->set_uisessionfd(uiSessionID);

    stMsg.mutable_stbody()->Clear();
}
