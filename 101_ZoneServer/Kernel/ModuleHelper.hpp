#ifndef __MODULE_HELPER_HPP__
#define __MODULE_HELPER_HPP__

#include "SessionManager.hpp"
#include "GameProtocolEngine.hpp"
#include "Kernel/ConfigManager.hpp"
#include "GameEventManager.hpp"

using namespace ServerLib;

class CModuleHelper
{
public:
    static CSessionManager* GetSessionManager(){return m_pSessionManager;};
    static void RegisterSessionManager(CSessionManager* pSessionManager);

    static CGameProtocolEngine* GetZoneProtocolEngine(){return m_pZoneProtocolEngine;};
    static void RegisterZoneProtocolEngine(CGameProtocolEngine* pZoneProtocolEngine);

    // 服务器ID
    static void RegisterServerID(int iWorldID, int iZoneID, int iInstanceID);

    static int GetWorldID() { return m_iWorldID; };
    static int GetZoneID() { return m_iZoneID; };
    static int GetInstanceID() {return m_iInstanceID;}
    
private:
    static int m_iWorldID;
    static int m_iZoneID;
    static int m_iInstanceID;

private:

    // 各种模块指针
    static CSessionManager* m_pSessionManager;
    static CGameProtocolEngine* m_pZoneProtocolEngine;
};


#endif
