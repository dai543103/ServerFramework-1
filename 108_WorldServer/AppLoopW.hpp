#ifndef __APP_LOOP_W_HPP__
#define __APP_LOOP_W_HPP__

#include "TimeUtility.hpp"

#include "WorldProtocolEngineW.hpp"
#include "SharedMemory.hpp"
#include "ModuleHelper.hpp"
#include "WorldObjectAllocatorW.hpp"
#include "AppTick.hpp"
#include "ConfigManager.hpp"
#include "AppDefW.hpp"

using namespace ServerLib;

class CAppLoopW
{
public:
    CAppLoopW();
    virtual ~CAppLoopW();

    virtual int Initialize(bool bResumeMode, int iWorlID);
    virtual int Run();
    static void SetAppCmd(int iAppCmd);

private:
    virtual int ReloadConfig();
    virtual int LoadConfig();

private:
    CWorldProtocolEngineW m_stWorldProtocolEngine;

    CWorldObjectAllocatorW m_stAllocator;

    CAppTick m_stAppTick;

private:
    static int ms_iAppCmd;
};

#endif


