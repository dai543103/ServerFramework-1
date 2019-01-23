#ifndef __APP_LOOP_HPP__
#define __APP_LOOP_HPP__

#include "GameProtocolEngine.hpp"
#include "HandlerList.hpp"
#include "ModuleHelper.hpp"
#include "ZoneObjectAllocator.hpp"
#include "TimeUtility.hpp"
#include "AppTick.hpp"

using namespace ServerLib;

class CAppLoop
{
public:
    // 信号处理
    static int ms_iAppCmd;
    static void SetAppCmd(int iAppCmd);

public:
    CAppLoop();

    int Initialize(bool bResume, int iWorldID, int iZoneID, int iInstanceID);
    int Run();

public:
    void OnTick();

    static CTimeValue m_stLotusMsgMaxProsessTime;
    static CTimeValue m_stWorldMsgMaxProcessTime;
    static CTimeValue m_stTickMaxProcessTime;

private:
    // 配置加载
    int LoadConfig();
    int ReloadConfig();

    // 停服
    bool Stop();

private:
    // 将数据类包含在CAppLoop中，在共享内存恢复
    CGameProtocolEngine m_stZoneProtocolEngine;
    CHandlerList m_stHandlerList;
    CSessionManager m_stSessionManager;
    CZoneObjectAllocator m_stAllocator;
    CAppTick m_stAppTick;

private:
    bool  m_bResumeMode;
    CTimeValue m_stStopService;
    timeval m_tvLastCheckTimeout;
    timeval m_tvLastCheckStat;
};

#endif


