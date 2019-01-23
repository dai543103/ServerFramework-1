
#include "GameProtocol.hpp"
#include "Kernel/GameRole.hpp"
#include "Kernel/ZoneObjectHelper.hpp"
#include "Kernel/ZoneMsgHelper.hpp"
#include "Kernel/ModuleHelper.hpp"
#include "Kernel/ConfigManager.hpp"
#include "FishpondObj.h"
#include "FishpondManager.h"
#include "FishUtility.h"

#include "FishpondHandler.h"

static GameProtocolMsg stMsg;

CFishpondHandler::CFishpondHandler()
{

}

CFishpondHandler::~CFishpondHandler(void)
{

}

int CFishpondHandler::OnClientMsg()
{
	int iRet = SecurityCheck();
	if (iRet)
	{
		LOGERROR("Security Check Failed: Uin = %u, iRet = %d\n", m_pRequestMsg->sthead().uin(), iRet);
		return -1;
	}

	switch (m_pRequestMsg->sthead().uimsgid())
	{
	case MSGID_ZONE_DOFISH_REQUEST:
	{
		OnRequestDoFish();
	}
	break;

	case MSGID_ZONE_EXITFISH_REQUEST:
	{
		OnRequestExitFish();
	}
	break;

	case MSGID_ZONE_CHANGEGUN_REQUEST:
	{
		OnRequestChangeGun();
	}
	break;

	case MSGID_ZONE_SHOOTBULLET_REQUEST:
	{
		OnRequestShootBullet();
	}
	break;

	case MSGID_ZONE_HITFISH_REQUEST:
	{
		OnRequestHitFish();
	}
	break;

	case MSGID_ZONE_USESKILL_REQUEST:
	{
		OnRequestUseSkill();
	}
	break;

	case MSGID_ZONE_CHOOSEAIMFISH_REQUEST:
	{
		OnRequestChooseAimFish();
	}
	break;

	default:
	{
		LOGERROR("Failed to handler request msg, invalid msgid: %u\n", m_pRequestMsg->sthead().uimsgid());
		return -1;
	}
	break;
	}

	return 0;
}

//进入鱼池消息处理
int CFishpondHandler::OnRequestDoFish()
{
	//返回消息
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_DOFISH_RESPONSE);
	Zone_DoFish_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_dofish_response();

	//玩家进入鱼池消息
	const Zone_DoFish_Request& rstRequest = m_pRequestMsg->stbody().m_stzone_dofish_request();
	if (m_pRoleObj->GetTableID() != 0)
	{
		//玩家还在鱼池中
		LOGERROR("Failed to do fish, already in table %u, uin %u\n", m_pRoleObj->GetTableID(), m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -1;
	}

	//创建鱼池
	CFishpondObj* pstFishpondObj = CFishpondManager::Instance()->CreateFishpond(rstRequest.ifishroomid());
	if (!pstFishpondObj)
	{
		LOGERROR("Failed to get fishpond obj, uin %u, fish room id %d\n", m_pRoleObj->GetUin(), rstRequest.ifishroomid());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -2;
	}

	//玩家进入鱼池
	int iRet = pstFishpondObj->EnterFishpond(*m_pRoleObj);
	if (iRet)
	{
		LOGERROR("Failed to enter fishpond, uin %u, ret %d\n", m_pRoleObj->GetUin(), iRet);
		pstFishpondObj->ExitFishpond(*m_pRoleObj, false);
		CFishpondManager::Instance()->DelFishpond(*pstFishpondObj);

		pstResp->set_iresult(iRet);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -3;
	}

	//处理成功返回
	pstResp->set_ifishroomid(rstRequest.ifishroomid());
	pstResp->set_iresult(T_SERVER_SUCCESS);
	CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);

	return T_SERVER_SUCCESS;
}

//退出鱼池消息处理
int CFishpondHandler::OnRequestExitFish()
{
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_EXITFISH_RESPONSE);
	Zone_ExitFish_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_exitfish_response();

	//玩家退出鱼池消息
	if (m_pRoleObj->GetTableID() == 0)
	{
		//玩家不在鱼池中
		LOGERROR("Failed to exit fish, not in table, uin %u\n", m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -1;
	}

	CFishpondObj* pstFishpondObj = CFishUtility::GetFishpondByID(m_pRoleObj->GetTableID());
	if (!pstFishpondObj)
	{
		//找不到鱼池对象
		LOGERROR("Failed to exit fish, table %u, uin %u\n", m_pRoleObj->GetTableID(), m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -2;
	}

	//玩家退出鱼池
	pstFishpondObj->ExitFishpond(*m_pRoleObj, false);
	CFishpondManager::Instance()->DelFishpond(*pstFishpondObj);

	//处理成功返回
	pstResp->set_iresult(T_SERVER_SUCCESS);
	CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);

	return T_SERVER_SUCCESS;
}

