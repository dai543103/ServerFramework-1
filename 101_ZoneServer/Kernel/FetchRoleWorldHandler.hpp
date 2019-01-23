#pragma once

#include "Kernel/Handler.hpp"

class CFetchRoleWorldHandler : public IHandler
{
public:
    virtual ~CFetchRoleWorldHandler();

public:
    virtual int OnClientMsg();

private:
    int OnFetchRole();

    ////////////////////////////////////////////////////////
    // 登录角色
    int LoginRole();

    // 初始化角色数据
    int InitRoleData();
    
    // 登录后初始化
    int InitRoleAfterLogin(CGameRoleObj& stRoleObj, const World_FetchRole_Response& stResp);
};
