#pragma once

#include <vector>
#include <string>
#include "GameProtocol.hpp"
#include "CommDefine.h"

struct SystemMailData
{
	int iVIPMin;	//VIP最小,为0没有限制
	int iVIPMax;	//VIP最大,为0没有限制
	std::vector<std::string> vChannelIDs;	//渠道号限制，没有为空
	MailData stMail;	//邮件内容
	int iEndTime;		//邮件结束时间

	SystemMailData()
	{
		Reset();
	}

	void Reset()
	{
		iVIPMin = 0;
		iVIPMax = 0;
		vChannelIDs.clear();
		stMail.Reset();
		iEndTime = 0;
	}
};

class CWorldMailManager
{
public:
	static CWorldMailManager* Instance();

	~CWorldMailManager();

	//初始化Mail
	void Init();

	//发送GM邮件
	int SendGMMail(const GameMaster_Request& stReq);

	//发送个人邮件
	static int SendPersonalMail(const World_SendMail_Request& stReq);

	//拉取系统邮件
	void GetSystemMail(unsigned uin, int iReason, unsigned uUniqID, int iVIPLevel, const std::string& strChannel, World_GetSystemMail_Response& stResp);

private:

	CWorldMailManager();

	//发送系统邮件
	void SendSystemMail(const GameMaster_Request& stReq, const OneMailInfo& stMailInfo);

	//推送
	void SendSystemMailIDNotify();

	//加载邮件信息
	void LoadMailInfo();

	//保存邮件信息
	void SaveMailInfo();

	//重置
	void Reset();

private:

	//系统邮件当前最大ID
	unsigned m_uSystemUniqID;

	//邮件信息
	std::vector<SystemMailData> m_vSystemMail;
};
