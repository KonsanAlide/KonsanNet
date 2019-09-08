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

#include "CXRPCRequest.h"
#include "PlatformFunctionDefine.h"

#include <stdio.h>
#include <string.h>
#include <chrono>
#include <ctime>

using namespace std;
using namespace CXCommunication;
CXRPCRequest::CXRPCRequest()
{
	m_strRequestID = "";
	m_dwReplyFunCode = 0;
	m_strRequesterObjectID = "";
    memset(m_byRequestIDBytes,0, CX_GUID_LEN);
    m_iBeginRequestTime = GetCurrentTimeMSValue();
    m_pRequesterObj = NULL;
}

CXRPCRequest::CXRPCRequest(string strRequestID, DWORD dwReplyFunCode, string strRequesterObjectID,
    byte byRequestIDBytes[], void *pRequesterObj)
{
	m_strRequestID = strRequestID;
	m_dwReplyFunCode = dwReplyFunCode;
	m_strRequesterObjectID = strRequesterObjectID;
    memcpy(m_byRequestIDBytes, byRequestIDBytes, CX_GUID_LEN);
    m_iBeginRequestTime = GetCurrentTimeMSValue();
    m_pRequesterObj = pRequesterObj;
}


CXRPCRequest::~CXRPCRequest()
{
}

//==true : all attributes had been valid
bool CXRPCRequest::IsParametersValid()
{
	if (m_strRequestID.length() == 0 || m_dwReplyFunCode == 0
		|| m_strRequesterObjectID.length()==0 || m_pRequesterObj==NULL)
	{
		return false;
	}

	return true;
}

int64 CXRPCRequest::GetCurrentTimeMSValue()
{
    typedef chrono::time_point<chrono::system_clock, chrono::milliseconds> MsClockType;
    MsClockType tp = chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now());
    return (int64)tp.time_since_epoch().count();
}
