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
#include "CXCommunicationServer.h"
#include "CXMessageProcessLevelBase.h"
#include "PlatformFunctionDefine.h"
#include <time.h>
//extern CXSpinLock g_lock;
//extern int g_iTotalCloseNum;
//extern int g_iTotalProcessCloseNum;

using namespace CXCommunication;

CXCommunicationServer::CXCommunicationServer()
{
    m_bRunning = false;
    m_uiTotalReceiveBuffers = 0;
    m_pDataParserHandle = NULL;

    m_pLogHandle = NULL;

	m_pJouralLogHandle = NULL;

    srand((unsigned)time(NULL));
}

CXCommunicationServer::~CXCommunicationServer()
{
    //dtor
}

int CXCommunicationServer::Start(unsigned short iListeningPort, int iWaitThreadNum)
{
    if (iListeningPort == 0 || iWaitThreadNum<1 || iWaitThreadNum>1000)
    {
        return INVALID_PARAMETER;
    }
    if (0 != m_networkInit.InitEnv())
    {
        return -7;
    }

    map<DWORD, DWORD>mapCacheConfig;
    mapCacheConfig[256] = 1024;
    mapCacheConfig[4096] = 1024;
    int nRet = m_memoryCacheManager.Init(mapCacheConfig);
    if (nRet != 0)
    {
        cout << "Failed to start memory cache manager" << endl;
        return -2;
    }

    m_connectionsManager.SetLogHandle(m_pLogHandle);

    m_socketServerKernel.SetOnReadCallbackFun(CXCommunicationServer::OnRecv);
    m_socketServerKernel.SetOnCloseCallbackFun(CXCommunicationServer::OnClose);
    m_socketServerKernel.SetOnAcceptCallbackFun(CXCommunicationServer::OnAccept);
    m_socketServerKernel.SetServer((void*)this);
    m_socketServerKernel.SetLogHandle(m_pLogHandle);

    int iMessageProcessObjInOneQueue =2;
    int iReadQueueNumber = 4;
    int iRet = m_dataDispathManager.Start(this, iReadQueueNumber);
    if (iRet != RETURN_SUCCEED)
    {
        Stop();
        return -3;
    }

    try
    {
        bool bSucceed = true;
        for (int i = 0; i<iReadQueueNumber; i++)
        {
            CXMessageQueue * pQueue = m_dataDispathManager.GetReadMessageQueue(i);
            for (int l = 0;l<iMessageProcessObjInOneQueue;l++)
            {
                CXMessageProcessLevelBase * pProcessObj = new CXMessageProcessLevelBase();
                pProcessObj->SetMessageQueue(pQueue);
				pProcessObj->SetLogHandle(m_pLogHandle);
                pProcessObj->SetRPCObjectManager(&m_rpcObjectManager);
                iRet = pProcessObj->Run();
                if (iRet != 0)
                {
                    bSucceed = false;
                    break;
                }
                else
                {
                    m_lstMessageProcess.push_back((void*)pProcessObj);
                }
            }

            if (!bSucceed)
                break;
        }
        if (!bSucceed)
        {
            for (int i = 0; i < iReadQueueNumber; i++)
            {
                CXMessageProcessLevelBase * pProcessObj = (CXMessageProcessLevelBase *)m_lstMessageProcess.front();
                m_lstMessageProcess.pop_front();
                pProcessObj->Stop();
                delete pProcessObj;
            }
            Stop();
            return -4;
        }
    }
    catch (const bad_alloc& e)
    {
        Stop();
        return -5;
    }

    m_connectionsManager.SetClosedCallbackFun(CXCommunicationServer::OnClose);
    if (!m_connectionsManager.StartDetectThread())
    {
        Stop();
        return -6;
    }

    if (m_socketServerKernel.Start(iListeningPort, iWaitThreadNum) != 0)
    {
        Stop();
        return -7;
    }
    m_bRunning = true;
    return RETURN_SUCCEED;
}

