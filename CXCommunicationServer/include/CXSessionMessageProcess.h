#pragma once
#include "CXSessionLevelBase.h"

namespace CXCommunication
{
    class CXSessionMessageProcess :public CXSessionLevelBase
    {
    public:
        CXSessionMessageProcess();
        ~CXSessionMessageProcess();

        int SessionLogin(PCXMessageData pMes, CXConnectionSession ** ppSession);
        int SessionLogout(PCXMessageData pMes, CXConnectionSession &session);
        int SessionSetting(PCXMessageData pMes, CXConnectionSession &session);

    };
}

