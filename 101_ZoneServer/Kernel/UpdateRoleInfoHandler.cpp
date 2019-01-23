
#include "ProtoDataUtility.hpp"
#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "TimeUtility.hpp"
#include "Kernel/UnitUtility.hpp"
#include "Kernel/ZoneMsgHelper.hpp"
#include "Kernel/HandlerHelper.hpp"
#include "Kernel/HandlerFactory.hpp"
#include "Kernel/ModuleHelper.hpp"
#include "Kernel/ZoneObjectHelper.hpp"
#include "Kernel/ZoneOssLog.hpp"
#include "Login/LogoutHandler.hpp"
#include "Login/KickRoleWorldHandler.hpp"
#include "Rank/RankInfoManager.h"

#include "UpdateRoleInfoHandler.hpp"

using namespace ServerLib;

CUpdateRoleInfo_Handler::~CUpdateRoleInfo_Handler(void)
{

}

int CUpdateRoleInfo_Handler::OnClientMsg()
{
	int iRet = -1;
	switch (m_pRequestMsg->sthead().uimsgid())
	{
	case MSGID_WORLD_UPDATEROLE_RESPONSE:
	{
		iRet = OnUpdateRoleInfoResponse();
	}
	break;

	default:
	{
	}
	break;
	}

	return iRet;
}

//更新玩家结算信息
void CUpdateRoleInfo_Handler::TallyRoleData(CGameRoleObj& stRoleObj)
{
	//更新玩家输赢
	stRoleObj.UpdateRoleWinNum();

	//上报结算数据
	CZoneOssLog::ReportTallyData(stRoleObj);

	//更新结算信息
	stRoleObj.UpdateTallyInfo();

	return;
}

int CUpdateRoleInfo_Handler::OnUpdateRoleInfoResponse()
{
	const World_UpdateRole_Response& rstResp = m_pRequestMsg->stbody().m_stworld_updaterole_response();
	unsigned int uiUin = rstResp.stroleid().uin();

	CGameRoleObj* pRoleObj = CUnitUtility::GetRoleByUin(uiUin);
	if (!pRoleObj)
	{
		LOGERROR("Update role info, cannot find role %u.\n", uiUin);
		return 0;
	}

	if (!CUnitUtility::IsUnitStatusSet(&pRoleObj->GetUnitInfo(), EGUS_LOGOUT))
	{
		return 0;
	}

	unsigned int uiResult = rstResp.iresult();
	if (T_SERVER_SUCCESS == uiResult)
	{
		LOGDEBUG("UpdateRole OK: Uin = %u\n", uiUin);

		// 设置下线状态
		pRoleObj->SetOffline();

		// 登出通告和数据删除，这个部分在收到 world成功响应之后再删除
		int iRet = CLogoutHandler::LogoutAction(pRoleObj);
		if (iRet < 0)
		{
			return iRet;
		}
	}

	return 0;
}

