/****************************************************************************
Copyright (c) 2018 Charles Yang

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Description£º
*****************************************************************************/
#ifndef CXSESSIONLEVLBASE_H
#define CXSESSIONLEVLBASE_H

#include "CXRPCObjectServer.h"
#include "CXServerStructDefine.h"
#include "CXSessionsManager.h"
namespace CXCommunication
{
    class CXSessionLevelBase: public CXRPCObjectServer
    {
    public:
        CXSessionLevelBase();
        virtual ~CXSessionLevelBase();

        virtual string GetObjectName() { return "CXConnectionLoginV1"; }

        virtual int ProcessMessage(PCXMessageData pMes);

        virtual int SendData(CXConnectionObject * pCon, const byte *pbyData, DWORD dwDataLen);

        virtual CXRPCObjectServer* CreateObject();

        virtual void Destroy();

        virtual void RecordSlowOps(PCXMessageData pMes);

    protected:
        virtual int SessionLogin(PCXMessageData pMes);
        virtual int SessionLogout(PCXMessageData pMes, CXConnectionSession &session);
        virtual int SessionSetting(PCXMessageData pMes, CXConnectionSession &session);
    };
}
#endif // CXSESSIONLEVLBASE_H
