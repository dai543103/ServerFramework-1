#pragma once

#include <string>

#include "MsgHandler.hpp"

//多线程处理Handler
class CGetCardNoHandler : public CMsgHandler
{
public:
	CGetCardNoHandler(DBClientWrapper* pDatabase, char** ppQueryBuff);

	//第三个参数传空
	virtual void OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult = NULL, TNetHead_V2* pstNetHead = NULL);

	//设置线程index
	void SetThreadIdx(int iThreadIndex);

private:

	//拉取卡密
	int OnRequestGetCardNo(SHandleResult& stHandleResult);

private:

	//访问数据库的指针
	DBClientWrapper* m_pDatabase;		

	//DB操作缓冲区
	char** m_ppQueryBuff;				

	// 待处理的消息
	GameProtocolMsg* m_pstRequestMsg;

	//线程idnex
	int m_iThreadIndex;
};
