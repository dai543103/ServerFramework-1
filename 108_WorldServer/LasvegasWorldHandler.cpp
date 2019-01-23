#include <assert.h>
#include <string.h>

#include "GameProtocol.hpp"
#include "ModuleHelper.hpp"
#include "WorldMsgHelper.hpp"
#include "LogAdapter.hpp"
#include "ErrorNumDef.hpp"
#include "WorldLasvegasManager.h"

#include "LasvegasWorldHandler.h"

static GameProtocolMsg stMsg;

CLasvegasWorldHandler::~CLasvegasWorldHandler()
{

}

int CLasvegasWorldHandler::OnClientMsg(GameProtocolMsg* pMsg)
{
	ASSERT_AND_LOG_RTN_INT(pMsg);

	m_pRequestMsg = (GameProtocolMsg*)pMsg;

	switch (m_pRequestMsg->sthead().uimsgid())
	{
	case MSGID_WORLD_UPDATEBETINFO_REQUEST:
	{
		//更新数字下注信息
		OnRequestUpdateBetInfo();
	}
	break;

	case MSGID_WORLD_UPDATEPRIZEINFO_REQUEST:
	{
		//更新中奖信息
		OnRequestUpdatePrizeInfo();
	}
	break;

	default:
		break;
	}

	return 0;
}

//更新数字下注信息
int CLasvegasWorldHandler::OnRequestUpdateBetInfo()
{
	LOGDEBUG("OnRequestUpdateBetInfo\n");

	const World_UpdateBetInfo_Request& stReq = m_pRequestMsg->stbody().m_stworld_updatebetinfo_request();

	//不需要发送返回
	for (int i = 0; i < stReq.stbetinfos_size(); ++i)
	{
		CWorldLasvegasManager::Instance()->UpdateBetNumber(stReq.stbetinfos(i).inumber(), stReq.stbetinfos(i).lbetcoins());
	}

	return T_SERVER_SUCCESS;
}

//更新中奖信息
int CLasvegasWorldHandler::OnRequestUpdatePrizeInfo()
{
	LOGDEBUG("OnRequestUpdatePrizeInfo\n");

	//获取请求
	const World_UpdatePrizeInfo_Request& stReq = m_pRequestMsg->stbody().m_stworld_updateprizeinfo_request();

	//不需要发送返回
	CWorldLasvegasManager::Instance()->UpdatePrizeInfo(stReq);

	return T_SERVER_SUCCESS;;
}