int CXCommunicationServer::Stop()
{
    m_socketServerKernel.Stop();
    m_connectionsManager.Stop();
    m_dataDispathManager.Stop();
    for (size_t i = 0; i < m_lstMessageProcess.size(); i++)
    {
        CXMessageProcessLevelBase * pProcessObj = (CXMessageProcessLevelBase *)m_lstMessageProcess.front();
        m_lstMessageProcess.pop_front();
        pProcessObj->Stop();
        delete pProcessObj;
    }
    m_bRunning = false;
    return RETURN_SUCCEED;
}

int  CXCommunicationServer::OnRecv(CXConnectionObject &conObj, PCXBufferObj pBufObj,DWORD dwTransDataOfBytes,
	byte* pbyThreadCache, DWORD dwCacheLen)
{
    //char szInfo[1024] = { 0 };
    //sprintf_s(szInfo, 1024, "OnRecv, Reveive a complete event ,dwNumberOfBytes=%d,pBufObj=%x,pBufObj->nOperate=%d\n",
    //    dwTransDataOfBytes, (DWORD)pBufObj, pBufObj->nOperate);
    //m_pLogHandle->Log(CXLog::CXLOG_INFO, szInfo);
    //printf_s(szInfo);

    //printf_s("OnRecv1:Reveive a complete event ,dwNumberOfBytes=%d,pBufObj=%x,pBufObj->nOperate=%d\n",
    //    dwTransDataOfBytes, (DWORD)pBufObj, pBufObj->nOperate);

    CXCommunicationServer * pComServer = (CXCommunicationServer*)conObj.GetServer();
    if (pComServer == NULL)
    {
        return -1;
    }
    pComServer->AddReceivedBuffers();

    //SetFileCompletionNotificationModes
    ConnectionClosedType emClosedType = SOCKET_CLOSED;
    bool bNeedClose = false;
    int iRet = conObj.PostRecv(CX_BUF_SIZE);
    if (iRet != 0)
    {
        bNeedClose = true;
        emClosedType = SOCKET_CLOSED;
    }
    //conObj.FreeCXBufferObj(pBufObj);
    //return 0;

    iRet = conObj.RecvPacket(pBufObj, dwTransDataOfBytes, pbyThreadCache, dwCacheLen);
    if (iRet != 0)
    {
        bNeedClose = true;
        emClosedType = ERROR_IN_PROCESS;
    }
    if (!bNeedClose)
    {
        if (conObj.GetState() == CXConnectionObject::CLOSING)//closing
        {
            bNeedClose = true;
        }
        else if (conObj.GetState() >= CXConnectionObject::CLOSED)//closed
        {
            return RETURN_SUCCEED;
        }
    }

    if (bNeedClose)
    {
        if (conObj.GetState() >= CXConnectionObject::CLOSED)//closed
        {
            return RETURN_SUCCEED;
        }

        char szInfo[1024] = {0};
        sprintf_s(szInfo,1024, "Failed to process the packet data,close this connection,connection id=%lld,error code is %d,desc is '%s'\n",
                  conObj.GetConnectionIndex(),errno,strerror(errno));
        pComServer->m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);

        pComServer->CloseConnection(conObj, emClosedType);
    }

    return RETURN_SUCCEED;
}

int  CXCommunicationServer::OnClose(CXConnectionObject &conObj, ConnectionClosedType emClosedType)
{
    if (conObj.GetState() >= CXConnectionObject::CLOSED)//closed
    {
        return RETURN_SUCCEED;
    }
    CXCommunicationServer * pComServer = (CXCommunicationServer*)conObj.GetServer();

    char szInfo[1024] = {0};
    sprintf_s(szInfo,1024, "Receive a commond to OnClose,close this connection,connection id=%lld,error code is %d,desc is '%s',closeType is %d\n",
                  conObj.GetConnectionIndex(),errno,strerror(errno),(int)emClosedType);
    //pComServer->m_pLogHandle->Log(CXLog::CXLOG_DEBUG, szInfo);

    pComServer->CloseConnection(conObj, emClosedType);

    return RETURN_SUCCEED;
}

