#include <assert.h>

#include "LogAdapter.hpp"

#include "ExternalClientPool.hpp"

//����SessionFD
static unsigned int GenerateSessionFD()
{
	static unsigned int uiSessionFD = 0;

	return ++uiSessionFD;
}

void CExternalClientPool::Initialize()
{
	for (unsigned int i = 0; i < FD_SIZE; i++)
	{
		m_astExternalSocket[i].m_iSocketFD = -1;
		m_astExternalSocket[i].m_uiSessionFD = 0;
		m_astExternalSocket[i].m_pPrevSocket = NULL;
		m_astExternalSocket[i].m_pNextSocket = NULL;
	}

	m_pFirstSocket = NULL;
}

TExternalClientSocket* CExternalClientPool::GetSocketByFD(const int iFD)
{
	assert(iFD > 0 && iFD < (int)FD_SIZE);

	return &m_astExternalSocket[iFD];
}

TExternalClientSocket* CExternalClientPool::GetSocketBySession(unsigned uiSessionFD)
{
	TExternalClientSocket* pstClientSocket = m_pFirstSocket;
	while (pstClientSocket)
	{
		if (pstClientSocket->m_uiSessionFD == uiSessionFD)
		{
			return pstClientSocket;
		}

		pstClientSocket = pstClientSocket->m_pNextSocket;
	}

	return NULL;
}

TExternalClientSocket* CExternalClientPool::GetFirstSocket()
{
	return m_pFirstSocket;
}

void CExternalClientPool::DeleteSocketByFD(const int iFD)
{
	assert(iFD > 0 && iFD < (int)FD_SIZE);

	m_astExternalSocket[iFD].m_iSocketFD = -1;
	m_astExternalSocket[iFD].m_uiSessionFD = 0;

	TExternalClientSocket* pstPrevSocket = m_astExternalSocket[iFD].m_pPrevSocket;
	TExternalClientSocket* pstNextSocket = m_astExternalSocket[iFD].m_pNextSocket;

	if (pstPrevSocket)
	{
		pstPrevSocket->m_pNextSocket = pstNextSocket;
	}

	if (pstNextSocket)
	{
		pstNextSocket->m_pPrevSocket = pstPrevSocket;
	}

	if (m_pFirstSocket == &m_astExternalSocket[iFD])
	{
		m_pFirstSocket = pstNextSocket;
	}

	m_astExternalSocket[iFD].m_pPrevSocket = NULL;
	m_astExternalSocket[iFD].m_pNextSocket = NULL;

	return;
}

void CExternalClientPool::AddSocketByFD(const int iFD)
{
	assert(iFD > 0 && iFD < (int)FD_SIZE);

	//����SessionFD
	m_astExternalSocket[iFD].m_uiSessionFD = GenerateSessionFD();

	m_astExternalSocket[iFD].m_pPrevSocket = NULL;
	m_astExternalSocket[iFD].m_pNextSocket = NULL;

	if (m_pFirstSocket == NULL)
	{
		m_pFirstSocket = &m_astExternalSocket[iFD];
		return;
	}

	m_astExternalSocket[iFD].m_pNextSocket = m_pFirstSocket;

	m_pFirstSocket->m_pPrevSocket = &m_astExternalSocket[iFD];

	m_pFirstSocket = &m_astExternalSocket[iFD];
}

