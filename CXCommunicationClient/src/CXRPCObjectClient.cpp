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
#include <map>
#include <chrono>
#include <ctime>
#include "PlatformFunctionDefine.h"

#ifdef WIN32
#else
#include <string.h>
#endif

using namespace std;
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

void CXRPCObjectClient::GetClassGuid()
{
	string strName = GetObjectClassName();
	map<string, string>::const_iterator  it = g_mapRPCObjectGuid.find(strName);
	if (it != g_mapRPCObjectGuid.end())
	{
		m_strObjectGuid = it->second;
		CXGuidObject guidObject(false);
		guidObject.ConvertGuid(m_strObjectGuid, m_byObjectGuid);
	}
}

void CXRPCObjectClient::SetDataPaserHandle(CXDataParserImpl *pHandle)
{
	m_cmmClient.SetDataParserHandle(pHandle);
	m_dataClient.SetDataParserHandle(pHandle);
}

void CXRPCObjectClient::SetEncryptParas(CXDataParserImpl::CXENCRYPT_TYPE encryptType)
{
	m_cmmClient.SetEncryptParas(encryptType);
	m_dataClient.SetEncryptParas(encryptType);
}
void CXRPCObjectClient::SetCompressParas(CXDataParserImpl::CXCOMPRESS_TYPE compressType)
{
	m_cmmClient.SetCompressParas(compressType);
	m_dataClient.SetCompressParas(compressType);
}
//pszTimeString: if not NULL,will save the time string , the format is: 2019-07-31_15:39:29
int64 CXRPCObjectClient::GetCurrentTimeMS(char *pszTimeString)
{
    typedef chrono::time_point<chrono::system_clock, chrono::milliseconds> MsClockType;
    MsClockType tp = chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now());

    if (pszTimeString != NULL)
    {
        time_t timeCur = chrono::system_clock::to_time_t(tp);
        std::strftime(pszTimeString, 60, "%Y-%m-%d_%H:%M:%S", std::localtime(&timeCur));
        char szMSTime[10] = { 0 };
        sprintf_s(szMSTime, 10, ".%03d", (int)(tp.time_since_epoch().count() % 1000));
        strcat(pszTimeString, szMSTime);
    }

    return (int64)tp.time_since_epoch().count();
}

void CXRPCObjectClient::SetUsedMemoryCachePool(bool bSet, CXMemoryCacheManager* pCacheObj)
{
	m_cmmClient.SetUsedMemoryCachePool(bSet, pCacheObj);
	m_dataClient.SetUsedMemoryCachePool(bSet, pCacheObj);
}