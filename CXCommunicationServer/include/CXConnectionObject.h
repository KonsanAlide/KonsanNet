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

Description：
*****************************************************************************/
#ifndef CXCONNECTIONOBJECT_H
#define CXCONNECTIONOBJECT_H

#include "PlatformDataTypeDefine.h"
#include "SocketDefine.h"
#include "CXSpinLock.h"
#include "CXServerStructDefine.h"
#include "CXMemoryCache.h"
#include "CXCommonPacketStructure.h"
#include <atomic>
#include "CXMutexLock.h";


using namespace std;

namespace CXCommunication
{
    class CXConnectionObject;
    typedef int(*POnProcessOnePacket)(CXConnectionObject& conObj, PCXBufferObj pBufObj);
    class CXConnectionObject
    {
        public:
            CXConnectionObject();
            virtual ~CXConnectionObject();
            void Reset();
            void Lock();
            void UnLock();
            void Build(cxsocket sock,uint64 uiConnIndex,sockaddr_in addr);
            void RecordCurrentPacketTime();

            uint64 GetLastPacketTime(){return m_tmLastPacketTime;}
            uint64 GetAcceptedTime(){return m_tmAcceptTime;}
            cxsocket GetSocket(){return m_sock;}
            void SetConnectionIndex(int iIndex) { m_uiConnectionIndex=iIndex; }
            uint64 GetConnectionIndex(){return m_uiConnectionIndex;}
            void   SetServer(void* pServer) { m_lpServer = pServer; }
            void*  GetServer() { return m_lpServer; }

            void   SetMemoryCache(CXMemoryCache* pCache) { m_lpCacheObj = pCache; }
            CXMemoryCache* GetMemoryCache() { return m_lpCacheObj; }

            void   SetLargeBlockMemoryCache(CXMemoryCache* pCache) { m_lpLargeBlockCacheObj = pCache; }
            CXMemoryCache* GetLargeBlockMemoryCache() { return m_lpLargeBlockCacheObj; }

            PCXBufferObj GetBuffer();
            void FreeBuffer(PCXBufferObj pBuf) { m_lpCacheObj->FreeObject(pBuf); }

            //
            int    RecvPacket(PCXBufferObj pBufObj,DWORD dwTransDataOfBytes);

            static int  OnProcessOnePacket(CXConnectionObject &conObj, PCXBufferObj pBufObj);

            void  SetProcessOnePacketFun(POnProcessOnePacket pfn) { m_pfnProcessOnePacket= pfn; }

            int PostSend(byte *pData,int iDataLen);
            int PostSend(PCXBufferObj pBufObj);
            int PostSendBlocking(PCXBufferObj pBufObj,DWORD &dwSendLen);

            int PostRecv(int iBufSize);
            

            void SetObjectSizeInCache(int iObjectSize) { m_iObjectSizeInCache = iObjectSize; }

            void SetDispacherQueueIndex(int iIndex) { m_iDispacherQueueIndex= iIndex; }
            int  GetDispacherQueueIndex() { return m_iDispacherQueueIndex; }

            uint64 GetProcessPacketNumber() { return m_iProcessPacketNumber; }
            void AddProcessPacketNumber();

            uint64 GetNumberOfReceivedBufferInList() { return m_uiNumberOfReceivedBufferInList; }
            void  AddReceivedBufferNumber();
            void  ReduceReceivedBufferNumber();

            uint64 GetNumberOfReceivedPacketInQueue() { return m_uiNumberOfReceivedPacketInQueue; }
            void  AddReceivedPacketNumber();
            void  ReduceReceivedPacketNumber();

            void  ReduceNumberOfPostBuffers();

            uint64 GetNumberOfPostBuffers() { return m_uiNumberOfPostBuffers; }

            void SetState(int iState) { m_nState=iState; }

            void Close();

            void SetConnectionType(int iType) { m_iConnectionType=iType; }
            int  GetConnectionType() { return m_iConnectionType; }

            void SetSessionsManager(void *pManager) { m_pSessionsManager = pManager; }
            void *  GetSessionsManager() { return m_pSessionsManager; }

            void SetSession(void *pSession) { m_pSession = pSession; }
            void *  GetSession() { return m_pSession; }

            

    
        protected:
        private:
            cxsocket m_sock;
            //==1 major message connection
            //==2 minor messsage connection
            //==3 data connection
            //==4 object connection
            int      m_iConnectionType;

            uint64 m_tmAcceptTime;

            uint64 m_uiConnectionIndex;

            atomic<uint64> m_uiRecvBufferIndex;
            uint64 m_uiLastProcessBufferIndex;

            //==0 initialize
            //==1 pending connection
            //==2 recv data packets
            //==3 closing
            //==4 closed
            int    m_nState;

            //last packet time
            uint64  m_tmLastPacketTime;

            //the client's ip address and port
            sockaddr_in m_addrRemote;

            // class object pointer, communication server
            void* m_lpServer;
            CXMemoryCache* m_lpCacheObj;
            CXMemoryCache* m_lpLargeBlockCacheObj;

            // 使用计数
            int  m_nUsedNumber;

            // the list header of the cache buffer list for receiving data
            PCXBufferObj m_pListReadBuf;

            // the list end of the cache buffer list for receiving data
            PCXBufferObj m_pListReadBufEnd;

            // the list header of the cache buffer list for sending data
            PCXBufferObj m_pListSendBuf;

            // the list end of the cache buffer list for sending data
            PCXBufferObj m_pListSendBufEnd;

            //CXSpinLock m_lock;
            CXMutexLock m_lock;
            CXSpinLock m_lockNumber;

            POnProcessOnePacket m_pfnProcessOnePacket;


            //the object size in the cache
            int m_iObjectSizeInCache;

            // the index of the queue in the CXDataDispathLevelImpl object
            int m_iDispacherQueueIndex;

            // the number of the received buffers in the receiving buffer list
            atomic<uint64> m_uiNumberOfReceivedBufferInList;

            // the number of the received packets in the message queue
            atomic<uint64> m_uiNumberOfReceivedPacketInQueue;

            // the number of the buffer by posting to iocp model
            atomic<uint64> m_uiNumberOfPostBuffers;

            uint64 m_iProcessPacketNumber;

            void * m_pSessionsManager;
            void * m_pSession;
    };

}
#endif // CXCONNECTIONOBJECT_H
