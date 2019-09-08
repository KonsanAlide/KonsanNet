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
#include "CXTaskCreateConnection.h"
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

	m_dwTaskPoolSize = 128;
	m_dwInitTaskQueueSize = 10240;

	m_bAsyncParseData = true;

	//use task pool to process the data needed to send, compress and encrypt it asynchronously
	m_bAsyncPrepareData = true;
}

CXCommunicationServer::~CXCommunicationServer()
{
	Stop();
}

int CXCommunicationServer::Start(unsigned short iListeningPort, int iWaitThreadNum)
{
    if (iListeningPort == 0 || iWaitThreadNum<1 || iWaitThreadNum>1000)
    {
        return INVALID_PARAMETER;
    }
	char szInfo[1024] = { 0 };
    if (0 != m_networkInit.InitEnv())
    {
		DWORD dwError = GetLastError();
		sprintf_s(szInfo, 1024, "Failed to call the WSAStartup() ,error code is %d\n", dwError);
		m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
        return -8;
    }

    map<DWORD, DWORD>mapCacheConfig;
    mapCacheConfig[256] = 1024;
    mapCacheConfig[4096] = 1024;
    mapCacheConfig[524288] = 64;
    mapCacheConfig[1048576] = 32;
    int nRet = m_memoryCacheManager.Init(mapCacheConfig);
    if (nRet != 0)
    {
        DWORD dwError = GetLastError();
        sprintf_s(szInfo, 1024, "Failed to create the memory cache pool ,error code is %d\n", dwError);
        m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
        return -2;
    }

    m_ioStat.SetLogHandle(m_pLogHandle);
	m_ioStat.Start();

    m_connectionsManager.SetLogHandle(m_pLogHandle);
	m_connectionsManager.SetObjectPool((void*)&m_rpcObjectManager);
	

    m_socketServerKernel.SetOnReadCallbackFun(CXCommunicationServer::OnRecv);
    m_socketServerKernel.SetOnCloseCallbackFun(CXCommunicationServer::OnClose);
    m_socketServerKernel.SetOnAcceptCallbackFun(CXCommunicationServer::OnAccept);
    m_socketServerKernel.SetServer((void*)this);
    m_socketServerKernel.SetLogHandle(m_pLogHandle);

	int iMessageProcessObjInOneQueue = 4;
	int iReadQueueNumber = 4;

	int iMesThreadPoolNum = (iMessageProcessObjInOneQueue * iReadQueueNumber)*2;
	if (!m_threadPoolDispatch.CreatePool(iMesThreadPoolNum))
	{
        DWORD dwError = GetLastError();
        sprintf_s(szInfo, 1024, "Failed to create the thread pool to process message ,error code is %d\n", dwError);
        m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
		return -10;
	}
    
    if (0!=m_taskPool.Create(m_dwTaskPoolSize, m_dwInitTaskQueueSize,&m_memoryCacheManager,true,1024*1024))
    {
        DWORD dwError = GetLastError();
        sprintf_s(szInfo, 1024, "Failed to create the task pool to run time-consuming tasks ,error code is %d\n", dwError);
        m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
        return -11;
    }
	m_taskPool.RegisterTaskClass(&m_taskCreateConnectionBase);
	m_taskPool.RegisterTaskClass(&m_taskParseMessageBase);
	m_taskPool.RegisterTaskClass(&m_taskPrepareMessageBase);

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
				CXThread* pThread = m_threadPoolDispatch.GetFreeThread();
				if (pThread != NULL)
				{
					if (0 != pThread->Start(pProcessObj->Run, (void*)pProcessObj))
					{
						bSucceed = false;
						break;
					}
				}
				else
				{
					bSucceed = false;
					break;
				}
                
                m_lstMessageProcess.push_back((void*)pProcessObj);  
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
	m_threadPoolDispatch.Destroy();
    m_bRunning = false;
	m_ioStat.Stop();
    return RETURN_SUCCEED;
}

