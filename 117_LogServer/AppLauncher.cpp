#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/file.h>
#include <stdlib.h>

#include "GameProtocol.hpp"
#include "AppDef.hpp"
#include "SignalUtility.hpp"
#include "NowTime.hpp"
#include "ShmObjectCreator.hpp"
#include "FileUtility.hpp"
#include "StringUtility.hpp"
#include "AppUtility.hpp"

#include "LogServerApp.hpp"

using namespace ServerLib;

int main(int argc, char* *argv)
{
    bool bResume;
    int iWorldID;
    int iInstanceID;

    CAppUtility::AppLaunch(argc, argv, CLogServerApp::SetAppCmd, bResume, iWorldID, iInstanceID);

	CLogServerApp* pApp = new CLogServerApp;
    int iRet = pApp->Initialize(bResume, iWorldID);
    if (iRet != 0)
    {
        exit(4);
    }

    pApp->Run();

    return 0;
}

