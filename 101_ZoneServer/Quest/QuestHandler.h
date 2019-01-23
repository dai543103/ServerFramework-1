#pragma once

#include "CommDefine.h"
#include "Kernel/Handler.hpp"
#include "QuestManager.h"

class CGameRoleObj;
class CQuestHandler : public IHandler
{
public:
	virtual ~CQuestHandler();

	virtual int OnClientMsg();

private:

	//玩家完成任务
	int OnRequestFinQuest();

	//玩家领取活跃度奖励
	int OnRequestGetLivnessReward();

	//发送回复
	int SendResponse(unsigned uiMsgID, const unsigned int uiResultID, const TNetHead_V2& rstNetHead);

protected:

	static GameProtocolMsg ms_stZoneMsg;
};
