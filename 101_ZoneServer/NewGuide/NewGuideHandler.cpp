
#include "GameProtocol.hpp"
#include "Kernel/GameRole.hpp"
#include "Kernel/ZoneObjectHelper.hpp"
#include "Kernel/ZoneMsgHelper.hpp"
#include "Kernel/ModuleHelper.hpp"
#include "Kernel/ConfigManager.hpp"
#include "Kernel/ZoneOssLog.hpp"

#include "NewGuideHandler.h"

static GameProtocolMsg stMsg;

CNewGuideHandler::~CNewGuideHandler(void)
{

}

CNewGuideHandler::CNewGuideHandler()
{

}

int CNewGuideHandler::OnClientMsg()
{
	switch (m_pRequestMsg->sthead().uimsgid())
	{
	case MSGID_ZONE_FINGUIDE_REQUEST:
	{
		//完成新手引导请求
		OnRequestFinGuide();
	}
	break;

	case MSGID_ZONE_UPDATENAME_REQUEST:
	{
		//玩家修改名字请求
		OnRequestUpdateName();
	}

	default:
	{
		LOGERROR("Failed to handler request msg, invalid msgid: %u\n", m_pRequestMsg->sthead().uimsgid());
		return -1;
	}
	break;
	}

	return 0;
}

//完成新手引导请求
int CNewGuideHandler::OnRequestFinGuide()
{
	int iRet = SecurityCheck();
	if (iRet < 0)
	{
		LOGERROR("Security Check Failed: Uin = %u, iRet = %d\n", m_pRequestMsg->sthead().uin(), iRet);
		return -1;
	}

	//获取请求
	const Zone_FinGuide_Request& stReq = m_pRequestMsg->stbody().m_stzone_finguide_request();

	//返回消息
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_FINGUIDE_RESPONSE);
	Zone_FinGuide_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_finguide_response();
	pstResp->set_iguideid(stReq.iguideid());

	//检查参数
	if (stReq.iguideid() <= 0 || stReq.iguideid() > 32)
	{
		LOGERROR("Failed to fin new guide, invalid guide id %d, uin %u\n", stReq.iguideid(), m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -2;
	}

	//设置为完成
	m_pRoleObj->SetFinGuide(stReq.iguideid());

	//打印运营日志，完成新手引导
	CZoneOssLog::TraceFinNewGuide(m_pRoleObj->GetUin(), m_pRoleObj->GetChannel(), m_pRoleObj->GetNickName(), stReq.iguideid());

	pstResp->set_iresult(T_SERVER_SUCCESS);
	CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);

	return T_SERVER_SUCCESS;
}

//玩家修改名字请求
int CNewGuideHandler::OnRequestUpdateName()
{
	int iRet = SecurityCheck();
	if (iRet < 0)
	{
		LOGERROR("Security Check Failed: Uin = %u, iRet = %d\n", m_pRequestMsg->sthead().uin(), iRet);
		return -1;
	}

	//获取请求
	const Zone_UpdateName_Request& stReq = m_pRequestMsg->stbody().m_stzone_updatename_request();

	//返回消息
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_UPDATENAME_RESPONSE);
	Zone_UpdateName_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_updatename_response();
	pstResp->set_strnewname(stReq.strnewname());
	pstResp->set_bissign(stReq.bissign());

	//检查参数
	if (stReq.strnewname().size() <= 0 || stReq.strnewname().size() >= MAX_NICK_NAME_LENGTH)
	{
		//名字长度不对
		LOGERROR("Failed to change name, invalid new name %s, uin %u\n", stReq.strnewname().c_str(), m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -2;
	}

	//是否名字包含屏蔽字
	if (CConfigManager::Instance()->GetBaseCfgMgr().IsContainMaskWord(stReq.strnewname()))
	{
		//包含屏蔽字
		LOGERROR("Failed to renew name, contains mask word, new name %s, uin %u\n", stReq.strnewname().c_str(), m_pRoleObj->GetUin());

		pstResp->set_iresult(T_ZONE_PARA_ERROR);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return -5;
	}

	if (stReq.bissign())
	{
		//修改签名
		m_pRoleObj->SetSign(stReq.strnewname().c_str());
	}
	else
	{
		//修改名字

		//是否已经修改过名字
		if (m_pRoleObj->GetNameChanged())
		{
			//已经修改过
			LOGERROR("Failed to change nick name, already changed, uin %u\n", m_pRoleObj->GetUin());

			pstResp->set_iresult(T_ZONE_PARA_ERROR);
			CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
			return -3;
		}

		//修改名字
		m_pRoleObj->SetNickName(stReq.strnewname().c_str());
		m_pRoleObj->SetNameChanged(true);
	}

	//强制更新排行榜
	m_pRoleObj->SetUpdateRank(true);

	pstResp->set_iresult(T_SERVER_SUCCESS);
	CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);

	return T_SERVER_SUCCESS;
}
