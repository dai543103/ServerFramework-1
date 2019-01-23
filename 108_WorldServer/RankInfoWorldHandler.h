#pragma once

#include "Handler.hpp"
#include "AppDefW.hpp"

class CRankInfoWorldHandler : public IHandler
{
public:
	virtual ~CRankInfoWorldHandler();

public:

	virtual int OnClientMsg(GameProtocolMsg* pMsg);

private:

	//更新排行榜
	int OnRequestUpdateRank();

	//拉取排行信息
	int OnRequestGetWorldRank();

private:
	GameProtocolMsg* m_pRequestMsg;
};