int CUpdateRoleInfo_Handler::UpdateRoleInfo(CGameRoleObj* pRoleObj, unsigned char ucNeedResponse)
{
	static GameProtocolMsg stMsg;

	ASSERT_AND_LOG_RTN_INT(pRoleObj);

	unsigned int uiUin = pRoleObj->GetRoleID().uin();

	//结算玩家信息
	TallyRoleData(*pRoleObj);

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// 填充要更新的玩家数据

	CZoneMsgHelper::GenerateMsgHead(stMsg, MSGID_WORLD_UPDATEROLE_REQUEST);
	stMsg.mutable_sthead()->set_uin(uiUin);

	World_UpdateRole_Request* pstReq = stMsg.mutable_stbody()->mutable_m_stworld_updaterole_request();
	pstReq->mutable_stroleid()->CopyFrom(pRoleObj->GetRoleID());
	pstReq->set_ireqid(CModuleHelper::GetZoneID());
	pstReq->set_bneedresponse(ucNeedResponse);

	GameUserInfo* pstUserInfo = pstReq->mutable_stuserinfo();
	pstUserInfo->set_uin(uiUin);
	pstUserInfo->set_uiseq(pRoleObj->GetRoleID().uiseq());

	//更新玩家的基本信息
	BASEDBINFO stBaseInfo;
	pRoleObj->UpdateBaseInfoToDB(stBaseInfo);
	if (!EncodeProtoData(stBaseInfo, *pstUserInfo->mutable_strbaseinfo()))
	{
		LOGERROR("Failed to encode base proto data, uin %u\n", uiUin);
		return -30;
	}

	LOGDEBUG("Compress base proto data rate %d:%zu, uin %u\n", stBaseInfo.ByteSize(), pstUserInfo->strbaseinfo().size(), uiUin);

	//更新玩家的任务数据
	QUESTDBINFO stQuestInfo;
	pRoleObj->UpdateQuestToDB(stQuestInfo);
	if (!EncodeProtoData(stQuestInfo, *pstUserInfo->mutable_strquestinfo()))
	{
		LOGERROR("Failed to encode quest proto data, uin %u\n", uiUin);
		return -31;
	}

	LOGDEBUG("Compress quest proto data rate %d:%zu, uin %u\n", stQuestInfo.ByteSize(), pstUserInfo->strquestinfo().size(), uiUin);

	//更新玩家的物品数据
	ITEMDBINFO stItemInfo;
	pRoleObj->UpdateRepThingsToDB(stItemInfo);
	if (!EncodeProtoData(stItemInfo, *pstUserInfo->mutable_striteminfo()))
	{
		LOGERROR("Failed to encode item proto data, uin %u\n", uiUin);
		return -32;
	}

	LOGDEBUG("Compress item proto data rate %d:%zu, uin %u\n", stItemInfo.ByteSize(), pstUserInfo->striteminfo().size(), uiUin);

	//更新玩家的好友信息
	FRIENDDBINFO stFriend;
	if (!EncodeProtoData(stFriend, *pstUserInfo->mutable_strfriendinfo()))
	{
		LOGERROR("Failed to encode friend proto data, uin %u\n", uiUin);
		return -35;
	}

	LOGDEBUG("Compress friend proto data rate %d:%zu, uin %u\n", stFriend.ByteSize(), pstUserInfo->strfriendinfo().size(), uiUin);

	//更新玩家的邮件信息
	MAILDBINFO stMail;
	pRoleObj->UpdateMailToDB(stMail);
	if (!EncodeProtoData(stMail, *pstUserInfo->mutable_strmailinfo()))
	{
		LOGERROR("Failed to encode mail proto data, uin %u\n", uiUin);
		return -36;
	}

	LOGDEBUG("Compress mail proto data rate %d:%zu, uin %u\n", stMail.ByteSize(), pstUserInfo->strmailinfo().size(), uiUin);

	//更新Reserve1字段
	RESERVED1DBINFO stReserved1;
	if (!EncodeProtoData(stReserved1, *pstUserInfo->mutable_strreserved1()))
	{
		LOGERROR("Failed to encode reserve1 proto data, uin %u\n", uiUin);
		return -37;
	}

	LOGDEBUG("Compress reserve1 proto data rate %d:%zu, uin %u\n", stReserved1.ByteSize(), pstUserInfo->strreserved1().size(), uiUin);

	//更新Reserve2字段
	RESERVED2DBINFO stReserved2;

	//更新兑换信息
	pRoleObj->UpdateExchangeToDB(*stReserved2.mutable_stexchangeinfo());

	//更新充值信息
	pRoleObj->UpdateRechargeToDB(*stReserved2.mutable_strechargeinfo());

	if (!EncodeProtoData(stReserved2, *pstUserInfo->mutable_strreserved2()))
	{
		LOGERROR("Failed to encode reserve2 proto data, uin %u\n", uiUin);
		return -38;
	}

	LOGDEBUG("Compress reserve2 proto data rate %d:%zu, uin %u\n", stReserved2.ByteSize(), pstUserInfo->strreserved2().size(), uiUin);

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// 发送到World服务器
	int iRet = CZoneMsgHelper::SendZoneMsgToWorld(stMsg);
	if (iRet < 0)
	{
		return iRet;
	}

	return 0;
}
