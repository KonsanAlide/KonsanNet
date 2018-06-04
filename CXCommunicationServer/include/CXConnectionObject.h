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
#ifndef CXCONNECTIONOBJECT_H
#define CXCONNECTIONOBJECT_H

#include "PlatformDataTypeDefine.h"
#include "SocketDefine.h"
#include "CXSpinLock.h"
#include "CXServerStructDefine.h"
//#include "CXMemoryCache.h"
#include "CXMemoryCacheManager.h"
#include "CXCommonPacketStructure.h"
#ifdef WIN32
#include <atomic>
#endif
#include "CXMutexLock.h"
#include "CXLog.h"
#include "CXDataParserImpl.h"
using namespace std;

namespace CXCommunication
{
    class CXConnectionObject;
    typedef int(*POnProcessOnePacket)(CXConnectionObject& conObj,PCXMessageData pMes);
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

            void   SetMemoryCache(CXMemoryCacheManager* pCache) { m_lpCacheObj = pCache; }
            CXMemoryCacheManager* GetMemoryCache() { return m_lpCacheObj; }

            PCXBufferObj GetCXBufferObj(DWORD dwBufSize=sizeof(CXBufferObj));
            void FreeCXBufferObj(PCXBufferObj pBuf) { m_lpCacheObj->FreeBuffer(pBuf); }

            void* GetBuffer(DWORD dwBufSize) { return m_lpCacheObj->GetBuffer(dwBufSize); }
            void  FreeBuffer(void* pBuf) { m_lpCacheObj->FreeBuffer(pBuf); }

            //
            int    RecvPacket(PCXBufferObj pBufObj,DWORD dwTransDataOfBytes);

            bool   VerifyPacketBodyData(PCXPacketHeader pHeader,byte *pData);

            static int  OnProcessOnePacket(CXConnectionObject &conObj, PCXMessageData pMes);

            void  SetProcessOnePacketFun(POnProcessOnePacket pfn) { m_pfnProcessOnePacket= pfn; }

            int PostSend(byte *pData,int iDataLen);
            int PostSend(PCXBufferObj pBufObj);
            int PostSendBlocking(PCXBufferObj pBufObj, DWORD &dwSendLen, bool bLockBySelf = true);

            int PostRecv(int iBufSize);

            void SetObjectSizeInCache(int iObjectSize) { m_iObjectSizeInCache = iObjectSize; }

            void SetDispacherQueueIndex(int iIndex) { m_iDispacherQueueIndex= iIndex; }
            int  GetDispacherQueueIndex() { return m_iDispacherQueueIndex; }

            uint64 GetProcessPacketNumber() { return m_iProcessPacketNumber; }
            void AddProcessPacketNumber();

            uint64 GetNumberOfReceivedBufferInList() { return m_uiNumberOfReceivedBufferInList; }

            uint64 GetNumberOfReceivedPacketInQueue() { return m_uiNumberOfReceivedPacketInQueue; }
            void  AddReceivedPacketNumber();
            void  ReduceReceivedPacketNumber();

            void  ReduceNumberOfPostBuffers();

            uint64 GetNumberOfPostBuffers() { return m_uiNumberOfPostBuffers; }

            void SetState(int iState) { m_nState=iState; }
            int  GetState() { return m_nState; }

            void Close();

            void SetConnectionType(int iType) { m_iConnectionType=iType; }
            int  GetConnectionType() { return m_iConnectionType; }

            void SetSessionsManager(void *pManager) { m_pSessionsManager = pManager; }
            void *  GetSessionsManager() { return m_pSessionsManager; }

            void SetSession(void *pSession) { m_pSession = pSession; }
            void *  GetSession() { return m_pSession; }

            int  RecvData(PCXBufferObj *ppBufObj,DWORD &dwReadLen);
            int  SendData(char *pBuf,int nBufLen, DWORD &dwSendLen,int nFlags=0);

            void SetLogHandle(CXLog * handle) { m_pLogHandle = handle; }
            CXLog *GetLogHandle() { return m_pLogHandle; }

            //send a packet to the peer
            bool  SendPacket(const byte* pbData,DWORD dwLen,DWORD dwMesCode, bool bLockBySelf = true);
            void  LockSend();
            void  UnlockSend();
            
            void SetDataParserHandle(CXDataParserImpl * handle) { m_pDataParserHandle = handle; }
            CXDataParserImpl *GetDataParserHandle() { return m_pDataParserHandle; }

            void  LockRead();
            void  UnlockRead();

        protected:
        private:
            cxsocket m_sock;
            //==1 major message connection
            //==2 minor messsage connection
            //==3 data connection
            //==4 object connection
            int    m_iConnectionType;

            uint64 m_tmAcceptTime;

            uint64 m_uiConnectionIndex;

#ifdef atomic
            atomic<uint64> m_uiRecvBufferIndex;
            atomic<uint64> m_uiSendBufferIndex;
#else
            uint64 m_uiRecvBufferIndex;
            uint64 m_uiSendBufferIndex;

#endif
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

            // the pointer of the memory cache object
            CXMemoryCacheManager* m_lpCacheObj;


            // the list header of the cache buffer list for receiving data
            PCXBufferObj m_pListReadBuf;

            // the list end of the cache buffer list for receiving data
            PCXBufferObj m_pListReadBufEnd;

            // the list header of the cache buffer list for sending data
            PCXBufferObj m_pListSendBuf;

            // the list end of the cache buffer list for sending data
            PCXBufferObj m_pListSendBufEnd;

            CXSpinLock  m_lock;
            CXSpinLock  m_lockRead;
            CXSpinLock  m_lockSend;

            POnProcessOnePacket m_pfnProcessOnePacket;


            //the object size in the cache
            int m_iObjectSizeInCache;

            // the index of the queue in the CXDataDispathLevelImpl object
            int m_iDispacherQueueIndex;

#ifdef WIN32
            // the number of the received buffers in the receiving buffer list
            atomic<uint64> m_uiNumberOfReceivedBufferInList;

            // the number of the received packets in the message queue
            atomic<uint64> m_uiNumberOfReceivedPacketInQueue;

            // the number of the buffer by posting to iocp model
            atomic<uint64> m_uiNumberOfPostBuffers;
            atomic<uint64> m_iProcessPacketNumber;
#else
            // the number of the received buffers in the receiving buffer list
            uint64 m_uiNumberOfReceivedBufferInList;

            // the number of the received packets in the message queue
            uint64 m_uiNumberOfReceivedPacketInQueue;

            // the number of the buffer by posting to iocp model
            uint64 m_uiNumberOfPostBuffers;

            uint64 m_iProcessPacketNumber;

#endif

            void * m_pSessionsManager;
            void * m_pSession;

            CXLog *m_pLogHandle;
            CXDataParserImpl *m_pDataParserHandle;
    };

}
#endif // CXCONNECTIONOBJECT_H
