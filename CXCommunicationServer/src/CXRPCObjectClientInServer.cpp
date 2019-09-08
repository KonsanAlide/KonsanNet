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

Description:
*****************************************************************************/
#include "CXRPCObjectClientInServer.h"
#include "CXCommonPacketStructure.h"
#include "CXFilePacketStructure.h"
#include "CXPacketCodeDefine.h"
#include "CXGuidObject.h"
#include <map>
#include <chrono>
#include <ctime>
#include "PlatformFunctionDefine.h"
#include "CXRPCObjectManager.h"
#include "CXCommunicationServer.h"
#ifdef WIN32
#else
#include <string.h>
#endif

using namespace std;
using namespace CXCommunication;
CXRPCObjectClientInServer::CXRPCObjectClientInServer()
{
	m_pAttachedConnection=NULL;
	m_pAttachedSession = NULL;
	m_strRemoteIp = "";
	m_wRemotePort = 0;
	m_bCreated = false;
    m_bAlignedIDWithServer = false;
}


CXRPCObjectClientInServer::~CXRPCObjectClientInServer()
{
	if (m_bCreated)
	{
		m_bCreated = false;
	}
}

int  CXRPCObjectClientInServer::Create(string strRemoteIp, WORD wRemotePort)
{
	if (m_pComServer == NULL)
	{
		return CXRET_INVALID_PARAMETER;
	}

    CXCommunicationServer* pComServer = (CXCommunicationServer*)m_pComServer;

    SetLogHandle(pComServer->GetLogHandle());
    SetJournalLogHandle(pComServer->GetJournalLogHandle());
    SetIOStat(pComServer->GetIOStatHandle());

	if (m_pAttachedConnection == NULL)
	{
		m_pAttachedConnection = pComServer->GetProxyConnection(strRemoteIp, wRemotePort);
		if (m_pAttachedConnection == NULL)
		{
			return CXRET_CREATE_SOCKET_FAILED;
		}
		m_pAttachedConnection->SetObjectID(m_byClassGuid);
		m_pAttachedSession = (CXConnectionSession*)m_pAttachedConnection->GetSession();
		if (m_pAttachedSession != NULL)
		{
			m_pAttachedSession->AddObject(this->GetObjectID(),(void*)this);
		}
		else
		{
			return CXRET_CREATE_SOCKET_FAILED;
		}
	}
	m_strRemoteIp = strRemoteIp;
	m_wRemotePort = wRemotePort;
	return CXRET_SUCCEED;
}

int CXRPCObjectClientInServer::AsyncConnect(string strNextRequestID)
{
	if (m_pComServer == NULL)
	{
		return CXRET_INVALID_PARAMETER;
	}
	if (m_pAttachedConnection->GetState()== CXConnectionObject::CREATING) //need to create
	{
		CXCommunicationServer* pComServer = (CXCommunicationServer*)m_pComServer;

		CXTaskPool* pTaskPool = (CXTaskPool*)pComServer->GetTaskPool();
		CXTaskCreateConnection *pTask = (CXTaskCreateConnection *)pComServer->GetCreateConnectionTask();
		if (pTask != NULL && pTaskPool != NULL)
		{
			//after logged to the peer, send the data in the sending list
			pTask->SetCreatingConnection(m_pAttachedConnection);
			pTask->SetFirstRequestID(strNextRequestID);
			pTask->SetRPCObj(this);
			pTaskPool->PushTask(pTask);
			return CXRET_ASYNC_CALLBACK;
		}
		else
		{
			m_pAttachedConnection->SetThisMesAsynchronous(false);
			return CXRET_ALLOCATE_TASK_FAILED;
		}
	}
	else
	{
		return CXRET_SUCCEED;
	}
}

void CXRPCObjectClientInServer::UnCreate()
{
    CXCommunicationServer* pComServer = (CXCommunicationServer*)m_pComServer;
    if (pComServer != NULL)
    {
        if (m_pAttachedConnection != NULL)
        {
            if (m_pAttachedSession != NULL)
            {
                m_pAttachedSession->RemoveObject(this->GetClassID());
            }

            m_pAttachedConnection->FreeSendingList();
            if (m_pAttachedConnection->GetNumberOfReceivedBufferInList() == 0
                && m_pAttachedConnection->GetPostedSentBuffersNumber() == 0
                && m_pAttachedConnection->GetNumberOfReceivedPacketInQueue() == 0)
            {
                pComServer->GetConnectionManager().DetachProxyConnection(m_pAttachedConnection);
            }
            else
            {
                m_pAttachedConnection->Lock();
                pComServer->CloseConnection(*m_pAttachedConnection, ERROR_IN_PROCESS, false);
                m_pAttachedConnection->UnLock();
            }

            m_pAttachedConnection = NULL;
        }
    }
}

