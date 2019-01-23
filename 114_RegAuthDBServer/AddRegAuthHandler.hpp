#pragma once

#include "DBClientWrapper.hpp"
#include "MsgHandler.hpp"
#include "AppDef.hpp"

//新增帐号相关请求的处理
// MSGID_REGAUTHDB_ADDACCOUNT_REQUEST 消息处理者
class CAddRegAuthHandler : public CMsgHandler
{
public:
	CAddRegAuthHandler(DBClientWrapper* pDatabase, char** ppQueryBuff);

	void SetThreadIdx(const int iThreadIdx){m_iThreadIdx = iThreadIdx;}

	//第三个参数不需要传
	virtual void OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult = NULL, TNetHead_V2* pstNetHead = NULL);

private:

    //进行必要的参数检查
    int CheckParams(const RegAuthDB_AddAccount_Request& stReq);

    void OnAddRegAuthRequest(SHandleResult& stHandleResult);

    //检查帐号是否存在
    int CheckAccountExist(const AccountInfo& stInfo, bool& bIsExist);

    //获取还未被使用的UIN
    int GetAvaliableUin(unsigned int& uin);

    //插入新的记录
    int AddNewRecord(const AccountInfo& stInfo, unsigned int uin, int iWorldID);

private:
	DBClientWrapper* m_pDatabase;		//访问数据库的指针
	GameProtocolMsg* m_pstRequestMsg;	//待处理的消息
	char** m_ppQueryBuff;				//DB操作缓冲区					
	int m_iThreadIdx;					//所属线程idx
};
