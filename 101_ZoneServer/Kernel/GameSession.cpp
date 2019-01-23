
#include "GameProtocol.hpp"
#include "Kernel/GameRole.hpp"
#include "Kernel/ModuleHelper.hpp"
#include "SocketUtility.hpp"
#include "Kernel/ZoneObjectHelper.hpp"

#include "Kernel/GameSession.hpp"

IMPLEMENT_DYN(CGameSessionObj)

CGameSessionObj::CGameSessionObj()
{
    
}

CGameSessionObj::~CGameSessionObj()
{

}

int CGameSessionObj::Initialize()
{
    m_iCreateTime = time(NULL);
    m_iRoleIdx = -1;

	memset(m_szPicID, 0, sizeof(m_szPicID));

	return 0;
}

void CGameSessionObj::SetNetHead(const TNetHead_V2* pNetHead)
{
	m_stNetHead = *pNetHead;
	
	CSocketUtility::IPInt32ToString(ntohl(pNetHead->m_uiSrcIP), m_szClientIP);
}

CGameRoleObj* CGameSessionObj::GetBindingRole()
{
    return GameTypeK32<CGameRoleObj>::GetByIdx(m_iRoleIdx);
}

void CGameSessionObj::SetBindingRole(const CGameRoleObj* pZoneRoleObj)
{
    m_iRoleIdx = pZoneRoleObj->GetObjectID();
}

//Íæ¼ÒÍ·Ïñ
void CGameSessionObj::SetPictureID(const std::string& strPicID)
{
	SAFE_SPRINTF(m_szPicID, sizeof(m_szPicID)-1, "%s", strPicID.c_str());
}

const char* CGameSessionObj::GetPictureID()
{
	return m_szPicID;
}
