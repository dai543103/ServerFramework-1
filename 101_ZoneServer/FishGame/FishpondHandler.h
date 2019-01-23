#pragma once

#include "GameProtocol.hpp"
#include "Kernel/Handler.hpp"
#include "CommDefine.h"
#include "ErrorNumDef.hpp"

class CGameRoleObj;
class CFishpondHandler : public IHandler
{
public:
	virtual ~CFishpondHandler(void);
	CFishpondHandler();
public:
	int OnClientMsg();

private:

	//进入鱼池消息处理
	int OnRequestDoFish();

	//退出鱼池消息处理
	int OnRequestExitFish();

	//玩家切换炮台
	int OnRequestChangeGun();

	//玩家发射子弹
	int OnRequestShootBullet();

	//玩家命中鱼
	int OnRequestHitFish();

	//玩家命中鱼阵鱼
	int OnRequestHitFormFish();

	//玩家使用技能
	int OnRequestUseSkill();

	//玩家选择瞄准鱼
	int OnRequestChooseAimFish();
};