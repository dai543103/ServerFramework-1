#pragma once

#include "DBClientWrapper.hpp"
#include "MsgHandler.hpp"
#include "AppDef.hpp"
#include "ErrorNumDef.hpp"

// MSGID_DELETENAME_REQUEST 消息处理者
class CDeleteNameHandler : public CMsgHandler
{
public:
	CDeleteNameHandler(DBClientWrapper* pDatabase, char** ppQueryBuff);
	
	void SetThreadIdx(const int iThreadIdx){m_iThreadIdx = iThreadIdx;}

	//第三个参数不需要传
	virtual void OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult = NULL, TNetHead_V2* pstNetHead = NULL);

private:

    //删除Name信息
    int DeleteName(const std::string& strName, int iType);

private:
	DBClientWrapper* m_pDatabase;		//访问数据库的指针
	GameProtocolMsg* m_pstRequestMsg;	//待处理的消息
	char** m_ppQueryBuff;				//DB操作缓冲区					
	int m_iThreadIdx;					//所属线程idx
};