void CXCommunicationServer::CloseConnection(CXConnectionObject &conObj, ConnectionClosedType emClosedType, bool bLockBySelf)
{
    bool bFreeConnection = false;

    if(bLockBySelf)
        conObj.Lock();

    char  szInfo[1024] = { 0 };
    CXConnectionSession * pSession = (CXConnectionSession *)conObj.GetSession();
    sprintf_s(szInfo, 1024, "Receive a command to close the connection , connection index is %lld,session is %x,connection is %x\n",
                    conObj.GetConnectionIndex(), pSession,&conObj);
    //m_pLogHandle->Log(CXLog::CXLOG_DEBUG, szInfo);

    //printf("Prepare to close connection ,connetcion id=%lld,connections=%lld,state:%d,post number=%lld,buffer in list=%lld,buffer in queue=%lld,emClosedType=%d\n",
    //     conObj.GetConnectionIndex(), m_connectionsManager.GetTotalConnectionsNumber(), conObj.GetState(),
    //    conObj.GetNumberOfPostBuffers(), conObj.GetNumberOfReceivedBufferInList(), conObj.GetNumberOfReceivedPacketInQueue(),emClosedType);

    if (conObj.GetState() == CXConnectionObject::CLOSED)
    {
        if (bLockBySelf)
            conObj.UnLock();
        return;
    }
    if (conObj.GetState() == CXConnectionObject::PENDING
        || conObj.GetState()== CXConnectionObject::ESTABLISHED)
    {
		GetSocketSeverKernel().DetachConnetionToModel(conObj);
    }


    conObj.Close(false);

    //conObj.LockRead();
    // if conObj.GetNumberOfPostBuffers() == 0 and conObj.GetNumberOfReceivedPacketInQueue() == 0
    // and the conObj.GetNumberOfReceivedBufferInList() !=0
    // there may be a situation that the left buffer contain one or more not complete packet,
    // how we process it?
    if (conObj.GetNumberOfReceivedBufferInList() == 0
        && conObj.GetNumberOfPostBuffers() == 0
        && conObj.GetNumberOfReceivedPacketInQueue() == 0)
    {
        conObj.SetState(CXConnectionObject::CLOSED);
        m_connectionsManager.RemoveUsingConnection(&conObj);

        
        //CXConnectionSession * pSession = (CXConnectionSession *)conObj.GetSession();
        if (pSession != NULL)
        {
            pSession->RemoveConnection(conObj);
            //m_sessionsManager.Lock();
            if (pSession->GetConnectionNumber() == 0)
            {
                char  szInfo[1024] = { 0 };
                sprintf_s(szInfo, 1024, "Close a session , guid is %s,connection index is %lld,session is %x,connection is %x\n",
                    pSession->GetSessionGuid().c_str(), conObj.GetConnectionIndex(), pSession,&conObj);
                m_pLogHandle->Log(CXLog::CXLOG_INFO, szInfo);

                m_sessionsManager.CloseSession(pSession);
            }
            //m_sessionsManager.UnLock();
        }

        bFreeConnection = true;
    }
    if (bLockBySelf)
        conObj.UnLock();
    if (bFreeConnection)
    {
        conObj.RecordReleasedTime();
        m_connectionsManager.ReleaseConnection(&conObj);
        //m_connectionsManager.AddFreeConnectionObj(&conObj);
        //printf("connection close ,connetcion id=%lld,connections=%lld,ClosedType=%d\n",
        //    conObj.GetConnectionIndex(), m_connectionsManager.GetTotalConnectionsNumber(), emClosedType);
    }

    //g_lock.Lock();
    //g_iTotalProcessCloseNum--;
    //g_lock.Unlock();

}

