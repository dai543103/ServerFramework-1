#pragma once

#include "DBClientWrapper.hpp"
#include "MsgHandler.hpp"
#include "AppDef.hpp"
#include "ErrorNumDef.hpp"

//增加玩家名字的请求
// MSGID_ADDNEWNAME_REQUEST 消息处理者
class CAddNameHandler : public CMsgHandler
{
public:
	CAddNameHandler(DBClientWrapper* pDatabase, char** ppQueryBuff);

	void SetThreadIdx(const int iThreadIdx){m_iThreadIdx = iThreadIdx;}

	//第三个参数不需要传
	virtual void OnClientMsg(GameProtocolMsg& stReqMsg, SHandleResult* pstHandleResult = NULL, TNetHead_V2* pstNetHead = NULL);

private:

    //进行必要的参数检查
    int CheckParams(const AddNewName_Request& stReq);

    void OnAddNameRequest(SHandleResult& stHandleResult);

    int CheckNameExist(const std::string& strName, int iType, bool& bIsExist);

    //插入新的记录
    int AddNewRecord(const std::string& strName, int iNameType, unsigned uNameID);

private:
	DBClientWrapper* m_pDatabase;		//访问数据库的指针
	GameProtocolMsg* m_pstRequestMsg;	//待处理的消息
	char** m_ppQueryBuff;				//DB操作缓冲区					
	int m_iThreadIdx;					//所属线程idx
};
