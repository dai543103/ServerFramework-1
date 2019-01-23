#pragma once

//游戏单位ID
#include "ObjIdxList.hpp"
#include "ObjAllocator.hpp"

using namespace ServerLib;

class CGameUnitID : public CObj
{
public:
    CGameUnitID() {};
    virtual ~CGameUnitID() {};
    virtual int Initialize();
    virtual int Resume();

public:
    DECLARE_DYN

public:
    // 绑定对象Idx
    void BindObject(int iObjectIdx);

    // 获取绑定对象的Idx
    int GetBindObjectIdx();

    // 获取绑定对象的属性
    TUNITINFO* GetBindUnitInfo();

private:
    // 设置对象的属性
    void BindUnitInfo();

private: 
    int m_iBindObjectIdx;
    TUNITINFO* m_pstUnitInfo;
};
