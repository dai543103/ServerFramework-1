#pragma once

#include "DBClientWrapper.hpp"
#include "MsgHandler.hpp"

// MSGID_WORLD_RECHARGE_REQUEST 消息处理者
class CRoleDBRechargeHandler : public CMsgHandler
{
public:
	CRoleDBRechargeHandler(DBClientWrapper* pDatabase, char** ppQueryBuff);

	void SetThreadIdx(const int iThreadIdx){m_iThreadIdx = iThreadIdx;}

	//第三个参数不需要传
	virtual void OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult = NULL, TNetHead_V2* pstNetHead = NULL);

private:
    
	//离线充值
	int OfflineRecharge(SHandleResult& stHandleResult);
	
	//拉取充值玩家信息
	int OnGetUserInfoRequest(SHandleResult& stHandleResult);

	//拉取离线数据
	int GetOfflineInfo(unsigned uin, RESERVED1DBINFO& stOfflineInfo);

	//增加离线充值信息
	void AddOfflineRechargeInfo(int iRechargeID, int iTime, const std::string& strOrderID, RESERVED1DBINFO& stOfflineInfo);

	//更新离线数据
	int UpdateOfflineInfo(unsigned uin, RESERVED1DBINFO& stOfflineInfo);

	//拉取基础数据
	int GetBaseInfo(unsigned uin, BASEDBINFO& stBaseInfo);

private:
	DBClientWrapper* m_pDatabase;		// 访问数据库的指针
	GameProtocolMsg* m_pstRequestMsg;	// 待处理的消息
	char** m_ppQueryBuff;				// 数据库操作的缓冲区			
	int m_iThreadIdx;					//所属线程idx
};
