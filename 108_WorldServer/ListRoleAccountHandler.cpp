#include "ModuleHelper.hpp"
#include "LogAdapter.hpp"
#include "WorldMsgHelper.hpp"
#include "ErrorNumDef.hpp"

#include "ListRoleAccountHandler.hpp"

CListRoleAccountHandler::~CListRoleAccountHandler()
{

}

GameProtocolMsg CListRoleAccountHandler::m_stWorldMsg;
int CListRoleAccountHandler::OnClientMsg(GameProtocolMsg* pMsg)
{
    ASSERT_AND_LOG_RTN_INT(pMsg);

    m_pRequestMsg = (GameProtocolMsg*)pMsg;

    switch (m_pRequestMsg->sthead().uimsgid())
    {
    case MSGID_ACCOUNT_LISTROLE_REQUEST:
        OnAccountListRoleRequest();
        break;

    case MSGID_ACCOUNT_LISTROLE_RESPONSE:
        OnAccountListRoleResponse();
        break;

    default:
        break;
    }

    return 0;
}

int CListRoleAccountHandler::OnAccountListRoleRequest()
{
    LOGDEBUG("Forward ListRoleRequest to RoleDB server\n");

    int iRet = CWorldMsgHelper::SendWorldMsgToRoleDB(*m_pRequestMsg);
    return iRet;
}

int CListRoleAccountHandler::OnAccountListRoleResponse()
{
    LOGDEBUG("Forward ListRoleResponse to Account server\n");

    int iRet = CWorldMsgHelper::SendWorldMsgToAccount(*m_pRequestMsg);
    return iRet;
}

