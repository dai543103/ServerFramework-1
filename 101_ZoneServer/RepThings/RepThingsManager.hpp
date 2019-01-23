#ifndef __REP_THINGS_MANAGER_HPP__
#define __REP_THINGS_MANAGER_HPP__

#include <vector>

#include "CommDefine.h"
#include "GameProtocol.hpp"

struct RepItem
{
    int iItemID;
    int iItemNum;

    RepItem()
    {
        Reset();
    };

    void Reset()
    {
        iItemID = 0;
        iItemNum = 0;
    };
};

class CGameRoleObj;
class CRepThingsManager
{
public:
    CRepThingsManager();
    ~CRepThingsManager();

public:

    //初始化
    int Initialize();

    void SetOwner(unsigned int uin);
    CGameRoleObj* GetOwner();

    //背包增加物品
    int AddRepItem(int iItemID, int iItemNum, int iItemChannel = 0);

    //背包删除物品
    int DeleteRepItem(int iItemID, int iItemNum, int iItemChannel = 0);

    //获取背包中物品的数量
    int GetRepItemNum(int iItemID);

    //背包物品数据库操作函数
    void UpdateRepItemToDB(ITEMDBINFO& stItemInfo);
    void InitRepItemFromDB(const ITEMDBINFO& stItemInfo);

private:

	RepItem* GetRepItemInfo(int iItemID);

    //发送背包物品变化的通知
    void SendRepChangeNotify(int iItemID, int iItemNum, int iItemChannel = 0);

private:

    //背包的uin
    unsigned int m_uiUin;

    //背包中的物品信息
    std::vector<RepItem> m_vstRepItems; 

    //玩家背包物品发生变化的通知
    static GameProtocolMsg m_stRepChangeNotify;
};

#endif
