#include <unistd.h>

#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "NowTime.hpp"
#include "TimeUtility.hpp"
#include "AppDefW.hpp"
#include "MsgStatistic.hpp"
#include "AppUtility.hpp"
#include "ShmObjectCreator.hpp"
#include "WorldMsgHelper.hpp"
#include "Random.hpp"
#include "ServerBusManager.h"
#include "WorldRankInfoManager.h"
#include "WorldExchangeLimitManager.h"
#include "WorldLimitLotteryManager.h"
#include "WorldLasvegasManager.h"
#include "WorldMailManager.hpp"

#include "AppLoopW.hpp"

using namespace ServerLib;

//初始化AppCmd
int CAppLoopW::ms_iAppCmd = APPCMD_NOTHING_TODO;

CAppLoopW::CAppLoopW()
{

}

CAppLoopW::~CAppLoopW()
{
}

int CAppLoopW::ReloadConfig()
{
    //GameServer.tcm
    CAppUtility::LoadLogConfig(APP_CONFIG_FILE, "WorldServer");

    //加载游戏配置
	int iRet = CConfigManager::Instance()->Initialize(true);
	if (iRet)
	{
		TRACESVR("Failed to reload server config, ret %d\n", iRet);
		return iRet;
	}

    return 0;
}

int CAppLoopW::LoadConfig()
{
    //GameServer.tcm
    CAppUtility::LoadLogConfig(APP_CONFIG_FILE, "WorldServer");

	//加载游戏配置
	int iRet = CConfigManager::Instance()->Initialize(false);
	if (iRet)
	{
		TRACESVR("Failed to load server config, ret %d\n", iRet);
		return iRet;
	}

    return 0;
}

int CAppLoopW::Initialize(bool bResumeMode, int iWorlID)
{
    // 读取配置
    int iRet = LoadConfig();
	if (iRet)
	{
		TRACESVR("Failed to load server config, ret %d\n", iRet);
		exit(1);
	}

    CModuleHelper::RegisterServerID(iWorlID);
    CModuleHelper::RegisterWorldProtocolEngine(&m_stWorldProtocolEngine);
	
	iRet = m_stAllocator.Initialize(bResumeMode);
    if(iRet < 0)
    {
        TRACESVR("Allocator initialize failed: iRet = %d\n", iRet);
        exit(2);
    }

    iRet = m_stWorldProtocolEngine.Initialize(bResumeMode);
    if (iRet < 0)
    {
        TRACESVR("Protocol Engine initialize failed: iRet = %d\n", iRet);
        exit(3);
    }
    m_stWorldProtocolEngine.RegisterAllHandler();

	//通信BUS管理器
    iRet = CServerBusManager::Instance()->Init("WorldServer", CConfigManager::Instance()->GetBusConfigMgr());
    if (iRet < 0)
    {
        TRACESVR("MsgTransceiver initialize failed: iRet = %d\n", iRet);
        exit(4);
    }

	//排行榜管理器
	CWorldRankInfoManager::Instance()->Init();

	//限量兑换管理器
	CWorldExchangeLimitManager::Instance()->Init();

	//限量抽奖管理器
	CWorldLimitLotteryManager::Instance()->Init();

	//大转盘管理器
	CWorldLasvegasManager::Instance()->Init();

	//邮件管理器
	CWorldMailManager::Instance()->Init();

    iRet = m_stAppTick.Initialize(bResumeMode);
    if (iRet < 0)
    {
        TRACESVR("Tick Initialize failed: %d.n\n", iRet);
        exit(6);
    }

    TRACESVR("Server Initialize Success.\n");

    return 0;
}

int CAppLoopW::Run()
{
    unsigned  int uiLoopCount = 0;

    TRACESVR("Run, run, run, never stop!\n");
    TRACESVR("WorldID: %d\n", CModuleHelper::GetWorldID());
    TRACESVR("==============================================================================\n");

    MsgStatisticSingleton::Instance()->Initialize();
    MsgStatisticSingleton::Instance()->Reset();
    static char szBuffer[MAX_MSGBUFFER_SIZE];
    int iBuffLength = sizeof(szBuffer);
    unsigned int uiNowTime = time(NULL);
    unsigned int uiStatTime = time(NULL);

    while(true)
    {
        NowTimeSingleton::Instance()->RefreshNowTime();
        NowTimeSingleton::Instance()->RefreshNowTimeVal();

        if(ms_iAppCmd == APPCMD_STOP_SERVICE)
        {
            TRACESVR("Receive Command: APPCMD_STOP_SERVICE\n");
            return 0;
        }

        if(ms_iAppCmd == APPCMD_RELOAD_CONFIG)
        {
            TRACESVR("Receive Command: APPCMD_RELOAD_CONFIG. \n");
            ReloadConfig();
            ms_iAppCmd = APPCMD_NOTHING_TODO;
        }

        if (ms_iAppCmd == APPCMD_QUIT_SERVICE)
        {
            TRACESVR("Receive Command: APPCMD_QUIT_SERVICE\n");
            break;
            ms_iAppCmd = APPCMD_NOTHING_TODO;
        }

        // 接收消息处理
        int iRecvMsgCount = 0;
        while(1)
        {
            iBuffLength = sizeof(szBuffer);
            int iMsgLength=0;
			SERVERBUSID stFromBusID;
            int iRet = CServerBusManager::Instance()->RecvOneMsg(szBuffer, iBuffLength, iMsgLength, stFromBusID);
            if (iRet < 0 || iMsgLength == 0)
            {
                break;
            }

            m_stWorldProtocolEngine.OnRecvCode(szBuffer, iMsgLength, stFromBusID);
            iRecvMsgCount++;
            if (iRecvMsgCount >= 1000)
            {
                break;
            }
        }

        m_stAppTick.OnTick();

        //统计
        uiNowTime = time(NULL);
        if(uiNowTime - uiStatTime >= 5 * 60) // 5分钟
        {
            MsgStatisticSingleton::Instance()->Print();
            MsgStatisticSingleton::Instance()->Reset();
            uiStatTime = uiNowTime;
        }

        uiLoopCount++;
        usleep(10);
    }

    return 0;
}

void CAppLoopW::SetAppCmd(int iAppCmd)
{
    ms_iAppCmd = iAppCmd;

}

