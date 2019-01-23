#pragma once

//玩家任务管理器
#include <string.h>
#include <vector>

#include "GameProtocol.hpp"
#include "CommDefine.h"
#include "BaseConfigManager.hpp"

class CGameRoleObj;
class CMailManager
{
public:
	CMailManager();
	~CMailManager();

public:

	//初始化
	int Initialize();

	void SetOwner(unsigned int uin);
	CGameRoleObj* GetOwner();

	//新增邮件
	void AddNewMail(const OneMailInfo& stMailInfo, bool bSendNotify = true);

	//操作邮件
	int OperateMail(int iOperaType, unsigned uUniqID);

	//服务器系统邮件唯一ID
	static unsigned GetServerMailUniqID();
	static void SetServerMailUniqID(unsigned uUniqID);

	//玩家系统邮件唯一ID
	unsigned GetRoleSystemUniqID();
	void SetRoleSystemMailUniqID(unsigned uUniqID);

	//定时器
	void OnTick(int iTimeNow);

	//更新邮件到DB
	void UpdateMailToDB(MAILDBINFO& stMailDBInfo);

	//从DB初始化邮件
	void InitMailFromDB(const MAILDBINFO& stMailDBInfo);

private:

	//获取邮件位置，不存在返回-1
	int GetMailIndexByUniqID(unsigned uUniqID);

	//推送新邮件消息
	void SendAddNewMailNotify(const MailData& stOneMail, const std::vector<unsigned>& vDelUniqIDs);

	//收取邮件附件
	int GetMailAppendix(MailData& stOneMail);

private:

	//玩家uin
	unsigned m_uiUin;

	//玩家系统邮件唯一ID
	unsigned m_uRoleSystemUniqID;

	//玩家个人邮件唯一ID
	unsigned m_uPersonalUniqID;
	
	//玩家邮件信息
	std::vector<MailData> m_vMails;

	//服务器系统邮件唯一ID
	static unsigned m_uServerSystemMailUniqID;
};
