#pragma once
#include "CXSessionLevelBase.h"
namespace CXCommunication
{
    class CXSessionMessageProcess :public CXSessionLevelBase
    {
    public:
        CXSessionMessageProcess();
        ~CXSessionMessageProcess();

        virtual int SessionLogin(PCXBufferObj pBuf, CXConnectionSession ** ppSession);
        virtual int SessionLogout(PCXBufferObj pBuf, CXConnectionSession &session);
        virtual int SessionSetting(PCXBufferObj pBuf, CXConnectionSession &session);

        virtual int SendCommonMessageReply(CXConnectionObject * pCon,
            DWORD deMessageCode, PCXBufferObj pBuf);
    };
}