int  CXCommunicationServer::OnRecv(CXConnectionObject &conObj, PCXBufferObj pBufObj,DWORD dwTransDataOfBytes)
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

    char szInfo[1024] = { 0 };
    uint64 uiIndex = pComServer->AddReceivedBuffers();
    sprintf_s(szInfo, 1024, "Begin to receive packet,SequenceNum:%lld, connection index:%lld\n",
        uiIndex, conObj.GetConnectionIndex());
    //pComServer->m_pLogHandle->Log(CXLog::CXLOG_DEBUG, szInfo);

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

    iRet = conObj.RecvPacket(pBufObj, dwTransDataOfBytes);
    if (iRet != 0)
    {
        bNeedClose = true;
        emClosedType = ERROR_IN_PROCESS;
    }

    sprintf_s(szInfo, 1024, "End to receive packet,SequenceNum:%lld, connection index:%lld\n",
        uiIndex, conObj.GetConnectionIndex());
    //pComServer->m_pLogHandle->Log(CXLog::CXLOG_DEBUG, szInfo);

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

    /*
    sprintf_s(szInfo, 1024, "Prepare to close connection ,connetcion id=%lld,connections=%lld,state:%d,post_sent number=%lld,post_recv number=%lld,buffer in list=%lld,buffer in queue=%lld,emClosedType=%d\n",
         conObj.GetConnectionIndex(), m_connectionsManager.GetTotalConnectionsNumber(), conObj.GetState(),
        conObj.GetPostedSentBuffersNumber(), conObj.GetPostedRecvBuffersNumber(), conObj.GetNumberOfReceivedBufferInList(),
		conObj.GetNumberOfReceivedPacketInQueue(),emClosedType);
    m_pLogHandle->Log(CXLog::CXLOG_DEBUG, szInfo);
    */
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
        && conObj.GetPostedSentBuffersNumber() == 0
        && conObj.GetPostedRecvBuffersNumber() == 0
        && conObj.GetNumberOfReceivedPacketInQueue() == 0)
    {
        conObj.SetState(CXConnectionObject::CLOSED);

        char  szInfo[1024] = { 0 };
        string strSessionID = "";
        if (pSession != NULL)
        {
            strSessionID = pSession->GetSessionGuid();
            pSession->RemoveConnection(conObj);
            if (pSession->GetConnectionNumber() == 0)
            {
                sprintf_s(szInfo, 1024, "Session closed, session_id:%s\n",pSession->GetSessionGuid().c_str());
                m_pLogHandle->Log(CXLog::CXLOG_INFO, szInfo);

                m_sessionsManager.CloseSession(pSession);
            }
        }

        conObj.RecordReleasedTime();
        m_connectionsManager.ReleaseConnection(&conObj);

        sprintf_s(szInfo, 1024, "Connection closed, session_id:%s, connection_id:%lld, running_connections_nubmer:%lld\n",
            strSessionID.c_str(), conObj.GetConnectionIndex(),
            m_connectionsManager.GetTotalConnectionsNumber());
        m_pLogHandle->Log(CXLog::CXLOG_INFO, szInfo);
    }
    if (bLockBySelf)
        conObj.UnLock();

    //g_lock.Lock();
    //g_iTotalProcessCloseNum--;
    //g_lock.Unlock();
}

//have beed locked by CXConnectionObject::lock
int  CXCommunicationServer::OnAccept(void *pServer, cxsocket sock, sockaddr_in &addrRemote)
{
    if (pServer == NULL)
    {
        return INVALID_PARAMETER;
    }

    CXCommunicationServer * pComServer = (CXCommunicationServer*)pServer;
    //ConnectionsManager.AddUsingConnection(pConObj);
    char szBuffer[64] = { 0 };
    byte *bytes = (byte*)&addrRemote.sin_addr;
    sprintf_s(szBuffer, 64, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);
    string strRemoteIP = szBuffer;
    WORD wRemotePort = ntohs(addrRemote.sin_port);

	CXConnectionObject *pConObj = NULL;

    //printf( "accept a connection from %s,connection id =%I64i\n",
    //    inet_ntoa(addrRemote.sin_addr), pConObj->GetConnectionIndex());

    return pComServer->BuildConnection(sock, strRemoteIP, wRemotePort,&pConObj);
}

