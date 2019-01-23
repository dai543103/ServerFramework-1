#pragma once

#include "AppDef.hpp"
#include "RoleDBHandlerSet.hpp"

// RoleDB应用程序
class CRoleDBApp
{
public:
    CRoleDBApp();
    virtual ~CRoleDBApp();

    int Initialize(bool bResume, int iWorldID);
    void Run();
    int HandleMsgIn(int& riRecvMsgCount);

    //process thread msg
    int HandleThreadMsg(int& riSendMsgCount);
	int GetWorldID() { return m_iWorldID; };

    static void SetAppCmd(int iAppCmd);

private:
    //receive msg and dispatch
    int ReceiveAndDispatchMsg();

    int LoadLogConfig(const char* pszConfigFile, const char* pszLogName);
    int ReloadConfig();
    int LoadConfig();

private:
	bool m_bResumeMode; // 共享内存恢复模式，如果程序上次运行所创建的共享内存未被删除则直接attach上去
	static int ms_iAppCmd;
	int m_iWorldID;

	char m_szCodeBuf[MAX_ROLEDB_MSGBUFFER_SIZE];
};
