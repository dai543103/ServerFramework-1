#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/file.h>
#include <stdlib.h>

#include "AppDef.hpp"
#include "RoleDBApp.hpp"
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

    CAppUtility::AppLaunch(argc, argv, CRoleDBApp::SetAppCmd, bResume, iWorldID, iInstanceID);

    CRoleDBApp* pApp = new CRoleDBApp;
    int iRet = pApp->Initialize(bResume, iWorldID);
    if (iRet)
    {
        exit(4);
    }

    pApp->Run();

    return 0;
}