int CXRPCObjectClientInServer::ReplyRequester(const CXRPCRequest& request, const CXRPCReply & rpcReply)
{
	if (m_pAttachedConnection == NULL)
	{
		return CXRET_INVALID_PARAMETER;
	}
	CXRPCObjectManager *pRPCObjPool = (CXRPCObjectManager *)m_pAttachedConnection->GetRPCObjectManager();
	if (pRPCObjPool == NULL)
	{
		return CXRET_INVALID_PARAMETER;
	}
    
    CXRPCObjectServer *pObj = (CXRPCObjectServer *)request.GetRequesterObject();
    if (pObj == NULL || (pObj != NULL && (!pObj->IsUsing() || pObj->GetObjectID() != request.GetRequesterObjectID())))
    {
        pObj = pRPCObjPool->FindUsingObject(request.GetRequesterObjectID());
        if (pObj == NULL || (pObj != NULL && (!pObj->IsUsing() || pObj->GetObjectID() != request.GetRequesterObjectID())))
        {
            char  szInfo[1024] = { 0 };
            sprintf_s(szInfo, 1024, "Failed to reply third-party data to the requested object, the object '%s' may have been closed.",
                request.GetRequesterObjectID().c_str());
            m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
            return CXRET_NOT_FOUND_REQUESTED_OBJ;
        }
    }

	return pObj->ReceiveInternalReplyData(request.GetRequestID(), request.GetReplyFunCode(), rpcReply);
	
	//return pCon->SendInternalReplyData(strPacketGUID, (byte*)pMes->bodyData.buf, dwDataLen, 0);
	
	return CXRET_SUCCEED;
}

void CXRPCObjectClientInServer::PushRequest(const CXRPCRequest& request)
{
	m_lock.Lock();
	map<string, CXRPCRequest>::iterator it = m_mapRequests.find(request.GetRequestID());
	if (it == m_mapRequests.end())
	{
		m_mapRequests[request.GetRequestID()]=request;
	}

	m_lock.Unlock();
}
CXRPCRequest CXRPCObjectClientInServer::PopRequest(string strRequestID)
{
	CXRPCRequest request;
	m_lock.Lock();
	map<string, CXRPCRequest>::iterator it = m_mapRequests.find(strRequestID);
	if (it != m_mapRequests.end())
	{
		request = it->second;
		m_mapRequests.erase(it);
	}
	m_lock.Unlock();
	return request;
}

int CXRPCObjectClientInServer::OnCreateConnectionError(const string &strRequestID, const string & strError)
{
    int iProcessRet = CXRET_SUCCEED;
	CXRPCRequest request = PopRequest(strRequestID);
	if (request.GetRequestID().length() > 0)
	{
		CXRPCReply reply;
		reply.SetReplyCode(503); //the server is not usable
		reply.SetReplyContent(strError);
        iProcessRet=ReplyRequester(request, reply);
	}
	return iProcessRet;
}

int CXRPCObjectClientInServer::OnSendDataError(const string &strRequestID, const string & strError)
{
	CXRPCRequest request = PopRequest(strRequestID);
	if (request.GetRequestID().length() > 0)
	{
		CXRPCReply reply;
		reply.SetReplyCode(503); //the server is not usable
		reply.SetReplyContent(strError);
		return ReplyRequester(request, reply);
	}
	return CXRET_SUCCEED;
}

int  CXRPCObjectClientInServer::SendPacketData(const CXRPCRequest& request, PCXBufferObj pBufObj, DWORD dwLen,
    DWORD dwMesCode, bool bLockBySelf)
{
    if (m_pAttachedConnection==NULL || pBufObj == NULL || dwLen == 0 || dwMesCode == 0)
    {
        return CXRET_INVALID_PARAMETER;
    }

    
    PushRequest(request);
    m_pAttachedConnection->LockSend();
    if (!m_bAlignedIDWithServer)
    {
        m_pAttachedConnection->PushRPCObj(request.GetRequestID(), (void*)this, false);
    }
    m_pAttachedConnection->SetRequestID(request.GetRequestIDBytes());
    bool bSentRet = m_pAttachedConnection->SendPacket(pBufObj, dwLen, dwMesCode);
    if (!bSentRet)
    {
        if (!m_bAlignedIDWithServer)
            m_pAttachedConnection->PopRPCObj(request.GetRequestID(), false);
        m_pAttachedConnection->UnlockSend();
        PopRequest(request.GetRequestID());
        return CXRET_SEND_DATA_FAILED;
    }

    if (m_pAttachedConnection->GetState() == CXConnectionObject::CREATING)
    {
        int iProcessRet = AsyncConnect(request.GetRequestID());
        if (CXRET_SUCCEED != iProcessRet && CXRET_ASYNC_CALLBACK != iProcessRet)
        {
            if (!m_bAlignedIDWithServer)
                m_pAttachedConnection->PopRPCObj(request.GetRequestID(), false);
            m_pAttachedConnection->UnlockSend();

            PopRequest(request.GetRequestID());
            m_pAttachedConnection->FreeSendingList();
            return iProcessRet;
        }
    }
    m_pAttachedConnection->UnlockSend();

    char szIndex[32] = { 0 };
    sprintf_s(szIndex, 32, "%lld", m_pAttachedConnection->GetConnectionIndex());
    string strPacketGUID = request.GetRequestID()+szIndex;
    m_pIOStatHandle->BeginIOStat("total", strPacketGUID, GetCurrentTimeMS());

    return CXRET_SUCCEED;
}

int CXRPCObjectClientInServer::GetSendBuffer(PCXBufferObj *ppOutCXBuf, byte **ppOutSendBuf, DWORD dwBufLen)
{
    *ppOutCXBuf = NULL;
    *ppOutSendBuf = NULL;
    *ppOutCXBuf = m_pAttachedConnection->GetSendCXBufferObj(dwBufLen);
    if (*ppOutCXBuf == NULL)
    {
        return CXRET_ALLOCATE_MEMORY_FAILED;
    }
    *ppOutSendBuf = m_pAttachedConnection->GetWritableBuffer(*ppOutCXBuf);
    if (*ppOutSendBuf == NULL)
    {
        m_pAttachedConnection->FreeSendCXBufferObj(*ppOutCXBuf);
        return CXRET_ALLOCATE_MEMORY_FAILED;
    }
    return CXRET_SUCCEED;
}

