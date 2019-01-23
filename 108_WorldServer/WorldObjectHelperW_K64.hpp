#ifndef __APP_ALLOCATOR_K64_HPP__
#define __APP_ALLOCATOR_K64_HPP__

#include "ObjAllocator.hpp"
#include "HashMap_K64.hpp"
#include "AppDefW.hpp"
#include "WorldObjectHelperW_K32.hpp"
#include "WorldRoleStatus.hpp"

using namespace ServerLib;

//64位Hash对象操作器
template <class TYPE_Object>
class WorldTypeK64
{
public:
    //////////////////////////////////////////////////////////////////////////
    static TYPE_Object* CreateByHashKey(const TData64& rstHashKey)
    {
        int iID = 0;
        iID = m_pAllocator->CreateObject();
        if(iID < 0)
        {
            return NULL;
        }

        int iRet = 0;
        iRet = m_pHashMap->InsertValueByKey(rstHashKey, iID);
        if(iRet < 0)
        {
            m_pAllocator->DestroyObject(iID);
            return NULL;
        }

        return (TYPE_Object*)m_pAllocator->GetObj(iID);
    }

    static TYPE_Object* GetByHashKey(const TData64& rstHashKey)
    {
        int iID = 0;
        int iRet = 0;
        iRet = m_pHashMap->GetValueByKey(rstHashKey, iID);
        if(iRet < 0)
        {
            return NULL;
        }

        return (TYPE_Object*)m_pAllocator->GetObj(iID);
    }

    //删除对象还必须删除Hash
    static int DeleteByHashKey(const TData64& rstHashKey)
    {
        int iID = 0;
        int iRet = 0;
        iRet = m_pHashMap->DeleteByKey(rstHashKey, iID);
        if(iRet < 0)
        {
            return -1;
        }

        iRet = m_pAllocator->DestroyObject(iID);
        if(iRet < 0)
        {
            return -2;
        }

        return 0;
    }

    static int GetUsedHead()
    {
        return m_pAllocator->GetUsedHead();
    }

    static int GetNextIdx(const int iIdx)
    {
        CIdx *pIdx = m_pAllocator->GetIdx(iIdx);
        if (pIdx)
        {
            return pIdx->GetNextIdx();
        }

        return -1;
    }

    static TYPE_Object* GetByIdx(const int iIdx)
    {
        return (TYPE_Object*) m_pAllocator->GetObj(iIdx);
    }

    static int GetUsedObjNumber()
    {
        return m_pAllocator->GetUsedCount();
    }

    static CIdx* GetIdx(const int iIdx)
    {
        return m_pAllocator->GetIdx(iIdx);
    }

public:
    static int RegisterHashAllocator(CObjAllocator* pAllocator, CHashMap_K64* pHashMap)
    {
        if(!pAllocator || !pHashMap)
        {
            return -1;
        }
        m_pAllocator = pAllocator;
        m_pHashMap = pHashMap;
        return 0;
    }

private:
    static CObjAllocator* m_pAllocator;
    static CHashMap_K64* m_pHashMap;
};

template <class TYPE_Object>
CObjAllocator* WorldTypeK64<TYPE_Object>::m_pAllocator = 0;

template <class TYPE_Object>
CHashMap_K64* WorldTypeK64<TYPE_Object>::m_pHashMap = 0;


#endif
