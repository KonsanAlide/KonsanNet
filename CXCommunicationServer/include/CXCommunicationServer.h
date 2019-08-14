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
#include "CXSpinLock.h"
#include <map>
#include <list>
#include "CXDataParserImpl.h"
#include "CXSessionLevelBase.h"
#include "CXLog.h"
#include "CXNetworkInitEnv.h"
#include "CXRPCObjectManager.h"
#include "CXIOStat.h"
using namespace std;

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

        static int  OnRecv(CXConnectionObject &conObj, PCXBufferObj pBufObj,
            DWORD dwTransDataOfBytes, byte* pbyThreadCache, DWORD dwCacheLen);

        static int  OnClose(CXConnectionObject &conObj,ConnectionClosedType emClosedType);

        static int  OnAccept(void *pServer, cxsocket sock, sockaddr_in &addrRemote);

        static int  OnWrite(CXConnectionObject &conObj);

        //push a message to the message queue
        static int  OnProcessOnePacket(CXConnectionObject &conObj, PCXMessageData pMes);

        CXSocketServerKernel &GetSocketSeverKernel() { return m_socketServerKernel; }
        CXConnectionsManager &GetConnectionManager() { return m_connectionsManager; }
        CXMemoryCacheManager &GetMemoryCacheManger() { return m_memoryCacheManager; }
        CXDataDispathLevelImpl &GetDataDispathManger() { return m_dataDispathManager; }

        void CloseConnection(CXConnectionObject &conObj, ConnectionClosedType emClosedType,bool bLockBySelf=true);

        CXSessionsManager &  GetSessionsManager() { return m_sessionsManager; }

        void   AddReceivedBuffers();
        uint64 GetTotalReceiveBuffers() { return m_uiTotalReceiveBuffers; }
        uint64 GetTotalConnectionsNumber() { return m_socketServerKernel.GetConnectionsNumber(); }

        void SetDataParserHandle(CXDataParserImpl * handle) { m_pDataParserHandle = handle; }
        CXDataParserImpl *GetDataParserHandle() { return m_pDataParserHandle; }

        void SetLogHandle(CXLog * handle) { m_pLogHandle = handle; }
        CXLog *GetLogHandle() { return m_pLogHandle; }

		void SetJournalLogHandle(CXLog * handle) { m_pJouralLogHandle = handle; }
		CXLog *GetJournalLogHandle() { return m_pJouralLogHandle; }

        void Register(const string &strObjectGuid, CXRPCObjectServer *pObj) { m_rpcObjectManager.Register(strObjectGuid, pObj); }

        CXIOStat *GetIOStatHandle() { return &m_ioStat; }

		CXRPCObjectManager *GetRPCObjectManager() { return &m_rpcObjectManager; }

    private:
        bool m_bRunning;
        CXSocketServerKernel m_socketServerKernel;
        CXConnectionsManager m_connectionsManager;
        CXMemoryCacheManager m_memoryCacheManager;
        CXDataDispathLevelImpl m_dataDispathManager;
        CXSessionsManager    m_sessionsManager;

        // the list of the message process object
        // the message process object has a message process thread ,waiting for the message queue object
        list<void*> m_lstMessageProcess;

        CXSpinLock m_lock;
        uint64 m_uiTotalReceiveBuffers;
        CXDataParserImpl *m_pDataParserHandle;
        CXLog                *m_pLogHandle;
        CXNetworkInitEnv      m_networkInit;

		//record journal log
		CXLog                *m_pJouralLogHandle;

        CXRPCObjectManager    m_rpcObjectManager;

		CXIOStat m_ioStat;
    };
}

#endif // CXCOMMUNICATIONSERVER_H


