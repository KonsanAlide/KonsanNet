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
#include "CXCommunicationServer.h"
#include "CXTaskCreateConnection.h"
#include <map>
#include <chrono>
#include <ctime>

using namespace std;
using namespace CXCommunication;
CXRPCObjectServer::CXRPCObjectServer()
{
	m_strClassGuid = "";
    m_strObjectGuid = "";
    memset(m_byObjectGuid, 0, CX_GUID_LEN);
	memset(m_byClassGuid, 0, CX_GUID_LEN);
    m_bIsUniqueInstance = false;
    
    m_dwTimeoutSecs=10;
    //if the process time of a operation is larger than  m_dwSlowOpsSecs senconds,
    //this operation is a slow operation ,must record it to log
    m_dwSlowOpsMS =100;

	m_pLogHandle=NULL;
	m_pJournalLogHandle = NULL;
	m_pIOStatHandle = NULL;
	m_bContinuousObject = false;
	m_bIsUsing = false;
    m_pComServer = NULL;
    m_bNeedToDestroy = false;
}


CXRPCObjectServer::~CXRPCObjectServer()
{
}

//initialize this object
void CXRPCObjectServer::Init()
{
	if (!m_bIsUniqueInstance)
	{
		CXGuidObject guidObject(false);
		m_strObjectGuid = guidObject.GenerateNewGuid(m_byObjectGuid);
		m_bIsUsing = true;
	}
}

void CXRPCObjectServer::Reset()
{
	m_strObjectGuid = "";
	memset(m_byObjectGuid, 0, CX_GUID_LEN);
	memset(m_byClassGuid, 0, CX_GUID_LEN);
	m_bIsUsing = false;
    m_bNeedToDestroy = false;
}

void CXRPCObjectServer::GetClassGuid()
{
    string strName = GetObjectClassName();
    map<string, string>::const_iterator  it = g_mapRPCObjectGuid.find(strName);
    if (it != g_mapRPCObjectGuid.end())
    {
		m_strClassGuid = it->second;
		CXGuidObject guidObject(false);
		guidObject.ConvertGuid(m_strClassGuid, m_byClassGuid);
    }
}

int CXRPCObjectServer::ProcessMessage(PCXMessageData pMes)
{
	pMes->iBeginProcessTime = GetCurrentTimeMS();

	CXGuidObject guidObj(false);
	string strRequestID = guidObj.ConvertGuid(pMes->bodyData.byRequestID);
	
    CXConnectionObject *pCon = (CXConnectionObject*)pMes->pConObj;
    if (pCon == NULL)
    {
        if(m_pIOStatHandle!=NULL)
            m_pIOStatHandle->EndIOStat("total", strRequestID, pMes->iBeginProcessTime);
        return CXRET_MESSAGE_INCOMPLETE;
    }

    if (m_pIOStatHandle != NULL)
        m_pIOStatHandle->PushIOStat("queue", strRequestID, pMes->iBeginProcessTime- pMes->iBeginTime);

	int iRet = 0;
	
	iRet = DispatchMes(pMes);

	pMes->iProcessedRet = iRet;

	if (!pCon->IsThisMesAsynchronous())
	{
		OutputResult(pMes, strRequestID);
	}

	return iRet;
}

void CXRPCObjectServer::OutputResult(PCXMessageData pMes, string strPacketGuid, bool bTimeout)
{
	if (pMes == NULL)
	{
		return;
	}
	CXConnectionObject *pCon = (CXConnectionObject*)pMes->pConObj;
	if (pCon == NULL)
	{
		return;
	}

	string strMes = "";
	//format the message content
	MessageToString(pMes, strMes);

	CXLog::CXLOG_LEVEL logLevel = CXLog::CXLOG_INFO;
	int64  iEndTime = GetCurrentTimeMS();
	int64  iIntervalMillTime = iEndTime - pMes->iBeginTime;
	uint64 uiIndex = ((CXConnectionObject*)pMes->pConObj)->GetConnectionIndex();
	char   szInfo[1024] = { 0 };
	if (bTimeout)
	{
		sprintf_s(szInfo, 1024, ",timeout,total_time:%lldms,queue_time:%lldms,process_time:%lldms,process_ret:%d,connetion index:%lld",
			iIntervalMillTime, pMes->iBeginProcessTime - pMes->iBeginTime,
			iEndTime - pMes->iBeginProcessTime, pMes->iProcessedRet, uiIndex);
		if (m_pLogHandle != NULL)
		{
            if (pCon->IsProxyConnection())
            {
                strMes = "[RPCMessage] "+ strMes;
            }
			strMes += szInfo;
			m_pLogHandle->Log(logLevel, strMes.c_str());
		}
		return;
	}
	else
	{
        if (pCon->IsProxyConnection())
        {
            strMes = "[RPCMessage] " + strMes;
        }

		sprintf_s(szInfo, 1024, ",total_time:%lldms,queue_time:%lldms,process_time:%lldms,process_ret:%d,connetion index:%lld",
			iIntervalMillTime, pMes->iBeginProcessTime - pMes->iBeginTime,
			iEndTime - pMes->iBeginProcessTime, pMes->iProcessedRet, uiIndex);
	}
	

	if (pMes->iProcessedRet != 0)
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
	{
		if (pCon->IsProxyConnection())
		{
			char szIndex[32] = { 0 };
			sprintf_s(szIndex, 32, "%lld", pCon->GetConnectionIndex());
			strPacketGuid += szIndex;
		}
		m_pIOStatHandle->EndIOStat("total", strPacketGuid, iEndTime);
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
        std::strftime(pszTimeString, 60, "%Y-%m-%d_%H:%M:%S", std::localtime(&timeCur));
        char szMSTime[10] = { 0 };
        sprintf_s(szMSTime, 10, ".%03d", (int)(tp.time_since_epoch().count() % 1000));
        strcat(pszTimeString, szMSTime);
    }

    return (int64)tp.time_since_epoch().count();
}

