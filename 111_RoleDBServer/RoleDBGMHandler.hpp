#pragma once

#include "DBClientWrapper.hpp"
#include "MsgHandler.hpp"

// MSGID_WORLD_GAMEMASTER_REQUEST 消息处理者
class CRoleDBGMHandler : public CMsgHandler
{
public:
	CRoleDBGMHandler(DBClientWrapper* pDatabase, char** ppQueryBuff);
	
	void SetThreadIdx(const int iThreadIdx){m_iThreadIdx = iThreadIdx;}

	//第三个参数不需要传
	virtual void OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult = NULL, TNetHead_V2* pstNetHead = NULL);

private:

	//离线数据GM
	int GMOperaOffline(SHandleResult& stHandleResult);

	//修改玩家数据
	int GMModifyRoleData(const GameMaster_Request& stReq);

	//更新玩家数据
	int GMUpdateRoleData(unsigned uiUin, const GameLoginInfo& stUserInfo, int iDataType);

	//拉取玩家数据
	int GMGetRoleData(unsigned uiUin, GameLoginInfo& stUserInfo, int iDataType);

private:
	DBClientWrapper* m_pDatabase;		// 访问数据库的指针
	GameProtocolMsg* m_pstRequestMsg;	// 待处理的消息
	char** m_ppQueryBuff;				// 数据库操作的缓冲区	
	int m_iThreadIdx;					//所属线程idx
};
