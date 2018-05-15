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

Description：The base wrraper class of the socket,can be using in tcp transmission and udp tranmission.
Notice:      The original code of this class was copied from the www network by me a long time ago,
             and i has forgotten the original author, if anyone find the original author ,
             and find the copyright problem in this class, please notice me ,thanks!
Email: konsan@126.com
*****************************************************************************/
#include "CXSocketImpl.h"
#ifndef WIN32
#include <errno.h>
#include<string.h>
#include <stdio.h>
#include <stdlib.h>
#include<sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#endif

namespace CXCommunication
{

    CXSocketImpl::CXSocketImpl() : m_sock(INVALID_SOCKET), m_bBlocking(false)
    {

    }

    CXSocketImpl::CXSocketImpl(cxsocket sock) : m_sock(sock), m_bBlocking(false)
    {

    }

    CXSocketImpl::CXSocketImpl(const CXSocketImpl& iml) : m_sock(iml.m_sock), m_bBlocking(iml.m_bBlocking)
    {

    }

    CXSocketImpl::~CXSocketImpl()
    {
        Close();
    }

    int CXSocketImpl::Create(int iFamily /* = AF_INET */, int iSocketType /* = SOCK_STREAM */, int iProto /* = 0 */)
    {
        m_sock = ::socket(iFamily, iSocketType, iProto);

        return m_sock == INVALID_SOCKET ? -1 : 0;
    }

    int CXSocketImpl::CreateByAccepted(cxsocket sock)
    {
        m_sock = sock;
        return m_sock == INVALID_SOCKET ? -1 : 0;
    }

    void CXSocketImpl::Close()
    {
        if (m_sock != INVALID_SOCKET)
        {
            closesocket(m_sock);
            m_sock = INVALID_SOCKET;
        }
    }

    int CXSocketImpl::Bind(const CXSocketAddress& address)
    {
        if (m_sock == INVALID_SOCKET)
            return -2;

        sockaddr_in addr;
        memset(&addr,0,sizeof(sockaddr_in));
        addr = address.GetAddress();
        if (::bind(m_sock, reinterpret_cast<const struct sockaddr *>(&addr), sizeof(sockaddr_in)) != 0)
            return -1;

        return 0;
    }

    int CXSocketImpl::Listen(int iBacklog /* = 64 */)
    {
        if (m_sock == INVALID_SOCKET)
            return -2;

        if (::listen(m_sock, iBacklog) != 0)
            return -1;

        return 0;
    }

    int CXSocketImpl::Accept(cxsocket &sock, CXSocketAddress& clientAddr)
    {
        if (m_sock == INVALID_SOCKET)
            return -2;


        sockaddr_in addr;
        memset(&addr, 0, sizeof(sockaddr_in));
        socklen_t uiAddLen = sizeof(sockaddr_in);

        sock = ::accept(m_sock, reinterpret_cast<struct sockaddr*>(&addr), &uiAddLen);
        if (sock == INVALID_SOCKET)
            return -1;
        clientAddr.SetAddress(addr);
        return 0;
    }

    int CXSocketImpl::Connect(const CXSocketAddress& address)
    {
        if (m_sock == INVALID_SOCKET)
            return -2;
        sockaddr_in addr;
        memset(&addr, 0, sizeof(sockaddr_in));
        addr = address.GetAddress();

        if (::connect(m_sock, reinterpret_cast<const struct sockaddr *>(&addr),
            sizeof(sockaddr_in)) != 0)
            return -1;

        return 0;
    }

    int CXSocketImpl::SendBytes(const byte* bpBuf, unsigned int iDataLen, int iFlag)
    {
        return ::send(m_sock, reinterpret_cast<const char*>(bpBuf), iDataLen, iFlag);
    }

    int CXSocketImpl::RecvBytes(byte* bpBuf, unsigned int iBufLen, int iFlag)
    {
        return ::recv(m_sock, reinterpret_cast<char*>(bpBuf), iBufLen, iFlag);
    }

    int CXSocketImpl::SendBytesTo(const byte* bpBuf, unsigned int iLen, const sockaddr_in * addr, int iFlag)
    {
        return ::sendto(m_sock, reinterpret_cast<const char*>(bpBuf), iLen, iFlag,
            reinterpret_cast<const struct sockaddr *>(&addr), sizeof(sockaddr_in));
    }

    int CXSocketImpl::RecvBytesFrom(byte* bpBuf, unsigned int iBufLen, sockaddr_in * addr, int iFlag)
    {
        socklen_t addrLen = sizeof(sockaddr_in);
        return ::recvfrom(m_sock, reinterpret_cast<char*>(bpBuf), iBufLen, iFlag,
            reinterpret_cast< struct sockaddr *>(&addr), &addrLen);
    }

    int CXSocketImpl::ShutdownReceive()
    {
        return (::shutdown(m_sock, SD_RECEIVE) != 0) ? -1 : 0;
    }

