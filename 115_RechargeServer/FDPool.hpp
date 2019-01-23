#pragma once

#include "AppDefine.hpp"
#include "ExternalClientPool.hpp"

class CFDPool
{
public:

	//初始化External、Internal套接字池
	int Initialize();

	//将指定FD设置为空
	void SetFDInactive(int iFD);

	// 设置激活
	void SetFDActive(int iFD);

	//是否是外部监听套接字
	bool IsListeningForExternalClient(int iFD);

	//设置为外部监听套接字
	void SetListeningForExternalClient(int iFD);

	//是否是外部连接套接字
	bool IsConnectedByExternalClient(int iFD);

	//设置为外部连接套接字
	void SetConnectedByExternalClient(int iFD);

	TExternalClientSocket* GetExternalSocketByFD(int iFD);
	TExternalClientSocket* GetExternalSocketBySession(unsigned uiSessionFD);

private:
	unsigned int m_auiSocketFlag[FD_SIZE];

	CExternalClientPool m_stExternalClientPool;
};
