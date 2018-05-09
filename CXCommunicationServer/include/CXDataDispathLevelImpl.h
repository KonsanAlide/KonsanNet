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
#ifndef CXDATADISPATHLEVELIMPL_H
#define CXDATADISPATHLEVELIMPL_H


#include "CXConnectionObject.h"
#include "CXMessageQueue.h"
#include "CXSpinLock.h"
#include <queue>
using std::queue;
namespace CXCommunication
{
    class CXCommunicationServer;
    class CXDataDispathLevelImpl
    {
        public:
            CXDataDispathLevelImpl();
            virtual ~CXDataDispathLevelImpl();

            int Start(CXCommunicationServer *pServer, int iQueueNumber);
            int Stop();

            //have beed locked by CXConnectionObject::lock
            static int  OnRecv(CXConnectionObject &conObj ,PCXBufferObj pBufObj,
                DWORD dwTransDataOfBytes);

            //have beed locked by CXConnectionObject::lock
            static int  OnClose(CXConnectionObject &conObj);

            int  SendPacket(uint64 uiConnIndex,char *pBuf,int nDataLen,bool bBlocking);

            int  GetQueueNumber() { return m_iQueueNumber; }

            int  PushReadPacket(int iIndex, PCXBufferObj pBufObj);
            int  PushWritePacket(int iIndex, PCXBufferObj pBufObj);

            CXMessageQueue *GetReadMessageQueue(int iIndex);

        protected:
        private:
            CXCommunicationServer *m_pServer;
            CXMessageQueue * m_ppReadMessageQueue;
            CXMessageQueue * m_ppWriteMessageQueue;
            int m_iQueueNumber;
    };
}

#endif // CXDATADISPATHLEVELIMPL_H
