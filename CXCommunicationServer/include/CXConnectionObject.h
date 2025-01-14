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

Description��
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
#include "CXIOStat.h"
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
				//this connection is creating
				CREATING,
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

			//sock: socket value, if is equal 0, show that this connection have not been created
            void Build(cxsocket sock,uint64 uiConnIndex, const string &strRemoteIp, WORD wRemotePort);

			//the socket have been created, set the accepted time
			void OnCreated(cxsocket sock);

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
            int    RecvPacket(PCXBufferObj pBufObj,DWORD dwTransDataOfBytes,bool bLockBySelf = true);

			//push received message to message queue
			void   PushReceivedMessage(PCXMessageData pMes,bool bAddRes=true);

            //verify the check sum of the message data
            bool   VerifyPacketBodyData(PCXPacketHeader pHeader,byte *pData);

            //post the message data to the user's message process level
            static int  OnProcessOnePacket(CXConnectionObject &conObj, PCXMessageData pMes);

            void  SetProcessOnePacketFun(POnProcessOnePacket pfn) { m_pfnProcessOnePacket= pfn; }

            int PostSend(byte *pData,int iDataLen);
            int PostSend(PCXBufferObj pBufObj, bool bLockBySelf = true);
            int PostSendBlocking(PCXBufferObj pBufObj, DWORD &dwSendLen, bool bLockBySelf = true);

            int PostRecv(int iBufSize);

            void SetObjectSizeInCache(int iObjectSize) { m_iObjectSizeInCache = iObjectSize; }

            void SetDispacherQueueIndex(int iIndex) { m_iDispacherQueueIndex= iIndex; }
            int  GetDispacherQueueIndex() { return m_iDispacherQueueIndex; }

            uint64 GetNumberOfReceivedBufferInList() { return m_uiNumberOfReceivedBufferInList; }

            uint64 GetNumberOfReceivedPacketInQueue() { return m_uiNumberOfReceivedPacketInQueue; }
            void  AddReceivedPacketNumber();
            void  ReduceReceivedPacketNumber();

            void  ReduceNumberOfPostedSentBuffers();
            void  ReduceNumberOfPostedRecvBuffers();

            uint64 GetPostedSentBuffersNumber() { return m_uiPostedSentBuffers; }
            uint64 GetPostedRecvBuffersNumber() { return m_uiPostedRecvBuffers; }

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
			//pbData: the data needed to send
            //dwLen:  the data size of the CXPacketBodyData.buf.
			//pMes:   the received message in this event from own connection, if NULL , this function will call by other object internally
            bool  SendPacket(const byte* pbData,DWORD dwLen,DWORD dwMesCode, PCXMessageData pMes = NULL, bool bLockBySelf = false);

			//send a packet to the peer
			//the pBufObj will be freed inside this function, the caller not need to free it
			//pBufObj: input packet buffer, must be allocate from memory cache
			//dwLen:   the data size of the CXPacketBodyData.buf.
			//pMes:    the received message in this event from own connection, if NULL , this function will call by other object internally
            bool  SendPacket(PCXBufferObj pBufObj, DWORD dwLen, DWORD dwMesCode, PCXMessageData pMes=NULL,
                bool bLockBySelf = false);

            //compress and encrypt the data, send it
			//the pBufObj will be freed inside this function, the caller not need to free it
			bool  PreparedAndSendPacket(PCXBufferObj pInputBuf, PCXMessageData pMes = NULL);

			//build the header and send the assembled packet, 
			//the pBufObj will be freed inside this function, the caller not need to free it
			bool  SendAssembledPacket(PCXBufferObj pBufObj, DWORD dwOrignDataLen);

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

			void SetJournalLogHandle(CXLog * handle) { m_pJournalLogHandle = handle; }
			CXLog *GetJournalLogHandle() { return m_pJournalLogHandle; }

			//pszTimeString: if not NULL,will save the time string , the format is: 2019-07-31_15:39:29
			//return value : the current time 
			int64  GetCurrentTimeMS(char *pszTimeString = NULL);

			//output the operation information to the journal log;
			void   OutputJournal(PCXMessageData pMes,int64 iBeginTimsMS);

			void   SetIOStat(CXIOStat * pHandle) { m_pIOStatHandle = pHandle; }
			CXIOStat*GetIOStat() { return m_pIOStatHandle; }

			uint64 GetLastProcessedMessageSequenceNum() { return m_uiLastProcessedMessageSequenceNum; }

			//if the message is not the next need-processed message, will add it in the list,
			//process the unpacked message,use the object to prcess the next message
			//pMes: maybe NULL, if need to the left message 
			int    ProcessUnpackedMessage(PCXMessageData pMes);

			//push this message in the message list, sorted by the sequence number of the message
			bool   PushMessage(PCXMessageData pMes);

			void SetRPCObjectManager(void * handle) { m_pRPCObjectManager = handle; }
			void *GetRPCObjectManager() { return m_pRPCObjectManager; }

			//push this buffer to in the sent list, sorted by the sequence number of the buffer
			bool   PushSendBuffer(PCXBufferObj pBufObj);

			void SetSocketKernel(void * handle) { m_pSocketKernel = handle; }

			// send the data in the sending list
			//return value:==-1 EAGIN error,
			//             ==-2 some error in sending,
			//             ==-3 socket had been closed by peer
			//             ==-4 failed to modify the epoll event for this connection
			int SendDataList();

			bool ParseMessage(PCXMessageData pMessageData, CXPacketHeader &packetHeader);

            //get a sending buffer to write data, use it to send,
            //if this packet need to compress or encrypt, convern the first thread cache to the PCXBufferObj
            PCXBufferObj GetSendCXBufferObj(DWORD dwSize);
            //if pBuf is the the first thread cache, not need to free
            void FreeSendCXBufferObj(PCXBufferObj pBuf);

            //get the writable data pointer in the CXBufferObj , use to write data
            byte* GetWritableBuffer(PCXBufferObj pBufObj);

            //==true, this connection is a proxy connection, start to third party by myself.
            bool IsProxyConnection() { return m_bProxyConnection; }
            void SetProxyConnection(bool bSet) { m_bProxyConnection= bSet; }

            string GetRemoteIP() { return m_strRemoteIP; }
            WORD   GetRemotePort() { return m_wRemotePort; }

			//when a asynchronous task had been finished, will call this function to process��
			//iTaskType: ==1 parsed data, uncompress and decrypt, pMes: NULL
			//           ==2 prepare data, compress and encrypt,
			//           ==3 time-consuming calculation
			//           ==4 connect to remote peer and sent data, pData: the packet guid string
            //pMes: == not empty, this message is myself received message, need to free it
            //      == NULL, strPacketGuid must be not empty, need to notify the another object to process
			int  CallbackTaskProcess(int iProcessRet, int iTaskType, PCXMessageData pMes=NULL, string strPacketGuid="");

			//when a asynchronous message process had been finished, will call this function to process 
			int  ProcessAsyncMessageEnd(PCXMessageData pMessageData);

			void SetEncryptType(CXDataParserImpl::CXENCRYPT_TYPE type) { m_encryptType= type; }
			void SetCompressType(CXDataParserImpl::CXCOMPRESS_TYPE type) { m_compressType = type; }
			CXDataParserImpl::CXENCRYPT_TYPE GetEncryptType() { return m_encryptType; }
			CXDataParserImpl::CXCOMPRESS_TYPE GetCompressType() { return m_compressType; }

			void SetTaskPool(void *pVoid) { m_pTaskPool=pVoid; }
			void*GetTaskPool() { return m_pTaskPool; }

			//==true: this message had been processed asynchronously
			bool IsThisMesAsynchronous(){ return m_bAsynchronouslyMes; }
            void SetThisMesAsynchronous(bool bSet) { m_bAsynchronouslyMes= bSet; }

			void SetAsynchronouslyPrepareData(bool bSet) { m_bAsyncPrepareData = bSet; }
			bool IsAsynchronouslyPrepareData() { return m_bAsyncPrepareData; }

			void SetContinuousProcessing(bool bSet) { m_bContinuousProcessing = bSet; }
			bool IsContinuousProcessing() { return m_bContinuousProcessing; }

			//push a key-value pair in the map
			void   PushRPCObj(string strPacketGuid, void* pObj, bool bLockBySelf = false);

			//pop the key-value pair from the map
			void*  PopRPCObj(string strPacketGuid, bool bLockBySelf = false);

            //get the rpc object from the map
            void*  GetRPCObj(string strPacketGuid, bool bLockBySelf = false);

			void   FreeSendingList();
			void   SetRequestID(byte byRequestID[]) { memcpy(m_byRequestID, byRequestID, CX_GUID_LEN); }
			void   SetObjectID(byte byObjectID[]) { memcpy(m_byObjectID, byObjectID, CX_GUID_LEN); }
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

			//the index of the last processed buffer
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
            string  m_strRemoteIP;
            WORD    m_wRemotePort;

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

            // the number of the sent buffer by posting to iocp model
            uint64 m_uiPostedSentBuffers;

            // the number of the received buffer posted to iocp model
            uint64 m_uiPostedRecvBuffers;

			//the sequence number of the last unpacked message,
			//this sequence number will be filled in the message structure CXMessageData,
			//will be used in the sequence number of the sent data list
            uint64 m_iLastUnpackedMessageSequenceNum;

			//the sequence number of the last processed message
			uint64 m_uiLastProcessedMessageSequenceNum;

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
			CXLog  *m_pJournalLogHandle;

			CXIOStat *m_pIOStatHandle;

			// the  header of the unpacked message list
			PCXMessageData m_plstMessageHead;

			// the end of the unpacked message list
			PCXMessageData m_plstMessageEnd;

			//rpc object manager 
			void *m_pRPCObjectManager;

			//socket server kernel object
			void *m_pSocketKernel;

			//the sequence number of the current sending packet
			uint64 m_uiCurSendingPacketSequenceNum;

			//the sequence number of the last sent packet
			uint64 m_uiLastSentPacketSequenceNum;

            CXDataParserImpl::CXENCRYPT_TYPE  m_encryptType;
            CXDataParserImpl::CXCOMPRESS_TYPE  m_compressType;

            //==true, this connection is a proxy connection, start to third party by myself.
            bool m_bProxyConnection;

			// the header of the inner message list
			PCXMessageData m_plstInnerMessageHead;

			// the end of the inner message list
			PCXMessageData m_plstInnerMessageEnd;

			//task pool pointer
			void *         m_pTaskPool;

			//use task pool to parse the received packet, uncompress and decrypt it asynchronously
			bool           m_bAsyncParseData;

			//use task pool to process the data needed to send, compress and encrypt it asynchronously
			bool           m_bAsyncPrepareData;

			// this message had been processed asynchronously
			bool           m_bAsynchronouslyMes;

			// this connection is continuous processing some message in a rpc object
			bool           m_bContinuousProcessing;

			// the continous rpc object pointer
			void          *m_pContinuousRPCObj;

			// the  decontinuous rpc object map
			// key: packet guid
			// value: rpc object pointer
			map<string, void*> m_mapRPCObj;

            // the internal session , another object request this connection for some thing,
            // this connection need to reply the another object
            // key:   the packet guid of the request
            // value: the connection index of the requested object
            map<string, int64> m_mapInternalPacket;

			byte           m_byObjectID[CX_GUID_LEN];
			byte           m_byRequestID[CX_GUID_LEN];
    };

}
#endif // CXCONNECTIONOBJECT_H
