#include "CXAdvancedSessionMessageProcessHandle.h"
#include "CXPacketCodeDefine.h"
#include "CXSessionPacketStructure.h"
#include "CXPacketCodeDefine.h"
#include <string>
#ifndef WIN32
#include<string.h>
#endif
using namespace CXCommunication;
using namespace std;

CXAdvancedSessionMessageProcessHandle::CXAdvancedSessionMessageProcessHandle()
{
}


CXAdvancedSessionMessageProcessHandle::~CXAdvancedSessionMessageProcessHandle()
{
}

int CXAdvancedSessionMessageProcessHandle::SessionLogin(PCXMessageData pMes, CXConnectionSession ** ppSession)
{
    
    return RETURN_SUCCEED;
}

int CXAdvancedSessionMessageProcessHandle::SessionLogout(PCXMessageData pMes, CXConnectionSession &session)
{
    return RETURN_SUCCEED;
}

int CXAdvancedSessionMessageProcessHandle::SessionSetting(PCXMessageData pMes, CXConnectionSession &session)
{
    return RETURN_SUCCEED;
}

