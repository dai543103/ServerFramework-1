#include "GameProtocol.hpp"
#include "StringUtility.hpp"
#include "SectionConfig.hpp"
#include "StringSplitter.hpp"
#include "AppDef.hpp"

#include "Kernel/ModuleHelper.hpp"

CSessionManager* CModuleHelper::m_pSessionManager = 0;
CGameProtocolEngine* CModuleHelper::m_pZoneProtocolEngine = 0;

int CModuleHelper::m_iZoneID = 0;
int CModuleHelper::m_iWorldID = 0;
int CModuleHelper::m_iInstanceID = 0;

void CModuleHelper::RegisterSessionManager(CSessionManager* pSessionManager)
{
    m_pSessionManager = pSessionManager;
}

void CModuleHelper::RegisterZoneProtocolEngine(CGameProtocolEngine* pZoneProtocolEngine)
{
    m_pZoneProtocolEngine = pZoneProtocolEngine;
}

void CModuleHelper::RegisterServerID(int iWorldID, int iZoneID, int iInstanceID)
{
    m_iWorldID = iWorldID;
    m_iZoneID = iZoneID;
    m_iInstanceID = iInstanceID;
}
