#include <string.h>

#include "ProtoDataUtility.hpp"
#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "ModuleHelper.hpp"
#include "WorldMsgHelper.hpp"
#include "ErrorNumDef.hpp"

#include "FetchRoleWorldHandler.hpp"

using namespace ServerLib;

CFetchRoleWorldHandler::~CFetchRoleWorldHandler()
{
}

int CFetchRoleWorldHandler::OnClientMsg(GameProtocolMsg* pMsg)
{
	ASSERT_AND_LOG_RTN_INT(pMsg);

	m_pMsg = (GameProtocolMsg*)pMsg;

	switch (m_pMsg->sthead().uimsgid())
	{
	case MSGID_WORLD_FETCHROLE_REQUEST:
	{
		OnRequestFetchRoleWorld();
	}
	break;

	case MSGID_WORLD_FETCHROLE_RESPONSE:
	{
		OnResponseFetchRoleWorld();
	}
	break;

	case MSGID_WORLD_CREATEROLE_RESPONSE:
	{
		//创建角色返回的处理入口
		OnResponseCreateRoleWorld();
	}
	break;

	default:
	{
		break;
	}
	}

	return 0;
}

int CFetchRoleWorldHandler::OnRequestFetchRoleWorld()
{
    const World_FetchRole_Request& rstRequest = m_pMsg->stbody().m_stworld_fetchrole_request();

    unsigned uiUin = rstRequest.stroleid().uin();
	if (uiUin == 0)
	{
		SendFailedFetchRoleResponse(rstRequest.stroleid(), rstRequest.ireqid());
		return -1;
	}
    
    LOGDEBUG("FetchRoleWorld: Uin = %d\n", uiUin);

    CWorldRoleStatusWObj* pWorldRoleStatusObj = WorldTypeK32<CWorldRoleStatusWObj>::GetByRoleID(rstRequest.stroleid());
    // 如果角色对象已经存在, 并且是登录请求, 则返回失败
    if (pWorldRoleStatusObj && rstRequest.bislogin())
    {
        LOGERROR("Role Already Login: Uin = %d.\n", uiUin);
        SendFailedFetchRoleResponse(rstRequest.stroleid(), rstRequest.ireqid());
        return -2;
    }

    // 如果不是登录请求，并且要查询的角色不存在，则返回失败
    if (!pWorldRoleStatusObj && !rstRequest.bislogin())
    {
        LOGERROR("Role Non-Exists: Uin = %u.\n", uiUin);
        SendFailedFetchRoleResponse(rstRequest.stroleid(), rstRequest.ireqid());
        return -3;
    }
    
    // 如果是登录或者读取存档请求, 创建角色对象并查询角色信息
    if (rstRequest.bislogin())
    {
        //登录请求需要重新创建缓存Obj
        pWorldRoleStatusObj = WorldTypeK32<CWorldRoleStatusWObj>::CreateByUin(uiUin);
        if (!pWorldRoleStatusObj)
        {
            LOGERROR("CreateRoleStatusObj Failed: Uin = %u.\n", uiUin);
            SendFailedFetchRoleResponse(rstRequest.stroleid(), rstRequest.ireqid());
            return -4;
        }

        pWorldRoleStatusObj->SetZoneID(rstRequest.ireqid());
        pWorldRoleStatusObj->SetUin(uiUin);

        // 从数据库中拉取数据
        int iRet = CWorldMsgHelper::SendWorldMsgToRoleDB(*m_pMsg);
        if (iRet < 0)
        {
            LOGERROR("SendFetchRoleDB Failed: Uin = %d, iRet = %d\n", uiUin, iRet);

            SendFailedFetchRoleResponse(rstRequest.stroleid(), rstRequest.ireqid());
            WorldTypeK32<CWorldRoleStatusWObj>::DeleteByUin(uiUin);
            return -6;
        }

        return 0;
    }
    
    // 否则直接返回角色数据
    SendFetchRoleResponse(rstRequest.stroleid(), rstRequest.ireqid(), rstRequest.stkicker());

	return 0;
}

