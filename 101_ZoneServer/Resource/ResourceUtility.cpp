
#include "GameProtocol.hpp"
#include "ErrorNumDef.hpp"
#include "Kernel/GameRole.hpp"
#include "Kernel/ZoneMsgHelper.hpp"
#include "Reward/RewardUtility.h"
#include "Vip/VipUtility.h"
#include "FishGame/FishUtility.h"
#include "FishGame/FishpondObj.h"

#include "ResourceUtility.h"

static GameProtocolMsg stMsg;

//增加玩家资源，iAddNum为负表示减少
bool CResourceUtility::AddUserRes(CGameRoleObj& stRoleObj, int iResType, long8 lAddNum)
{
	if (!stRoleObj.AddResource(iResType, lAddNum))
	{
		return false;
	}

	//推送资源变化的通知
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_RESOURCECHANGE_NOTIFY);

	Zone_ResourceChange_Notify* pstNotify = stMsg.mutable_stbody()->mutable_m_stzone_resourcechange_notify();
	UserResourceChange* pstChangeInfo = pstNotify->add_stchangeinfos();
	pstChangeInfo->set_irestype(iResType);
	pstChangeInfo->set_inewresnum(stRoleObj.GetResource(iResType));

	CZoneMsgHelper::SendZoneMsgToRole(stMsg, &stRoleObj);

	if (stRoleObj.GetTableID() != 0 && (iResType == RESOURCE_TYPE_COIN || iResType == RESOURCE_TYPE_TICKET))
	{
		//玩家在捕鱼中，推送金币鱼票更新给桌子上玩家
		CFishpondObj* pstFishpondObj = CFishUtility::GetFishpondByID(stRoleObj.GetTableID());
		if (pstFishpondObj)
		{
			pstFishpondObj->SendFishUserUpdate(stRoleObj.GetUin(), iResType, lAddNum, 0, 0);
		}
	}

	//是否触发救济金
	if (stRoleObj.GetResource(RESOURCE_TYPE_COIN) == 0)
	{
		//触发救济金
		CVipUtility::TriggerAlmsUpdate(stRoleObj);
	}

	return true;
}
