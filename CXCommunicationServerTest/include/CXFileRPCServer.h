/****************************************************************************
Copyright (c) 2018-2019 Charles Yang

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
#ifndef __CXFILERPCSERVER_H__
#define __CXFILERPCSERVER_H__

#include "CXConnectionObject.h"
#include "CXConnectionSession.h"
#include "CXCommonPacketStructure.h"
#include "CXRPCObjectServer.h"
#include "CXFile64.h"
namespace CXCommunication
{
    class CXFileRPCServer : public CXRPCObjectServer
    {
    public:
        CXFileRPCServer();
        ~CXFileRPCServer();

        virtual string GetObjectName() { return "CXFileTcpV1"; }

        virtual int ProcessMessage(PCXMessageData pMes);

        virtual int SendData(CXConnectionObject * pCon, const byte *pbyData, DWORD dwDataLen);

        virtual CXRPCObjectServer* CreateObject();

        virtual void Destroy();

        virtual void RecordSlowOps(PCXMessageData pMes);

    private:
        CXFile64 m_file;
    };
}
#endif //__CXFILERPCSERVER_H__