//查询请求返回到World的处理入口
int CFetchRoleWorldHandler::OnResponseFetchRoleWorld()
{
    const World_FetchRole_Response& stResp = m_pMsg->stbody().m_stworld_fetchrole_response();
    unsigned uiUin = stResp.stroleid().uin();

    //查找World缓存的本地玩家数据
    CWorldRoleStatusWObj* pUserStatusObj = WorldTypeK32<CWorldRoleStatusWObj>::GetByRoleID(stResp.stroleid());
    if(!pUserStatusObj)
    {
        LOGERROR("World No Cache Data: Uin = %u\n", uiUin);
        return -1;
    }

	int iZoneID = pUserStatusObj->GetZoneID();
	
	//如果从DB下拉数据失败
	if (stResp.iresult())
	{
		LOGERROR("Fail to fetch role from DB, uin %u, ret 0x%0x\n", uiUin, stResp.iresult());

		WorldTypeK32<CWorldRoleStatusWObj>::DeleteByUin(uiUin);
		SendFailedFetchRoleResponse(stResp.stroleid(), iZoneID);
		return -2;
	}

	//如果不是登录并且角色不存在
	if (!stResp.bislogin() && !stResp.bexist())
	{
		LOGERROR("Fail to fetch role from DB, uin %u, ret 0x%0x\n", uiUin, stResp.iresult());

		WorldTypeK32<CWorldRoleStatusWObj>::DeleteByUin(uiUin);
		SendFailedFetchRoleResponse(stResp.stroleid(), iZoneID);
		return -3;
	}

	if (!stResp.bexist())
	{
		//角色不存在，走创建流程
		int iRet = SendCreateRoleRequestToDBSvr(stResp.stkicker());
		if (iRet)
		{
			//发送创建角色消息失败
			LOGERROR("Failed to create role, ret %d, uin %u\n", iRet, stResp.stroleid().uin());

			WorldTypeK32<CWorldRoleStatusWObj>::DeleteByUin(uiUin);
			SendFailedFetchRoleResponse(stResp.stroleid(), iZoneID);
			return -4;
		}

		pUserStatusObj->GetRoleInfo().CopyFrom(m_stWorldMsg.stbody().m_stworld_createrole_request().stbirthdata());
		return 0;
	}
	else
	{
		//角色存在，走拉取成功流程
		int iRet = OnFetchSuccess(stResp.stroleid(), stResp.stuserinfo(), stResp.bislogin(), stResp.stkicker());
		if (iRet)
		{
			LOGERROR("Failed to fetch role, uin %u, ret %d\n", uiUin, iRet);

			WorldTypeK32<CWorldRoleStatusWObj>::DeleteByUin(uiUin);
			SendFailedFetchRoleResponse(stResp.stroleid(), iZoneID);
			return -5;
		}
	}

    return 0;
}

//创建角色返回的处理入口
int CFetchRoleWorldHandler::OnResponseCreateRoleWorld()
{
	const World_CreateRole_Response& stResp = m_pMsg->stbody().m_stworld_createrole_response();
	unsigned uiUin = stResp.stroleid().uin();

	//查找World缓存的本地玩家数据
	CWorldRoleStatusWObj* pUserStatusObj = WorldTypeK32<CWorldRoleStatusWObj>::GetByRoleID(stResp.stroleid());
	if (!pUserStatusObj)
	{
		LOGERROR("World No Cache Data: Uin = %u\n", uiUin);
		return -1;
	}

	int iZoneID = pUserStatusObj->GetZoneID();
	if (stResp.iresult() != 0)
	{
		//创建失败
		LOGERROR("Failed to create role, uin %u, zone id %d\n", uiUin, iZoneID);

		WorldTypeK32<CWorldRoleStatusWObj>::DeleteByUin(uiUin);
		SendFailedFetchRoleResponse(stResp.stroleid(), iZoneID);
		return -2;
	}

	//创建成功
	int iRet = OnFetchSuccess(stResp.stroleid(), pUserStatusObj->GetRoleInfo(), true, stResp.stkicker());
	if (iRet)
	{
		LOGERROR("Failed to fetch role, uin %u, ret %d\n", uiUin, iRet);

		WorldTypeK32<CWorldRoleStatusWObj>::DeleteByUin(uiUin);
		SendFailedFetchRoleResponse(stResp.stroleid(), iZoneID);
		return iRet;
	}

	return 0;
}

