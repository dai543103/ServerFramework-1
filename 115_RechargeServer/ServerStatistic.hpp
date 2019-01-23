#pragma once

#include "AppDefine.hpp"
#include "SingletonTemplate.hpp"

class CServerStatistic
{
public:
	static CServerStatistic* Instance();
	~CServerStatistic();

	void ClearAllStat();

	int RecordAllStat(unsigned int uiIntervalTime);

	inline void AddInputCodeNumber(unsigned int uiCodeNumber)
	{
		m_uiInputCodeNumber += uiCodeNumber;
	};

	inline void AddInputPacket(unsigned int uiPacketLength)
	{
		m_uiInputPacketNumber++;
		m_uiInputPacketLength += uiPacketLength;
	};

	inline void AddOutputPacket(unsigned int uiPacketLength)
	{
		m_uiOutputPacketNumber++;
		m_uiOutputPacketLength += uiPacketLength;
	};

private:
	CServerStatistic();

private:

	//Input
	//Output
	//External
	//Internal
	unsigned int m_uiInputCodeNumber;
	unsigned int m_uiInputPacketNumber;
	unsigned int m_uiInputPacketLength;
	unsigned int m_uiOutputPacketNumber;
	unsigned int m_uiOutputPacketLength;
};
