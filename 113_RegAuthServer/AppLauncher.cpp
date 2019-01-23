#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/file.h>
#include <stdlib.h>

#include "RegAuthApp.hpp"
#include "SignalUtility.hpp"
#include "NowTime.hpp"
#include "ShmObjectCreator.hpp"
#include "FileUtility.hpp"
#include "StringUtility.hpp"
#include "AppUtility.hpp"

using namespace ServerLib;

int main(int argc, char* *argv)
{
    bool bResume;
    int iWorldID;
    int iInstanceID;

    CAppUtility::AppLaunch(argc, argv, CRegAuthApp::SetAppCmd, bResume, iWorldID, iInstanceID);

	CRegAuthApp* pApp = new CRegAuthApp;
    int iRet = pApp->Initialize(bResume);
    if (iRet)
    {
        exit(4);
    }

    pApp->Run();

    return 0;
}

