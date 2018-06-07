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

Description:
*****************************************************************************/
#ifdef WIN32
#include<ws2tcpip.h>
#else //WIN32

#include  <sys/socket.h>      /* basic socket definitions */
#include  <netinet/in.h>      /* sockaddr_in{} and other Internet defns */
#include  <arpa/inet.h>       /* inet(3) functions */
#include  <sys/epoll.h> /* epoll function */
#include  <sys/resource.h> /*setrlimit */
#include  <pthread.h>
#endif //WIN32

#include "CXSocketServerKernel.h"
#include "CXConnectionObject.h"

using namespace CXCommunication;

void* ThreadListen(void* lpvoid);
void* ThreadWork(void* lpvoid);

CXSocketServerKernel::CXSocketServerKernel()
{
    m_iWaitThreadNum = 4;
    m_nListeningPort=4355;
    m_sockListen = 0;
    m_epollHandle = 0;
    m_uiConnectionNumber = 0;
}

CXSocketServerKernel::~CXSocketServerKernel()
{
    //dtor
}

int CXSocketServerKernel::CreateTcpListenPort(cxsocket & sock,
    unsigned short usPort,char *pszLocalIP )
{
    if (usPort == 0 )
    {
        return INVALID_PARAMETER;
    }

    struct sockaddr_in sockLocal;
    memset(&sockLocal, 0, sizeof(sockLocal));
    socklen_t nAddrLen = sizeof(sockLocal);

    sockLocal.sin_family = AF_INET;
    sockLocal.sin_port = htons(usPort);
    if (pszLocalIP == NULL)
    {
        sockLocal.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        sockLocal.sin_addr.s_addr = inet_addr(pszLocalIP);
    }


    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        cout << "Failed to creat listen socket" << endl;
        return -2;
    }
    #ifndef WIN32
    int iValue=1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&iValue, sizeof(iValue))==-1)
        return -4;

    if (-1 == setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &iValue, sizeof(iValue))) {
        return -5;
    }
    #endif // WIN32

    int nRet = bind(sock, (sockaddr*)&sockLocal, nAddrLen);
    if (nRet == -1)
    {
        cout << "Failed to bind the listen socket to the local address" << endl;
        closesocket(sock);
        return -3;
    }
    return 0;
}

//virtual bool SetNonblocking(int sock)=0;
int CXSocketServerKernel::Start(unsigned short iListeningPort,int iWaitThreadNum)
{
    if(iListeningPort==0 || iWaitThreadNum<1 || iWaitThreadNum>1000)
    {
        return INVALID_PARAMETER;
    }


#ifdef WIN32
    m_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (NULL == m_iocpHandle)
    {
        cout << "Failed to iocp handle " << endl;
        return -4;
    }

    //SYSTEM_INFO mySysInfo;
    //GetSystemInfo(&mySysInfo);
#else
    m_epollHandle = epoll_create(256);
    if (m_epollHandle == -1)
    {
        cout << "Failed to create epoll " << endl;
        return -4;
    }

#endif

    int iRet = CreateTcpListenPort(m_sockListen, iListeningPort);
    if (iRet != 0)
    {
        printf("Failed to creat listen socket\n");
#ifdef WIN32
        CloseHandle(m_iocpHandle);
#else
        close(m_epollHandle);
#endif
        return iRet;
    }

    m_nListeningPort = iListeningPort;
    m_iWaitThreadNum = iWaitThreadNum;

    /*
    iRet = m_threadMonitor.Start(this->ThreadDetectConnection, (void*)this);
    if (iRet != 0)
    {
        cout<< "The monitor thread pthread_create fails"<<endl;
        Stop();

        return -8;
    }
    */

    SetStarted(true);

    bool bCreateThread = true;
    for(int i=0;i<m_iWaitThreadNum;i++)
    {
        CXThread *pWaitThread = NULL;
        try
        {
            pWaitThread = new CXThread();
        }
        catch (const bad_alloc& e)
        {
            bCreateThread = false;
            printf("Failed to start socket server kernel, allocate memory failed \n");
            pWaitThread = NULL;
        }

        if (pWaitThread != NULL)
        {
            RunFun funThread = &ThreadWork;
            iRet = pWaitThread->Start(funThread, (void*)this);
            if (iRet != 0)
            {
                printf("Failed to start socket server kernel, create waitting thread fails \n");
                bCreateThread = false;
            }
        }
        if (!bCreateThread)
        {
            Stop();
            return -9;
        }
        m_lstWaitThreads.push_back(pWaitThread);
    }

    iRet = listen(m_sockListen, SOMAXCONN);
    if (iRet == -1)
    {
        cout << "Failed to listen the socket, error : "<< strerror(errno) << endl;
        Stop();
        return -10;
    }

    iRet = m_threadListen.Start(&ThreadListen, (void*)this);
    if (iRet != 0)
    {
        cout<< "CUserManagement pthread_create fails"<<endl;
        Stop();

        return -7;
    }

    return RETURN_SUCCEED;
}