//发送创建角色的请求
int CFetchRoleWorldHandler::SendCreateRoleRequestToDBSvr(const KickerInfo& stKicker)
{
	//生成消息头
	CWorldMsgHelper::GenerateMsgHead(m_stWorldMsg, m_pMsg->sthead().uisessionfd(),
		MSGID_WORLD_CREATEROLE_REQUEST, m_pMsg->sthead().uin());

	//生成创建角色帐号的消息体
	World_CreateRole_Request* pstRequest = m_stWorldMsg.mutable_stbody()->mutable_m_stworld_createrole_request();

	int iRet = InitRoleBirthInfo(*pstRequest, stKicker);
	if (iRet)
	{
		LOGERROR("Fail to init role birth info, uin %u, ret 0x%0x\n", m_pMsg->sthead().uin(), iRet);
		return iRet;
	}

	iRet = CWorldMsgHelper::SendWorldMsgToRoleDB(m_stWorldMsg);

	return iRet;
}

//初始化玩家信息
int CFetchRoleWorldHandler::InitRoleBirthInfo(World_CreateRole_Request& rstRequest, const KickerInfo& stKicker)
{
	rstRequest.set_uin(m_pMsg->sthead().uin());
	rstRequest.set_world(CModuleHelper::GetWorldID());
	rstRequest.set_irealnamestat(stKicker.irealnamestat());
	rstRequest.mutable_stkicker()->CopyFrom(stKicker);

	//初始化玩家基础属性
	unsigned int uTimeNow = time(NULL);

	BaseConfigManager& stBaseCfgMgr = CConfigManager::Instance()->GetBaseCfgMgr();

	BASEDBINFO stBaseInfo;
	stBaseInfo.set_sznickname(stKicker.strnickname());
	stBaseInfo.set_strchannel(stKicker.strchannel());
	stBaseInfo.set_icreatetime(uTimeNow);

	//初始化资源
	for (int i = RESOURCE_TYPE_INVALID; i<RESOURCE_TYPE_MAX; ++i)
	{
		stBaseInfo.add_iresources(0);
	}
	stBaseInfo.set_iresources(RESOURCE_TYPE_COIN, stBaseCfgMgr.GetBirthInitConfig(BIRTH_INIT_COIN));
	stBaseInfo.set_luserwinnum(stBaseInfo.iresources(RESOURCE_TYPE_COIN));

	//初始化炮台和样式
	stBaseInfo.mutable_stweapon()->set_iweaponid(stBaseCfgMgr.GetBirthInitConfig(BIRTH_INIT_WEAPON));
	stBaseInfo.mutable_stweapon()->set_iweaponstyleid(stBaseCfgMgr.GetBirthInitConfig(BIRTH_INIT_WEAPONSTYLE));
	stBaseInfo.mutable_stweapon()->add_iunlockstyleids(stBaseCfgMgr.GetBirthInitConfig(BIRTH_INIT_WEAPONSTYLE));

	//初始化头像
	char szPicID[32] = { 0 };
	SAFE_SPRINTF(szPicID, sizeof(szPicID)-1, "%d", stBaseCfgMgr.GetBirthInitConfig(BIRTH_INIT_PICID));
	stBaseInfo.set_strpicid(szPicID);

	//初始化VIP等级和抽奖
	stBaseInfo.set_iviplevel(stBaseCfgMgr.GetBirthInitConfig(BIRTH_INIT_VIPLEVEL));
	stBaseInfo.set_lvipexp(0);

	//读取VIP配置
	const VipLevelConfig* pstVIPConfig = stBaseCfgMgr.GetVipConfig(stBaseInfo.iviplevel());
	if (!pstVIPConfig)
	{
		LOGERROR("Failed to get vip config, invalid level %d\n", stBaseInfo.iviplevel());
		return -1;
	}

	int iVIPPriv = 0;
	for (unsigned i = 0; i < pstVIPConfig->vPrivs.size(); ++i)
	{
		iVIPPriv |= pstVIPConfig->vPrivs[i].iPrivType;
	}
	stBaseInfo.set_ivippriv(iVIPPriv);

	stBaseInfo.set_ilotterynum(0);

	//初始化体验线数据
	for (int i = 0; i < MAX_ROOM_ALGORITHM_TYPE; ++i)
	{
		stBaseInfo.add_stexplines()->set_iexplinetype(stBaseCfgMgr.GetBirthInitConfig(BIRTH_INIT_EXPTYPE));
	}

	//初始化新手红包
	stBaseInfo.set_inownewrednum(stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_NEWREDNUM)/stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_NEWREDTIMES));
	stBaseInfo.set_iremainnewrednum(stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_NEWREDNUM)-stBaseInfo.inownewrednum());

	//初始化玩家的任务信息
	QUESTDBINFO stQuestInfo;

	//从配置初始化任务
	const std::vector<QuestConfig>& stAllQuests = stBaseCfgMgr.GetQuestConfig();
	for (unsigned i = 0; i < stAllQuests.size(); ++i)
	{
		switch (stAllQuests[i].iType)
		{
		case QUEST_TYPE_NEW:
		case QUEST_TYPE_DAILY:
		{
			OneQuest* pstOneQuestInfo = stQuestInfo.add_stquestinfos();
			pstOneQuestInfo->set_iquestid(stAllQuests[i].iID);
			pstOneQuestInfo->set_iquesttype(stAllQuests[i].iType);
			pstOneQuestInfo->set_ineedtype(stAllQuests[i].iNeedType);
			pstOneQuestInfo->set_inum(0);
			pstOneQuestInfo->set_bisfin(false);
		}
		break;

		case QUEST_TYPE_ACHIEVE:
		{
			//只增加第一阶段
			if (stAllQuests[i].iQuestIndex == 1)
			{
				//起始成就任务
				OneQuest* pstOneQuestInfo = stQuestInfo.add_stquestinfos();
				pstOneQuestInfo->set_iquestid(stAllQuests[i].iID);
				pstOneQuestInfo->set_iquesttype(stAllQuests[i].iType);
				pstOneQuestInfo->set_ineedtype(stAllQuests[i].iNeedType);
				pstOneQuestInfo->set_inum(0);
				pstOneQuestInfo->set_bisfin(false);
			}
		}
		break;

		case QUEST_TYPE_ADVENTURE:
		{

		}
		break;

		default:
			break;
		}
	}

	int iNextDayNowTime = CTimeUtility::GetNowTime() + 24 * 60 * 60;
	stQuestInfo.set_iadventnextupdatetime(CTimeUtility::GetTodayTime(iNextDayNowTime, stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_ADVENTRESETTIME)));
	stQuestInfo.set_idailynextupdatetime(CTimeUtility::GetTodayTime(iNextDayNowTime, stBaseCfgMgr.GetGlobalConfig(GLOBAL_TYPE_DAILYRESETTIME)));

	//初始化玩家的背包信息
	ITEMDBINFO stItemInfo;

	//增加月卡体验道具
	OneSlotInfo* pstItemInfo = stItemInfo.mutable_stitemslot()->add_stslots();
	pstItemInfo->set_iitemid(stBaseCfgMgr.GetBirthInitConfig(BIRTH_INIT_MONTHCARD));
	pstItemInfo->set_iitemnum(1);

	//初始化玩家的好友信息
	FRIENDDBINFO stFriendInfo;

	//初始化玩家的邮件信息
	MAILDBINFO stMailInfo;

	//初始化保留字段1
	RESERVED1DBINFO stReserved1;

	//初始化保留字段2
	RESERVED2DBINFO stReserved2;

	//1.将玩家基础信息编码到请求中
	if (!EncodeProtoData(stBaseInfo, *rstRequest.mutable_stbirthdata()->mutable_strbaseinfo()))
	{
		LOGERROR("Failed to encode base proto data, uin %u!\n", rstRequest.uin());
		return -1;
	}

	LOGDEBUG("Base proto Info compress rate %d:%zu, uin %u\n", stBaseInfo.ByteSize(), rstRequest.stbirthdata().strbaseinfo().size(), rstRequest.uin());

	//2.将玩家任务信息编码到请求中
	if (!EncodeProtoData(stQuestInfo, *rstRequest.mutable_stbirthdata()->mutable_strquestinfo()))
	{
		LOGERROR("Failed to encode quest proto data, uin %u!\n", rstRequest.uin());
		return -2;
	}

	LOGDEBUG("quest proto Info compress rate %d:%zu, uin %u\n", stQuestInfo.ByteSize(), rstRequest.stbirthdata().strquestinfo().size(), rstRequest.uin());

	//3.将玩家物品信息编码到请求中
	if (!EncodeProtoData(stItemInfo, *rstRequest.mutable_stbirthdata()->mutable_striteminfo()))
	{
		LOGERROR("Failed to encode item proto data, uin %u!\n", rstRequest.uin());
		return -3;
	}

	LOGDEBUG("item proto Info compress rate %d:%zu, uin %u\n", stItemInfo.ByteSize(), rstRequest.stbirthdata().striteminfo().size(), rstRequest.uin());

	//4.将玩家好友信息信息编码到请求中
	if (!EncodeProtoData(stFriendInfo, *rstRequest.mutable_stbirthdata()->mutable_strfriendinfo()))
	{
		LOGERROR("Failed to encode friend proto data, uin %u!\n", rstRequest.uin());
		return -4;
	}

	LOGDEBUG("friend proto Info compress rate %d:%zu, uin %u\n", stFriendInfo.ByteSize(), rstRequest.stbirthdata().strfriendinfo().size(), rstRequest.uin());

	//5.将玩家邮件信息编码到请求中
	if (!EncodeProtoData(stMailInfo, *rstRequest.mutable_stbirthdata()->mutable_strmailinfo()))
	{
		LOGERROR("Failed to encode mail proto data, uin %u!\n", rstRequest.uin());
		return -4;
	}

	LOGDEBUG("mail proto Info compress rate %d:%zu, uin %u\n", stMailInfo.ByteSize(), rstRequest.stbirthdata().strmailinfo().size(), rstRequest.uin());

	//6.将玩家Reserved1字段编码到请求中
	if (!EncodeProtoData(stReserved1, *rstRequest.mutable_stbirthdata()->mutable_strreserved1()))
	{
		LOGERROR("Failed to encode reserved1 proto data, uin %u!\n", rstRequest.uin());
		return -6;
	}

	LOGDEBUG("reserved1 proto Info compress rate %d:%zu, uin %u\n", stReserved1.ByteSize(), rstRequest.stbirthdata().strreserved1().size(), rstRequest.uin());

	//7.将玩家Reserved2字段编码到请求中
	if (!EncodeProtoData(stReserved2, *rstRequest.mutable_stbirthdata()->mutable_strreserved2()))
	{
		LOGERROR("Failed to encode reserved2 proto data, uin %u!\n", rstRequest.uin());
		return -7;
	}

	LOGDEBUG("reserved2 proto Info compress rate %d:%zu, uin %u\n", stReserved2.ByteSize(), rstRequest.stbirthdata().strreserved2().size(), rstRequest.uin());

	return 0;
}

