#ifndef __UPDATE_ROLEINFO_HANDLER__HPP__
#define __UPDATE_ROLEINFO_HANDLER__HPP__

#include <assert.h>
#include <string.h>

#include "Kernel/Handler.hpp"
#include "ErrorNumDef.hpp"
#include "Kernel/GameRole.hpp"


using namespace ServerLib;

class CUpdateRoleInfo_Handler: public IHandler
{
public:
	virtual ~CUpdateRoleInfo_Handler();

public:
	virtual int OnClientMsg();

public:
	// 同步玩家数据到数据库
    static int UpdateRoleInfo(CGameRoleObj* pRoleObj, unsigned char ucNeedResponse);

	//结算玩家信息
	static void TallyRoleData(CGameRoleObj& stRoleObj);

	int OnUpdateRoleInfoResponse();
};

#endif
