#pragma once

#include "AppDefine.hpp"

class CExternalClientPool
{
public:
	void Initialize();

	TExternalClientSocket* GetSocketByFD(const int iFD);
	TExternalClientSocket* GetSocketBySession(unsigned uiSessionFD);
	TExternalClientSocket* GetFirstSocket();

	void DeleteSocketByFD(const int iFD);
	void AddSocketByFD(const int iFD);

private:
	TExternalClientSocket m_astExternalSocket[FD_SIZE];
	
	TExternalClientSocket* m_pFirstSocket;
};
