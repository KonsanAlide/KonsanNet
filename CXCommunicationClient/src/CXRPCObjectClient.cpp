/****************************************************************************
Copyright (c) 2018-2019 Chance Yang

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
#include "CXRPCObjectClient.h"
#include "CXCommonPacketStructure.h"
#include "CXFilePacketStructure.h"
#include "CXPacketCodeDefine.h"
#include "CXGuidObject.h"

#ifdef WIN32
#else
#include <string.h>
#endif


using namespace CXCommunication;
CXRPCObjectClient::CXRPCObjectClient()
{
    m_strRemoteIP="";
    m_unRemotePort=0;
    m_strRemoteFilePath = "";
    m_strRemoteUser = "";
    m_strRemotePassword = "";
    m_bIsOpened = false;
	m_strObjectGuid = "";
	memset(m_byObjectGuid, 0, 16);
}


CXRPCObjectClient::~CXRPCObjectClient()
{
	if (m_bIsOpened)
	{
		m_cxSession.Close();
		m_bIsOpened = false;
	}
}


void CXRPCObjectClient::SetRemoteServerInfo(string strRemoteIP, unsigned short unRemotePort,
    string strRemoteUser, string strRemotePassword)
{
    m_strRemoteIP = strRemoteIP;
    m_unRemotePort = unRemotePort;
    m_strRemoteUser = strRemoteUser;
    m_strRemotePassword = strRemotePassword;
    m_cxSession.SetUserInfo(m_strRemoteUser, m_strRemotePassword);
    CXSocketAddress addr(m_strRemoteIP.c_str(), m_unRemotePort);
    m_cxSession.SetRemoteAddress(addr);
}

void CXRPCObjectClient::GetObjectGuid()
{
	string strName = GetObjectName();
	map<string, string>::const_iterator  it = g_mapRPCObjectGuid.find(strName);
	if (it != g_mapRPCObjectGuid.end())
	{
		m_strObjectGuid = it->second;
		CXGuidObject guidObject(false);
		guidObject.ConvertGuid(m_strObjectGuid, m_byObjectGuid);
	}
}
