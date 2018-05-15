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
#ifndef CXCOMMUNICATIONSERVER_H
#define CXCOMMUNICATIONSERVER_H

#include "SocketDefine.h"
#include "CXEvent.h"
#include "CXThread.h" 
#include "CXSocketServerKernel.h"
#include "CXConnectionsManager.h"
#include "CXDataDispathLevelImpl.h"
#include "CXMemoryCacheManager.h"
#include "CXSessionsManager.h"

#include <list>
using std::list;

namespace CXCommunication
{
    class CXCommunicationServer
    {
    private:

    public:
        CXCommunicationServer();
        virtual ~CXCommunicationServer();

        int Start(unsigned short iListeningPort, int iWaitThreadNum);
        int Stop();

        //have beed locked by CXConnectionObject::lock
        static int  OnRecv(CXConnectionObject &conObj, PCXBufferObj pBufObj,
            DWORD dwTransDataOfBytes);

        //have beed locked by CXConnectionObject::lock
        static int  OnClose(CXConnectionObject &conObj);

        //have beed locked by CXConnectionObject::lock
        static int  OnAccept(void *pServer, cxsocket sock, sockaddr_in &addrRemote);

        //have beed locked by CXConnectionObject::lock
        static int  OnWrite(CXConnectionObject &conObj);

        static int  OnProcessOnePacket(CXConnectionObject &conObj, PCXBufferObj pBufObj);

        CXSocketServerKernel &GetSocketSeverKernel() { return m_socketServerKernel; }
        CXConnectionsManager &GetConnectionManager() { return m_connectionsManager; }
        CXMemoryCacheManager &GetMemoryCacheManger() { return m_memoryCacheManager; }
        CXDataDispathLevelImpl &GetDataDispathManger() { return m_dataDispathManager; }

        void CloseConnection(CXConnectionObject &conObj,bool bLockBySelf=true);

        CXSessionsManager &  GetSessionsManager() { return m_sessionsManager; }

    public:
            
    protected:
        int  ParsePackets(CXConnectionObject& conObj, PCXBufferObj pBufObj,
            DWORD dwTransDataOfBytes);

    private:
        bool m_bRunning;
        CXSocketServerKernel m_socketServerKernel;
        CXConnectionsManager m_connectionsManager;
        CXMemoryCacheManager m_memoryCacheManager;
        CXDataDispathLevelImpl m_dataDispathManager;
        CXSessionsManager    m_sessionsManager;
        list<void*> m_lstMessageProcess;

    };
}

#endif // CXCOMMUNICATIONSERVER_H