int CXSocketServerKernel::Stop()
{
    SetStarted(false);
    closesocket(m_sockListen);

    CXThread * pThreadObj = NULL;
    std::list<CXThread*>::iterator it = m_lstWaitThreads.begin();
    for (; it != m_lstWaitThreads.end(); it++)
    {
#ifdef WIN32
        PostQueuedCompletionStatus(m_iocpHandle, (DWORD)0xFFFFFFFF, NULL, NULL);
#else
#endif
    }

    it = m_lstWaitThreads.begin();
    for (; it != m_lstWaitThreads.end();)
    {
        pThreadObj = *it;
        pThreadObj->Wait();
        m_lstWaitThreads.pop_front();
        it = m_lstWaitThreads.begin();
        delete[] pThreadObj;
    }

    m_threadListen.Wait();

#ifdef WIN32
    CloseHandle(m_iocpHandle);
#else
    close(m_epollHandle);
#endif


    return RETURN_SUCCEED;
}

int CXSocketServerKernel::OnAccept(void *pServer, cxsocket sock, sockaddr_in &addrRemote)
{
    return RETURN_SUCCEED;
}

int CXSocketServerKernel::OnRead(CXConnectionObject& conObj, PCXBufferObj pBufObj,
    DWORD dwTransDataOfBytes)
{
    return RETURN_SUCCEED;
}

int CXSocketServerKernel::OnWrite(CXConnectionObject& conObj)
{
    return RETURN_SUCCEED;
}

void CXSocketServerKernel::OnClose(CXConnectionObject& conObj, ConnectionClosedType emClosedType)
{

}

bool CXSocketServerKernel::SetNonblocking(int sock)
{
#ifdef WIN32
    unsigned long iNonblocking =  1;
    if (ioctlsocket(sock, FIONBIO, (unsigned long*)&iNonblocking) == SOCKET_ERROR)
    {
        return false;
    }


#else //WIN32
    int opts;
    opts = fcntl(sock, F_GETFL);
    if (opts<0)
    {
        perror("fcntl(sock,GETFL)");
        return false;
    }
    opts = opts | O_NONBLOCK;
    if (fcntl(sock, F_SETFL, opts)<0)
    {
        perror("fcntl(sock,SETFL,opts)");
        return false;
    }
#endif //WIN32
    return true;
}

void* ThreadListen(void* lpvoid)
{
    CXSocketServerKernel* pServer = (CXSocketServerKernel*)lpvoid;
    pServer->ListenThread();
    return 0;
    //return NULL;
}

void* ThreadWork(void* lpvoid)
{
    CXSocketServerKernel* pServer = (CXSocketServerKernel*)lpvoid;
    pServer->WaitThread();
    return 0;
}