void CXRPCObjectServer::AsynCallbackResult(PCXMessageData pMes)
{
	int iRet = 0;
	if (pMes==NULL)
	{
		return;
	}
	CXConnectionObject *pCon = (CXConnectionObject*)pMes->pConObj;
	if (pCon == NULL)
	{
		return;
	}

	CXGuidObject guidObj(false);
	string strRequestID = guidObj.ConvertGuid(pMes->bodyData.byRequestID);
	OutputResult(pMes, strRequestID);
}

void CXRPCObjectServer::PushState(string strPacketGuid, void* pMes)
{
	m_lock.Lock();
	map<string, deque<void*>>::iterator it = m_mapStateMachine.find(strPacketGuid);
	if (it != m_mapStateMachine.end())
	{
		it->second.push_back(pMes);
	}
	else
	{
		deque<void*> dequeMes;
		dequeMes.push_back(pMes);
		m_mapStateMachine.insert(make_pair(strPacketGuid, dequeMes));
	}
	m_lock.Unlock();
}
void* CXRPCObjectServer::PopState(string strPacketGuid)
{
	void* pMes = NULL;
	m_lock.Lock();
	map<string, deque<void*>>::iterator it = m_mapStateMachine.find(strPacketGuid);
	if (it != m_mapStateMachine.end())
	{
		pMes = it->second.back();
		it->second.pop_back();
		if (it->second.size() == 0)
		{
			m_mapStateMachine.erase(it);
		}
	}
	m_lock.Unlock();
	return pMes;
}
string CXRPCObjectServer::GetPacketGuid(PCXMessageData pMes)
{
	CXGuidObject guidObject(false);
	return guidObject.ConvertGuid(pMes->bodyData.byRequestID);
}


// receive a reply from the other object
int CXRPCObjectServer::ReceiveInternalReplyData(const string &strRequestID,
	DWORD dwReplyFunCode, const CXRPCReply & rpcReply)
{
	//int iLastRet, void* pData, DWORD dwLen,
	//	CXConnectionObject *pCon, string strPacketGuid, CXConnectionObject *pSourceCon

    if (dwReplyFunCode == 0 || strRequestID.length()==0)
        return INVALID_PARAMETER;

    int iProcessRet = RETURN_SUCCEED;

    PCXStateStepInfo pStepInfo = (PCXStateStepInfo)PopState(strRequestID);
    if (pStepInfo == NULL)
    {
        return INVALID_PARAMETER;
    }
    PCXMessageData pPrevMes = (PCXMessageData)pStepInfo->pMes;
    if (pPrevMes == NULL) // not found the previous step in  the state machine
    {
        return iProcessRet;
    }
    else
    {
		CXConnectionObject *pCon = (CXConnectionObject *)pPrevMes->pConObj;
		if (pPrevMes == NULL) // not found the previous step in  the state machine
		{
			return INVALID_PARAMETER;
		}

        ReplyFun funReply = m_mapStepFuns[pStepInfo->dwNextState];

		pCon->LockSend();

		bool bOldMesAsync = pCon->IsThisMesAsynchronous();
		pCon->SetThisMesAsynchronous(false);

        iProcessRet = funReply(pPrevMes, pCon, rpcReply);

		bool bThisMesAsync = pCon->IsThisMesAsynchronous();
		pCon->SetThisMesAsynchronous(bOldMesAsync);

		pCon->UnlockSend();
		if (!bThisMesAsync)
		{
			pPrevMes->dwType = 2;
			pCon->PushReceivedMessage(pPrevMes, false);
		}
        if (iProcessRet != CXRET_ASYNC_CALLBACK)
        {
            if (iProcessRet != 0)
            {
                printf("Process message fails\n");
            }
        }
    }
    return iProcessRet;
}

void CXRPCObjectServer::RegisterStepFun(DWORD dwStateCode, ReplyFun fun)
{
    m_mapStepFuns[dwStateCode]= fun;
}

void CXRPCObjectServer::RegisterEntryFun(DWORD dwStateCode, EntryFun fun)
{
    m_mapEntryFuns[dwStateCode] = fun;
}
void CXRPCObjectServer::DetectTimeoutRequests()
{
	m_lock.Lock();
	int64 iUsedTime = 0;
	PCXStateStepInfo pStepInfo = NULL;
	PCXMessageData pPrevMes = NULL;
	map<string, deque<void*>>::iterator it = m_mapStateMachine.begin();
	for (;it != m_mapStateMachine.end();++it)
	{
		//it->second.push_back(pMes);
		pStepInfo = (PCXStateStepInfo)(*(it->second.begin()));
		if (pStepInfo != NULL)
		{
			pPrevMes = (PCXMessageData)pStepInfo->pMes;
			if (pPrevMes != NULL)
			{
				iUsedTime = GetCurrentTimeMS()- pPrevMes->iBeginTime;
				if (iUsedTime > m_dwTimeoutSecs * 1000)
				{
					CXGuidObject guidObj(false);
					string strRequestID = guidObj.ConvertGuid(pPrevMes->bodyData.byRequestID);
					OutputResult(pPrevMes, strRequestID,true);
				}
			}
		}
	}

	m_lock.Unlock();
}