//玩家切换炮台
int CFishpondHandler::OnRequestChangeGun()
{
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_CHANGEGUN_RESPONSE);
	Zone_ChangeGun_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_changegun_response();

	if (m_pRoleObj->GetTableID() == 0)
	{
		//玩家不在鱼池中
		LOGERROR("Failed to change gun, not in table, uin %u\n", m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -1;
	}

	CFishpondObj* pstFishpondObj = CFishUtility::GetFishpondByID(m_pRoleObj->GetTableID());
	if (!pstFishpondObj)
	{
		//找不到鱼池对象
		LOGERROR("Failed to change gun, table %u, uin %u\n", m_pRoleObj->GetTableID(), m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -2;
	}

	//切换炮台的请求
	const Zone_ChangeGun_Request& stRequest = m_pRequestMsg->stbody().m_stzone_changegun_request();
	pstResp->set_inewgunid(stRequest.inewgunid());
	pstResp->set_bisstyle(stRequest.bisstyle());

	int iRet = pstFishpondObj->ChangeGun(*m_pRoleObj, stRequest.inewgunid(), stRequest.bisstyle());
	if (iRet)
	{
		LOGERROR("Failed to change gun, uin %u, new gun %d, ret %d\n", m_pRoleObj->GetUin(), stRequest.inewgunid(), iRet);

		pstResp->set_iresult(iRet);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return iRet;
	}

	//处理成功返回
	pstResp->set_iresult(T_SERVER_SUCCESS);
	CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);

	return T_SERVER_SUCCESS;
}

//玩家发射子弹
int CFishpondHandler::OnRequestShootBullet()
{
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_SHOOTBULLET_RESPONSE);
	Zone_ShootBullet_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_shootbullet_response();

	if (m_pRoleObj->GetTableID() == 0)
	{
		//玩家不在鱼池中
		LOGERROR("Failed to shoot bullet, not in table, uin %u\n", m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -1;
	}

	CFishpondObj* pstFishpondObj = CFishUtility::GetFishpondByID(m_pRoleObj->GetTableID());
	if (!pstFishpondObj)
	{
		//找不到鱼池对象
		LOGERROR("Failed to shoot bullet, table %u, uin %u\n", m_pRoleObj->GetTableID(), m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -2;
	}

	//发射子弹
	const Zone_ShootBullet_Request& stRequest = m_pRequestMsg->stbody().m_stzone_shootbullet_request();
	int iRet = pstFishpondObj->ShootBullet(*m_pRoleObj, stRequest.lshoottime(), stRequest.iposx(), stRequest.iposy(), stRequest.bautoshoot());
	if (iRet)
	{
		//不打印
		//LOGERROR("Failed to shoot bullet, uin %u, shoot time %ld, ret %d\n", m_pRoleObj->GetUin(), stRequest.lshoottime(), iRet);

		pstResp->set_iresult(iRet);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return iRet;
	}

	//处理成功返回
	pstResp->set_iresult(T_SERVER_SUCCESS);
	CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);

	return T_SERVER_SUCCESS;
}

//玩家命中鱼
int CFishpondHandler::OnRequestHitFish()
{
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_HITFISH_RESPONSE);
	Zone_HitFish_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_hitfish_response();

	if (m_pRoleObj->GetTableID() == 0)
	{
		//玩家不在鱼池中
		LOGERROR("Failed to hit fish, not in table, uin %u\n", m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -1;
	}

	CFishpondObj* pstFishpondObj = CFishUtility::GetFishpondByID(m_pRoleObj->GetTableID());
	if (!pstFishpondObj)
	{
		//找不到鱼池对象
		LOGERROR("Failed to hit fish, table %u, uin %u\n", m_pRoleObj->GetTableID(), m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -2;
	}

	//命中普通鱼
	const Zone_HitFish_Request& stRequest = m_pRequestMsg->stbody().m_stzone_hitfish_request();
	int iRet = pstFishpondObj->HitFish(*m_pRoleObj, stRequest.lhittime(), stRequest.ubulletuniqid(), stRequest.ufishuniqid(), stRequest.ifishindex());
	if (iRet)
	{
		LOGERROR("Failed to hit fish, uin %u, hit time %ld, ret %d\n", m_pRoleObj->GetUin(), stRequest.lhittime(), iRet);

		pstResp->set_iresult(iRet);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return iRet;
	}

	//处理成功返回
	pstResp->set_iresult(T_SERVER_SUCCESS);
	CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);

	return T_SERVER_SUCCESS;
}