int CXSocketServerKernel::ListenThread()
{
    struct sockaddr_in sockRemote;
    memset(&sockRemote,0,sizeof(sockRemote));
    socklen_t nAddrLen = sizeof(sockRemote);

    while(IsStarted())
    {
        memset(&sockRemote,0,sizeof(sockRemote));
        nAddrLen = sizeof(sockRemote);
        cxsocket sockAccept = accept(m_sockListen,(sockaddr *)&sockRemote, &nAddrLen);
        if(sockAccept ==-1)
        {
            printf("Failed to accept a connection the socket in port \n");
            closesocket(m_sockListen);
            return 0;
        }
        else
        {
            m_uiConnectionNumber++;
            //closesocket(sockAccept);
            //continue;
#ifndef WIN32
            SetNonblocking(sockAccept);
#endif
            if (m_pfOnAccept != NULL)
            {
                int iRet = m_pfOnAccept(GetServer(), sockAccept, sockRemote);
                if (iRet != 0)
                {
                    closesocket(sockAccept);
                }
            }
            else
            {
                OnAccept(GetServer(),sockAccept, sockRemote);
            }


        }
    }
    return 0;
}

int  CXSocketServerKernel::WaitThread()
{

    CXConnectionObject *pConObj = NULL;
    DWORD nBytes = 0;
    DWORD dwFlags = 0;
    DWORD dwErrorCode = 0;

    DWORD dwNumberOfBytes = 0;
    BOOL bGetStauts = FALSE;

    PCXBufferObj pBufObj = NULL;

#ifdef WIN32
    void * lpCompletionKey = NULL;
    LPOVERLAPPED lpOverlapped = NULL;
#else
    struct epoll_event ev, events[20];
#endif // WIN32

    while ( IsStarted())
    {

#ifdef WIN32
        bGetStauts = GetQueuedCompletionStatus(m_iocpHandle, &dwNumberOfBytes,
            (PULONG_PTR)&pConObj,&lpOverlapped, INFINITE);

        if (dwNumberOfBytes == 0xFFFFFFFF)// get the notification from the stop function
        {
            return 0;
        }

        if (!bGetStauts)
        {
            if (!ProcessIocpErrorEvent(*pConObj,lpOverlapped, dwNumberOfBytes))
            {
                return -2;
            }
        }
        else //succeed
        {
            dwErrorCode = NO_ERROR;
            pBufObj = CONTAINING_RECORD(lpOverlapped, CXBufferObj, ol);

            if (dwNumberOfBytes == 0 && (pBufObj->wsaBuf.len!=0))
            {
                if (!ProcessIocpErrorEvent(*pConObj, lpOverlapped, dwNumberOfBytes))
                {
                    return -2;
                }
            }

            //char szInfo[1024] = {0};
            //sprintf_s(szInfo,1024, "Reveive a complete event ,dwNumberOfBytes=%d,pBufObj=%x\n", dwNumberOfBytes, (DWORD)pBufObj);
            //m_pLogHandle->Log(CXLog::CXLOG_INFO, szInfo);
            //printf_s(szInfo);
            if (!ProcessIOCPEvent(*pConObj, pBufObj, dwNumberOfBytes))
            {
                return -2;
            }

        }

#else
        int nfds = epoll_wait(m_epollHandle, events, 20, -1);

        //cout << "\nepoll_wait returns" << endl;

        for(int i=0;i<nfds;++i)
        {
            if(events[i].data.fd==m_sockListen)
            {
                //m_connectionManager.AddPendingConnection();
            }
            else if(events[i].events & EPOLLIN)//receive data 。
            {
                //cout << "A Data Packet received" << endl;
                pConObj = (CXConnectionObject*)(events[i].data.ptr);
                int iProcessRet = ProcessEpollEvent(*pConObj);

            }
            else if(events[i].events & EPOLLOUT) // 如果有数据发送
            {
            }

            if(events[i].events & EPOLLERR || events[i].events & EPOLLHUP)
            {
                pConObj = (CXConnectionObject*)(events[i].data.ptr);
                printf("A error occur , connection id=%lld \n",pConObj->GetConnectionIndex());
                if (m_pfOnClose != NULL)
                {
                    m_pfOnClose(*pConObj, SOCKET_CLOSED);
                }
                else
                {
                    this->OnClose(*pConObj, SOCKET_CLOSED);
                }
            }

        }
#endif // WIN32
    }
    return 0;
}

