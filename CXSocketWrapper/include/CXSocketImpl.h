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

Description£ºThe base wrraper class of the socket,can be using in tcp transmission and udp tranmission.
Notice:      The original code of this class was copied from the www network by me a long time ago,
             and i has forgotten the original author, if anyone find the original author ,
             and find the copyright problem in this class, please notice me ,thanks!
Email: konsan@126.com
*****************************************************************************/
#ifndef __CXSOKCETIMPL_H__
#define __CXSOKCETIMPL_H__

#include "CXSocketAddress.h"

namespace CXCommunication
{
	class CXSocketImpl
	{
	private:
		cxsocket m_sock;
		bool     m_bBlocking;
		//==true close the socket in the deconstruction function
		bool     m_bCloseInDeconstruction;

	public:
		CXSocketImpl();
		CXSocketImpl(cxsocket sockfd);
		CXSocketImpl(const CXSocketImpl& iml);
		virtual ~CXSocketImpl();

	public:
		virtual int Create(int iFamily = AF_INET, int iSocketType = SOCK_STREAM, int iProto = 0);
        virtual int CreateByAccepted(cxsocket sock);

		virtual void Close();

		virtual int Bind(const CXSocketAddress& address);

		virtual int Listen(int iBacklog = 64);

		virtual int Accept(cxsocket &sock, CXSocketAddress& clientAddr);

		virtual int Connect(const CXSocketAddress& address);

		virtual int SendBytes(const byte* bpBuf, unsigned int iDataLen, int iFlag = 0);
		virtual int RecvBytes(byte* bpBuf, unsigned int iBufLen, int iFlag = 0);

		virtual int SendBytesTo(const byte* bpBuf, unsigned int iDataLen, const sockaddr_in* addr, int iFlag = 0);
		virtual int RecvBytesFrom(byte* bpBuf, unsigned int iBufLen, sockaddr_in * addr, int iFlag = 0);

		virtual int ShutdownReceive();
		virtual int ShutdownSend();
		virtual int Shutdown();

		virtual int SetSendBufferSize(int size);
		virtual int GetSendBufferSize(int& size);

		virtual int SetReceiveBufferSize(int size);
		virtual int GetReceiveBufferSize(int& size);

		virtual int GetLocalAddress(CXSocketAddress& localAddr);
		virtual int GetRemoteAddress(CXSocketAddress& remoteAddr);

		virtual int GetAvailableReadDataLen(int& iDataLen);

		int SetOption(int level, int option, int value);
		int SetOption(int level, int option, unsigned value);
		int SetOption(int level, int option, unsigned char value);
		virtual int SetRawOption(int level, int option, const void* value, int length);

		int GetOption(int level, int option, int& value);
		int GetOption(int level, int option, unsigned& value);
		int GetOption(int level, int option, unsigned char& value);
		virtual int GetRawOption(int level, int option, void* value, int length);

		int SetLinger(bool bIsOn, int iSeconds);
		int GetLinger(bool& bIsOn, int& iSeconds);

		int SetNoDelay(bool bIsNoDelay);
		int GetNoDelay(bool& bIsNoDelay);

		int SetKeepAlive(bool bKeepAlive);
		int GetKeepAlive(bool& bKeepAlive);

		int SetReuseAddress(bool bReuse);
		int GetReuseAddress(bool& bReuse);

		virtual int SetBlocking(bool bBlocking);
		virtual bool IsBlocking() const;

		cxsocket GetSocketValue() const;
		const cxsocket* GetSocketHandle() const
		{
			return reinterpret_cast<const cxsocket*>(&m_sock);
		}

		int GetSocketErrorCode();
        string GetSocketErrorDescription();

		virtual int Ioctl(int request, int& arg);
		virtual int Ioctl(int request, void* arg);

		void    SetCloseInDeconstruction(bool bSet) { m_bCloseInDeconstruction = bSet; }
		bool    IsCloseInDeconstruction() { return m_bCloseInDeconstruction; }

	};
}

#endif