//创建成功
int CFetchRoleWorldHandler::OnFetchSuccess(const RoleID& stRoleID, const GameUserInfo& stUserInfo, bool bIsLogin, const KickerInfo& stKicker)
{
	unsigned uiUin = stRoleID.uin();

	//查找World缓存的本地玩家数据
	CWorldRoleStatusWObj* pUserStatusObj = WorldTypeK32<CWorldRoleStatusWObj>::GetByRoleID(stRoleID);
	if (!pUserStatusObj)
	{
		LOGERROR("World No Cache Data: Uin = %u\n", uiUin);
		return -1;
	}

	int iZoneID = pUserStatusObj->GetZoneID();

	//如果是登录，验证玩家的不在线状态
	if (bIsLogin && (pUserStatusObj->GetRoleStatus() & EGUS_ONLINE))
	{
		LOGERROR("Fail to fetch role from DB, already online, uin %u\n", uiUin);
		return -2;
	}

	LOGDEBUG("Fetch ROLEDB OK, uin %u, from zone id %d\n", uiUin, iZoneID);

	//更新World的玩家缓存数据信息
	BASEDBINFO stBaseInfo;
	if (!DecodeProtoData(stUserInfo.strbaseinfo(), stBaseInfo))
	{
		LOGERROR("Failed to decode base proto data, uin %u\n", uiUin);
		return -3;
	}

	pUserStatusObj->GetRoleInfo().CopyFrom(stUserInfo);
	pUserStatusObj->SetRoleStatus(stBaseInfo.ustatus() | EGUS_ONLINE);

	//返回客户端查询结果
	CWorldMsgHelper::GenerateMsgHead(m_stWorldMsg, 0, MSGID_WORLD_FETCHROLE_RESPONSE, stRoleID.uin());

	World_FetchRole_Response* pstResp = m_stWorldMsg.mutable_stbody()->mutable_m_stworld_fetchrole_response();
	pstResp->set_iresult(0);
	pstResp->mutable_stroleid()->CopyFrom(stRoleID);
	pstResp->set_bexist(true);
	pstResp->set_bislogin(bIsLogin);
	pstResp->mutable_stkicker()->CopyFrom(stKicker);
	pstResp->mutable_stuserinfo()->CopyFrom(stUserInfo);

	int iRet = CWorldMsgHelper::SendWorldMsgToWGS(m_stWorldMsg, iZoneID);
	if (iRet)
	{
		LOGERROR("SendFetchRoleResponse to GS failed, uin %u, zone id %d\n", uiUin, iZoneID);
		return iRet;
	}

	return 0;
}