#ifdef WIN32
BOOL  CXSocketServerKernel::ProcessIocpErrorEvent(CXConnectionObject &conObj, LPOVERLAPPED lpOverlapped,
    DWORD dwTransDataOfBytes)
{
    DWORD dwErrorCode = GetLastError();
    //printf_s("GetQueuedCompletionStatus failed,error code= %d,dwTransDataOfBytes=%d\n", dwErrorCode, dwTransDataOfBytes);

    if (lpOverlapped == NULL)//may be time out or other error
    {
        if (dwErrorCode == WAIT_TIMEOUT)//time out
        {
            return TRUE;
        }
        else if (dwErrorCode == ERROR_ABANDONED_WAIT_0) // the completion port handle was closed
        {
            return FALSE;
        }
        else//other error
        {
            return TRUE;
        }
    }
    else// socket error of socket be closed
    {
        PCXBufferObj pBufObj = NULL;
        pBufObj = CONTAINING_RECORD(lpOverlapped, CXBufferObj, ol);

        if (dwTransDataOfBytes == 0) //the connection had been closed or interrupted
        {
            if (pBufObj != NULL)
            {
                conObj.ReduceNumberOfPostBuffers();
                conObj.FreeCXBufferObj(pBufObj);
            }
            //conObj.SetState(3);

            if (m_pfOnClose != NULL)
            {
                m_pfOnClose(conObj, SOCKET_CLOSED);
            }
            else
            {
                this->OnClose(conObj, SOCKET_CLOSED);
            }
        }
        else //unknown exception
        {
            //return TRUE;
            printf_s("Get a error event , code= %d,dwTransDataOfBytes=%d\n", dwErrorCode, dwTransDataOfBytes);
        }
    }
    return TRUE;
}


//windows iocp event process
BOOL CXSocketServerKernel::ProcessIOCPEvent(CXConnectionObject& conObj, PCXBufferObj pBufObj,
    DWORD dwTransDataOfBytes)
{
    char szInfo[1024] = { 0 };
    //sprintf_s(szInfo, 1024, "ProcessIOCPEvent:Reveive a complete event ,dwNumberOfBytes=%d,pBufObj=%x,pBufObj->nOperate=%d,pBufObj->nSequenceNum=%I64i\n",
    //    dwTransDataOfBytes, (DWORD)pBufObj, pBufObj->nOperate, pBufObj->nSequenceNum);
    //m_pLogHandle->Log(CXLog::CXLOG_INFO, szInfo);
    //printf_s(szInfo);



    if (pBufObj->nOperate == OP_READ)
    {
        pBufObj->wsaBuf.len = dwTransDataOfBytes;
        if (m_pfOnRead != NULL)
        {
            m_pfOnRead(conObj, pBufObj, dwTransDataOfBytes);
        }
        else
        {
            OnRead(conObj, pBufObj, dwTransDataOfBytes);
        }

        return TRUE;
    }
    else if (pBufObj->nOperate == OP_WRITE)
    {
        conObj.FreeCXBufferObj(pBufObj);
        conObj.ReduceNumberOfPostBuffers();
        return TRUE;
    }
    else
    {
        sprintf_s(szInfo, 1024, "Error Packet : receive a error packet, pBufObj = %x, pBufObj->nOperate = %d, pBufObj->nSequenceNum = %I64i, free buffer\n",
            (void*)pBufObj, pBufObj->nOperate, pBufObj->nSequenceNum);
        m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
        printf_s(szInfo);
        conObj.FreeCXBufferObj(pBufObj);
        conObj.ReduceNumberOfPostBuffers();
    }


    return TRUE;
}
#endif // WIN32

#ifndef WIN32
//windows epoll event process
int CXSocketServerKernel::ProcessEpollEvent(CXConnectionObject& conObj)
{
    int iRet = 0;
    int nRet = 0;
    while (nRet >= 0)
    {
        DWORD  dwReadLen = 0;

        PCXBufferObj pBufObj=NULL;
        nRet = conObj.RecvData(&pBufObj,dwReadLen);
        if (dwReadLen>0)
        {
            if (m_pfOnRead != NULL)
            {
                m_pfOnRead(conObj,pBufObj,dwReadLen);
            }
            else
            {
                OnRead(conObj, pBufObj, dwReadLen);
            }
        }

        // socket had been closed by peer
        // some error or connection had been closed
        if (nRet == -3 || nRet == -2)
        {
            if (m_pfOnClose != NULL)
            {
                m_pfOnClose(conObj, SOCKET_CLOSED);
            }
            else
            {
                OnClose(conObj, SOCKET_CLOSED);
            }
        }
        else if (nRet == -1)//read to end
        {
            break;
        }
    }
    return iRet;
}

