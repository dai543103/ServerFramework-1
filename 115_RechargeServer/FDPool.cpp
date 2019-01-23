#include <assert.h>

#include "LogAdapter.hpp"

#include "FDPool.hpp"

int CFDPool::Initialize()
{
	memset(m_auiSocketFlag, 0, sizeof(m_auiSocketFlag));
	m_stExternalClientPool.Initialize();

	return 0;
}

void CFDPool::SetFDInactive(int iFD)
{
	assert(iFD > 0 && iFD < (int)FD_SIZE);

    //¹Ø±Õsocket
    m_stExternalClientPool.DeleteSocketByFD(iFD);

	m_auiSocketFlag[iFD] = 0;

	return;
}

void CFDPool::SetFDActive(int iFD)
{
	assert(iFD > 0 && iFD < (int)FD_SIZE);

	m_stExternalClientPool.AddSocketByFD(iFD);

}

bool CFDPool::IsListeningForExternalClient(int iFD)
{
	return (m_auiSocketFlag[iFD] & ELSF_ListeningForExternalClient);
}

void CFDPool::SetListeningForExternalClient(int iFD)
{
	m_auiSocketFlag[iFD] = ELSF_ListeningForExternalClient;
}

bool CFDPool::IsConnectedByExternalClient(int iFD)
{
	return (m_auiSocketFlag[iFD] & ELSF_ConnectedByExternalClient);
}

void CFDPool::SetConnectedByExternalClient(int iFD)
{
	m_auiSocketFlag[iFD] = ELSF_ConnectedByExternalClient;
}

TExternalClientSocket* CFDPool::GetExternalSocketByFD(int iFD)
{
    return m_stExternalClientPool.GetSocketByFD(iFD);
}

TExternalClientSocket* CFDPool::GetExternalSocketBySession(unsigned uiSessionFD)
{
	return m_stExternalClientPool.GetSocketBySession(uiSessionFD);
}