// 返回失败信息
int CFetchRoleWorldHandler::SendFailedFetchRoleResponse(const RoleID& stRoleID, int iReqID)
{
    CWorldMsgHelper::GenerateMsgHead(m_stWorldMsg, 0, MSGID_WORLD_FETCHROLE_RESPONSE, stRoleID.uin());
    
    World_FetchRole_Response* rstResp = m_stWorldMsg.mutable_stbody()->mutable_m_stworld_fetchrole_response();
    rstResp->mutable_stroleid()->CopyFrom(stRoleID);
    rstResp->set_iresult(T_WORLD_FETCHROLE_FAIED);
	
    int iRet = CWorldMsgHelper::SendWorldMsgToWGS(m_stWorldMsg, iReqID);

	return iRet;
}

// 返回角色数据
int CFetchRoleWorldHandler::SendFetchRoleResponse(const RoleID& stRoleID, int iReqID, const KickerInfo& stKicker)
{
    CWorldRoleStatusWObj* pWorldRoleStatusObj = WorldTypeK32<CWorldRoleStatusWObj>::GetByRoleID(stRoleID);
    if (!pWorldRoleStatusObj)
    {
        return -1;
    }

    CWorldMsgHelper::GenerateMsgHead(m_stWorldMsg, 0, MSGID_WORLD_FETCHROLE_RESPONSE, stRoleID.uin());

    World_FetchRole_Response* pstResp = m_stWorldMsg.mutable_stbody()->mutable_m_stworld_fetchrole_response();
    pstResp->set_iresult(T_SERVER_SUCCESS);
    pstResp->mutable_stroleid()->CopyFrom(stRoleID);

    pstResp->mutable_stuserinfo()->CopyFrom(pWorldRoleStatusObj->GetRoleInfo());
	pstResp->mutable_stkicker()->CopyFrom(stKicker);

    int iRet = CWorldMsgHelper::SendWorldMsgToWGS(m_stWorldMsg, iReqID);

    return iRet;
}

