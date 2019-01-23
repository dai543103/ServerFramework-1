#pragma once

#include <vector>
#include "ObjAllocator.hpp"
#include "Kernel/ConfigManager.hpp"

using namespace ServerLib;

const unsigned int MAX_PLAYER = 4;				//单个鱼池最大玩家数量
const unsigned int MAX_BULLET_PER_SECOND = 4;	//1s内玩家发射子弹上限
const unsigned int MAX_PLAYER_BULLET_NUM = 20;	//单个玩家子弹最大数量
const unsigned int MAX_FORM_FISH_NUM = 200;		//单个鱼阵轨迹最大出鱼数量
const unsigned int MAX_FISH_NUM = 30;			//鱼池中最大支持的鱼数量
const unsigned int MAX_FORM_FISHKILL_NUM = 200;	//鱼阵单条轨迹最大可捕数量

enum CycleTimeType
{
	CYCLE_TIME_LAST = 0,		//上一个结算周期
	CYCLE_TIME_NOW	= 1,		//当前结算周期
	CYCLE_TIME_MAX	= 2,		//结算周期最大
};

//鱼的类型
enum FishType
{
	FISH_TYPE_SMALLFISH		= 0,	//小鱼
	FISH_TYPE_MIDDLEFISH	= 1,	//中型鱼
	FISH_TYPE_BIGFISH		= 2,	//大型鱼
	FISH_TYPE_BIGBOSS		= 3,	//大BOSS鱼
	FISH_TYPE_PHONETICKET	= 4,	//鱼票鱼
	FISH_TYPE_WARHEAD		= 5,	//弹头鱼
	FISH_TYPE_MULTIPLE		= 6,	//多倍鱼
	FISH_TYPE_SMALLGROUP	= 9,	//小鱼组
	FISH_TYPE_FLASHFISH		= 10,	//电鳗
	FISH_TYPE_BOOMFISH		= 11,	//炸弹鱼
};

//座位上玩家数据
struct SeatUserData
{
	int iSeat;					//座位号
	unsigned uiUin;				//玩家uin
	int iActiveTime;			//最近活跃时间

	long lAimEndTime;			//瞄准结束时间
	unsigned uAimFishUniqID;	//瞄准的鱼唯一ID
	int iAimFishIndex;			//如果瞄准小鱼组，小鱼的index
	bool bAutoShoot;			//是否使用自动发炮
	long lWildEndTime;			//狂暴结束时间
	int iWildNum;				//狂暴次数

	//玩家发射子弹信息，数量和射速有关
	int iIndex;
	long alShootTime[MAX_PLAYER_BULLET_NUM];	//发射子弹的时间
	int iBulletNum;				//当前鱼池中玩家的子弹数目

	//玩家当前收益和支出
	long8 lUserSilverCost;			//玩家的总支出
	long8 lUserTicketSilverCost;	//玩家点券鱼支出
	long8 lUserSilverReward;		//玩家的总收益
	long8 lUserFlow[CYCLE_TIME_MAX];//当前周期玩家发炮流水
	int iLossNum;					//当前周期玩家亏损，负表示亏损，正表示盈利

	//玩家的结算周期ID
	unsigned uCycleID;		//玩家结算周期ID
	int iCycleWinNum;		//玩家结算周期内输赢
	long8 lCycleNewUsedNum;	//玩家结算周期新手红包使用金额

	//红包信息
	unsigned uReturnID;		//获取红包的ID
	int iReturnType;		//获取红包的类型
	int iEffectCountdown;	//生效倒计时,相对时间, ms
	long lUnEffectTime;		//红包失效时间，绝对时间, ms

	SeatUserData()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

//出鱼规则
struct OutFishRule
{
	int iFishSeqID;  //出鱼鱼组ID
	long lOutTime;   //出鱼的时间,单位ms
	int iDeltaTime;	//出鱼时间和当前时间差值，单位ms
	int iFishID;    //出鱼的ID
	int iPower;		//出鱼权重
	int iLimitType;	//出鱼数量限制类型
	int iType;		//出鱼的类型

	OutFishRule()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

//子弹数据结构
struct BulletData
{
	unsigned uUniqueID;		//子弹的唯一ID
	int iGunID;				//发射子弹的炮台ID
	int iSeat;				//发射子弹的位置
	FISHPOS stTargetPos;	//发射子弹的目标位置
	long lShootTime;		//子弹发射时间,单位 ms
	unsigned uFishUniqID;	//瞄准目标鱼唯一ID，为0表示非瞄准子弹
	bool bIsAimFormFish;	//如果是瞄准，是否瞄准鱼阵中的鱼
	int iFishIndex;			//如果瞄准小鱼组中的鱼，为小鱼的index，1-5
	int iWildNum;			//如果使用了狂暴，狂暴次数
	int iCost;				//发射该子弹的消耗

