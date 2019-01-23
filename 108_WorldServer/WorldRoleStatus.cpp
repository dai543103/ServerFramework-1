#include <string.h>
#include <assert.h>

#include "WorldObjectHelperW_K64.hpp"
#include "ModuleHelper.hpp"

#include "WorldRoleStatus.hpp"

IMPLEMENT_DYN(CWorldRoleStatusWObj)

CWorldRoleStatusWObj::~CWorldRoleStatusWObj()
{
}

int CWorldRoleStatusWObj::Initialize()
{
	m_stUserInfo.Clear();

	m_uiUin = 0;

	m_ushZoneID = 0;

    m_uStatus = 0;

	return 0;
}                                

void CWorldRoleStatusWObj::SetRoleInfo(const GameUserInfo& rstRoleInfo)
{
	//设置玩家的角色数据信息
	m_stUserInfo.CopyFrom(rstRoleInfo);
}

GameUserInfo& CWorldRoleStatusWObj::GetRoleInfo()
{
	return m_stUserInfo;
}

void CWorldRoleStatusWObj::SetZoneID(unsigned short ushZoneID)
{
	m_ushZoneID=ushZoneID;    
}

unsigned short& CWorldRoleStatusWObj::GetZoneID()
{
	return m_ushZoneID;
}

void CWorldRoleStatusWObj::SetUin(unsigned uiUin)
{
	m_uiUin = uiUin;
}

unsigned CWorldRoleStatusWObj::GetUin()
{
    return m_uiUin;
}
