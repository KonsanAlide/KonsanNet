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
#include "CXRPCObjectServer.h"
#include "CXGuidObject.h"
#include "CXPacketCodeDefine.h"
#include "PlatformFunctionDefine.h"
#include <map>
#include <chrono>
#include <ctime>
using namespace std;

using namespace CXCommunication;
CXRPCObjectServer::CXRPCObjectServer()
{
    m_strObjectGuid = "";
    memset(m_byObjectGuid, 0, 16);
    m_bIsOpened = false;
    m_bIsUniqueInstance = false;
    
    m_dwTimeoutSecs=10;
    //if the process time of a operation is larger than  m_dwSlowOpsSecs senconds,
    //this operation is a slow operation ,must record it to log
    m_dwSlowOpsMS =1000;
}


CXRPCObjectServer::~CXRPCObjectServer()
{
}

void CXRPCObjectServer::GetObjectGuid()
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

//pszTimeString: if not NULL,will save the time string , the format is: 2019-07-31_15:39:29
int64 CXRPCObjectServer::GetCurrentTimeMS(char *pszTimeString)
{
    typedef chrono::time_point<chrono::system_clock, chrono::milliseconds> MsClockType;
    MsClockType tp = chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now());

    if (pszTimeString != NULL)
    {
        time_t timeCur = chrono::system_clock::to_time_t(tp);
        DWORD dwMillsSecond = (DWORD)(timeCur % 1000);
        std::strftime(pszTimeString, 60, "%Y-%m-%d_%H:%M:%S", std::localtime(&timeCur));
        char szMSTime[10] = { 0 };
        sprintf_s(szMSTime, 10, ".%d", dwMillsSecond);
        strcat(pszTimeString, szMSTime);
    }

    return (int64)tp.time_since_epoch().count();
}