    int CXSocketImpl::ShutdownSend()
    {
        return (::shutdown(m_sock, SD_SEND) != 0) ? -1 : 0;
    }

    int CXSocketImpl::Shutdown()
    {
        return (::shutdown(m_sock, SD_BOTH) != 0) ? -1 : 0;
    }

    int CXSocketImpl::SetSendBufferSize(int size)
    {
        return SetOption(SOL_SOCKET, SO_SNDBUF, size);
    }

    int CXSocketImpl::GetSendBufferSize(int& size)
    {
        return GetOption(SOL_SOCKET, SO_SNDBUF, size);
    }

    int CXSocketImpl::SetReceiveBufferSize(int size)
    {
        return SetOption(SOL_SOCKET, SO_RCVBUF, size);
    }

    int CXSocketImpl::GetReceiveBufferSize(int& size)
    {
        return GetOption(SOL_SOCKET, SO_RCVBUF, size);
    }

    int CXSocketImpl::GetLocalAddress(CXSocketAddress& localAddr)
    {
        socklen_t addrLen = sizeof(struct sockaddr_in);
        sockaddr_in addr;
        memset(&addr, 0, sizeof(sockaddr_in));

        int iRet = ::getsockname(m_sock, reinterpret_cast<struct sockaddr*>(&addr), &addrLen);
        if (iRet == 0)
        {
            localAddr.SetAddress(addr);
        }

        return iRet != 0 ? -1 : 0;
    }

    int CXSocketImpl::GetRemoteAddress(CXSocketAddress& remoteAddr)
    {
        socklen_t addrLen = sizeof(struct sockaddr_in);
        sockaddr_in addr;
        memset(&addr, 0, sizeof(sockaddr_in));
        int iRet = ::getpeername(m_sock, reinterpret_cast<struct sockaddr*>(&addr), &addrLen);
        if (iRet == 0)
        {
            remoteAddr.SetAddress(addr);
        }

        return iRet != 0 ? -1 : 0;
    }

    int CXSocketImpl::GetAvailableReadDataLen(int& iDataLen)
    {
        return Ioctl(FIONREAD, iDataLen);
    }


    int CXSocketImpl::SetOption(int level, int option, int value)
    {
        return SetRawOption(level, option, &value, sizeof(value));
    }

    int CXSocketImpl::SetOption(int level, int option, unsigned value)
    {
        return SetRawOption(level, option, &value, sizeof(value));
    }

    int CXSocketImpl::SetOption(int level, int option, unsigned char value)
    {
        return SetRawOption(level, option, &value, sizeof(value));
    }

    int CXSocketImpl::SetRawOption(int level, int option, const void* value, int length)
    {
        int rc = ::setsockopt(m_sock, level, option, reinterpret_cast<const char*>(value), length);

        return rc == -1 ? -1 : 0;
    }

    int CXSocketImpl::GetOption(int level, int option, int& value)
    {
        return GetRawOption(level, option, &value, sizeof(value));
    }

    int CXSocketImpl::GetOption(int level, int option, unsigned& value)
    {
        return GetRawOption(level, option, &value, sizeof(value));
    }

    int CXSocketImpl::GetOption(int level, int option, unsigned char& value)
    {
        return GetRawOption(level, option, &value, sizeof(value));
    }

    int CXSocketImpl::GetRawOption(int level, int option, void* value, int length)
    {
        socklen_t valueLen = length;
        int rc = ::getsockopt(m_sock, level, option, reinterpret_cast<char*>(value), &valueLen);

        return rc == -1 ? -1 : 0;
    }

    int CXSocketImpl::SetLinger(bool bIsOn, int iSeconds)
    {
        struct linger li;
        li.l_onoff = bIsOn ? 1 : 0;
        li.l_linger = iSeconds;
        return SetRawOption(SOL_SOCKET, SO_LINGER, &li, sizeof(li));
    }

    int CXSocketImpl::GetLinger(bool& bIsOn, int& iSeconds)
    {
        struct linger l;
        int len = sizeof(l);
        int ret = GetRawOption(SOL_SOCKET, SO_LINGER, &l, len);
        if (ret == 0)
        {
            bIsOn = (l.l_onoff != 0);
            iSeconds = l.l_linger;
        }

        return ret;
    }

    int CXSocketImpl::SetNoDelay(bool bIsNoDelay)
    {
        int iValue = bIsNoDelay ? 1 : 0;
        return SetOption(IPPROTO_TCP, TCP_NODELAY, iValue);
    }

    int CXSocketImpl::GetNoDelay(bool& bIsNoDelay)
    {
        int iValue = 0;
        int ret = GetOption(IPPROTO_TCP, TCP_NODELAY, iValue);
        if (ret == 0)
            bIsNoDelay = (iValue != 0);

        return ret;
    }

    int CXSocketImpl::SetKeepAlive(bool bKeepAlive)
    {
        int iValue = bKeepAlive ? 1 : 0;
        return SetOption(SOL_SOCKET, SO_KEEPALIVE, iValue);
    }