//have beed locked by CXConnectionObject::lock
int  CXCommunicationServer::OnAccept(void *pServer, cxsocket sock, sockaddr_in &addrRemote)
{
    CXCommunicationServer * pComServer = (CXCommunicationServer*)pServer;

    CXSocketServerKernel & SocketKernel = pComServer->GetSocketSeverKernel();
    CXConnectionsManager & ConnectionsManager = pComServer->GetConnectionManager();
    CXMemoryCacheManager & MemoryCacheManager = pComServer->GetMemoryCacheManger();
    CXDataDispathLevelImpl & DataDispather = pComServer->GetDataDispathManger();

    CXConnectionObject *pConObj = ConnectionsManager.GetFreeConnectionObj();
    //CXElasticMemoryCache *pCache = MemoryCacheManager.GetMemoryCache(sizeof(CXBufferObj));

    CXSessionsManager & sessionsManager = pComServer->GetSessionsManager();

    //PSocketObj  pObj = m_connectionManager.AddPendingConnection(nAcceptSock,(void*)this);
    if (pConObj != NULL )
    {
        pConObj->Build(sock, ConnectionsManager.GetCurrentConnectionIndex(), addrRemote);
        pConObj->SetServer(pServer);
        pConObj->SetObjectSizeInCache(sizeof(CXBufferObj));
        pConObj->SetProcessOnePacketFun(CXCommunicationServer::OnProcessOnePacket);
        pConObj->SetSessionsManager(&sessionsManager);
        pConObj->SetLogHandle(pComServer->GetLogHandle());
		pConObj->SetJournalLogHandle(pComServer->GetJournalLogHandle());


        //char *str = inet_ntoa(sockRemote.sin_addr);
        //cout << "accept a connection from " << str << endl;

        int iQueueIndex = rand()%DataDispather.GetQueueNumber();
        pConObj->SetDispacherQueueIndex(iQueueIndex);
        pConObj->SetMemoryCache(&MemoryCacheManager);

        int iRet = SocketKernel.AttachConnetionToModel(*pConObj);
        if (iRet == 0)
        {
            ConnectionsManager.AddUsingConnection(pConObj);
#ifdef WIN32
            bool bNeedClose = false;
            //when accept a socket, post some
            for (int i = 0; i<1; i++)
            {
                int iRet = pConObj->PostRecv(sizeof(CXBufferObj));
                if (iRet != RETURN_SUCCEED)
                {
                    bNeedClose = true;
                    break;
                }
            }
            if (bNeedClose)
            {
                pComServer->CloseConnection(*pConObj, SOCKET_CLOSED);
            }
#endif
        }
        else
        {
            pConObj->Close();
            pConObj->SetState(CXConnectionObject::CLOSED);
            //pComServer->CloseConnection(*pConObj);
            ConnectionsManager.AddFreeConnectionObj(pConObj);
            return -4;
        }
    }
    else
    {
        closesocket(sock);
        if (pConObj!=NULL)
        {
            pConObj->Close();
            pConObj->SetState(CXConnectionObject::CLOSED);
            ConnectionsManager.AddFreeConnectionObj(pConObj);
        }

        return -2;
    }

    //ConnectionsManager.AddUsingConnection(pConObj);


    //printf( "accept a connection from %s,connection id =%I64i\n",
    //    inet_ntoa(addrRemote.sin_addr), pConObj->GetConnectionIndex());

    return RETURN_SUCCEED;
}

//have beed locked by CXConnectionObject::lock
int  CXCommunicationServer::OnWrite(CXConnectionObject &conObj)
{
    return RETURN_SUCCEED;
}

int  CXCommunicationServer::OnProcessOnePacket(CXConnectionObject &conObj, PCXMessageData pMes)
{
    CXCommunicationServer * pComServer = (CXCommunicationServer*)conObj.GetServer();
    CXDataDispathLevelImpl & DataDispather = pComServer->GetDataDispathManger();
    CXMessageQueue * pQueue = DataDispather.GetReadMessageQueue(conObj.GetDispacherQueueIndex());
    CXDataParserImpl * pHandle = pComServer->GetDataParserHandle();
    if (pHandle !=NULL)
    {
        //pHandle->ParseData();
    }
    pQueue->PushMessage((void*)pMes);
    return RETURN_SUCCEED;
}

void CXCommunicationServer::AddReceivedBuffers()
{
    m_lock.Lock();
    m_uiTotalReceiveBuffers++;
    m_lock.Unlock();
}

