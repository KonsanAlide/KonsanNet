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

#include "CXRPCReply.h"
#include "PlatformFunctionDefine.h"

#include <stdio.h>
#include <string.h>

using namespace CXCommunication;
CXRPCReply::CXRPCReply()
{
	m_strReplyContent = "";
	m_dwReplyCode = 0;
	m_pbyReplyData = NULL;
	m_dwReplyDataLen = 0;
	m_dwReserveValue1 = 0;
	m_dwReserveValue2 = 0;
}


CXRPCReply::CXRPCReply(string strContent, DWORD dwReplyCode, byte *pbyReplyData, DWORD  dwReplyDataLen,
	DWORD dwReserveValue1, DWORD dwReserveValue2)
{
	m_strReplyContent = strContent;
	m_dwReplyCode = dwReplyCode;
	m_pbyReplyData = pbyReplyData;
	m_dwReplyDataLen = dwReplyDataLen;
	m_dwReserveValue1 = dwReserveValue1;
	m_dwReserveValue2 = dwReserveValue2;
}


CXRPCReply::~CXRPCReply()
{
}


