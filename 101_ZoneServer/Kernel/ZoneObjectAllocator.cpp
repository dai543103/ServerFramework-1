#include "GameProtocol.hpp"
#include "ObjAllocator.hpp"
#include "Kernel/ConfigManager.hpp"
#include "Kernel/ZoneObjectHelper.hpp"
#include "Kernel/GameRole.hpp"
#include "GameUnitID.hpp"
#include "Kernel/GameSession.hpp"

#include "Kernel/ModuleHelper.hpp"
#include "FishGame/FishpondObj.h"

#include "ZoneObjectAllocator.hpp"

template <typename TYPE_Obj> 
CObjAllocator* AllocateShmObj(CSharedMemory& rstSharedMemory, const int iObjCount, bool bResume)
{
    CObjAllocator* pAllocator = NULL;

    if (!bResume)
    {
	    pAllocator = CObjAllocator::CreateByGivenMemory(
		    (char*)rstSharedMemory.GetFreeMemoryAddress(),
		    rstSharedMemory.GetFreeMemorySize(),
		    sizeof(TYPE_Obj),
		    iObjCount,
		    TYPE_Obj::CreateObject);

        LOGDEBUG("MEMLOG Free:%zu,size:%zu,num:%d,all:%zu\n",
            rstSharedMemory.GetFreeMemorySize(),
            sizeof(TYPE_Obj),
            iObjCount,
            iObjCount * sizeof(TYPE_Obj));
    }
    else
    {
        pAllocator = CObjAllocator::ResumeByGivenMemory(
            (char*)rstSharedMemory.GetFreeMemoryAddress(),
            rstSharedMemory.GetFreeMemorySize(),
            sizeof(TYPE_Obj),
            iObjCount,
            TYPE_Obj::CreateObject);
    }

    if (!pAllocator)
    {
        LOGERROR("Failed to allocate object segment\n");
        return NULL;
    }

    if (bResume)
    {
        // 恢复使用中的obj
        int iUsedIdx = pAllocator->GetUsedHead();
        while (iUsedIdx != -1)
        {
            CObj* pObj = pAllocator->GetObj(iUsedIdx);
            pObj->Resume();
            iUsedIdx = pAllocator->GetIdx(iUsedIdx)->GetNextIdx();
        }
    }

    rstSharedMemory.UseShmBlock(CObjAllocator::CountSize(sizeof(TYPE_Obj), iObjCount));

	int iRet = GameType<TYPE_Obj>::RegisterAllocator(pAllocator);
	if(iRet < 0)
	{
		return NULL;
	}

	return pAllocator;
}

template <typename TYPE_Obj> 
CObjAllocator* AllocateShmObjK32(CSharedMemory& rstSharedMemory, const int iObjCount, bool bResume)
{
	CObjAllocator* pAllocator = NULL;
    
    if (!bResume)
    {
        pAllocator = CObjAllocator::CreateByGivenMemory(
		    (char*)rstSharedMemory.GetFreeMemoryAddress(),
		    rstSharedMemory.GetFreeMemorySize(),
		    sizeof(TYPE_Obj),
		    iObjCount,
		    TYPE_Obj::CreateObject);

        LOGDEBUG("MEMLOG Free:%zu,size:%zu,num:%d,all:%zu\n",
            rstSharedMemory.GetFreeMemorySize(),
            sizeof(TYPE_Obj),
            iObjCount,
            iObjCount * sizeof(TYPE_Obj));
    }
    else
    {
        pAllocator = CObjAllocator::ResumeByGivenMemory(
            (char*)rstSharedMemory.GetFreeMemoryAddress(),
            rstSharedMemory.GetFreeMemorySize(),
            sizeof(TYPE_Obj),
            iObjCount,
            TYPE_Obj::CreateObject);
    }

    if (!pAllocator)
    {
        LOGERROR("Failed to allocate object segment\n");
        return NULL;
    }

    if (bResume)
    {
        // 恢复使用中的obj
        int iUsedIdx = pAllocator->GetUsedHead();
        while (iUsedIdx != -1)
        {
            CObj* pObj = pAllocator->GetObj(iUsedIdx);
            pObj->Resume();
            iUsedIdx = pAllocator->GetIdx(iUsedIdx)->GetNextIdx();
        }
    }

    rstSharedMemory.UseShmBlock(CObjAllocator::CountSize(sizeof(TYPE_Obj), iObjCount));

    CHashMap_K32* pHashMap = NULL; //节点数量为对象数量两倍
    if (!bResume)
    {
        pHashMap = CHashMap_K32::CreateHashMap(
            (char*)rstSharedMemory.GetFreeMemoryAddress(),
            rstSharedMemory.GetFreeMemorySize(),
            iObjCount << 1);
    }
    else
    {
        pHashMap = CHashMap_K32::ResumeHashMap(
            (char*)rstSharedMemory.GetFreeMemoryAddress(),
            rstSharedMemory.GetFreeMemorySize(),
            iObjCount << 1);
    }

    if (!pHashMap)
    {
        LOGERROR("Failed to allocate hashmap\n");
        return NULL;
    }

    rstSharedMemory.UseShmBlock(CHashMap_K32::CountSize(iObjCount << 1));

	int iRet = GameTypeK32<TYPE_Obj>::RegisterHashAllocator(pAllocator, pHashMap);
	if(iRet < 0)
	{
		return NULL;
	}

	return pAllocator;
}

