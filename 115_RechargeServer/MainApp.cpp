#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <sys/file.h>

#include "SignalUtility.hpp"
#include "IOHandler.hpp"
#include "LogAdapter.hpp"
#include "ServerBusManager.h"
#include "Statistic.hpp"
#include "MsgStatistic.hpp"
#include "ServerStatistic.hpp"
#include "OrderManager.h"

#define RECHARGE_PID_FILE "../bin/App.pid"

using namespace ServerLib;

//检查文件锁，防止重复运行
void CheckLock(const char* pszLockFile)
{
    int iLockFD = open(pszLockFile, O_RDWR | O_CREAT, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    if(iLockFD < 0)
    {
        printf("Open LockFile %s Failed, Server CheckLock Failed!\n", pszLockFile);
        exit(1);
    }

    if(flock(iLockFD, LOCK_EX | LOCK_NB) < 0)
    {
        printf("Server is already Running!\n");
        exit(1);
    }
}


//初始化为守护进程的函数
void DaemonLaunch(void)
{
    pid_t pid;
    if((pid = fork()) != 0)
    {
        exit(0);
    }

    setsid();

	CSignalUtility::IgnoreSignalSet();

    if((pid = fork()) != 0)
    {
        exit(0);
    }

    umask(0);
}


//如进程正常退出，pid文件将被删除；否则为异常终止
//进程pid可被脚本用来关闭进程、重新读取配置文件
void WritePidFile(int id, const char* pszFilename)
{
    FILE* fp;

    fp = fopen(pszFilename, "w");
    if(NULL == fp)
    {
        printf("Failed to open pid file: %s\n", pszFilename);
    }

    fprintf(fp, "%d", id);

    fclose(fp);
}

// 读取PID文件
int ReadPidFile(const char* pszFilename)
{
    FILE* fp;

    fp = fopen(pszFilename, "r");
    if(NULL == fp)
    {
        printf("Failed to open pid file: %s\n", pszFilename);
        return -1;
    }

    char szPid[100];
    fread(szPid, sizeof(szPid), 1, fp);
    int iPid = atoi(szPid);

    fclose(fp);

    printf("ReadPid: %d\n", iPid);

    return iPid;
}

int main(int argc, char* *argv)
{
    bool bDaemonLaunch = true;
    bool bNagelOff = true; // 默认关闭nagel算法

    if(argc > 1)
    {
        if(!strcasecmp(argv[1], "-v"))
        {
            exit(0);
        }
        else if(!strcasecmp(argv[1], "-d"))
        {
            bDaemonLaunch = false;
        }
        else if (!strcasecmp(argv[1], "-n")) // 打开收发套接字为nagel算法，默认是关闭的
        {
            bNagelOff = false; // 打开nagel算法
        }

        // 兼容TApp的stop/reload参数
        for (int i = 0; i < argc; i++)
        {
            if (!strcasecmp(argv[i], "stop"))
            {
                int iPid = ReadPidFile(RECHARGE_PID_FILE);
                if (iPid > 0)
                {
                    kill(iPid, SIGUSR2);
                }

                exit(0);
            }
            else if (!strcasecmp(argv[i], "reload"))
            {
                int iPid = ReadPidFile(RECHARGE_PID_FILE);
                if (iPid > 0)
                {
                    kill(iPid, SIGUSR1);
                }

                exit(0);
            }
        }
    }

    CheckLock(".App.lock");

    if(bDaemonLaunch)
    {
        DaemonLaunch();
    }

    WritePidFile(getpid(), RECHARGE_PID_FILE);

    CSignalUtility::SetHandler_USR1(CIOHandler::SetAppCmd, APPCMD_RELOAD_CONFIG);
	CSignalUtility::SetHandler_USR2(CIOHandler::SetAppCmd, APPCMD_STOP_SERVICE);

	//加载配置
	CIOHandler::Instance()->ReloadConfig();

    int iRet = CIOHandler::Instance()->Initialize(bNagelOff);
    if(iRet < 0)
    {
        printf("IOHandler Initialize Failed(%d)!\n", iRet);
        return -3;
    }

    iRet = CIOHandler::Instance()->CreateEpoll();
    if(iRet < 0)
    {
        printf("IOHandler CreateEpoll Failed(%d)!\n", iRet);
        return -4;
    }

    iRet = CIOHandler::Instance()->CreateAllListener();
    if(iRet < 0)
    {
        printf("IOHandler CreateAllListener Failed(%d)!\n", iRet);
        return -5;
    }

	//订单信息初始化
	COrderManager::Instance()->Init();

    CServerStatistic::Instance()->ClearAllStat();
    MsgStatisticSingleton::Instance()->Initialize();
    MsgStatisticSingleton::Instance()->Reset();

    unsigned int uiNowTime = time(NULL);
    unsigned int uiStatTime = uiNowTime;

    while(true)
    {
		CIOHandler::Instance()->CheckIO();
		CIOHandler::Instance()->CheckBus();
		CIOHandler::Instance()->CheckAppCmd();
		CIOHandler::Instance()->CheckTimeOut();

		uiNowTime = time(NULL);

		//清理过期订单信息
		COrderManager::Instance()->OnTick(uiNowTime);

        //1小时统计一次
        if(uiNowTime - uiStatTime >= 60*60)
        {
            MsgStatisticSingleton::Instance()->Print();
            MsgStatisticSingleton::Instance()->Reset();
            LOGDEBUG("================Begin Statistic================\n");
            CServerStatistic::Instance()->RecordAllStat(uiNowTime - uiStatTime);
            CServerStatistic::Instance()->ClearAllStat();

            uiStatTime = uiNowTime;
            LOGDEBUG("================End Statistic================\n");
        }
    }

    unlink(RECHARGE_PID_FILE);

    return 0;
}

