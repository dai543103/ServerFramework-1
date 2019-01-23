#pragma once

#include "AppDef.hpp"
#include "ErrorNumDef.hpp"
#include "NameDBHandlerSet.hpp"

// NameDB应用程序
class CNameDBApp
{
public:
    CNameDBApp();
    virtual ~CNameDBApp();

    int Initialize(bool bResume, int iWorldID);
    void Run();
    int HandleMsgIn(int& riRecvMsgCount);
    //process thread msg
    int HandleThreadMsg(int& riSendMsgCount);

public:
    static void SetAppCmd(int iAppCmd);

private:
    //receive msg and dispatch
    int ReceiveAndDispatchMsg();

private:
    bool m_bResumeMode; // 共享内存恢复模式，如果程序上次运行所创建的共享内存未被删除则直接attach上去
    static int ms_iAppCmd;

	static unsigned short m_usWorldID;

private:
    int LoadLogConfig(const char* pszConfigFile, const char* pszLogName);
    int ReloadConfig();
    int LoadConfig();

private:
    char m_szCodeBuf[MAX_NAMEDB_MSGBUFFER_SIZE];
};