#endif // Linux


BOOL CXSocketServerKernel::PostAccept(PCXBufferObj pBufObj)
{
    if (pBufObj == NULL)
        return FALSE;

    BOOL bRet = TRUE;

    pBufObj->nOperate = OP_ACCEPT;
    pBufObj->nSequenceNum = 0;
    /*
    pBufObj->sockAccept = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (pBufObj->sockAccept != INVALID_SOCKET)
    {
        int nAddrLen = sizeof(sockaddr_in) + 16;
        DWORD dwRecv = 0;

        ::EnterCriticalSection(&m_csPendingAcceptBufList);
        pBufObj->pNext = m_pPendingAcceptBufObjList;
        m_pPendingAcceptBufObjList = pBufObj;
        m_nPendingAcceptBufNum++;
        if (!m_lpfnAcceptEx(m_sockListen, pBufObj->sockAccept,
            pBufObj->buf, pBufObj->nBufLen - nAddrLen * 2,
            nAddrLen, nAddrLen, &dwRecv, &pBufObj->ol))
        {
            DWORD dwEr = ::WSAGetLastError();
            if (dwEr != WSA_IO_PENDING)
            {
                bRet = FALSE;
                closesocket(pBufObj->sockAccept);
                pBufObj->sockAccept = INVALID_SOCKET;
                m_pPendingAcceptBufObjList = pBufObj->pNext;
                m_nPendingAcceptBufNum--;
            }
        }

        ::LeaveCriticalSection(&m_csPendingAcceptBufList);
        return bRet;
    }
    else
    {
        return FALSE;
    }
    */
    return TRUE;
}

int  CXSocketServerKernel::AttachConnetionToModel(CXConnectionObject &conObj)
{

#ifdef WIN32
    HANDLE hRet = ::CreateIoCompletionPort((HANDLE)conObj.GetSocket(), m_iocpHandle, (ULONG_PTR)&conObj, 0);
    if (hRet == NULL)
    {
        //closesocket(conObj.GetSocket());
        //cout << "Failed to add the new connection socket to the iocp model" << endl;
        return -1;
    }
    //printf_s("BindConnetionToModel,sock=%d\n", conObj.GetSocket());

#else
    //sprintf("BindConnetionToModel linux\n");
    //register ev
    struct epoll_event ev;
    //ev.events=EPOLLIN |EPOLLOUT | EPOLLET;
    //ev.events=EPOLLIN;
    ev.events = EPOLLIN | EPOLLET| EPOLLERR | EPOLLHUP;
    ev.data.ptr = (void*)&conObj;
    if (-1 == epoll_ctl(m_epollHandle, EPOLL_CTL_ADD, conObj.GetSocket(), &ev))
    {
        closesocket(conObj.GetSocket());
        //cout << "Failed to add the new connection socket to the epoll model" << endl;
        return -1;
    }
#endif // WIN32

    return 0;
}

int  CXSocketServerKernel::DetachConnetionToModel(CXConnectionObject &conObj)
{
    //printf("Detach connection from socket model,connection id=%lld\n", conObj.GetConnectionIndex());
#ifdef WIN32

#else
    //register ev
    struct epoll_event ev;
    //ev.events=EPOLLIN |EPOLLOUT | EPOLLET;
    //ev.events=EPOLLIN;
    ev.events = EPOLLIN | EPOLLET| EPOLLERR | EPOLLHUP;
    ev.data.ptr = (void*)&conObj;
    if (-1 == epoll_ctl(m_epollHandle, EPOLL_CTL_DEL, conObj.GetSocket(), &ev))
    {
        //closesocket(conObj.GetSocket());
        printf("Failed to add the new connection socket to the epoll model,errno=%d,error=%s", errno, strerror(errno));

        return -1;
    }
#endif // WIN32

    return 0;
}

