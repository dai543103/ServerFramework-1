#include <assert.h>
#include <string.h>

#include "GameProtocol.hpp"
#include "ModuleHelper.hpp"
#include "WorldMsgHelper.hpp"
#include "LogAdapter.hpp"
#include "ErrorNumDef.hpp"
#include "WorldLimitLotteryManager.h"

#include "LimitLotteryWorldHandler.h"

static GameProtocolMsg stMsg;

CLimitLotteryWorldHandler::~CLimitLotteryWorldHandler()
{

}

int CLimitLotteryWorldHandler::OnClientMsg(GameProtocolMsg* pMsg)
{
	ASSERT_AND_LOG_RTN_INT(pMsg);

	m_pRequestMsg = (GameProtocolMsg*)pMsg;

	switch (m_pRequestMsg->sthead().uimsgid())
	{
	case MSGID_ZONE_LIMITLOTTERY_REQUEST:
	{
		//玩家限量抽奖
		OnRequestLimitLottery();
	}
	break;

	case MSGID_ZONE_PAYLOTTERYRECORD_REQUEST:
	{
		//玩家拉取充值抽奖记录
		OnRequestPayLotteryRecord();
	}
	break;

	default:
		break;
	}

	return 0;
}

//玩家限量抽奖
int CLimitLotteryWorldHandler::OnRequestLimitLottery()
{
	LOGDEBUG("OnRequestLimitLottery\n");

	//获取请求
	const Zone_LimitLottery_Request& stReq = m_pRequestMsg->stbody().m_stzone_limitlottery_request();

	//返回消息
	CWorldMsgHelper::GenerateMsgHead(stMsg, 0, MSGID_ZONE_LIMITLOTTERY_RESPONSE, stReq.uin());
	Zone_LimitLottery_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_limitlottery_response();
	pstResp->set_uin(stReq.uin());
	pstResp->set_ilotterytype(stReq.ilotterytype());

	//抽奖
	std::vector<int> vLotteryIDs;
	CWorldLimitLotteryManager::Instance()->LimitLottery(stReq.ilotterytype(), stReq.strnickname(), stReq.bistentimes(), vLotteryIDs);
	for (unsigned i = 0; i < vLotteryIDs.size(); ++i)
	{
		pstResp->add_ilotteryids(vLotteryIDs[i]);
	}
	
	pstResp->set_iresult(T_SERVER_SUCCESS);

	//发送返回
	CWorldMsgHelper::SendWorldMsgToWGS(stMsg, stReq.izoneid());

	return T_SERVER_SUCCESS;
}

//玩家拉取重置抽奖记录
int CLimitLotteryWorldHandler::OnRequestPayLotteryRecord()
{
	LOGDEBUG("OnRequestPayLotteryRecord\n");

	//返回消息
	CWorldMsgHelper::GenerateMsgHead(stMsg, 0, MSGID_ZONE_PAYLOTTERYRECORD_RESPONSE, m_pRequestMsg->sthead().uin());
	Zone_PayLotteryRecord_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_paylotteryrecord_response();

	//获取请求
	const Zone_PayLotteryRecord_Request& stReq = m_pRequestMsg->stbody().m_stzone_paylotteryrecord_request();

	//获取记录
	CWorldLimitLotteryManager::Instance()->GetLotteryRecord(stReq.ifrom(), stReq.inum(), *pstResp);
	pstResp->set_iresult(T_SERVER_SUCCESS);

	//发送返回
	CWorldMsgHelper::SendWorldMsgToWGS(stMsg, stReq.izoneid());

	return T_SERVER_SUCCESS;
}
