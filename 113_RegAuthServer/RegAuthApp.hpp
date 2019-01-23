#pragma once

#include "CodeQueue.hpp"
#include "RegAuthPublic.hpp"
#include "CommDefine.h"

using namespace ServerLib;

// RegAuth应用程序
class CRegAuthApp
{
public:
    CRegAuthApp();
    virtual ~CRegAuthApp();

    virtual int Initialize(bool bResume);
    virtual void Run();

public:
    static void SetAppCmd(int iAppCmd);

private:
    virtual int ReloadConfig();
    virtual int LoadConfig();

	int LoadLogConfig(const char* pszConfigFile, const char* pszLogName);

private:
	bool m_bResumeMode; // 共享内存恢复模式，如果程序上次运行所创建的共享内存未被删除则直接attach上去
	static int ms_iAppCmd;

	static const unsigned int MAIN_LOOP_SLEEP = 5 * 1000; // 主循环中每次循环后会睡眠 5ms
	unsigned int m_uiLoopTimes; // 主循环次数
};

