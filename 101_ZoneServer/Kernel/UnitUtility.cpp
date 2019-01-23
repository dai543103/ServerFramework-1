
#include "GameProtocol.hpp"
#include "Kernel/ZoneObjectHelper.hpp"
#include "Kernel/ModuleHelper.hpp"
#include "GameUnitID.hpp"
#include "Kernel/ModuleHelper.hpp"
#include "Kernel/GameRole.hpp"
#include "TimeUtility.hpp"

#include "Kernel/UnitUtility.hpp"

int CUnitUtility::m_iUpdateRoleIdx = -1;

TUNITINFO* CUnitUtility::GetUnitInfo(const int iUnitID)
{
    CGameUnitID* pGameUnitID = GameType<CGameUnitID>::Get(iUnitID);
    if (!pGameUnitID)
    {
        return NULL;
    }
	
	if (!GetRoleObj(iUnitID))
	{
		return NULL;
	}

    return pGameUnitID->GetBindUnitInfo();
}

CGameRoleObj* CUnitUtility::GetRoleObj(const int iUnitID)
{
    CGameUnitID* pGameUnitID = GameType<CGameUnitID>::Get(iUnitID);
    if (!pGameUnitID)
    {
        return NULL;
    }

    int iObjectIdx = pGameUnitID->GetBindObjectIdx();

    CGameRoleObj* pRoleObj = GameTypeK32<CGameRoleObj>::GetByIdx(iObjectIdx);

    return pRoleObj;
}

// 获取下一个角色索引
int CUnitUtility::IterateRoleIdx()
{
    if (m_iUpdateRoleIdx < 0)
    {
        // 定位起始单位
        m_iUpdateRoleIdx = GameTypeK32<CGameRoleObj>::GetUsedHead();
    }
    else
    {
        // 定位下一个单位
        m_iUpdateRoleIdx = GameTypeK32<CGameRoleObj>::GetNextIdx(m_iUpdateRoleIdx);
    }

    return m_iUpdateRoleIdx;
}

//推送时间同步消息
void CUnitUtility::SendSyncTimeNotify(CGameRoleObj* pstRoleObj, long8 lNowTime)
{
	static GameProtocolMsg stMsg;
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_SYNCTIME_NOTIFY);

	Zone_SyncTime_Notify* pstNotify = stMsg.mutable_stbody()->mutable_m_stzone_synctime_notify();
	pstNotify->set_lservertime(lNowTime);

	CZoneMsgHelper::SendZoneMsgToRole(stMsg, pstRoleObj);

	return;
}

int CUnitUtility::AllocateUnitID()
{
    int iUnitID = GameType<CGameUnitID>::Create();

    // 不使用0号UnitID
    if (iUnitID == 0)
    {
        iUnitID = AllocateUnitID();
    }

    if (iUnitID < 0)
    {
        TRACESVR("Too many Units in Zone!!\n");
    }

    TRACESVR("Allocate UnitID = %d\n", iUnitID);

    return iUnitID;
}

void CUnitUtility::FreeUnitID(const int iUnitID)
{
    CGameUnitID* pGameUnitID = GameType<CGameUnitID>::Get(iUnitID);
    if (!pGameUnitID)
    {
        return;
    }
	
	if (iUnitID == m_iUpdateRoleIdx)
	{
		IterateRoleIdx();
	}
           

    GameType<CGameUnitID>::Del(iUnitID);

    TRACESVR("Free UnitID = %d\n", iUnitID);
}

// 绑定Unit到Object
int CUnitUtility::BindUnitToObject(int iUnitID, int iObjectIdx)
{
    CGameUnitID* pGameUnitID = GameType<CGameUnitID>::Get(iUnitID);
    ASSERT_AND_LOG_RTN_INT(pGameUnitID);

    pGameUnitID->BindObject(iObjectIdx);

    return 0;
}

// 创建一个单位
CObj* CUnitUtility::CreateUnit(unsigned int uiKey)
{
    // 创建单位ID
    int iUnitID = CUnitUtility::AllocateUnitID();
    if (iUnitID <= 0)
    {
        TRACESVR("Cannot create role unit, Alloc unitid fail!\n");
        return NULL;
    }

    // 按类型创建单位
    CObj* pObj = (CObj*)GameTypeK32<CGameRoleObj>::CreateByKey(uiKey);
	CGameRoleObj* pRoleObj = (CGameRoleObj*)pObj;
	if (pRoleObj)
	{
		pRoleObj->GetRoleInfo().stUnitInfo.iUnitID = iUnitID;
		pRoleObj->SetUin(uiKey);
	}

    if (pObj)
    {
        CUnitUtility::BindUnitToObject(iUnitID, pObj->GetObjectID());
    }
    else
    {
        CUnitUtility::FreeUnitID(iUnitID);
        TRACESVR("Cannot create role unit, Alloc obj fail!\n");
    }

    return pObj;
}

// 删除一个单位, 释放UnitID, 销毁对象ID.
// 对象删除是一个非常危险的行为, 因此要先将单位置为EUS_DELETED状态, 在AppTick中删除
int CUnitUtility::DeleteUnit(TUNITINFO* pUnitInfo)
{
    int iUnitID = pUnitInfo->iUnitID;
	
	CGameRoleObj* pRoleObj = CUnitUtility::GetRoleObj(iUnitID);
	ASSERT_AND_LOG_RTN_INT(pRoleObj);
	
	unsigned uin = pRoleObj->GetRoleID().uin();

	//重置GameRole
	pRoleObj->ResetRole();

	//删除GameRole
	GameTypeK32<CGameRoleObj>::DeleteByKey(uin);

    // 删除UnitID
    CUnitUtility::FreeUnitID(iUnitID);

    return 0;
}

// 获取角色单位
CGameRoleObj* CUnitUtility::GetRoleByUin(const int uiUin)
{
    return GameTypeK32<CGameRoleObj>::GetByKey(uiUin);
}

CGameRoleObj* CUnitUtility::GetRoleByID(const RoleID& rstRoleID)
{
    CGameRoleObj* pRoleObj = GetRoleByUin(rstRoleID.uin());
    if (!pRoleObj || pRoleObj->GetRoleID().uiseq() != rstRoleID.uiseq())
    {
        return NULL;
    }

    return pRoleObj;
}

// 设置单位状态
void CUnitUtility::SetUnitStatus(TUNITINFO* pstUnit, EUnitStatus enStatus)
{
    ASSERT_AND_LOG_RTN_VOID(pstUnit);

    pstUnit->uiUnitStatus |= enStatus;
}

// 清除单位状态
void CUnitUtility::ClearUnitStatus(TUNITINFO* pstUnit, EUnitStatus enStatus)
{
    ASSERT_AND_LOG_RTN_VOID(pstUnit);

    pstUnit->uiUnitStatus &= ~(enStatus);
}

