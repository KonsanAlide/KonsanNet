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
#ifndef CXMESSAGEPROCESSLEVLBASE_H
#define CXMESSAGEPROCESSLEVLBASE_H

#include "PlatformDataTypeDefine.h"
#ifdef WIN32
#include <winsock2.h>
#else
#include  <unistd.h>
#include  <sys/types.h>       /* basic system data types */
#include <pthread.h>
#endif

#include "CXThread.h" 
#include "CXMessageQueue.h"
#include "CXConnectionObject.h"
#include "CXSessionsManager.h"
#include "CXLog.h"
#include "CXRPCObjectManager.h"
namespace CXCommunication
{
    class CXMessageProcessLevelBase
    {
    public:
        CXMessageProcessLevelBase();
        virtual ~CXMessageProcessLevelBase();
        void SetMessageQueue(CXMessageQueue* pQueue) { m_pMessageQueue = pQueue; }
        CXMessageQueue * GetMessageQueue() { return m_pMessageQueue; }
        int  Run();
        bool IsStart() { return m_bStart; }
        virtual int  ProcessMessage();

        void Stop();
        virtual int ProcessConnectionError(CXConnectionObject * pCon);

		void SetLogHandle(CXLog * handle) { m_pLogHandle = handle; }
		CXLog *GetLogHandle() { return m_pLogHandle; }

        void SetRPCObjectManager(CXRPCObjectManager * handle) { m_pRPCObjectManager = handle; }
        CXRPCObjectManager *GetRPCObjectManager() { return m_pRPCObjectManager; }

    private:
        CXThread m_threadProcess;
        CXMessageQueue *m_pMessageQueue;

        bool   m_bStart;
		CXLog *m_pLogHandle;

        CXRPCObjectManager *m_pRPCObjectManager;
    };
}
#endif // CXMESSAGEPROCESSLEVLBASE_H
