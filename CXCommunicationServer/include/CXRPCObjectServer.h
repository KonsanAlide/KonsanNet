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
#ifndef __CXRPCOBJECTSERVER_H__
#define __CXRPCOBJECTSERVER_H__

#include "CXServerStructDefine.h"
#include "CXCommonPacketStructure.h"
#include "CXConnectionObject.h"
#include "CXConnectionSession.h"
#include "CXGuidObject.h"
#include "CXLog.h"
#include <string>
#include <map>
#include <deque>
#include <functional>
#include "CXIOStat.h"
#include "CXSpinLock.h"
#include "CXRPCReply.h"
using namespace std;
namespace CXCommunication
{
    
    typedef std::function<int(PCXMessageData pMes, CXConnectionObject *pCon, const CXRPCReply & rpcReply)> ReplyFun;
    typedef std::function<int(PCXMessageData pMes, CXConnectionObject *pCon)> EntryFun;

    typedef struct CX_REQUEST_THIRD_OBJ {
        string   strObjectName;
        DWORD    dwMessageCode;
        byte    *pbySendData;
        DWORD    dwSendDataLen;
        DWORD    dwNextState;

        CX_REQUEST_THIRD_OBJ()
        {
            strObjectName = "";
            dwMessageCode = 0;
            pbySendData = NULL;
            dwSendDataLen = 0;
            dwNextState = 0;
        }
    }CXRequstThirdObj,*PCXRequstThirdObj;

    class CXRPCObjectServer
    {
    public:
		CXRPCObjectServer();
        virtual ~CXRPCObjectServer();

		//generate a new object id
		void           Init();
		void           Reset();
		string         GetClassID() { return m_strClassGuid; }

        virtual string GetObjectClassName() { return "CXRPCObjectV1"; }

		//process the received message
        virtual int    ProcessMessage(PCXMessageData pMes);

		//this function will be called by the ProcessMessage function,
        virtual int    DispatchMes(PCXMessageData pMes)=0;

        virtual int    SendData(CXConnectionObject * pCon, const byte *pbyData, DWORD dwDataLen) = 0;

        virtual CXRPCObjectServer* CreateObject()=0;

		//close this object, let it to use again
        virtual void   Destroy() = 0;

		//serialize the message to a string and output to the journal log
        virtual void   MessageToString(PCXMessageData pMes,string &strMes) = 0;

        string         GetObjectID() { return m_strObjectGuid; }
        byte *         GetObjectIDBytes() { return m_byObjectGuid; }

        //the login object can be unique instance
        bool    IsUniqueInstance() { return m_bIsUniqueInstance; }

        void    SetTimeOut(DWORD dwSecs) { m_dwTimeoutSecs = dwSecs; }

        void    SetSlowOpsMS(DWORD dwMilliseconds) { m_dwSlowOpsMS = dwMilliseconds; }

        //pszTimeString: if not NULL,will save the time string , the format is: 2019-07-31_15:39:29
        //return value : the current time 
        int64   GetCurrentTimeMS(char *pszTimeString = NULL);

		void    SetLogHandle(CXLog * handle) { m_pLogHandle = handle; }
		void    SetJournalLogHandle(CXLog * handle) { m_pJournalLogHandle = handle; }
		void    SetIOStat(CXIOStat * pHandle) { m_pIOStatHandle = pHandle; }
		void    OutputResult(PCXMessageData pMes, string strPacketGuid, bool bTimeout = false);
		
		void    AsynCallbackResult(PCXMessageData pMes);

		void    SetContinuousObject(bool bSet) { m_bContinuousObject = bSet; }
		bool    IsContinuousObject() { return m_bContinuousObject; }

		string  GetPacketGuid(PCXMessageData pMes);

		// receive a reply from the other object
		int     ReceiveInternalReplyData(const string &strRequestID,
			            DWORD dwReplyFunCode, const CXRPCReply & rpcReply);

        void    RegisterStepFun(DWORD dwStateCode, ReplyFun fun);

        void    RegisterEntryFun(DWORD dwStateCode, EntryFun fun);

		void    DetectTimeoutRequests();

		bool    IsUsing() { return m_bIsUsing; }

        void    SetCommunicationServer(void *pServer) { m_pComServer = pServer; }

        void    SetNeedToDestroy(bool bSet) { m_bNeedToDestroy = bSet; }
        bool    IsNeedToDestroy() { return m_bNeedToDestroy; }

    protected:
		void    GetClassGuid();

        void    PushState(string strPacketGuid, void* pMes);
        void*   PopState(string strPacketGuid);

    protected:
		bool     m_bIsUsing;
		string   m_strClassGuid;
        string   m_strObjectGuid;
		byte     m_byClassGuid[CX_GUID_LEN];
        byte     m_byObjectGuid[CX_GUID_LEN];
        bool     m_bIsUniqueInstance;
        DWORD    m_dwTimeoutSecs;
        //if the process time of a operation is larger than  m_dwSlowOpsSecs milliseconds,
        //this operation is a slow operation ,must record it to log
        DWORD    m_dwSlowOpsMS;
		CXLog   *m_pLogHandle;
		CXLog   *m_pJournalLogHandle;

		// this object need to continously process some message
		bool     m_bContinuousObject;
		CXIOStat*m_pIOStatHandle;

		//key: the guid of the packet
		map<string,deque<void*>> m_mapStateMachine;
		CXSpinLock m_lock;

        //key: the index of the step in the state machine,
        //value: the pointer of the function of the step
        map<DWORD, ReplyFun> m_mapStepFuns;

        //key: the message code,
        //value: the pointer of the function of the entry function
        map<DWORD, EntryFun> m_mapEntryFuns;

        void                *m_pComServer;

        bool                 m_bNeedToDestroy;
    };
}
#endif // __CXRPCOBJECTSERVER_H__
