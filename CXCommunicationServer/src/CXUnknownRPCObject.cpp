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

Description£º
*****************************************************************************/
#include "CXUnknownRPCObject.h"
using namespace CXCommunication;
CXUnknownRPCObject::CXUnknownRPCObject()
{
    m_bIsUniqueInstance = true;
    GetObjectGuid();
}


CXUnknownRPCObject::~CXUnknownRPCObject()
{
}

//return value:==-2 need to close the connection
int CXUnknownRPCObject::DispatchMes(PCXMessageData pMes)
{
	int     iRet = -2;
    return iRet;
}


CXRPCObjectServer* CXUnknownRPCObject::CreateObject()
{
    return (CXRPCObjectServer*)new CXUnknownRPCObject;
}

int CXUnknownRPCObject::SendData(CXConnectionObject * pCon, const byte *pbyData, DWORD dwDataLen)
{
    return RETURN_SUCCEED;
}

void CXUnknownRPCObject::Destroy()
{
}

void CXUnknownRPCObject::MessageToString(PCXMessageData pMes)
{
	char  szInfo[1024] = { 0 };
	CXGuidObject guidObj(false);
	string strPacketGUID = guidObj.ConvertGuid(pMes->bodyData.byPacketGuid);

	sprintf_s(szInfo, 1024, "unknown object message, packet_guid:%s,message_code:%04d,message length:%d",
		strPacketGUID.c_str(), pMes->bodyData.dwMesCode, pMes->dwDataLen);
	m_strLastMessageContent = szInfo;
}





