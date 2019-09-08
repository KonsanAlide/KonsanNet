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
#ifndef __CXTASK_CREATE_CONNECTION_H__
#define __CXTASK_CREATE_CONNECTION_H__
#include "CXServerStructDefine.h"
#include "CXTaskBase.h"
#include "CXCommonPacketStructure.h"
#include "CXConnectionObject.h"
namespace CXCommunication
{
	class CXTaskCreateConnection :public CXTaskBase
	{
	public:
		CXTaskCreateConnection();
		virtual ~CXTaskCreateConnection();
		virtual CXTaskBase* CreateObject();
		virtual DWORD ProcessTask();
		virtual void  Reset();

		void SetAddrInfo(const string &strRemoteIp, WORD wRemotePort,
			const string &strLocalIp, WORD wLocalPort,void *pComServer);
		void SetUserInfo(const string &strUserName, const string &strPwd);
		void SetKey(const string &strKey, bool bUsedKeyAuth);
		void SetRPCObj(void *pRPCObj) { m_pRPCObject = pRPCObj; }
		void SetFirstRequestID(string strRequestID) { m_strRequestID = strRequestID; }
		void SetCreatingConnection(CXConnectionObject *pCon) { m_pConnection = pCon; }
	private:
		string m_strRemoteIp;
		WORD   m_wRemotePort;
		string m_strLocalIp;
		WORD   m_wLocalPort;
		void * m_pComServer;
		string m_strUserName;
		string m_strPwd;
		string m_strKey;
		bool   m_bUsedKeyAuth;
		string m_strRequestID;
        void * m_pRPCObject;
		CXConnectionObject *m_pConnection;
		
	};
}
#endif // __CXTASK_CREATE_CONNECTION_H__