    int CXSocketImpl::GetKeepAlive(bool& bKeepAlive)
    {
        int iValue = 0;
        int ret = GetOption(SOL_SOCKET, SO_KEEPALIVE, iValue);
        if (ret == 0)
            bKeepAlive = (iValue != 0);

        return ret;
    }

    int CXSocketImpl::SetReuseAddress(bool bReuse)
    {
        int value = bReuse ? 1 : 0;
        return SetOption(SOL_SOCKET, SO_REUSEADDR, value);
    }

    int CXSocketImpl::GetReuseAddress(bool& bReuse)
    {
        int value(0);
        int ret = GetOption(SOL_SOCKET, SO_REUSEADDR, value);
        if (ret == 0)
            bReuse = (value != 0);

        return ret;
    }

    int CXSocketImpl::SetBlocking(bool bBlocking)
    {
#ifdef WIN32
        unsigned long ulBlocking = bBlocking ? 1 : 0;
        if (ioctlsocket(m_sock, FIONBIO, (unsigned long*)&ulBlocking) == SOCKET_ERROR)
            return -1;
#else
        int opts;
        opts = fcntl(m_sock, F_GETFL);
        if (opts < 0)
            return -1;
        if (bBlocking)
            opts = opts | O_NONBLOCK;
        else
            opts = opts & (~O_NONBLOCK);
        if (fcntl(m_sock, F_SETFL, opts) < 0)
            return -1;
#endif
        m_bBlocking = bBlocking;

        return 0;
    }

    bool CXSocketImpl::IsBlocking() const
    {
        return m_bBlocking;
    }

    cxsocket CXSocketImpl::GetSocketValue() const
    {
        return m_sock;
    }

    int CXSocketImpl::Ioctl(int request, int& arg)
    {
        int rc;
#ifdef WIN32
        rc = ioctlsocket(m_sock, request, reinterpret_cast<u_long*>(&arg));
#else
        rc = ::ioctl(m_sock, request, &arg);
#endif

        return (rc != 0) ? -1 : 0;
    }

    int CXSocketImpl::Ioctl(int request, void* arg)
    {
        int iRet = 0;
#ifdef WIN32
        iRet = ioctlsocket(m_sock, request, reinterpret_cast<u_long*>(arg));
#else
        iRet = ::ioctl(m_sock, request, arg);
#endif

        return (iRet != 0) ? -1 : 0;
    }


    int CXSocketImpl::GetSocketErrorCode()
    {
#ifdef WIN32
        return GetLastError();
#else
        return errno;
#endif
    }

    string CXSocketImpl::GetSocketErrorDescription()
    {


#ifdef WIN32
        DWORD dwErrorCode = GetLastError();
        char szErrMsg[1024] = {0};


        LPVOID lpMsgBuf = NULL;
        DWORD dwLen = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            dwErrorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
            (LPTSTR)&lpMsgBuf,
            0,
            NULL
        );
        if (dwLen == 0)
        {
            DWORD dwFmtErrCode = GetLastError(); //FormatMessage 引起的错误代码
            printf_s(szErrMsg, "FormatMessage failed with %u\n", dwFmtErrCode);
        }

        if (lpMsgBuf)
        {
            printf_s(szErrMsg, "Error Code = %u, description = %s",
                dwErrorCode, (LPCTSTR)lpMsgBuf);

        }

        if (lpMsgBuf)
        {
            // Free the buffer.
            LocalFree(lpMsgBuf);
            lpMsgBuf = NULL;
        }

        return szErrMsg;
#else
        string strDesc = "";
        char *pszErr = strerror(errno);
        if(pszErr!=NULL)
            strDesc = strerror(errno);
        return strDesc;
#endif

    }
    /*
    int CXSocketImpl::SetSendTimeOut(int timeout)
    {
        if (Create() != 0)
            return -1;

        if (timeout < 0)
            timeout = NET_INFINITE;

        m_SndTimeOut = timeout;

#ifdef WIN32
        return m_sock->SetRawOption(SOL_SOCKET, SO_SNDTIMEO, &m_SndTimeOut, sizeof(m_SndTimeOut));
#else
        struct timeval tv;
        tv.tv_sec = m_SndTimeOut / 1000;
        tv.tv_usec = m_SndTimeOut % 1000 * 1000;
        return m_sock->SetRawOption(SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));
#endif
    }

    int CXSocketImpl::SetRecvTimeOut(int timeout)
    {
        if (Create() != 0)
            return -1;

        if (timeout < 0)
            timeout = NET_INFINITE;

        m_RcvTimeOut = timeout;

#ifdef WIN32
        return m_sock->SetRawOption(SOL_SOCKET, SO_RCVTIMEO, &m_RcvTimeOut, sizeof(m_RcvTimeOut));
#else
        struct timeval tv;
        tv.tv_sec = m_SndTimeOut / 1000;
        tv.tv_usec = m_SndTimeOut % 1000 * 1000;
        return m_sock->SetRawOption(SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
#endif
    }
    */
}
