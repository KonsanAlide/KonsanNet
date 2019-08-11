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
#ifndef __CXSOKCETADDRESS_H__
#define __CXSOKCETADDRESS_H__

#include "SocketDefine.h"
#include "PlatformDataTypeDefine.h"
#ifdef WIN32
#include <Ws2ipdef.h>
#else
#include <netinet/in.h>
#endif

#include <string>
using namespace std;


namespace CXCommunication
{
	class CXSocketAddress
	{
	private:
		sockaddr_in m_addr;
        sockaddr_in6 m_addrV6;
        bool m_b64bitAddr;
        bool m_bSet;

	public:
        CXSocketAddress(const char *pszIP, unsigned short usPort, bool b64bitAddr = false);
		CXSocketAddress(const sockaddr_in &addr);
        CXSocketAddress() { }
		virtual ~CXSocketAddress();

	public:
		inline struct sockaddr_in GetAddress() const
		{
			return m_addr;
		}

        inline void SetAddress(const struct sockaddr_in addr)
        {
            m_addr = addr;
            m_bSet = true;
        }

		inline unsigned short GetPort() const
		{
			return ntohs(m_addr.sin_port);
		}

        virtual string GetIP()const;

        bool IsHaveAddress() { return m_bSet; }

		string  GetAddressString();

		void SetAddress(const string &strIP, unsigned short usPort, bool b64bitAddr = false);
	};
}

#endif