template <typename TYPE_Obj>
CObjAllocator* AllocateShmObjK64(CSharedMemory& rstSharedMemory, const int iObjCount, bool bResume)
{
    CObjAllocator* pAllocator = NULL;
    
    if (!bResume)
    {
        pAllocator = CObjAllocator::CreateByGivenMemory(
            (char*)rstSharedMemory.GetFreeMemoryAddress(),
            rstSharedMemory.GetFreeMemorySize(),
            sizeof(TYPE_Obj),
            iObjCount,
            TYPE_Obj::CreateObject);
    }
    else
    {
        pAllocator = CObjAllocator::ResumeByGivenMemory(
            (char*)rstSharedMemory.GetFreeMemoryAddress(),
            rstSharedMemory.GetFreeMemorySize(),
            sizeof(TYPE_Obj),
            iObjCount,
            TYPE_Obj::CreateObject);
    }

    if (!pAllocator)
    {
        LOGERROR("Failed to allocate object segment\n");
        return NULL;
    }

    if (bResume)
    {
        // 恢复使用中的obj
        int iUsedIdx = pAllocator->GetUsedHead();
        while (iUsedIdx != -1)
        {
            CObj* pObj = pAllocator->GetObj(iUsedIdx);
            pObj->Resume();
            iUsedIdx = pAllocator->GetIdx(iUsedIdx)->GetNextIdx();
        }
    }

    rstSharedMemory.UseShmBlock(CObjAllocator::CountSize(sizeof(TYPE_Obj), iObjCount));

    CHashMap_K64* pHashMap = NULL; //节点数量为对象数量两倍

    if (!bResume)
    {
        pHashMap = CHashMap_K64::CreateHashMap(
            (char*) rstSharedMemory.GetFreeMemoryAddress(),
            rstSharedMemory.GetFreeMemorySize(),
            iObjCount << 1);
    }
    else
    {
        pHashMap = CHashMap_K64::ResumeHashMap(
            (char*) rstSharedMemory.GetFreeMemoryAddress(),
            rstSharedMemory.GetFreeMemorySize(),
            iObjCount << 1);
    }

    if (!pHashMap)
    {
        LOGERROR("Failed to allocate hashmap\n");
        return NULL;
    }

    rstSharedMemory.UseShmBlock(CHashMap_K64::CountSize(iObjCount << 1));

    int iRet = GameTypeK64<TYPE_Obj>::RegisterHashAllocator(pAllocator, pHashMap);
    if(iRet < 0)
    {
        return NULL;
    }

    return pAllocator;
}


CZoneObjectAllocator::CZoneObjectAllocator()
{
    m_iObjTotalSize = 0;
}

