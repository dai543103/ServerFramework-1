#pragma once

#include "CommDefine.h"

class IHandler;
class CHandlerFactory
{
public:
    static IHandler* GetHandler(const unsigned int uiMsgID);
    static int RegisterHandler(const unsigned int uiMsgID, const IHandler* pHandler);

protected:
    static IHandler* m_apHandler[MAX_HANDLER_NUMBER];
};
