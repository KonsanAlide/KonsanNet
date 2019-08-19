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
    m_dwSlowOpsMS =100;

	m_pLogHandle=NULL;
	m_pJournalLogHandle = NULL;
	m_pSession = NULL;
	m_pIOStatHandle = NULL;
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

int CXRPCObjectServer::ProcessMessage(PCXMessageData pMes)
{
	int64 iBeginProcessTime = GetCurrentTimeMS();

	CXGuidObject guidObj(false);
	string strPacketGUID = guidObj.ConvertGuid(pMes->bodyData.byPacketGuid);
	if(m_pIOStatHandle!=NULL)
		m_pIOStatHandle->PushIOStat("queue", strPacketGUID, iBeginProcessTime-pMes->iBeginTime);
	string strMes = "";
	//format the message content
	MessageToString(pMes, strMes);

	CXLog::CXLOG_LEVEL logLevel = CXLog::CXLOG_INFO;

	int    iRet = DispatchMes(pMes);

	int64  iEndTime = GetCurrentTimeMS();
	int64  iIntervalMillTime = iEndTime - pMes->iBeginTime;
    uint64 uiIndex = ((CXConnectionObject*)pMes->pConObj)->GetConnectionIndex();
	char   szInfo[1024] = { 0 };
	sprintf_s(szInfo, 1024, ",total_time:%lldms,queue_time:%lldms,process_time:%lldms,process_ret:%d,connetion index:%lld", 
        iIntervalMillTime, iBeginProcessTime -pMes->iBeginTime,
        iEndTime- iBeginProcessTime, iRet, uiIndex);
	
	if (iRet != 0)
	{
		logLevel = CXLog::CXLOG_ERROR;
	}

	bool bSlowOps = false;
	if (iIntervalMillTime > m_dwSlowOpsMS)
	{
		strMes += ",slow_ops";
		strMes += szInfo;
		if (m_pLogHandle != NULL)
		{
			m_pLogHandle->Log(logLevel, strMes.c_str());
		}
	}
	else
	{
		strMes += szInfo;
	}

	if (m_pJournalLogHandle != NULL)
	{	
		m_pJournalLogHandle->Log(logLevel, strMes.c_str());
	}

	if (m_pIOStatHandle != NULL)
		m_pIOStatHandle->EndIOStat("total", strPacketGUID, iEndTime);
	
	return iRet;
}

//pszTimeString: if not NULL,will save the time string , the format is: 2019-07-31_15:39:29
int64 CXRPCObjectServer::GetCurrentTimeMS(char *pszTimeString)
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

