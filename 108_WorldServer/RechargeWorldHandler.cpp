#include <assert.h>
#include <string.h>

#include "GameProtocol.hpp"
#include "ModuleHelper.hpp"
#include "WorldMsgHelper.hpp"
#include "LogAdapter.hpp"
#include "ErrorNumDef.hpp"


#include "RechargeWorldHandler.h"

static GameProtocolMsg stMsg;

CRechargeWorldHandler::~CRechargeWorldHandler()
{

}

int CRechargeWorldHandler::OnClientMsg(GameProtocolMsg* pMsg)
{
	ASSERT_AND_LOG_RTN_INT(pMsg);

	m_pRequestMsg = (GameProtocolMsg*)pMsg;

	switch (m_pRequestMsg->sthead().uimsgid())
	{
	case MSGID_WORLD_USERRECHARGE_REQUEST:
	{
		//系统充值的请求
		OnRequestUserRecharge();
	}
	break;

	case MSGID_WORLD_USERRECHARGE_RESPONSE:
	{
		//系统充值的返回
		OnResponseUserRecharge();
	}
	break;

	case MSGID_ZONE_GETPAYORDER_REQUEST:
	{
		//玩家拉取充值订单号请求
		OnRequestGetPayOrder();
	}
	break;

	case MSGID_WORLD_GETUSERINFO_REQUEST:
	{
		//拉取账号信息的请求
		OnRequestGetUserInfo();
	}
	break;

	case MSGID_WORLD_GETUSERINFO_RESPONSE:
	{
		//拉取账号信息的返回
		OnResponseGetUserInfo();
	}
	break;

	default:
		break;
	}

	return 0;
}

//系统充值的请求
int CRechargeWorldHandler::OnRequestUserRecharge()
{
	LOGDEBUG("OnRequestUserRecharge\n");

	//获取返回
	CWorldMsgHelper::GenerateMsgHead(stMsg, m_pRequestMsg->sthead().uisessionfd(), MSGID_WORLD_USERRECHARGE_RESPONSE, 0);
	World_UserRecharge_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stworld_userrecharge_response();

	//获取请求
	const World_UserRecharge_Request& stReq = m_pRequestMsg->mutable_stbody()->m_stworld_userrecharge_request();
	if (m_pRequestMsg->sthead().uin() != stReq.uin())
	{
		LOGERROR("Failed to recharge user, uin head:body %u:%u\n", m_pRequestMsg->sthead().uin(), stReq.uin());
		pstResp->set_iresult(T_WORLD_PARA_ERROR);
		CWorldMsgHelper::SendWorldMsgToRecharge(stMsg);
		return -1;
	}

	pstResp->set_uin(stReq.uin());
	pstResp->set_irechargeid(stReq.irechargeid());
	pstResp->set_strorderid(stReq.strorderid());
	pstResp->set_iplatform(stReq.iplatform());

	//读取配置
	const RechargeConfig* pstConfig = CConfigManager::Instance()->GetBaseCfgMgr().GetRechargeConfig(stReq.irechargeid());
	if (!pstConfig)
	{
		LOGERROR("Failed to get recharge config, invalid id %d, uin %u, order %s\n", stReq.irechargeid(), stReq.uin(), stReq.strorderid().c_str());

		pstResp->set_iresult(T_WORLD_PARA_ERROR);
		CWorldMsgHelper::SendWorldMsgToRecharge(stMsg);
		return -2;
	}

	//校验价格
	if (pstConfig->iCostRMB != stReq.irmb())
	{
		LOGERROR("Failed to recharge id %d, real:cost %d:%d, uin %u, order %s\n", pstConfig->iID, stReq.irmb(), pstConfig->iCostRMB, stReq.uin(), stReq.strorderid().c_str());
		
		pstResp->set_iresult(T_WORLD_PARA_ERROR);
		CWorldMsgHelper::SendWorldMsgToRecharge(stMsg);
		return -3;
	}

	//转发充值消息
	int iRet = 0;
	CWorldRoleStatusWObj* pstRoleStatObj = WorldTypeK32<CWorldRoleStatusWObj>::GetByUin(stReq.uin());
	if (!pstRoleStatObj)
	{
		//玩家不在线，转发给RoleDB
		iRet = CWorldMsgHelper::SendWorldMsgToRoleDB(*m_pRequestMsg);
	}
	else
	{
		//转发对应Zone
		iRet = CWorldMsgHelper::SendWorldMsgToWGS(*m_pRequestMsg, pstRoleStatObj->GetZoneID());
	}

	if (iRet)
	{
		LOGERROR("Failed to send recharge msg, uin %u, order %s, recharge id %d\n", stReq.uin(), stReq.strorderid().c_str(), stReq.irechargeid());

		pstResp->set_iresult(iRet);
		CWorldMsgHelper::SendWorldMsgToRecharge(stMsg);
		return -4;
	}

	return T_SERVER_SUCCESS;;
}

