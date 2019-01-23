#include <assert.h>
#include <string.h>

#include "GameProtocol.hpp"
#include "ModuleHelper.hpp"
#include "WorldMsgHelper.hpp"
#include "LogAdapter.hpp"
#include "ErrorNumDef.hpp"
#include "WorldRankInfoManager.h"

#include "RankInfoWorldHandler.h"

static GameProtocolMsg stMsg;

CRankInfoWorldHandler::~CRankInfoWorldHandler()
{

}

int CRankInfoWorldHandler::OnClientMsg(GameProtocolMsg* pMsg)
{
	ASSERT_AND_LOG_RTN_INT(pMsg);

	m_pRequestMsg = (GameProtocolMsg*)pMsg;

	switch (m_pRequestMsg->sthead().uimsgid())
	{
	case MSGID_WORLD_UPDATERANK_REQUEST:
	{
		//更新排行信息
		OnRequestUpdateRank();
	}
	break;

	case MSGID_WORLD_GETRANKINFO_REQUEST:
	{
		//拉取排行信息
		OnRequestGetWorldRank();
	}
	break;

	default:
		break;
	}

	return 0;
}

//更新排行榜
int CRankInfoWorldHandler::OnRequestUpdateRank()
{
	//LOGDEBUG("OnRequestUpdateRank\n");

	const World_UpdateRank_Request& stReq = m_pRequestMsg->stbody().m_stworld_updaterank_request();

	CWorldMsgHelper::GenerateMsgHead(stMsg, 0, MSGID_WORLD_UPDATERANK_RESPONSE, 0);
	World_UpdateRank_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stworld_updaterank_response();

	int iRet = T_SERVER_SUCCESS;
	for (int i = 0; i < stReq.stupdateranks_size(); ++i)
	{
		iRet = CWorldRankInfoManager::Instance()->UpdateWorldRank(stReq.stupdateranks(i).iranktype(), stReq.stupdateranks(i).strank());
		if (iRet)
		{
			LOGERROR("Failed to update world rank info, ret %d, from %d, type %d\n", iRet, stReq.ifromzoneid(), stReq.stupdateranks(i).iranktype());
			
			pstResp->set_iresult(iRet);
			CWorldMsgHelper::SendWorldMsgToWGS(stMsg, stReq.ifromzoneid());
			return iRet;
		}
	}

	pstResp->set_iresult(T_SERVER_SUCCESS);
	CWorldMsgHelper::SendWorldMsgToWGS(stMsg, stReq.ifromzoneid());

	return T_SERVER_SUCCESS;;
}

//拉取排行榜信息
int CRankInfoWorldHandler::OnRequestGetWorldRank()
{
	//LOGDEBUG("OnRequestGetWorldRank\n");

	//拉取的请求
	const World_GetRankInfo_Request& stReq = m_pRequestMsg->stbody().m_stworld_getrankinfo_request();

	CWorldMsgHelper::GenerateMsgHead(stMsg, 0, MSGID_WORLD_GETRANKINFO_RESPONSE, 0);
	World_GetRankInfo_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stworld_getrankinfo_response();

	int iRet = CWorldRankInfoManager::Instance()->GetWorldRank(stReq.itype(), stReq.iversionid(), *pstResp);
	if (iRet)
	{
		LOGERROR("Failed to get rank info, ret %d, type %d, version %u\n", iRet, stReq.itype(), stReq.iversionid());
		
		pstResp->set_iresult(iRet);
		CWorldMsgHelper::SendWorldMsgToWGS(stMsg, stReq.ifromzoneid());
		return iRet;
	}
	
	pstResp->set_iresult(T_SERVER_SUCCESS);
	CWorldMsgHelper::SendWorldMsgToWGS(stMsg, stReq.ifromzoneid());

	return 0;
}
