#ifndef __FETCHROLE_WORLD_HANDLER_HPP__
#define __FETCHROLE_WORLD_HANDLER_HPP__

#include "Handler.hpp"
#include "AppDefW.hpp"
#include "WorldObjectHelperW_K64.hpp"
#include "WorldRoleStatus.hpp"

class CFetchRoleWorldHandler : public IHandler
{
public:
	virtual ~CFetchRoleWorldHandler();

public:
	virtual int OnClientMsg(GameProtocolMsg* pMsg);

public:
    // 返回角色查询数据
    int SendFetchRoleResponse(const RoleID& stRoleID, int iReqID, const KickerInfo& stKicker);

    // 返回失败查询
    int SendFailedFetchRoleResponse(const RoleID& stRoleID, int iReqID);

private:
    // 查询请求处理入口
	int OnRequestFetchRoleWorld();

    //查询请求返回到World的处理入口
    int OnResponseFetchRoleWorld();

	//创建角色返回的处理入口
	int OnResponseCreateRoleWorld();

	//发送创建角色的请求
	int SendCreateRoleRequestToDBSvr(const KickerInfo& stKicker);

	//初始化玩家信息
	int InitRoleBirthInfo(World_CreateRole_Request& rstRequest, const KickerInfo& stKicker);

	//创建成功
	int OnFetchSuccess(const RoleID& stRoleID, const GameUserInfo& stUserInfo, bool bIsLogin, const KickerInfo& stKicker);

private:
	GameProtocolMsg* m_pMsg;

	GameProtocolMsg m_stWorldMsg;
};

#endif


