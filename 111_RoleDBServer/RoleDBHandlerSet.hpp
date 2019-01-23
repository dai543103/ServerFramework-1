#pragma once

#include "MsgHandlerSet.hpp"
#include "RoleDBFetchRoleHandler.hpp"
#include "RoleDBUpdateRoleHandler.hpp"
#include "RoleDBCreateRoleHandler.hpp"
#include "RoleDBListRoleHandler.hpp"
#include "RoleDBRechargeHandler.hpp"
#include "RoleDBMailHandler.hpp"
#include "RoleDBGMHandler.hpp"

// RoleDB应用中的消息处理者管理器
class CRoleDBHandlerSet : public CMsgHandlerSet
{
public:
    CRoleDBHandlerSet();
    ~CRoleDBHandlerSet();

    // 初始化该集合中的消息处理者和数据库配置并连接数据库
	virtual int Initialize(int iThreadIndex = -1);

private:
	int RegisterAllHandlers();

private:
	// 该集合管理的所有消息处理者
	CRoleDBFetchRoleHandler m_FetchRoleHandler;
	CRoleDBUpdateRoleHandler m_UpdateRoleHandler;
	CRoleDBCreateRoleHandler m_CreateRoleHandler;
	CRoleDBListRoleHandler m_ListRoleHandler;
	CRoleDBRechargeHandler m_RechargeHandler;
	CRoleDBGMHandler m_GameMasterHandler;
	CRoleDBMailHandler m_MailHandler;

private:
	// 消息处理者处理消息时需要访问数据库
	DBClientWrapper m_oDBClient;

	//数据库操作缓冲区
	char* m_pQueryBuff;

	int m_iThreadIdx;
};
