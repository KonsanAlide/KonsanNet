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

Description:
*****************************************************************************/
#ifndef __CXRPCREPLY_H__
#define __CXRPCREPLY_H__
#ifdef WIN32
#include "windows.h"
#endif
#include "PlatformDataTypeDefine.h"

#include <string>
using namespace std;
namespace CXCommunication
{
	class CXRPCReply
	{
	public:
		CXRPCReply();
		CXRPCReply(string strContent, DWORD dwReplyCode, byte *pbyReplyData, DWORD  dwReplyDataLen,
			DWORD dwReserveValue1, DWORD dwReserveValue2);
		~CXRPCReply();

		void   SetReplyContent(string strContent) { m_strReplyContent = strContent; }
		void   SetReplyCode(DWORD dwReplyCode) { m_dwReplyCode = dwReplyCode; }
		void   SetReplyData(byte *pbyReplyData) { m_pbyReplyData = pbyReplyData; }
		void   SetReplyDataLen(DWORD  dwReplyDataLen) { m_dwReplyDataLen = dwReplyDataLen; }
		void   SetReserveValue1(DWORD dwValue) { m_dwReserveValue1 = dwValue; }
		void   SetReserveValue2(DWORD dwValue) { m_dwReserveValue2 = dwValue; }

		string GetReplyContent() const { return m_strReplyContent; }
		DWORD  GetReplyCode() const { return m_dwReplyCode; }
		byte * GetReplyData()const { return m_pbyReplyData; }
		uint64 GetReplyDataLen() const { return m_dwReplyDataLen; }
		DWORD  GetReserveValue1()const { return m_dwReserveValue1; }
		DWORD  GetReserveValue2()const { return m_dwReserveValue2; }

	private:
		string m_strReplyContent;
		DWORD  m_dwReplyCode;
		byte  *m_pbyReplyData;
		DWORD  m_dwReplyDataLen;
		DWORD  m_dwReserveValue1;
		DWORD  m_dwReserveValue2;
	};
}
#endif //__CXRPCREPLY_H__