int CXCommunicationServer::BuildConnection(cxsocket sock, const string &strRemoteIp, WORD wRemotePort,
	CXConnectionObject **ppConObj, bool bProxy)
{
    CXCommunicationServer * pComServer = (CXCommunicationServer*)this;

    CXSocketServerKernel & SocketKernel = pComServer->GetSocketSeverKernel();
    CXConnectionsManager & ConnectionsManager = pComServer->GetConnectionManager();
    CXMemoryCacheManager & MemoryCacheManager = pComServer->GetMemoryCacheManger();
    CXDataDispathLevelImpl & DataDispather = pComServer->GetDataDispathManger();

    CXConnectionObject *pConObj = ConnectionsManager.GetFreeConnectionObj();
    CXConnectionSession *pSession = NULL;
	*ppConObj = NULL;

    if (bProxy)
    {
        pSession = m_sessionsManager.GetFreeSession();
        if (pSession == NULL)
        {
			closesocket(sock);
            return -7;
        }

#ifndef WIN32
		SocketKernel.SetNonblocking(sock);
#endif
    }

    CXSessionsManager & sessionsManager = pComServer->GetSessionsManager();

    if (pConObj != NULL)
    {
        pConObj->Build(sock, ConnectionsManager.GetCurrentConnectionIndex(), strRemoteIp, wRemotePort);
        pConObj->SetServer((void*)this);
        pConObj->SetObjectSizeInCache(sizeof(CXBufferObj));
        pConObj->SetProcessOnePacketFun(CXCommunicationServer::OnProcessOnePacket);
        pConObj->SetSessionsManager(&sessionsManager);
        pConObj->SetLogHandle(pComServer->GetLogHandle());
        pConObj->SetJournalLogHandle(pComServer->GetJournalLogHandle());
        pConObj->SetIOStat(pComServer->GetIOStatHandle());
        pConObj->SetRPCObjectManager(pComServer->GetRPCObjectManager());
        pConObj->SetSocketKernel((void*)&pComServer->GetSocketSeverKernel());
        pConObj->SetDataParserHandle(pComServer->GetDataParserHandle());
        pConObj->SetProxyConnection(bProxy);
		pConObj->SetTaskPool(pComServer->GetTaskPool());

        //char *str = inet_ntoa(sockRemote.sin_addr);
        //cout << "accept a connection from " << str << endl;

        int iQueueIndex = rand() % DataDispather.GetQueueNumber();
        pConObj->SetDispacherQueueIndex(iQueueIndex);
        pConObj->SetMemoryCache(&MemoryCacheManager);

        int iRet = SocketKernel.AttachConnetionToModel(*pConObj);
        if (iRet == 0)
        {
            ConnectionsManager.AddUsingConnection(pConObj);
			pConObj->SetState(CXConnectionObject::ESTABLISHED);
#ifdef WIN32
            bool bNeedClose = false;
            //when accept a socket, post some
            for (int i = 0; i < 1; i++)
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
                return -8;
            }
#endif
            if (bProxy)
            {
                string strGuid = m_sessionsManager.BuildSessionGuid();
                pSession->AddMainConnection(*pConObj);
                pSession->SetSesssionGuid(strGuid);
                pSession->ResetVerificationInfo();
                m_sessionsManager.AddUsingSession(pSession);
                pConObj->SetSession(pSession);
            }

			*ppConObj = pConObj;
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

    /*
    int64  iEndTime = conObj.GetCurrentTimeMS();
    uint64 uiIndex = ((CXConnectionObject*)pMes->pConObj)->GetConnectionIndex();
    if (iEndTime - pMes->iBeginTime > 1000)
    {
        char   szInfo[1024] = { 0 };
        sprintf_s(szInfo, 1024, ",push_queue_time:%lldms,connetion index:%lld",
            iEndTime - pMes->iBeginTime, uiIndex);
        //pComServer->m_pLogHandle->Log(CXLog::CXLOG_WARNNING, szInfo);
    }
    */
    return RETURN_SUCCEED;
}

uint64 CXCommunicationServer::AddReceivedBuffers()
{
    uint64 uiCurBufIndex = 0;
    m_lock.Lock();
    m_uiTotalReceiveBuffers++;
    uiCurBufIndex = m_uiTotalReceiveBuffers;
    m_lock.Unlock();
    return uiCurBufIndex;
}


unsigned int CXCommunicationServer::GetCurrentThreadID()
{
#ifdef WIN32
    return::GetCurrentThreadId();
#else
    return pthread_self();
#endif
}

CXThread *CXCommunicationServer::GetCurrrentThread()
{
    CXThread * pThread = NULL;
    DWORD uiThradID = GetCurrentThreadID();
	pThread = m_socketServerKernel.FindThread((DWORD)uiThradID);
	if (pThread == NULL)
	{
		pThread = m_threadPoolDispatch.FindThread((DWORD)uiThradID);
	}
    return pThread;
}

byte *CXCommunicationServer::GetFirstThreadCache(DWORD dwSize)
{
	CXThread *pThread = GetCurrrentThread();
	if (pThread == NULL)
	{
		return NULL;
	}
	byte* pbyCache = pThread->GetFirstCache();
	DWORD dwCacheLen = pThread->GetFirstCacheSize();

	if (dwCacheLen < dwSize)
	{
		if (pThread->AllocateFirstCache(dwSize))
		{
            pbyCache = pThread->GetFirstCache();
            if(pbyCache!=NULL)
                memset(pbyCache, 0, dwSize);
			return pbyCache;
		}
		else
		{
			return NULL;
		}
	}
	else
	{
        if (pbyCache != NULL)
            memset(pbyCache, 0, dwSize);
		return pbyCache;
	}
}
byte *CXCommunicationServer::GetSecondThreadCache(DWORD dwSize)
{
	CXThread *pThread = GetCurrrentThread();
	if (pThread == NULL)
	{
		return NULL;
	}
	byte* pbyCache = pThread->GetSecondCache();
	DWORD dwCacheLen = pThread->GetSecondCacheSize();

	if (dwCacheLen < dwSize)
	{
		if (pThread->AllocateSecondCache(dwSize))
		{
			pbyCache = pThread->GetSecondCache();
            if (pbyCache != NULL)
                memset(pbyCache, 0, dwSize);
			return pbyCache;
		}
		else
		{
			return NULL;
		}
	}
	else
	{
        if (pbyCache != NULL)
            memset(pbyCache, 0, dwSize);
		return pbyCache;
	}
}

CXTaskBase *CXCommunicationServer::GetCreateConnectionTask()
{
	CXTaskPool* pTaskPool = (CXTaskPool*)GetTaskPool();
	if (pTaskPool == NULL)
		return NULL;
	CXTaskCreateConnection *pTask = (CXTaskCreateConnection *)pTaskPool->GetFreeTaskObject(CXTaskBase::CX_RUNING_TASK_TYPE_CREATE_CONNECITON);
	if (pTask!=NULL)
	{
		//string strRemoteIp = "127.0.0.1";
        string strRemoteIp = "192.168.0.118";
		string strLocalIp = "";
		WORD   wRemotePort = 4354;
		WORD   wLocalPort = 0;
		string strUserName = "test";
		string strPwd = "123";
		string strKey = "123";
		pTask->SetAddrInfo(strRemoteIp, wRemotePort,strLocalIp, wLocalPort,(void*)this);
		pTask->SetUserInfo(strUserName, strPwd);
		//pTask->SetKey(strKey, true);
	}

	return pTask;
}

CXConnectionObject *CXCommunicationServer::GetProxyConnection(string strRemoteIp, WORD wRemotePort)
{
	CXConnectionObject* pConObj = m_connectionsManager.GetFreeProxyClient(strRemoteIp, wRemotePort);
	if (pConObj == NULL)
	{
		return NULL;
	}
    if (pConObj->GetState() == CXConnectionObject::ESTABLISHED)
    {
        return pConObj;
    }

	CXConnectionSession *pSession = NULL;
	pSession = m_sessionsManager.GetFreeSession();
	if (pSession == NULL)
	{
		m_connectionsManager.AddFreeConnectionObj(pConObj);
		return NULL;
	}
	
	pConObj->Build(0, m_connectionsManager.GetCurrentConnectionIndex(), strRemoteIp, wRemotePort);
	pConObj->SetState(CXConnectionObject::CREATING);
	pConObj->SetServer((void*)this);
	pConObj->SetObjectSizeInCache(sizeof(CXBufferObj));
	pConObj->SetProcessOnePacketFun(CXCommunicationServer::OnProcessOnePacket);
	pConObj->SetSessionsManager(&m_sessionsManager);
	pConObj->SetLogHandle(GetLogHandle());
	pConObj->SetJournalLogHandle(GetJournalLogHandle());
	pConObj->SetIOStat(GetIOStatHandle());
	pConObj->SetRPCObjectManager(GetRPCObjectManager());
	pConObj->SetSocketKernel((void*)&GetSocketSeverKernel());
	pConObj->SetDataParserHandle(GetDataParserHandle());
	pConObj->SetProxyConnection(true);
	pConObj->SetTaskPool(GetTaskPool());

	int iQueueIndex = rand() % m_dataDispathManager.GetQueueNumber();
	pConObj->SetDispacherQueueIndex(iQueueIndex);
	pConObj->SetMemoryCache(&m_memoryCacheManager);

	//m_connectionsManager.AddUsingConnection(pConObj);

	string strGuid = m_sessionsManager.BuildSessionGuid();
	//pSession->AddMainConnection(*pConObj);
	pSession->SetSesssionGuid(strGuid);
	pSession->ResetVerificationInfo();
	//m_sessionsManager.AddUsingSession(pSession);
	pConObj->SetSession(pSession);
	return pConObj;
}

//the proxy connection have been created, add it to the using map and communication model
int  CXCommunicationServer::OnProxyConnectionCreated(CXConnectionObject *pConObj)
{
	if (pConObj == NULL)
	{
		return CXRET_INVALID_PARAMETER;
	}
	CXConnectionSession *pSession = (CXConnectionSession *)pConObj->GetSession();
#ifndef WIN32
    m_socketServerKernel.SetNonblocking(pConObj->GetSocket());
#endif
	int iRet = m_socketServerKernel.AttachConnetionToModel(*pConObj);
	if (iRet == 0)
	{
		m_connectionsManager.AttachProxyConnection(pConObj);
		pConObj->SetState(CXConnectionObject::ESTABLISHED);
#ifdef WIN32
		bool bNeedClose = false;
		//when accept a socket, post some
		for (int i = 0; i < 1; i++)
		{
			int iRet = pConObj->PostRecv(sizeof(CXBufferObj));
			if (iRet != CXRET_SUCCEED)
			{
				bNeedClose = true;
				break;
			}
		}
		if (bNeedClose)
		{
			CloseConnection(*pConObj, SOCKET_CLOSED);
			return -8;
		}
#endif
		pSession->AddMainConnection(*pConObj);
		m_sessionsManager.AddUsingSession(pSession);
		return CXRET_SUCCEED;
	}
	else
	{
		pConObj->Close();
		pConObj->SetState(CXConnectionObject::CLOSED);
		//pComServer->CloseConnection(*pConObj);
		m_connectionsManager.AddFreeConnectionObj(pConObj);
		return -4;
	}

}

void CXCommunicationServer::DetachProxyConnection(CXRPCObjectClientInServer *pObj)
{
    CXConnectionObject *pCon = pObj->GetAttachConnection();
    if (pCon != NULL)
    {
        m_connectionsManager.DetachProxyConnection(pCon);
    }
}



