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

Description：
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

#include "CXLog.h"
extern CXLog g_cxLog;

using namespace CXCommunication;

void* ThreadListen(void* lpvoid);
void* ThreadWork(void* lpvoid);

CXSocketServerKernel::CXSocketServerKernel()
{
    m_iWaitThreadNum = 4;
    m_nListeningPort=4355;
    m_sockListen = 0;
    m_epollHandle = 0;
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
        printf_s("Failed to creat listen socket\n");
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
        cout<< "CUserManagement pthread_create fails"<<endl;
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
            cout << "CXSocketServerKernel create epoll wait thread fails, allocate memory failed" << endl;
            pWaitThread = NULL;
        }
        
        if (pWaitThread != NULL)
        {
            RunFun funThread = &ThreadWork;
            iRet = pWaitThread->Start(funThread, (void*)this);
            if (iRet != 0)
            {
                cout << "CXSocketServerKernel create epoll wait thread fails" << endl;
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

    iRet = listen(m_sockListen, 100);
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
    CXConnectionObject *pObj = new CXConnectionObject;

    //PSocketObj  pObj = m_connectionManager.AddPendingConnection(nAcceptSock,(void*)this);
    if (pObj != NULL)
    {
        pObj->Build(sock,1, addrRemote);
        //pObj->sock = sockAccept;
        //pObj->lpServer = (void*)this;
        //memcpy(&pObj->addrRemote, &sockRemote, sizeof(sockRemote));
        int iRet = AttachConnetionToModel(*pObj);
        if (iRet == 0)
        {
            //when accept a socket, post some
            for (int i = 0; i<4; i++)
            {
                PCXBufferObj pBufObj = new CXBufferObj;
                memset(pBufObj, 0, sizeof(CXBufferObj));
                pBufObj->wsaBuf.buf = pBufObj->buf;
                pBufObj->wsaBuf.len = BUF_SIZE;
                //WSAEVENT event = WSACreateEvent();
                //pBufObj->ol.hEvent = event;

                if (!PostRecv(*pObj, pBufObj))
                {
                    break;
                }
            }
        }
    }
    else
    {
        closesocket(sock);
    }
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

void CXSocketServerKernel::OnClose(CXConnectionObject& conObj)
{

}

bool CXSocketServerKernel::SetNonblocking(int sock)
{
    int opts;
#ifdef WIN32
    unsigned long iNonblocking =  1;
    if (ioctlsocket(sock, FIONBIO, (unsigned long*)&iNonblocking) == SOCKET_ERROR)
    {
        return false;
    }
        
    
#else //WIN32
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


bool CXSocketServerKernel::WaitThreadsExit()
{
    m_threadListen.Wait();
    return true;
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
            cout<<"Failed to accept a connection the socket in port "<<m_nListeningPort<<endl;
            closesocket(m_sockListen);
            return 0;
        }
        else
        {
            //SetNonblocking(sockAccept);

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
            (LPDWORD)&pConObj,&lpOverlapped, INFINITE);

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
            //g_cxLog.Log(CXLog::CXLOG_INFO, szInfo);
            //printf_s(szInfo);
            if (!ProcessIOCPEvent(*pConObj, pBufObj, dwNumberOfBytes))
            {
                return -2;
            }

        }

#else 
        int nfds = epoll_wait(m_epollHandle, events, 20, -1);

        cout << "\nepoll_wait returns" << endl;

        for(int i=0;i<nfds;++i)
        {
            if(events[i].data.fd==m_sockListen)
            {
                //m_connectionManager.AddPendingConnection();
            }
            else if(events[i].events & EPOLLIN)//receive data 。
            {
                cout << "A Data Packet received" << endl;
                pConObj = (PConnectionObj)(events[i].data.ptr);
                int iProcessRet = ProcessEpollEvent(pConObj);

            }
            else if(events[i].events & EPOLLOUT) // 如果有数据发送
            {
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
                conObj.FreeBuffer(pBufObj);
            }
            conObj.SetState(3);

            if (m_pfOnClose != NULL)
            {
                m_pfOnClose(conObj);
            }
            else
            {
                this->OnClose(conObj);
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
#endif

//windows iocp event process 
BOOL CXSocketServerKernel::ProcessIOCPEvent(CXConnectionObject& conObj, PCXBufferObj pBufObj,
    DWORD dwTransDataOfBytes)
{
    char szInfo[1024] = { 0 };
    //sprintf_s(szInfo, 1024, "ProcessIOCPEvent:Reveive a complete event ,dwNumberOfBytes=%d,pBufObj=%x,pBufObj->nOperate=%d,pBufObj->nSequenceNum=%I64i\n",
    //    dwTransDataOfBytes, (DWORD)pBufObj, pBufObj->nOperate, pBufObj->nSequenceNum);
    //g_cxLog.Log(CXLog::CXLOG_INFO, szInfo);
    //printf_s(szInfo);
    /*
    if (pBufObj->nOperate != 2)
    {
        int k = 0;
    }
    conObj.PostRecv(4096);
    conObj.FreeBuffer(pBufObj);
    return TRUE;
    */
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
        conObj.FreeBuffer(pBufObj);
        return TRUE;
    }
    else
    {
        sprintf_s(szInfo, 1024, "Error Packet : receive a error packet, pBufObj = %x, pBufObj->nOperate = %d, pBufObj->nSequenceNum = %I64i, free buffer\n",
            (DWORD)pBufObj, pBufObj->nOperate, pBufObj->nSequenceNum);
        g_cxLog.Log(CXLog::CXLOG_ERROR, szInfo);
        printf_s(szInfo);

        conObj.FreeBuffer(pBufObj);
    }
    

    return TRUE;
}
#ifndef WIN32
//windows epoll event process 
int CXSocketServerKernel::ProcessEpollEvent(CXConnectionObject& conObj)
{
    int iRet = 0;
    int nRet = 0;
    while (nRet >= 0)
    {
        char *pszData = new char[4096];
        memset(pszData, 0, 4096);
        int nReadLen = 0;
        //need to lock
        nRet = RecvData(conObj, pszData, 4096, 0, nReadLen);
        if (nReadLen>0)
        {
            if (m_pfOnRead != NULL)
            {
                //m_pfOnRead(pSock,pszData,nReadLen);
            }
            else
            {
                OnRead(conObj, pszData, nReadLen);
            }
        }
        //need to unlock

        if (nRet == 0)
        {
            if (m_pfOnRead != NULL)
            {
                //m_pfOnClose(conObj);
            }
            else
            {
                OnClose(conObj);
            }
        }
        else if (nRet == -1)//read to end
        {
            break;
        }
        else if (nRet == -2) //some error or connection had been closed
        {
            if (m_pfOnClose != NULL)
            {
                //m_pfOnClose(conObj);
            }
            else
            {
                OnClose(conObj);
            }
            break;
        }
    }
    return iRet;
}

#endif // Linux

//have lock by PConnectionObj::lock
int  CXSocketServerKernel::RecvData(CXConnectionObject& conObj,char *pBuf,int nBufLen,int nFlags,int &nReadLen)
{
    int nRet = 0;
    int sockCur = conObj.GetSocket();

    int nRecv = 0;
    int nTotalRead = 0;
    bool bReadOk = false;
    int  nLeftBufLen = nBufLen;
    while(nLeftBufLen>0)
    {
        // 确保sockfd是nonblocking的
        nRecv = recv(sockCur, pBuf + nTotalRead, nLeftBufLen, nFlags);
        if(nRecv < 0)
        {
            if(errno == EAGAIN)
            {
                // 由于是非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读
                // 在这里就当作是该次事件已处理处.
                bReadOk = true;
                nRet = -1;
                break;
            }
            else if (errno == ECONNRESET)
            {
                    // 对方发送了RST
                closesocket(sockCur);
                    //CloseAndDisable(sockCur, events[i]);
                    cout << "counterpart send out RST\n";
                    nRet = -2;
                    break;
                }
            else if (errno == EINTR)
            {
                // 被信号中断
                continue;
            }
            else
            {
                //其他不可弥补的错误
                closesocket(sockCur);
                //CloseAndDisable(sockCur, events[i]);
                cout << "unrecovable error\n";
                nRet = -2;
                break;
            }
        }
        else if( nRecv == 0)
        {
            // 这里表示对端的socket已正常关闭.发送过FIN了。
            //CloseAndDisable(sockCur, events[i]);
            closesocket(sockCur);
            cout << "counterpart has shut off\n";
            nRet = -2;
            break;
        }
        else// recvNum > 0
        {
            nTotalRead += nRecv;
            nLeftBufLen-=nRecv;
            if ( nLeftBufLen == 0)
            {
                // 安全读完
                bReadOk = true;
                break; // 退出while(1),表示已经全部读完数据
            }
        }
    }

    nReadLen = nTotalRead;
    return nRet;
}


//have lock by PConnectionObj::lock
int  CXSocketServerKernel::SendDataCallback(CXConnectionObject& conObj,char *pBuf,
    int nBufLen,int nFlags,int &nSendLen)
{

    int nRet = 0;
    int sockCur = conObj.GetSocket();


    bool bWritten = false;
    int nWritenLen = 0;
    int nTotalWritenLen = 0;
    int nLeftDataLen = nBufLen;

    while(nLeftDataLen>0)
    {
        // 确保sockfd是非阻塞的
        nWritenLen = send(sockCur, pBuf + nTotalWritenLen, nLeftDataLen, 0);
        if (nWritenLen == -1)
        {
            if (errno == EAGAIN)
            {
                // 对于nonblocking 的socket而言，这里说明了已经全部发送成功了
                bWritten = true;
                nRet = -1;
                break;
            }
            else if(errno == ECONNRESET)
            {
                // 对端重置,对方发送了RST
                //CloseAndDisable(sockCur, events[i]);
                closesocket(sockCur);
                cout << "counterpart send out RST\n";
                nRet = -2;
                break;
            }
            else if (errno == EINTR)
            {
                // 被信号中断
                continue;
            }
            else
            {
                // 其他错误
                nRet = -2;
                break;
            }
        }

        if (nWritenLen == 0)
        {
            // 这里表示对端的socket已正常关闭.
            //CloseAndDisable(sockCur, events[i]);
            closesocket(sockCur);
            cout << "counterpart has shut off\n";
            nRet = -2;
            break;
        }

        // nTotalWritenLen > 0
        nTotalWritenLen += nWritenLen;
        if (nWritenLen == nLeftDataLen)
        {
            break;
        }
        nLeftDataLen-=nWritenLen;
    }


/*
    if (bWritten == true)
    {
        //设置用于读操作的文件描述符
        ev.data.fd=sockCur;

        //设置用于注测的读操作事件
        ev.events=EPOLLIN | EPOLLET;

        epoll_ctl(m_epollHandle,EPOLL_CTL_MOD,sockCur,&ev);
    }
*/
    return nRet;
}


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


BOOL CXSocketServerKernel::PostSend(CXConnectionObject& conObj, PCXBufferObj pBufObj)
{
    if (pBufObj == NULL)
        return FALSE;

    /*if (pConObj->nSendPendingIOnum>m_nMaxSend)
    {
        return FALSE;
    }*/

    pBufObj->nOperate = OP_WRITE;

    DWORD dwSend = 0;
    WSABUF buf;
    buf.buf = pBufObj->buf;
    //buf.len = pBufObj->nBufLen;
    //::EnterCriticalSection(&pSockObj->csLock);
    if (WSASend(conObj.GetSocket(), &buf, 1, &dwSend, 0, &pBufObj->ol, NULL) != 0)
    {
        DWORD dwEr = ::WSAGetLastError();
        if (dwEr != WSA_IO_PENDING)
        {
            //::LeaveCriticalSection(&pSockObj->csLock);
            return FALSE;
        }
        //pConObj->nSendPendingIOnum++;
    }
    else
    {
        //pConObj->nSendPendingIOnum++;
    }
    //::LeaveCriticalSection(&pSockObj->csLock);
    return TRUE;

}

BOOL CXSocketServerKernel::PostRecv(CXConnectionObject& conObj, PCXBufferObj pBufObj)
{
    if (pBufObj == NULL)
        return FALSE;


    pBufObj->nOperate = OP_READ;

    DWORD dwRecv = 0;
    DWORD dwFlag = 0;

    //::EnterCriticalSection(&pConObj->csLock);
    if (WSARecv(conObj.GetSocket(), &pBufObj->wsaBuf, 1, 
        &dwRecv, &dwFlag, &pBufObj->ol, NULL) != 0)
    {
        DWORD dwEr = ::WSAGetLastError();
        if (dwEr != WSA_IO_PENDING)
        {
            //::LeaveCriticalSection(&pConObj->csLock);
            return FALSE;
        }
        //pConObj->nRecvPendingIOnum++;
        //pConObj->nNextRecvSequenceNum++;
    }
    else
    {
        //pConObj->nRecvPendingIOnum++;
        //pConObj->nNextRecvSequenceNum++;
    }
    //::LeaveCriticalSection(&pSockObj->csLock);
    return TRUE;
}

int  CXSocketServerKernel::AttachConnetionToModel(CXConnectionObject &conObj)
{
#ifdef WIN32     
    HANDLE hRet = ::CreateIoCompletionPort((HANDLE)conObj.GetSocket(), m_iocpHandle, (DWORD)&conObj, 0);
    if (hRet == NULL)
    {
        closesocket(conObj.GetSocket());
        cout << "Failed to add the new connection socket to the iocp model" << endl;
        return -1;
    }
    //printf_s("BindConnetionToModel,sock=%d\n", conObj.GetSocket());
  
#else    
    printf("BindConnetionToModel linux\n");
    //register ev
    struct epoll_event ev;
    //ev.events=EPOLLIN |EPOLLOUT | EPOLLET;
    //ev.events=EPOLLIN;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.ptr = (void*)&conObj;
    if (-1 == epoll_ctl(m_epollHandle, EPOLL_CTL_ADD, conObj.GetSocket(), &ev))
    {
        closesocket(conObj.GetSocket());
        cout << "Failed to add the new connection socket to the epoll model" << endl;
        return -1;
    }
#endif // WIN32

    return 0;
}

int  CXSocketServerKernel::DetachConnetionToModel(CXConnectionObject &conObj)
{
    HANDLE hRet = ::CreateIoCompletionPort((HANDLE)conObj.GetSocket(), m_iocpHandle, (DWORD)&conObj, 0);
    if (hRet == NULL)
    {
        closesocket(conObj.GetSocket());
        cout << "Failed to add the new connection socket to the iocp model" << endl;
        return -1;
    }
    printf_s("BindConnetionToModel,sock=%d\n", conObj.GetSocket());
#ifdef WIN32    
#else    
    printf("BindConnetionToModel linux\n");
    //register ev
    struct epoll_event ev;
    //ev.events=EPOLLIN |EPOLLOUT | EPOLLET;
    //ev.events=EPOLLIN;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.ptr = (void*)&conObj;
    if (-1 == epoll_ctl(m_epollHandle, EPOLL_CTL_ADD, conObj.GetSocket(), &ev))
    {
        closesocket(conObj.GetSocket());
        cout << "Failed to add the new connection socket to the epoll model" << endl;
        return -1;
    }
#endif // WIN32

    return 0;
}

