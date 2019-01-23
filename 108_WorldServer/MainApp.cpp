#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/file.h>

#include "GameProtocol.hpp"
#include "SignalUtility.hpp"
#include "NowTime.hpp"
#include "AppDefW.hpp"
#include "AppLoopW.hpp"
#include "FileUtility.hpp"
#include "StringUtility.hpp"
#include "AppUtility.hpp"

using namespace ServerLib;

int main(int argc, char* *argv)
{
    bool bResume;
    int iWorldID;
    int iInstanceID;

    CAppUtility::AppLaunch(argc, argv, CAppLoopW::SetAppCmd, bResume, iWorldID, iInstanceID, NULL, NULL, NULL, true);

	CAppLoopW* pAppLoop = new CAppLoopW;
	int iRet = pAppLoop->Initialize(bResume, iWorldID);
	if(iRet)
	{
		printf("MainLoop Initialize Error:%d. So Quit!\n", iRet);
		exit(4);
	}

	pAppLoop->Run();

	return 0;
}

