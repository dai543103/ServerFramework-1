#ifndef __CREATEROLE_ACCOUNT_HANDLER_HPP__
#define __CREATEROLE_ACCOUNT_HANDLER_HPP__

#include <string>

#include "Handler.hpp"
#include "AppDefW.hpp"
#include "GameProtocol.hpp"
#include "ConfigManager.hpp"

class CCreateRoleAccountHandler : public IHandler
{
public:
    virtual ~CCreateRoleAccountHandler();

public:
    virtual int OnClientMsg(GameProtocolMsg* pMsg);

private:
    int OnRequestCreateRoleAccount();
    int OnResponseCreateRoleAccount();
    int SendCreateRoleRequestToDBSvr(const std::string& szNickName);
    int SendCreateRoleResponseToAccount();

    int InitBirthInfo(World_CreateRole_Request& rstRequest, const std::string& strNickName);

private:

private:
    GameProtocolMsg* m_pMsg;
    GameProtocolMsg m_stWorldMsg;
};

#endif


