#pragma once

#include "WorldProtocolEngineW.hpp"
#include "AppLoopW.hpp"
#include "ConfigManager.hpp"
#include "ZoneTick.hpp"

using namespace ServerLib;

class CModuleHelper
{
public:
	static CWorldProtocolEngineW* GetWorldProtocolEngine();
	static void RegisterWorldProtocolEngine(CWorldProtocolEngineW* pWorldProtocolEngine);

    // ·þÎñÆ÷ID
    static void RegisterServerID(int iWorldID) {m_iWorldID = iWorldID;};
    static int GetWorldID() {return m_iWorldID;};
    
    // ZoneTick
    static void RegisterZoneTick(CZoneTick* pZoneTick) {m_pZoneTick = pZoneTick;};
    static CZoneTick* GetZoneTick() {return m_pZoneTick;};

	static bool IsRealGM(unsigned int uiUin);

private:
	static CWorldProtocolEngineW* m_pWorldProtocolEngine;
	static CAppLoopW* m_pAppLoop;
    static int m_iWorldID;
    static CZoneTick* m_pZoneTick;
};
