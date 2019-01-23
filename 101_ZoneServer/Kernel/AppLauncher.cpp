#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/file.h>

#include "GameProtocol.hpp"
#include "AppDef.hpp"
#include "SignalUtility.hpp"
#include "NowTime.hpp"
#include "AppUtility.hpp"
#include "ShmObjectCreator.hpp"
#include "FileUtility.hpp"
#include "StringUtility.hpp"
#include "Random.hpp"
#include "AppLoop.hpp"

using namespace ServerLib;

int main(int argc, char** argv)
{
	bool bResume;
	int iWorldID;
	int iZoneID;
	int iInstanceID;

	CAppUtility::AppLaunch(argc, argv, CAppLoop::SetAppCmd, bResume, iWorldID, iInstanceID, &iZoneID, NULL, NULL, true);

	CAppLoop* pAppLoop = new CAppLoop;
	int iRet = pAppLoop->Initialize(bResume, iWorldID, iZoneID, iInstanceID);
	if (iRet)
	{
		TRACESVR("MainLoop Initialize Error:%d. So Quit!\n", iRet);
		return -1;
	}

	pAppLoop->Run();

	return 0;
}