int CZoneObjectAllocator::Initialize(bool bResumeMode)
{
	int iRet = -1;

	CObjAllocator* pAllocator = NULL;

    size_t iSharedMmorySize = GetObjTotalSize();

	iRet = m_stSharedMemory.CreateShmSegmentByKey(CSharedMemory::GenerateShmKey(GAME_SERVER_ZONE, 
				CModuleHelper::GetWorldID()*100 + (CModuleHelper::GetZoneID() + 1)*10 + CModuleHelper::GetInstanceID()), 
			iSharedMmorySize);
	if(iRet < 0)
	{
		LOGERROR("Create shm fail: %d\n", iRet);
		return -1;
	}

    pAllocator = AllocateShmObjK32<CGameSessionObj>(m_stSharedMemory, MAX_GAMESESSION_NUMBER, bResumeMode);
    if (!pAllocator)
    {
		LOGERROR("Alloc sessionobj fail!\n");
        return -2;
    }
	
	pAllocator = AllocateShmObjK32<CGameRoleObj>(m_stSharedMemory, MAX_ROLE_OBJECT_NUMBER_IN_ZONE, bResumeMode);
	if(!pAllocator)
	{
		LOGERROR("Alloc role fail!\n");
		return -5;
	}

    pAllocator = AllocateShmObj<CGameUnitID>(m_stSharedMemory, MAX_UNIT_NUMBER_IN_ZONE, bResumeMode);
    if(!pAllocator)
    {
        LOGERROR("Alloc GameUnit fail!\n");
        return -6;
    }

	pAllocator = AllocateShmObjK32<CFishpondObj>(m_stSharedMemory, MAX_FISHPOND_NUMBER_IN_ZONE, bResumeMode);
	if (!pAllocator)
	{
		LOGERROR("Alloc CFishpondObj fail!\n");
		return -7;
	}

    TRACESVR("Obj shared memory: Total allocated %lu, Used %lu, Free %lu\n", iSharedMmorySize, GetObjTotalSize(), m_stSharedMemory.GetFreeMemorySize());

	return 0;
}

size_t CZoneObjectAllocator::GetObjTotalSize()
{
    if (m_iObjTotalSize > 0)
    {
        return m_iObjTotalSize;
    }

    TRACESVR("********************** OBJECT MEMORY STATICS *********************\n");

    // Session
    size_t iSessionSize = CObjAllocator::CountSize(sizeof(CGameSessionObj), MAX_GAMESESSION_NUMBER);
    m_iObjTotalSize += iSessionSize;
    TRACESVR("Session: Number = %d, UnitSize = %lu, Total = %lu\n", 
        MAX_GAMESESSION_NUMBER, sizeof(CGameSessionObj), iSessionSize);

    // Session Hash
    size_t iSessionHashSize = CHashMap_K32::CountSize(MAX_GAMESESSION_NUMBER << 1);
    m_iObjTotalSize += iSessionHashSize;
    TRACESVR("SessionHash: Number = %d, Size = %lu\n", 
        MAX_GAMESESSION_NUMBER << 1, iSessionHashSize);

    // 角色对象
    size_t iRoleSize = CObjAllocator::CountSize(sizeof(CGameRoleObj), MAX_ROLE_OBJECT_NUMBER_IN_ZONE); 
    m_iObjTotalSize += iRoleSize;
    TRACESVR("Role: Number = %d, UnitSize = %lu, Total = %lu\n", 
        MAX_ROLE_OBJECT_NUMBER_IN_ZONE, sizeof(CGameRoleObj), iRoleSize);

    // 角色Hash
    size_t iRoleHashSize = CHashMap_K32::CountSize(MAX_ROLE_OBJECT_NUMBER_IN_ZONE << 1);
    m_iObjTotalSize += iRoleHashSize;
    TRACESVR("RoleHash: Number = %d,  Size = %lu\n", 
        MAX_ROLE_OBJECT_NUMBER_IN_ZONE << 1, iRoleHashSize);

    //场景单位的UnitID
    size_t iSceneUnitSize = CObjAllocator::CountSize(sizeof(CGameUnitID), MAX_UNIT_NUMBER_IN_ZONE); 
    m_iObjTotalSize += iSceneUnitSize;
    TRACESVR("Unit: Number = %d, UnitSize = %lu, Total = %lu\n", 
        MAX_UNIT_NUMBER_IN_ZONE, sizeof(CGameUnitID), iSceneUnitSize);

	// 鱼池对象
	size_t iFishpondObjSize = CObjAllocator::CountSize(sizeof(CFishpondObj), MAX_FISHPOND_NUMBER_IN_ZONE);
	m_iObjTotalSize += iFishpondObjSize;
	TRACESVR("FishpondObj: Number = %d, UnitSize = %lu, Total = %lu\n",
		MAX_FISHPOND_NUMBER_IN_ZONE, sizeof(CFishpondObj), iFishpondObjSize);

	// 鱼池Hash
	size_t iFishpondHashSize = CHashMap_K32::CountSize(MAX_FISHPOND_NUMBER_IN_ZONE << 1);
	m_iObjTotalSize += iFishpondHashSize;
	TRACESVR("FishpondHash: Number = %d,  Size = %lu\n",
		MAX_FISHPOND_NUMBER_IN_ZONE << 1, iFishpondHashSize);

    // 总计
    TRACESVR("Total Memory: %luB, %luMB\n", m_iObjTotalSize, m_iObjTotalSize/1024/1024);
    TRACESVR("*****************************************************************\n");

    return m_iObjTotalSize;
}

