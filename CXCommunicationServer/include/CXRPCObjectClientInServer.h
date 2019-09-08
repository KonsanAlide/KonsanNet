/****************************************************************************
Copyright (c) 2018-2019 Chance Yang

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
#ifndef __CXRPCOBJECTCLIENTINSERVER_H__
#define __CXRPCOBJECTCLIENTINSERVER_H__

#include "CXRPCObjectServer.h"
#include "PlatformDataTypeDefine.h"
#include <string>
#include "CXRPCRequest.h"
#include "CXRPCReply.h"
using namespace std;

namespace CXCommunication
{
    class CXRPCObjectClientInServer: public CXRPCObjectServer
    {
    public:
        enum CX_RPC_OBJECT_OPENTYPE
        {
            modeRead=0,
            modeWrite,
            modeReadWrite
        };

    public:
		CXRPCObjectClientInServer();
        virtual ~CXRPCObjectClientInServer();

        int          Create(string strRemoteIp, WORD wRemotePort);
		int          AsyncConnect(string strNextRequestID);
		void         UnCreate();
		bool         IsCreated() { return m_bCreated; }

		int          ReplyRequester(const CXRPCRequest& request, const CXRPCReply & rpcReply);
		int          OnCreateConnectionError(const string &strRequestID,const string & strError);
		int          OnSendDataError(const string &strRequestID, const string & strError);

		CXConnectionObject  *GetAttachConnection() { return m_pAttachedConnection; }
		CXConnectionSession *GetAttachSession() { return m_pAttachedSession; }

        virtual int  SendPacketData(const CXRPCRequest& request, PCXBufferObj pBufObj, DWORD dwLen,
                              DWORD dwMesCode, bool bLockBySelf = false);

        int          GetSendBuffer(PCXBufferObj *ppOutCXBuf,byte **ppOutSendBuf,DWORD dwBufLen);

        bool         IsAlignedIDWithServer() { return m_bAlignedIDWithServer; }
        void         SetAlignedIDWithServer(bool bSet) { m_bAlignedIDWithServer= bSet; }
        
	protected:
		void         PushRequest(const CXRPCRequest& request);
		//if not found the request object, the object.GetRequestID() is empty.
		CXRPCRequest PopRequest(string strRequestID);

    protected:
		bool                 m_bCreated;
		string               m_strRemoteIp;
		WORD                 m_wRemotePort;
		CXConnectionObject * m_pAttachedConnection;
		CXConnectionSession* m_pAttachedSession;

		//key: the request id
		map<string, CXRPCRequest> m_mapRequests;
        bool                 m_bAlignedIDWithServer;
    };
}

#endif
