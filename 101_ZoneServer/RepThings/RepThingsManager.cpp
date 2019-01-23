
#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "ErrorNumDef.hpp"
#include "Kernel/ZoneObjectHelper.hpp"
#include "Kernel/ZoneMsgHelper.hpp"
#include "Kernel/ConfigManager.hpp"
#include "Kernel/GameRole.hpp"
#include "RepThingsUtility.hpp"

#include "RepThingsManager.hpp"

using namespace ServerLib;

//玩家背包物品发生变化的通知
GameProtocolMsg CRepThingsManager::m_stRepChangeNotify;

CRepThingsManager::CRepThingsManager()
{
    Initialize();
}

CRepThingsManager::~CRepThingsManager()
{

}

//初始化
int CRepThingsManager::Initialize()
{
    m_uiUin = 0;
	m_vstRepItems.clear();

    return T_SERVER_SUCCESS;
}

void CRepThingsManager::SetOwner(unsigned int uin)
{
    m_uiUin = uin;
}

CGameRoleObj* CRepThingsManager::GetOwner()
{
    return GameTypeK32<CGameRoleObj>::GetByKey(m_uiUin);
}

//背包增加物品
int CRepThingsManager::AddRepItem(int iItemID, int iItemNum, int iItemChannel)
{
    if(iItemID<=0 || iItemNum<=0)
    {
        return T_ZONE_PARA_ERROR;
    }

	//先查找是否存在
	RepItem* pstItemInfo = GetRepItemInfo(iItemID);
	if (!pstItemInfo)
	{
		//找不到增加一个
		if (m_vstRepItems.size() >= (unsigned)MAX_REP_BLOCK_NUM)
		{
			//超过最大格子数，不能增加
			return T_ZONE_PARA_ERROR;
		}

		RepItem stItem;
		stItem.iItemID = iItemID;
		stItem.iItemNum = iItemNum;

		m_vstRepItems.push_back(stItem);
	}
	else
	{
		pstItemInfo->iItemNum += iItemNum;
	}

    //发送背包变化的通知
    SendRepChangeNotify(iItemID, iItemNum, iItemChannel);

    return T_SERVER_SUCCESS;
}

//背包删除固定数量的物品
int CRepThingsManager::DeleteRepItem(int iItemID, int iItemNum, int iItemChannel)
{
	if (iItemID <= 0 || iItemNum <= 0)
	{
		return T_ZONE_PARA_ERROR;
	}

    //先判断数量是否足够
	RepItem* pstItemInfo = GetRepItemInfo(iItemID);
	if (!pstItemInfo || pstItemInfo->iItemNum < iItemNum)
	{
		return T_ZONE_PARA_ERROR;
	}

    //够删，实际删除物品
	pstItemInfo->iItemNum -= iItemNum;
	if (pstItemInfo->iItemNum <= 0)
	{
		//从背包数据删除
		for (unsigned i = 0; i < m_vstRepItems.size(); ++i)
		{
			if (m_vstRepItems[i].iItemID == iItemID)
			{
				m_vstRepItems.erase(m_vstRepItems.begin()+i);
				break;
			}
		}
	}

	//发送物品通知
	SendRepChangeNotify(iItemID, -iItemNum, iItemChannel);

    return T_SERVER_SUCCESS;
}

//获取背包中物品的数量
int CRepThingsManager::GetRepItemNum(int iItemID)
{
	RepItem* pstItemInfo = GetRepItemInfo(iItemID);
	if (!pstItemInfo)
	{
		return 0;
	}

	return pstItemInfo->iItemNum;
}

//背包物品数据库操作函数
void CRepThingsManager::UpdateRepItemToDB(ITEMDBINFO& rstItemInfo)
{
    ItemSlotInfo* pstItemInfo = rstItemInfo.mutable_stitemslot();
    for(unsigned i=0; i<m_vstRepItems.size(); ++i)
    {
        OneSlotInfo* pstOneItemInfo = pstItemInfo->add_stslots();
		pstOneItemInfo->set_iitemid(m_vstRepItems[i].iItemID);
		pstOneItemInfo->set_iitemnum(m_vstRepItems[i].iItemNum);
    }

    return;
}

void CRepThingsManager::InitRepItemFromDB(const ITEMDBINFO& stItemInfo)
{
	m_vstRepItems.clear();
	
    const ItemSlotInfo& stSlotInfo = stItemInfo.stitemslot();
	RepItem stItem;
    for(int i=0; i<stSlotInfo.stslots_size(); ++i)
    {
		stItem.iItemID = stSlotInfo.stslots(i).iitemid();
		stItem.iItemNum = stSlotInfo.stslots(i).iitemnum();

		m_vstRepItems.push_back(stItem);
    }

    return;
}

RepItem* CRepThingsManager::GetRepItemInfo(int iItemID)
{
	for (unsigned i = 0; i < m_vstRepItems.size(); ++i)
	{
		if (m_vstRepItems[i].iItemID == iItemID)
		{
			return &m_vstRepItems[i];
		}
	}

	return NULL;
}

//发送背包物品变化的通知
void CRepThingsManager::SendRepChangeNotify(int iItemID, int iItemNum, int iItemChannel)
{
	static GameProtocolMsg stMsg;

    CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_REPCHANGE_NOTIFY);

    Zone_RepChange_Notify* pstNotify = stMsg.mutable_stbody()->mutable_m_stzone_repchange_notify();
	
	OneRepSlot* pstRepSlot = pstNotify->add_stitems();
	pstRepSlot->set_ichangechannel(iItemChannel);

	//物品详细信息
	OneSlotInfo* pstSlotInfo = pstRepSlot->mutable_stslotinfo();
	pstSlotInfo->set_iitemid(iItemID);
	pstSlotInfo->set_iitemnum(iItemNum);

    //发送消息给客户端
    CZoneMsgHelper::SendZoneMsgToRole(stMsg, GetOwner());

    return;
}