	BulletData()
	{
		memset(this, 0, sizeof(*this));
	}

	BulletData& operator=(const BulletData& stBullet)
	{
		if (this == &stBullet)
		{
			return *this;
		}

		this->uUniqueID = stBullet.uUniqueID;
		this->iGunID = stBullet.iGunID;
		this->iSeat = stBullet.iSeat;
		this->stTargetPos = stBullet.stTargetPos;
		this->lShootTime = stBullet.lShootTime;
		this->uFishUniqID = stBullet.uFishUniqID;
		this->bIsAimFormFish = stBullet.bIsAimFormFish;
		this->iFishIndex = stBullet.iFishIndex;
		this->iWildNum = stBullet.iWildNum;
		this->iCost = stBullet.iCost;

		return *this;
	}
};

//鱼冰冻信息
struct FreezeData
{
	unsigned uUniqID;		//鱼阵中鱼唯一ID
	long lFreezeBeginTime;	//冰封开始时间
	int iFreezeContTime;	//冰封持续时间
	int iTotalFreezeTime;	//该鱼总共冰冻时间
	bool bIsFormFish;		//是否鱼阵中的鱼

	FreezeData()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

//鱼阵出鱼信息
struct FormFishOutData
{
	int iOutID;			//出鱼唯一ID
	int iFishID;		//出鱼的ID
	int iFishSeqID;		//鱼所属序列ID
	int iTraceID;		//对应轨迹ID
	long lBeginTime;	//开始出鱼时间, ms
	int iRemainNum;		//出鱼条数
	int iInterval;		//出鱼时间间隔, ms
	int iMultiple;		//鱼阵出鱼的倍数
	int iType;			//出鱼的类型

	int aiCostSilver[MAX_PLAYER][MAX_FORM_FISH_NUM];	//玩家打鱼阵中鱼的消耗

	//鱼是否存活,鱼的index对应位表示鱼是否活着
	ulong8 szAliveFish[4];

	FormFishOutData()
	{
		memset(this, 0, sizeof(*this));
	}

	bool IsFishAlive(int iIndex)
	{
		return (szAliveFish[iIndex / 64] >> (iIndex % 64)) & (ulong8)0x01;
	}

	void SetFishAlive(int iIndex, bool bAlive)
	{
		if (bAlive)
		{
			szAliveFish[iIndex / 64] |= ((ulong8)0x01 << (iIndex % 64));
		}
		else
		{
			szAliveFish[iIndex / 64] &= ~((ulong8)0x01 << (iIndex % 64));
		}
	}

	int GetLiveFishNum()
	{
		int iFishNum = 0;
		for (unsigned i = 0; i < sizeof(szAliveFish) / sizeof(ulong8); ++i)
		{
			ulong8 n = szAliveFish[i];
			while (n != 0) {
				n = n & (n - 1);
				++iFishNum;
			}
		}

		return iFishNum;
	}
};

//鱼阵信息数据
struct FishFormData
{
	long lNextUpdateTime;	//下次出现鱼阵的时间
	int iFormTypeID;		//出的鱼阵的ID
	long lFormEndTime;		//鱼阵消失的时间
	bool bIsCleared;		//是否已经清场
	bool bIsInForm;			//是否在鱼阵中

	std::vector<FormFishOutData> vFishOutData;		//鱼阵出鱼的详细信息

	std::vector<FreezeData> vFormFreezeInfo;	//鱼阵中被冰冻鱼的信息

	FishFormData()
	{
		Reset();
	}

	void Reset()
	{
		lNextUpdateTime = 0;
		iFormTypeID = 0;
		lFormEndTime = 0;
		bIsInForm = false;
		bIsCleared = false;

		vFishOutData.clear();
		vFormFreezeInfo.clear();
	}
};

//鱼的数据结构
struct FishData
{
	unsigned uUniqueID;				//鱼的唯一ID
	int iFishID;					//鱼的ID
	int iFishSeqID;					//所属鱼组的ID
	int iTraceID;					//鱼的轨迹ID
	long lBornTime;					//出现时间,单位:ms
	long lDeadTime;					//死亡时间，达到则消失,单位ms
	int aiCostSilver[MAX_PLAYER];	//玩家在该鱼上的消耗
	long lFreezeBeginTime;			//冰冻开始时间
	int iFreezeContTime;			//冰冻持续时间
	int iTotalFreezeTime;			//总共被冰冻的时间
	int iType;						//鱼的类型
	int iMultiple;					//鱼的倍数
	USHORT cIndex;					//如果是小鱼组，小鱼组存活鱼的信息
	int iLimitType;					//出鱼上限类型

	FishData()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}

