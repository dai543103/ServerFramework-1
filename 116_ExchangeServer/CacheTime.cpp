#include "TimeUtility.hpp"

#include "CacheTime.hpp"

using namespace ServerLib;

CCacheTime::CCacheTime()
{
}

void CCacheTime::SetCreatedTime(int iTime)
{
	m_iCreatedTime = iTime;
}

int CCacheTime::GetCreatedTime()
{
    return m_iCreatedTime;
}

void CCacheTime::GetCreatedTime(char* szTimeString)
{
	CTimeUtility::ConvertUnixTimeToTimeString(m_iCreatedTime, szTimeString);
}
