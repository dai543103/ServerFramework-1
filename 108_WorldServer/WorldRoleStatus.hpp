#ifndef __WORLD_ROLE_STATUS_HPP__
#define __WORLD_ROLE_STATUS_HPP__

#include "GameProtocol.hpp"
#include "ObjAllocator.hpp"
#include "AppDefW.hpp"

using namespace ServerLib;

//世界中保持的玩家状态信息
class CWorldRoleStatusWObj : public CObj
{
public:
	virtual ~CWorldRoleStatusWObj();
	virtual int Initialize();

public:
	void SetRoleInfo(const GameUserInfo& rstUserInfo);
	GameUserInfo& GetRoleInfo();

	void SetZoneID(unsigned short ushZoneID);
	unsigned short& GetZoneID();

	void SetUin(unsigned int uiUin);
	unsigned GetUin();

	void SetRoleStatus(unsigned uStatus)
	{
		m_uStatus = uStatus;
	};

	unsigned GetRoleStatus()
	{
		return m_uStatus;
	};

public:

    DECLARE_DYN

private:

	unsigned m_uiUin;

    unsigned short m_ushZoneID;

	unsigned int m_uStatus;

	GameUserInfo m_stUserInfo;
};

#endif

