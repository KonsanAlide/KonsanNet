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
#include "CXMessageProcessLevelBase.h"
#include "CXPacketCodeDefine.h"
#include "CXConnectionsManager.h"
#include "CXCommunicationServer.h"
#include "CXGuidObject.h"
//#include "PlatformDataTypeDefine.h"
#include "PlatformFunctionDefine.h"

using namespace CXCommunication;
void* ThreadProcess(void* lpvoid);

//extern CXSpinLock g_lock;
//extern int g_iTotalProcessMessage;

CXMessageProcessLevelBase::CXMessageProcessLevelBase()
{
    m_pMessageQueue = NULL;
    m_bStart = false;
	m_pLogHandle = NULL;
    m_pRPCObjectManager = NULL;
	m_pThread = NULL;
}


CXMessageProcessLevelBase::~CXMessageProcessLevelBase()
{
}

DWORD CXMessageProcessLevelBase::Run(void *pThis)
{
	if (pThis == NULL)
	{
		return DWORD(-1);
	}
	return ((CXMessageProcessLevelBase*)pThis)->ProcessMessage();
}
/*
int  CXMessageProcessLevelBase::Run()
{
    RunFun funThread = &ThreadProcess;
    int iRet = m_threadProcess.Start(funThread, (void*)this);
    if (iRet != 0)
    {
        printf("Create message process thread fails\n");
        m_bStart = false;
        return -2;
    }

    m_bStart = true;
    return RETURN_SUCCEED;
}
*/

int CXMessageProcessLevelBase::ProcessMessage()
{
    int iRet = 0;
    int iLoopNum = 0;
	int64 iBeginTimeInProcess = 0;
    CXGuidObject guidObj(false);
	m_bStart = true;
    CXMessageQueue * pQueue = GetMessageQueue();
    while (IsStart())
    {
        pQueue->Wait(10);
        //if (pQueue->Wait(1000) == WAIT_OBJECT_0)
        {
            iLoopNum = 0;
            PCXMessageData pMes = (PCXMessageData)pQueue->GetMessage();
            while (pMes != NULL)
            {
                CXConnectionObject *pCon = (CXConnectionObject*)pMes->pConObj;
                if (pCon != NULL)
                {
                    //pCon->AddProcessPacketNumber();
                }
				else
				{
					char szInfo[1024] = { 0 };
					sprintf_s(szInfo, 1024, "Find a incorrect packet, the connection object is empty,pMes=%x\n", (byte*)pMes);
					m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
					return -1;
				}

                //string strPacketGUID = guidObj.ConvertGuid(pMes->bodyData.byPacketGuid);
                //int64  iBeginTime = pCon->GetCurrentTimeMS();
                //pCon->GetIOStat()->PushIOStat("wait_queue", strPacketGUID, iBeginTime - pMes->iBeginTime);

                char szInfo[1024] = { 0 };
                uint64 uiSequenceNum = pMes->iSequenceNum;
                sprintf_s(szInfo, 1024, "Begin to ProcessUnpackedMessage message,SequenceNum:%lld, connection index:%lld,pCurMes:%x\n",
                    uiSequenceNum, pCon->GetConnectionIndex(), pMes);
                //m_pLogHandle->Log(CXLog::CXLOG_DEBUG, szInfo);

				if (pMes->dwType == 2) //async call
				{
					pCon->ProcessAsyncMessageEnd(pMes);
					pCon->FreeBuffer(pMes);
					pCon->ProcessUnpackedMessage(NULL);
				}
				else
				{
					iRet = pCon->ProcessUnpackedMessage(pMes);
				}
                
                /*
                int64  iEndTime = pCon->GetCurrentTimeMS();
                uint64 uiIndex = ((CXConnectionObject*)pMes->pConObj)->GetConnectionIndex();
                if (iEndTime - iBeginTime > 30)
                {
                    char   szInfo[1024] = { 0 };
                    sprintf_s(szInfo, 1024, ",process_time:%lldms,connetion index:%lld",
                        iEndTime - iBeginTime, uiIndex);
                    m_pLogHandle->Log(CXLog::CXLOG_WARNNING, szInfo);
                }

                //pCon->GetIOStat()->PushIOStat("process", strPacketGUID, iEndTime - iBeginTime);
                //pCon->FreeBuffer(pMes);
                */

                pMes = (PCXMessageData)pQueue->GetMessage();
               /*
                while (pMes==NULL && iLoopNum<1000)
                {
                    pMes = (PCXMessageData)pQueue->GetMessage();
                    iLoopNum++;
                }
                if(pMes)
                    iLoopNum = 0;
                */
            }
        }
    }
    return RETURN_SUCCEED;
}

void* ThreadProcess(void* lpvoid)
{
    CXMessageProcessLevelBase* pServer = (CXMessageProcessLevelBase*)lpvoid;
    pServer->ProcessMessage();
    return 0;
}


void CXMessageProcessLevelBase::Stop()
{
    m_bStart = false;
    if(m_pThread!=NULL)
        m_pThread->Wait();
}

int CXMessageProcessLevelBase::ProcessConnectionError(CXConnectionObject * pCon)
{
    CXCommunicationServer *pServer = (CXCommunicationServer *)pCon->GetServer();
    CXConnectionsManager & connectionsManager = pServer->GetConnectionManager();

    char szInfo[1024] = {0};
    sprintf_s(szInfo,1024, "Failed to process message,close this connection,connection id=%lld,error code is %d,desc is '%s'\n",
                  pCon->GetConnectionIndex(),errno,strerror(errno));
    //m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);

    pServer->CloseConnection(*pCon, ERROR_IN_PROCESS,false);
    return 0;
}
