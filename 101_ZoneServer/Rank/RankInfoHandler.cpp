
#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "ErrorNumDef.hpp"
#include "Kernel/ModuleHelper.hpp"
#include "Kernel/ZoneMsgHelper.hpp"
#include "RankInfoManager.h"

#include "RankInfoHandler.h"

using namespace ServerLib;

static GameProtocolMsg stMsg;

CRankInfoHandler::~CRankInfoHandler()
{

}

int CRankInfoHandler::OnClientMsg()
{
	switch (m_pRequestMsg->sthead().uimsgid())
	{
	case MSGID_ZONE_GETRANKINFO_REQUEST:
	{
		//拉取排行榜信息
		OnRequestGetRankInfo();
	}
	break;

	case MSGID_WORLD_GETRANKINFO_RESPONSE:
	{
		//拉取World排行返回
		OnResponseGetWorldRank();
	}
	break;

	case MSGID_WORLD_UPDATERANK_RESPONSE:
	{
		//不需要处理
	}
	break;

	default:
		break;
	}

	return 0;
}

//玩家拉取排行榜信息
int CRankInfoHandler::OnRequestGetRankInfo()
{
	int iRet = SecurityCheck();
	if (iRet < 0)
	{
		LOGERROR("Security Check Failed: Uin = %u, iRet = %d\n", m_pRequestMsg->sthead().uin(), iRet);
		return -1;
	}

	//返回消息
	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_ZONE_GETRANKINFO_RESPONSE);
	Zone_GetRankInfo_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_getrankinfo_response();

	//获取请求
	const Zone_GetRankInfo_Request& stReq = m_pRequestMsg->stbody().m_stzone_getrankinfo_request();
	pstResp->set_itype(stReq.itype());
	pstResp->set_blastrank(stReq.blastrank());

	iRet = CRankInfoManager::Instance()->GetRankInfo(m_pRoleObj->GetUin(), stReq.itype(), stReq.ifromindex(), stReq.inum(), stReq.blastrank(), *pstResp);
	if (iRet)
	{
		LOGERROR("Failed to get rank info, ret %d, type %d, from %d\n", iRet, stReq.itype(), stReq.ifromindex());
		
		pstResp->set_iresult(iRet);
		CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);
		return iRet;
	}

	//发送返回
	pstResp->set_iresult(T_SERVER_SUCCESS);
	CZoneMsgHelper::SendZoneMsgToRole(stMsg, m_pRoleObj);

	return T_SERVER_SUCCESS;
}

//拉取World排行榜的返回
int CRankInfoHandler::OnResponseGetWorldRank()
{
	//获取响应
	const World_GetRankInfo_Response& stResp = m_pRequestMsg->stbody().m_stworld_getrankinfo_response();
	if (stResp.iresult() != 0)
	{
		LOGERROR("Failed to get rank from world, ret %d\n", stResp.iresult());
		return -1;
	}

	int iRet = CRankInfoManager::Instance()->SetRankListInfo(stResp);
	if (iRet)
	{
		LOGERROR("Failed to set rank info, ret %d\n", iRet);
		return iRet;
	}

	return T_SERVER_SUCCESS;
}