//系统充值的返回
int CRechargeWorldHandler::OnResponseUserRecharge()
{
	LOGDEBUG("OnResponseUserRecharge\n");

	//直接转发给RechargeServer
	int iRet = CWorldMsgHelper::SendWorldMsgToRecharge(*m_pRequestMsg);
	if (iRet)
	{
		LOGERROR("Failed to send recharge msg, ret %d\n", iRet);
		return -1;
	}

	return T_SERVER_SUCCESS;
}

//玩家拉取订单号
int CRechargeWorldHandler::OnRequestGetPayOrder()
{
	LOGDEBUG("OnRequestGetPayOrder\n");

	//获取请求
	const Zone_GetPayOrder_Request& stReq = m_pRequestMsg->stbody().m_stzone_getpayorder_request();

	//返回消息
	CWorldMsgHelper::GenerateMsgHead(stMsg, 0, MSGID_ZONE_GETPAYORDER_RESPONSE, stReq.uin());
	Zone_GetPayOrder_Response* pstResp = stMsg.mutable_stbody()->mutable_m_stzone_getpayorder_response();
	
	//获取订单号
	pstResp->set_strorderid(GetNewOrderID(stReq.uin()));

	pstResp->set_iresult(T_SERVER_SUCCESS);
	CWorldMsgHelper::SendWorldMsgToWGS(stMsg, stReq.ifromzoneid());

	return T_SERVER_SUCCESS;
}

//拉取账号信息的请求
int CRechargeWorldHandler::OnRequestGetUserInfo()
{
	LOGDEBUG("OnRequestGetUserInfo\n");

	//直接转发给RoleDBServer
	int iRet = CWorldMsgHelper::SendWorldMsgToRoleDB(*m_pRequestMsg);
	if (iRet)
	{
		LOGERROR("Failed to send get user info req msg, ret %d\n", iRet);
		return -1;
	}

	return T_SERVER_SUCCESS;
}

//拉取账号信息的返回
int CRechargeWorldHandler::OnResponseGetUserInfo()
{
	LOGDEBUG("OnResponseGetUserInfo\n");

	//直接转发给RechargeServer
	int iRet = CWorldMsgHelper::SendWorldMsgToRecharge(*m_pRequestMsg);
	if (iRet)
	{
		LOGERROR("Failed to send get user info resp msg, ret %d\n", iRet);
		return -1;
	}

	return T_SERVER_SUCCESS;
}

//获取充值订单号
std::string CRechargeWorldHandler::GetNewOrderID(unsigned uiUin)
{
	//订单号生成规则： timeNow(10个字符) + uin(8个字符) + (0-999)(3个字符), 按字符串方式拼接
	static int iIndex = 0;
	iIndex = (iIndex+1) % 1000;

	int iTimeNow = CTimeUtility::GetNowTime();

	char szOrderID[32] = { 0 };
	SAFE_SPRINTF(szOrderID, sizeof(szOrderID) - 1, "%10d%08u%03d", iTimeNow, uiUin, iIndex);

	return szOrderID;
}
