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
#include "CXMemoryCacheManager.h"
#include "CXCommonPacketStructure.h"
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
            enum CONNECTION_STATE
            {
                //not accepted a socket
                INITIALIZED=0,
                //accepted a socket,but need to check the effectiveness of the source
                PENDING,
                //accepted the socket, and the source is effective, and allowed to receive and send data
                ESTABLISHED,
                //the connection is closing, the socket had been closed,this socket can not receive or send any data,
                //the received data is in process.
                CLOSING,
                //all the receive data had been processed, this connection had been freed
                CLOSED
            };
        public:
            CXConnectionObject();
            virtual ~CXConnectionObject();
            void Reset();
            void Lock();
            void UnLock();
            void Build(cxsocket sock,uint64 uiConnIndex,sockaddr_in addr);
			//record the time of the last received packet
            void RecordCurrentPacketTime();

            uint64   GetLastPacketTime(){return m_tmLastPacketTime;}
            uint64   GetAcceptedTime(){return m_tmAcceptTime;}
            cxsocket GetSocket(){return m_sock;}
            void   SetConnectionIndex(int iIndex) { m_uiConnectionIndex=iIndex; }
            uint64 GetConnectionIndex(){return m_uiConnectionIndex;}
            void   SetServer(void* pServer) { m_lpServer = pServer; }
            void*  GetServer() { return m_lpServer; }

            void   SetMemoryCache(CXMemoryCacheManager* pCache) { m_lpCacheObj = pCache; }
            CXMemoryCacheManager* GetMemoryCache() { return m_lpCacheObj; }

            PCXBufferObj GetCXBufferObj(DWORD dwBufSize=sizeof(CXBufferObj));
            void FreeCXBufferObj(PCXBufferObj pBuf) { m_lpCacheObj->FreeBuffer(pBuf); }

            void* GetBuffer(DWORD dwBufSize) { return m_lpCacheObj->GetBuffer(dwBufSize); }
            void  FreeBuffer(void* pBuf) { m_lpCacheObj->FreeBuffer(pBuf); }

            //push the received buffer to the end of the read buffer list
            bool  PushReceivedBuffer(PCXBufferObj pBufObj);

			//receive a buffer, add it to the read buffer list
			//parse the read buffer list, restructure the message packet
			//return value: ==-2 the read list has some error
			//              ==-3 occur error when allocat memory
			//              ==-4 received a incorrect packet
            int    RecvPacket(PCXBufferObj pBufObj,DWORD dwTransDataOfBytes, byte* pbyThreadCache,
				DWORD dwCacheLen,bool bLockBySelf = true);

            //verify the check sum of the message data
            bool   VerifyPacketBodyData(PCXPacketHeader pHeader,byte *pData);

            //post the message data to the user's message process level
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

            void SetState(CONNECTION_STATE emState) { m_nState= emState; }
            CONNECTION_STATE GetState() { return m_nState; }

            void Close(bool bLockBySelf = true);

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

            DWORD GetTimeOutMSeconds() { return m_dwTimeOutMSeconds; }

            void RecordReleasedTime();
            uint64 GetReleasedTime() { return m_tmReleasedTime; }

            // get the tick count value 
            uint64 GetTickCountValue();

			void SetJournalLogHandle(CXLog * handle) { m_pJouralLogHandle = handle; }
			CXLog *GetJournalLogHandle() { return m_pJouralLogHandle; }

			//pszTimeString: if not NULL,will save the time string , the format is: 2019-07-31_15:39:29
			//return value : the current time 
			int64  GetCurrentTimeMS(char *pszTimeString = NULL);

			//output the operation information to the journal log;
			void   OutputJournal(PCXMessageData pMes,int64 iBeginTimsMS);

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

            uint64 m_uiRecvBufferIndex;
            uint64 m_uiSendBufferIndex;

            uint64 m_uiLastProcessBufferIndex;

            //==0 initialize
            //==1 pending connection
            //==2 recv data packets
            //==3 closing
            //==4 closed
            CONNECTION_STATE  m_nState;

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

            // the number of the received buffers in the receiving buffer list
            uint64 m_uiNumberOfReceivedBufferInList;

            // the number of the received packets in the message queue
            uint64 m_uiNumberOfReceivedPacketInQueue;

            // the number of the buffer by posting to iocp model
            uint64 m_uiNumberOfPostBuffers;

            uint64 m_iProcessPacketNumber;

            void * m_pSessionsManager;
            void * m_pSession;

            CXLog *m_pLogHandle;
            CXDataParserImpl *m_pDataParserHandle;

            //the milliseconds of the connections timeout value
            //if the value is 0xffffffff, the connection will be not time out, the value show that the time is infinite
            DWORD  m_dwTimeOutMSeconds;

            // the released time of this connection
            // when a connection had been closed, release it
            uint64 m_tmReleasedTime;

			//record journal log
			CXLog  *m_pJouralLogHandle;
    };

}
#endif // CXCONNECTIONOBJECT_H
