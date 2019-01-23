#pragma once

#include "AppDef.hpp"
#include "ErrorNumDef.hpp"
#include "ThreadLogManager.hpp"

// RegAuthDB应用程序
class CRegAuthDBApp
{
public:
    CRegAuthDBApp();
    virtual ~CRegAuthDBApp();

    int Initialize(bool bResume, int iWorldID);
    void Run();
    int HandleMsgIn(int& riRecvMsgCount);
    int HandleThreadMsg(int& riSendMsgCount);

public:
    static void SetAppCmd(int iAppCmd);

private:
    int ReceiveAndDispatchMsg();
    int LoadLogConfig(const char* pszConfigFile, const char* pszLogName);
    int ReloadConfig();
    int LoadConfig();

private:
	bool m_bResumeMode; // 共享内存恢复模式，如果程序上次运行所创建的共享内存未被删除则直接attach上去
	static int ms_iAppCmd;

    char m_szCodeBuf[MAX_REGAUTHDB_MSGBUFFER_SIZE];
};