	FishData& operator=(const FishData& stFish)
	{
		if (this == &stFish)
		{
			return *this;
		}

		this->uUniqueID = stFish.uUniqueID;
		this->iFishID = stFish.iFishID;
		this->iFishSeqID = stFish.iFishSeqID;
		this->iTraceID = stFish.iTraceID;
		this->lBornTime = stFish.lBornTime;
		this->lDeadTime = stFish.lDeadTime;
		this->lFreezeBeginTime = stFish.lFreezeBeginTime;
		this->iFreezeContTime = stFish.iFreezeContTime;
		this->iTotalFreezeTime = stFish.iTotalFreezeTime;
		this->iType = stFish.iType;
		this->iMultiple = stFish.iMultiple;
		this->cIndex = stFish.cIndex;
		this->iLimitType = stFish.iLimitType;

		for (unsigned i = 0; i < MAX_PLAYER; ++i)
		{
			this->aiCostSilver[i] = stFish.aiCostSilver[i];
		}

		return *this;
	}
};

//出鱼限量信息
struct FishLimitData
{
	int iLimitMaxNum;	//能出鱼最大数量
	int iRemainNum;		//当前剩余能出鱼数量

	FishLimitData()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

//捕鱼鱼池对象
class CGameRoleObj;
class CFishpondObj : public CObj
{
public:
	CFishpondObj();
	virtual ~CFishpondObj();
	virtual int Initialize();
	virtual int Resume();
	DECLARE_DYN

public:

	//设置鱼池信息
	void SetTableInfo(unsigned uTableUniqID, int iFishRoomID, const FishRoomConfig& stConfig);

	//玩家进入鱼池
	int EnterFishpond(CGameRoleObj& stRoleObj);

	//玩家退出鱼池
	void ExitFishpond(CGameRoleObj& stRoleObj, bool bForceExit);

	//玩家切换炮台
	int ChangeGun(CGameRoleObj& stRoleObj, int iNewGunID, bool bIsStyle);

	//玩家发射子弹
	int ShootBullet(CGameRoleObj& stRoleObj, long lShootTime, int iPosX, int iPosY, bool bAutoShoot);

	//玩家命中鱼
	int HitFish(CGameRoleObj& stRoleObj, long lHitTime, unsigned uBulletUniqID, unsigned uFishUniqID, int iFishIndex);

	//命中鱼阵中的鱼
	int HitFormFish(CGameRoleObj& stRoleObj, long lHitTime, unsigned uBulletUniqID, int iFishOutID, int iFishIndex);

	//玩家使用技能
	int UseSkill(CGameRoleObj& stRoleObj, const Zone_UseSkill_Request& stReq);

	//玩家选择瞄准鱼
	int ChooseAimFish(CGameRoleObj& stRoleObj, unsigned uFishUniqID, int iFishIndex);

public:
	//获取桌子ID
	unsigned GetTableID();

	//获取桌子房间ID
	int GetFishRoomID();

	//获取玩家人数
	int GetPlayerNum();

	//获取鱼池配置
	const FishRoomConfig* GetRoomConfig();

	//定时器
	int OnTick(CGameRoleObj& stRoleObj);

	//推送捕鱼玩家信息更新
	void SendFishUserUpdate(unsigned uiUin, int iResType, long8 lAddNum, int iItemID, int iItemNum);

	//重置Fishpond
	void ResetFishpond();

private:
	//初始化鱼信息
	int InitFishInfo();

	//初始化出鱼信息
	int InitOutFishRule();

	//初始化鱼阵信息
	int InitFishFormRule(bool bIsFormEnd);

	//更新单个出鱼信息
	void UpdateOneOutFishRule(int iFishSeqID, OutFishRule& stFishRule, int iAddTime);

	//更新玩家信息
	void UpdateSeatUserInfo(CGameRoleObj& stRoleObj, long lTimeNow);

	//更新出鱼信息
	int UpdateOutFishRule(long lTimeNow);

	//更新鱼池中鱼的信息
	int UpdateFishInfo(long lTimeNow);

	//更新鱼阵信息
	int UpdateFishFormInfo(long lTimeNow);

	//更新玩家信息
	int UpdateFishUserInfo(long lTimeNow);

	//鱼池中加鱼
	int AddNewFish(int iFishSeqID, int iFishID, long lOutTime, int iTraceID = 0);

	long GetLastTraceBornTime(int iTraceID);

	//是否对应类型的房间
	bool IsRoomType(int iRoomType);

	//是否对应房间模式
	bool IsRoomPattern(int iRoomPattern);

	//玩家坐下
	int PlayerSitDown(CGameRoleObj& stRoleObj);

	//删除玩家所有子弹
	void ClearUserBullets(int iSeat);

	//获取玩家座位信息
	SeatUserData* GetSeatUserByUin(unsigned uiUin);

	//获取玩家子弹
	BulletData* GetBulletData(unsigned uBulletUniqID);

	//获取鱼的数据
	FishData* GetFishData(unsigned uUniqID);

