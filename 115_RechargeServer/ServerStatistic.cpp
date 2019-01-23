#include <string.h>

#include "LogAdapter.hpp"

#include "ServerStatistic.hpp"

using namespace ServerLib;

CServerStatistic* CServerStatistic::Instance()
{
	static CServerStatistic* pInstance = NULL;
	if (!pInstance)
	{
		pInstance = new CServerStatistic;
	}

	return pInstance;
}

CServerStatistic::CServerStatistic()
{
	ClearAllStat();
}

CServerStatistic::~CServerStatistic()
{

}

void CServerStatistic::ClearAllStat()
{
    m_uiInputCodeNumber = 0;
    m_uiInputPacketNumber = 0;
    m_uiInputPacketLength = 0;
    m_uiOutputPacketNumber = 0;
    m_uiOutputPacketLength = 0;
}

int CServerStatistic::RecordAllStat(unsigned int uiIntervalTime)
{
    LOGDEBUG("IntervalTime:%u InputCodeNumber:%u InputPacketNumber:%u InputPacketLength:%u "
        "OutputPacketNumber:%u OutputPacketLength:%u\n",
        uiIntervalTime, m_uiInputCodeNumber, m_uiInputPacketNumber, m_uiInputPacketLength, m_uiOutputPacketNumber, m_uiOutputPacketLength);

    return 0;
}
