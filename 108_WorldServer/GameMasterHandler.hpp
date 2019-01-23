#ifndef __GAMEMASTER_HANDLER_HPP__
#define __GAMEMASTER_HANDLER_HPP__

#include "Handler.hpp"
#include "AppDefW.hpp"
#include "WorldObjectHelperW_K64.hpp"
#include "WorldRoleStatus.hpp"

class CGameMasterHandler : public IHandler
{
public:
    virtual ~CGameMasterHandler();

public:
    virtual int OnClientMsg(GameProtocolMsg* pMsg);

private:

    int OnRequestGameMaster();
    int OnResponseGameMaster();

private:
    GameProtocolMsg* m_pRequestMsg;
};

#endif


