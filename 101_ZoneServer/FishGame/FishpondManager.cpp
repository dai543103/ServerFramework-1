
#include "GameProtocol.hpp"
#include "Kernel/GameRole.hpp"
#include "Kernel/ZoneObjectHelper.hpp"
#include "Kernel/ZoneMsgHelper.hpp"
#include "Kernel/ModuleHelper.hpp"
#include "Kernel/ConfigManager.hpp"
#include "FishUtility.h"

#include "FishpondManager.h"

CFishpondManager* CFishpondManager::Instance()
{
	static CFishpondManager* pInstance = NULL;
	if (!pInstance)
	{
		pInstance = new CFishpondManager;
	}

	return pInstance;
}

CFishpondManager::CFishpondManager()
{
	m_mpFreeHead.clear();
}

CFishpondManager::~CFishpondManager()
{

}

//创建鱼池
CFishpondObj* CFishpondManager::CreateFishpond(int iFishRoomID)
{
	//读取配置
	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();
	const FishRoomConfig* pstRoomConfig = stBaseCfgMgr.GetFishRoomConfig(iFishRoomID);
	if (!pstRoomConfig)
	{
		LOGERROR("Failed to get fish room config, room id %d\n", iFishRoomID);
		return NULL;
	}

	//先从空闲的获取
	CFishpondObj* pstFishpondObj = NewFromFishpondList(*pstRoomConfig);
	if (pstFishpondObj)
	{
		//有空闲的
		return pstFishpondObj;
	}

	//没有空闲的，创建一个
	unsigned uTableUniqID = CFishUtility::GetFishUniqID();
	pstFishpondObj = GameTypeK32<CFishpondObj>::CreateByKey(uTableUniqID);
	if (!pstFishpondObj)
	{
		LOGERROR("Failed to create fishpond obj!\n");
		return NULL;
	}

	//设置桌子信息
	pstFishpondObj->SetTableInfo(uTableUniqID, iFishRoomID, *pstRoomConfig);

	//添加到空闲链表
	AddToFishpondList(*pstRoomConfig, *pstFishpondObj);

	return pstFishpondObj;
}

//销毁鱼池
void CFishpondManager::DelFishpond(CFishpondObj& stFishpondObj)
{
	const FishRoomConfig* pstConfig = stFishpondObj.GetRoomConfig();
	if (!pstConfig)
	{
		LOGERROR("Failed to del fishpond, invalid fish config!\n");
		return;
	}

	if (stFishpondObj.GetPlayerNum() == 0)
	{
		//鱼池已经没有人了，可以销毁
		if (stFishpondObj.m_pNext)
		{
			stFishpondObj.m_pNext->m_pPrev = stFishpondObj.m_pPrev;
		}

		if (stFishpondObj.m_pPrev)
		{
			stFishpondObj.m_pPrev->m_pNext = stFishpondObj.m_pNext;
		}

		if (m_mpFreeHead[pstConfig->iRoomID] == &stFishpondObj)
		{
			m_mpFreeHead[pstConfig->iRoomID] = stFishpondObj.m_pNext;
		}

		stFishpondObj.m_pPrev = NULL;
		stFishpondObj.m_pNext = NULL;

		unsigned uTableID = stFishpondObj.GetTableID();

		//重置Fishpond
		stFishpondObj.ResetFishpond();

		//删除Fishpond
		GameTypeK32<CFishpondObj>::DeleteByKey(uTableID);
	}
	else if ((stFishpondObj.GetPlayerNum() + 1) == pstConfig->iPlayerNum)
	{
		//由人满变为人不满

		//增加到空闲链表
		stFishpondObj.m_pNext = m_mpFreeHead[pstConfig->iRoomID];
		stFishpondObj.m_pPrev = NULL;
		if (m_mpFreeHead[pstConfig->iRoomID])
		{
			m_mpFreeHead[pstConfig->iRoomID]->m_pPrev = &stFishpondObj;
		}

		m_mpFreeHead[pstConfig->iRoomID] = &stFishpondObj;
	}

	return;
}

//重新加载鱼池配置
void CFishpondManager::ReloadFishConfig()
{
	int iFishpondIndex = GameTypeK32<CFishpondObj>::GetUsedHead();
	while (iFishpondIndex >= 0)
	{
		CFishpondObj* pstFishpondObj = GameTypeK32<CFishpondObj>::GetByIdx(iFishpondIndex);
		if (pstFishpondObj)
		{
			unsigned uTableUniqID = pstFishpondObj->GetTableID();
			int iFishRoomID = pstFishpondObj->GetFishRoomID();

			const FishRoomConfig* pstRoomConfig = CConfigManager::Instance()->GetBaseCfgMgr().GetFishRoomConfig(iFishRoomID);
			if (pstRoomConfig)
			{
				pstFishpondObj->SetTableInfo(uTableUniqID, iFishRoomID, *pstRoomConfig);
			}
			else
			{
				LOGERROR("Failed to reload fish room config, invalid room id %d\n", iFishRoomID);
			}
		}

		iFishpondIndex = GameTypeK32<CFishpondObj>::GetNextIdx(iFishpondIndex);
	}

	return;
}

//获取空闲鱼池
CFishpondObj* CFishpondManager::NewFromFishpondList(const FishRoomConfig& stConfig)
{
	if (m_mpFreeHead.find(stConfig.iRoomID) == m_mpFreeHead.end() || m_mpFreeHead[stConfig.iRoomID]==NULL)
	{
		//空闲链表中不存在
		return NULL;
	}

	CFishpondObj* pstTempObj = m_mpFreeHead[stConfig.iRoomID];
	if ((pstTempObj->GetPlayerNum() + 1) < stConfig.iPlayerNum)
	{
		//鱼池还未坐满
		return pstTempObj;
	}

	//鱼池坐满了,从空闲链表取出
	m_mpFreeHead[stConfig.iRoomID] = pstTempObj->m_pNext;
	pstTempObj->m_pNext = NULL;
	pstTempObj->m_pPrev = NULL;

	return pstTempObj;
}

//增加鱼池到空闲链表
void CFishpondManager::AddToFishpondList(const FishRoomConfig& stConfig, CFishpondObj& stFishpondObj)
{
	if ((stFishpondObj.GetPlayerNum() + 1) >= stConfig.iPlayerNum)
	{
		//坐满了
		return;
	}

	if (m_mpFreeHead.find(stConfig.iRoomID) != m_mpFreeHead.end())
	{
		//已经存在
		if (m_mpFreeHead[stConfig.iRoomID] != NULL)
		{
			stFishpondObj.m_pNext = m_mpFreeHead[stConfig.iRoomID];
			stFishpondObj.m_pPrev = NULL;

			m_mpFreeHead[stConfig.iRoomID]->m_pPrev = &stFishpondObj;

			m_mpFreeHead[stConfig.iRoomID] = &stFishpondObj;
		}
		else
		{
			stFishpondObj.m_pNext = NULL;
			stFishpondObj.m_pPrev = NULL;
			m_mpFreeHead[stConfig.iRoomID] = &stFishpondObj;
		}
	}
	else
	{
		stFishpondObj.m_pNext = NULL;
		stFishpondObj.m_pPrev = NULL;
		m_mpFreeHead[stConfig.iRoomID] = &stFishpondObj;
	}

	return;
}
