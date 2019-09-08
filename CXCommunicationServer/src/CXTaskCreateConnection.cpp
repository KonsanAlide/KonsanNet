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
#include "CXTaskCreateConnection.h"
#include "CXCommunicationServer.h"
#include "CXSessionPacketStructure.h"
#include "CXGuidObject.h"
#include "CXPacketCodeDefine.h"
#include "CXClientSocketChannel.h"
#include "CXRPCObjectClientInServer.h"
#include "CXConnectionSession.h"
#include "PlatformFunctionDefine.h"
using namespace CXCommunication;
CXTaskCreateConnection::CXTaskCreateConnection()
{
	m_taskType = CX_RUNING_TASK_TYPE_CREATE_CONNECITON;
	Reset();
}


CXTaskCreateConnection::~CXTaskCreateConnection()
{
}


CXTaskBase* CXTaskCreateConnection::CreateObject()
{
	CXTaskBase* pTask = NULL;
	try
	{
		pTask = new CXTaskCreateConnection();
	}
	catch (...)
	{
	}

	return pTask;
}


DWORD CXTaskCreateConnection::ProcessTask()
{
    char  szInfo[1024] = { 0 };
	CXCommunicationServer *pComServer = (CXCommunicationServer*)m_pComServer;
	CXConnectionObject *   pConObj= m_pConnection;
	CXClientSocketChannel  client;

	client.SetUsedMemoryCachePool(true,m_lpCacheObj);
    if (pComServer != NULL)
    {
        client.SetDataParserHandle(pComServer->GetDataParserHandle());
    }

	CXRPCObjectClientInServer * pRPCObject = (CXRPCObjectClientInServer*)m_pRPCObject;

    int iProcessRet = 0;
	bool bSucceed = false;
	int iFunRet = client.Create();
	if (0== iFunRet)
	{
		CXSocketAddress addressLocal(m_strLocalIp.c_str(), m_wLocalPort);
        if (m_wLocalPort != 0)
        {
            iFunRet = client.Bind(addressLocal);
        }
		
		if (0 == iFunRet)
		{
			CXSocketAddress addressRemote(m_strRemoteIp.c_str(), m_wRemotePort);
			iFunRet = client.Connect(addressRemote);
			if (0 == iFunRet)
			{
				client.SetEncryptParas(pConObj->GetEncryptType());
				client.SetCompressParas(pConObj->GetCompressType());
				if (m_bUsedKeyAuth)
				{
					iFunRet = client.Login(m_strKey, "", 2, 1);
				}
				else
				{
					iFunRet = client.Login(m_strUserName, m_strPwd, 0, 1);
				}
				if (0 == iFunRet)
				{
					bSucceed = true;
				}
                else
                {
                    sprintf_s(szInfo, 1024, "Failed to log in the remote server %s\n", addressRemote.GetAddressString().c_str());
                    iProcessRet = CXRET_CONNECTION_LOGIN_FAILED;
                }
			}
            else
            {
				sprintf_s(szInfo, 1024, "Failed to connect to the remote server %s\n", addressRemote.GetAddressString().c_str());
                iProcessRet = CXRET_CONNECT_REMOTE_FAILED;
            }
		}
        else
        {
			sprintf_s(szInfo, 1024, "Failed to bind the proxy socket to the local address %s\n", addressLocal.GetAddressString().c_str());
            iProcessRet = CXRET_BIND_ADDR_FAILED;
        }
	}
    else
    {
		sprintf_s(szInfo, 1024, "Failed to create a proxy socket in the local address %s:%d\n", m_strLocalIp.c_str(), m_wLocalPort);
        iProcessRet = CXRET_CREATE_SOCKET_FAILED;
    }

	if (bSucceed)
	{
		client.SetCloseInDeconstruction(false);
		if (pConObj != NULL)
		{
			pConObj->OnCreated(client.GetSocket());
		}
		
		iFunRet = pComServer->OnProxyConnectionCreated(pConObj);
		if (0 == iFunRet)
		{
			if (m_strRequestID.length()!=0 && m_pRPCObject!=NULL)
			{
                pConObj->LockSend();
                pConObj->SetAsynchronouslyPrepareData(false);
				if(0!=pConObj->SendDataList())
				{
                    pConObj->UnlockSend();
                    iProcessRet = CXRET_SEND_DATA_FAILED;
				}
                else
                {
                    pConObj->UnlockSend();
                }
                pConObj->SetAsynchronouslyPrepareData(true);
			}
		}
        else
        {
            iProcessRet = CXRET_BUILD_CONNECTION_FAILED;
        }
	}
	else
	{
		client.Close();
	}

    if (iProcessRet != CXRET_SUCCEED)
    {
		CXRPCObjectClientInServer * pRPCObject = (CXRPCObjectClientInServer*)m_pRPCObject;
        if (pRPCObject != NULL)
        {
            //m_pParentMes->iProcessRet = iProcessRet;
			string strError = szInfo;
			pRPCObject->OnCreateConnectionError(m_strRequestID, strError);
        }
    }

	//"Failed to notify the third party to read file"

	return (DWORD)iProcessRet;
}

void CXTaskCreateConnection::SetAddrInfo(const string &strRemoteIp, WORD wRemotePort,
	const string &strLocalIp, WORD wLocalPort, void *pComServer)
{
	m_strRemoteIp = strRemoteIp;
	m_wRemotePort = wRemotePort;
	m_strLocalIp = strLocalIp;
	m_wLocalPort = wLocalPort;
	m_pComServer = pComServer;

	if (pComServer != NULL && strRemoteIp.length() != 0 && m_wRemotePort!=0)
	{
		m_bSetParas = true;
	}
}

void CXTaskCreateConnection::Reset()
{
	m_strRemoteIp = "";
	m_wRemotePort = 0;
	m_strLocalIp = "";
	m_wLocalPort = 0;
	m_pComServer = NULL;
	m_strUserName = "";
	m_strPwd = "";
	m_strKey = "";
	m_bUsedKeyAuth = false;
	m_pRPCObject = NULL;
	m_strRequestID = "";
	m_pConnection = NULL;
}

void CXTaskCreateConnection::SetUserInfo(const string &strUserName, const string &strPwd)
{
	m_strUserName = strUserName;
	m_strPwd = strPwd;
}
void CXTaskCreateConnection::SetKey(const string &strKey, bool bUsedKeyAuth)
{
	m_strKey = strKey;
	m_bUsedKeyAuth = bUsedKeyAuth;
}