//玩家命中鱼阵鱼
int CFishpondHandler::OnRequestHitFormFish()
{
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_HITFORMFISH_RESPONSE);
	Zone_HitFormFish_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_hitformfish_response();

	if (m_pRoleObj->GetTableID() == 0)
	{
		//玩家不在鱼池中
		LOGERROR("Failed to hit fish, not in table, uin %u\n", m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -1;
	}

	CFishpondObj* pstFishpondObj = CFishUtility::GetFishpondByID(m_pRoleObj->GetTableID());
	if (!pstFishpondObj)
	{
		//找不到鱼池对象
		LOGERROR("Failed to hit fish, table %u, uin %u\n", m_pRoleObj->GetTableID(), m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -2;
	}

	//命中鱼阵鱼
	const Zone_HitFormFish_Request& stRequest = m_pRequestMsg->stbody().m_stzone_hitformfish_request();
	int iRet = pstFishpondObj->HitFormFish(*m_pRoleObj, stRequest.lhittime(), stRequest.ubulletuniqid(), stRequest.iformoutid(), stRequest.ifishindex());
	if (iRet)
	{
		LOGERROR("Failed to hit form fish, uin %u, hit time %ld, ret %d\n", m_pRoleObj->GetUin(), stRequest.lhittime(), iRet);

		pstResp->set_iresult(iRet);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return iRet;
	}

	//处理成功返回
	pstResp->set_iresult(T_SERVER_SUCCESS);
	CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);

	return T_SERVER_SUCCESS;
}

//玩家使用技能
int CFishpondHandler::OnRequestUseSkill()
{
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_USESKILL_RESPONSE);
	Zone_UseSkill_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_useskill_response();

	if (m_pRoleObj->GetTableID() == 0)
	{
		//玩家不在鱼池中
		LOGERROR("Failed to use skill, not in table, uin %u\n", m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -1;
	}

	CFishpondObj* pstFishpondObj = CFishUtility::GetFishpondByID(m_pRoleObj->GetTableID());
	if (!pstFishpondObj)
	{
		//找不到鱼池对象
		LOGERROR("Failed to use skill, table %u, uin %u\n", m_pRoleObj->GetTableID(), m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -2;
	}

	//使用技能
	const Zone_UseSkill_Request& stRequest = m_pRequestMsg->stbody().m_stzone_useskill_request();
	int iRet = pstFishpondObj->UseSkill(*m_pRoleObj, stRequest);
	if (iRet)
	{
		LOGERROR("Failed to use skill, uin %u, skill type %d, ret %d\n", m_pRoleObj->GetUin(), stRequest.itype(), iRet);
		
		pstResp->set_iresult(iRet);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return iRet;
	}

	//处理成功返回
	pstResp->set_itype(stRequest.itype());
	pstResp->set_iresult(T_SERVER_SUCCESS);
	CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);

	return T_SERVER_SUCCESS;
}

//玩家选择瞄准鱼
int CFishpondHandler::OnRequestChooseAimFish()
{
	//获取请求
	const Zone_ChooseAimFish_Request& stRequest = m_pRequestMsg->stbody().m_stzone_chooseaimfish_request();

	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_CHOOSEAIMFISH_RESPONSE);
	Zone_ChooseAimFish_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_chooseaimfish_response();
	pstResp->set_ufishuniqid(stRequest.ufishuniqid());
	pstResp->set_ifishindex(stRequest.ifishindex());

	if (m_pRoleObj->GetTableID() == 0)
	{
		//玩家不在鱼池中
		LOGERROR("Failed to choose aim fish, not in table, uin %u\n", m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -1;
	}

	CFishpondObj* pstFishpondObj = CFishUtility::GetFishpondByID(m_pRoleObj->GetTableID());
	if (!pstFishpondObj)
	{
		//找不到鱼池对象
		LOGERROR("Failed to choose aim fish, table %u, uin %u\n", m_pRoleObj->GetTableID(), m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -2;
	}

	//选择瞄准鱼
	int iRet = pstFishpondObj->ChooseAimFish(*m_pRoleObj, stRequest.ufishuniqid(), stRequest.ifishindex());
	if (iRet)
	{
		LOGERROR("Failed to choose aim fish, uin %u, fish uniq id %u, ret %d\n", m_pRoleObj->GetUin(), stRequest.ufishuniqid(), iRet);

		pstResp->set_iresult(iRet);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return iRet;
	}

	//处理成功返回
	pstResp->set_iresult(T_SERVER_SUCCESS);
	CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);

	return T_SERVER_SUCCESS;
}
