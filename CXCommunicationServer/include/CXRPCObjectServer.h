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
#include <string>
using namespace std;
namespace CXCommunication
{
    class CXRPCObjectServer
    {
    public:
		CXRPCObjectServer();
        virtual ~CXRPCObjectServer();

        virtual string GetObjectName() { return "CXRPCObjectV1"; }

        virtual int ProcessMessage(PCXMessageData pMes) = 0;

        virtual int SendData(CXConnectionObject * pCon, const byte *pbyData, DWORD dwDataLen) = 0;

        virtual CXRPCObjectServer* CreateObject()=0;

        virtual void Destroy() = 0;

        virtual void RecordSlowOps(PCXMessageData pMes) = 0;

        string  GetGuid() { return m_strObjectGuid; }

        //the login object can be unique instance
        bool    IsUniqueInstance() { return m_bIsUniqueInstance; }

        void    SetTimeOut(DWORD dwSecs) { m_dwTimeoutSecs = dwSecs; }

        void    SetSlowOpsMS(DWORD dwMilliseconds) { m_dwSlowOpsMS = dwMilliseconds; }

        //pszTimeString: if not NULL,will save the time string , the format is: 2019-07-31_15:39:29
        //return value : the current time 
        int64   GetCurrentTimeMS(char *pszTimeString = NULL);

    protected:
        void     GetObjectGuid();

    protected:
        bool     m_bIsOpened;
        string   m_strObjectGuid;
        byte     m_byObjectGuid[CX_GUID_LEN];
        bool     m_bIsUniqueInstance;
        DWORD    m_dwTimeoutSecs;
        //if the process time of a operation is larger than  m_dwSlowOpsSecs milliseconds,
        //this operation is a slow operation ,must record it to log
        DWORD    m_dwSlowOpsMS;
    };
}
#endif // __CXRPCOBJECTSERVER_H__
