#ifndef __HANDLER_LIST_HPP__
#define __HANDLER_LIST_HPP__

#include "Login/LoginHandler.hpp"
#include "Login/LogoutHandler.hpp"
#include "FetchRoleWorldHandler.hpp"
#include "Kernel/UpdateRoleInfoHandler.hpp"
#include "Login/KickRoleWorldHandler.hpp"
#include "GM/GameMasterHandler.hpp"
#include "RepThings/RepThingsHandler.hpp"
#include "Chat/ChatHandler.hpp"
#include "FishGame/FishpondHandler.h"
#include "Lottery/LotteryHandler.h"
#include "Exchange/ExchangeHandler.h"
#include "Quest/QuestHandler.h"
#include "Rank/RankInfoHandler.h"
#include "Mail/MailHandler.h"
#include "Vip/VipPrivHandler.h"
#include "Reward/RewardHandler.h"
#include "Recharge/RechargeHandler.h"
#include "NewGuide/NewGuideHandler.h"

class CHandlerList
{
public:
	int RegisterAllHandler();

private:
	// 所有的消息处理函数
	CLoginHandler m_stLoginHandler;
	CLogoutHandler m_stLogoutHandler;
    CFetchRoleWorldHandler m_stFetchRoleWorldHandler;
	CUpdateRoleInfo_Handler m_stUpdateRoleInfoHandler;
    CKickRoleWorldHandler m_stKickRoleWorldHandler;
    CGameMasterHandler m_stGMCommandHandler;
    CRepThingsHandler m_stRepThingsHandler;
    CChatHandler m_stChatHandler;
	CFishpondHandler m_stFishpondHandler;
	CLotteryHandler m_stLotteryHandler;
	CQuestHandler m_stQuestHandler;
	CExchangeHandler m_stExchangeHandler;
	CRankInfoHandler m_stRankInfoHandler;
	CMailHandler m_stMailHandler;
	CVipPrivHandler m_stVipPrivHandler;
	CRewardHandler m_stRewardHandler;
	CRechargeHandler m_stRechargeHandler;
	CNewGuideHandler m_stNewGuideHandler;
};

#endif
