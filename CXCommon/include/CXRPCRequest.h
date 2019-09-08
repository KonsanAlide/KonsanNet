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

Description:
*****************************************************************************/
#ifndef __CXRPCREQUEST_H__
#define __CXRPCREQUEST_H__
#ifdef WIN32
#include "windows.h"
#else
#include <string.h>
#endif

#include "PlatformDataTypeDefine.h"
#include "CXRPCReply.h"
#include <string>
#include <functional>
using namespace std;

namespace CXCommunication
{
	//typedef std::function<int(const string &strRequestID, DWORD dwReplyFunCode, const CXRPCReply & rpcReply)> CXRPCReplyFun;
	class CXRPCRequest
	{
	public:
		CXRPCRequest();
		CXRPCRequest(string strRequestID,DWORD dwReplyFunCode,string strRequesterObjectID, 
            byte byRequestIDBytes[],void *pRequesterObj);
		~CXRPCRequest();

		void   SetRequestID(string strID) { m_strRequestID = strID; }
        void   SetRequestIDBytes(byte byRequestIDBytes[]) { memcpy(m_byRequestIDBytes, byRequestIDBytes, CX_GUID_LEN); }
		void   SetReplyFunCode(DWORD dwReplyCode) { m_dwReplyFunCode = dwReplyCode; }
		void   SetRequesterObjectID(string strID) { m_strRequesterObjectID = strID; }
        void   SetRequesterObject(void *pRequesterObj) { m_pRequesterObj = pRequesterObj; }

		string GetRequestID() const { return m_strRequestID; }
        byte*  GetRequestIDBytes() const { return (byte*)&m_byRequestIDBytes; }
		DWORD  GetReplyFunCode() const { return m_dwReplyFunCode; }
		string GetRequesterObjectID() const { return m_strRequesterObjectID; }
        int64  GetBeginRequestTime()const { return m_iBeginRequestTime; }
        void  *GetRequesterObject() const { return m_pRequesterObj; }

		//==true : all attributes had been valid
		bool   IsParametersValid();

    protected:
        int64   GetCurrentTimeMSValue();

	private:
		string m_strRequestID;
		//the code of the callback function in the caller,
		DWORD  m_dwReplyFunCode;
		string m_strRequesterObjectID;
        byte   m_byRequestIDBytes[CX_GUID_LEN];
        int64  m_iBeginRequestTime;
        void  *m_pRequesterObj;
	};
}
#endif //__CXRPCREQUEST_H__