	//获取鱼阵中鱼的信息
	FormFishOutData* GetFormFishInfo(int iFishOutID, int iFishIndex, FishData& stFishInfo);

	//删除鱼
	void DeleteFishData(unsigned uUniqID);

	//删除子弹
	void DeleteBulletData(unsigned uUniqID);

	//检查是否有效命中
	bool CheckIsValidHit(long lHitTime, BulletData& stBulletInfo, FishData& stFishInfo, int iFishIndex = 0);

	//检查逻辑是否命中
	bool CheckIsLogicHit(CGameRoleObj& stRoleObj, SeatUserData& stUserData, const GunConfig& stGunConfig, FishData& stFishInfo, int iAdjust, bool bIsForm);

	//获取玩家概率状态
	int GetUserStatus(SeatUserData& stUserData);

	//获取玩家体验线
	TEXPLINEINFO* GetRoleExpLineInfo(unsigned uiUin, const ExpLineConfig*& pstConfig);

	//处理电鳗和炸弹鱼逻辑
	void BoomFish(SeatUserData& stUserData, FishData*& pstFishInfo, bool bIsFormFish);

	//鱼是否在屏幕中
	bool IsFishInScreen(FishData& stFishInfo, long lTimeNow);

	//掉落点券
	bool AddUserTicket(CGameRoleObj& stRoleObj, unsigned uFishUniqID, bool bIsFormFish, int iMultiple, int iNum, bool bIsTicketFish = false);

	//更新红包信息
	void UpdateRedPacketInfo(CGameRoleObj& stRoleObj, SeatUserData& stUserData);

	//发送走马灯
	void SendFishHorseLamp(CGameRoleObj& stRoleObj, int iLampID, int iParam1, int iParam2, int iParam3);

	//重置鱼池
	void Reset();

private:

	//发送鱼阵时间
	void SendFishFormTime(CGameRoleObj* pstRoleObj, bool bIsBegin);

	//推送玩家座位信息
	void SendSeatUserInfo(CGameRoleObj* pstToRole, CGameRoleObj* pstSeatRole);

	//推送鱼信息
	void SendFishInfoToUser(CGameRoleObj* pstTargetRole, const std::vector<FishData>& vFishes);

	//推送桌子上子弹信息给玩家
	void SendBulletInfoToUser(CGameRoleObj& stRoleObj);

	//推送桌子信息给玩家
	void SendFishpondInfoToUser(CGameRoleObj& stRoleObj);

	//推送鱼阵详细信息
	void SendFishFormInfo(CGameRoleObj* pstRoleObj);

	//推送退出椅子消息
	void SendExitFishpondAll(int iSeat, bool bForceExit);

	//推送炮台切换的通知
	void SendChangeGunNotify(SeatUserData& stUserData, int iNewGunID, bool bIsStyle);

	//推送子弹消息
	void SendShootBulletNotify(BulletData& stBulletInfo);

	//推送命中鱼的消息
	void SendHitFishInfoNotify(SeatUserData& stUserData, unsigned uBulletUniqID, unsigned uFishUniqID, int iRewardSilver, 
		bool bIsForm, int iCost, int iFishIndex, int iMultipleFish);

	//推送爆炸死的鱼信息
	void SendBoomFishInfoNotify(SeatUserData& stUserData, const std::vector<unsigned>& vFishUniqIDs, const std::vector<BYTE>& vSmallFishs, 
		const std::vector<unsigned>& vFormUniqIDs);

	//推送获得点券的消息
	void SendAddTicketNotify(unsigned uiUin, unsigned uFishUniqID, bool bIsFormFish, long8 lRandTicketNum);

	//发送消息给桌子上所有人
	void SendZoneMsgToFishpond(GameProtocolMsg& stMsg);

public:
	CFishpondObj* m_pPrev;		//前一个鱼池节点
	CFishpondObj* m_pNext;		//后一个鱼池节点

private:
	//鱼池ID
	unsigned m_uTableID;

	//鱼池所在房间ID
	int m_iFishRoomID;

	//鱼池配置
	const FishRoomConfig* m_pstRoomConfig;

	//座位信息
	std::vector<SeatUserData> m_vSeatUserData;

	//鱼池出鱼规则
	std::vector<OutFishRule> m_vOutFishRule;

	//鱼池中的子弹
	std::vector<BulletData> m_vBulletData;

	//鱼阵信息
	FishFormData m_stFishFormData;

	//鱼池中鱼的信息
	std::vector<FishData> m_vFishData;

	//鱼池出鱼上限信息
	std::map<int, FishLimitData> m_mFishLimitData;

	//是否停止出鱼
	bool m_bStopOutFish;

	//发送给客户端的消息
	static GameProtocolMsg ms_stZoneMsg;

	//鱼池上次tick时间,单位ms
	long m_lLastTickTime;
};
