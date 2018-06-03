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
//#include "PlatformDataTypeDefine.h"

using namespace CXCommunication;
void* ThreadProcess(void* lpvoid);

CXMessageProcessLevelBase::CXMessageProcessLevelBase()
{
    m_pMessageQueue = NULL;
    m_bStart = false;
    m_pSessionLevelProcess = NULL;
    m_pUserMessageProcess = NULL;
}


CXMessageProcessLevelBase::~CXMessageProcessLevelBase()
{
    if (m_pSessionLevelProcess)
        delete m_pSessionLevelProcess;
    if (m_pUserMessageProcess)
        delete m_pUserMessageProcess;
    m_pSessionLevelProcess = NULL;
    m_pUserMessageProcess = NULL;
}

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

int CXMessageProcessLevelBase::ProcessMessage()
{
    int iRet = 0;
    int iLoopNum = 0;
    CXMessageQueue * pQueue = GetMessageQueue();
    while (IsStart())
    {
        if (pQueue->Wait(1000) == WAIT_OBJECT_0)
        {
            iLoopNum = 0;
            PCXMessageData pMes = (PCXMessageData)pQueue->GetMessage();
            while (pMes != NULL)
            {
                CXConnectionObject *pCon = (CXConnectionObject*)pMes->pConObj;
                if (pMes != NULL)
                {
                    pCon->AddProcessPacketNumber();
                }

                if (pCon->GetSession() == NULL)
                {
                    if (pMes->dwMesCode != CX_SESSION_LOGIN_CODE)
                    {
                        printf("The first packet is not the login packet of the session\n");
                    }
                    else
                    {
                        CXConnectionSession * pSession = NULL;
                        if (m_pSessionLevelProcess == NULL)
                        {
                            m_pSessionLevelProcess = new CXSessionMessageProcess();
                        }

                        iRet = m_pSessionLevelProcess->SessionLogin(pMes,&pSession);
                        if (iRet == RETURN_SUCCEED)
                        {
                            //pCon->SetSession((void*)pSession);
                        }
                        else
                        {
                            printf("Login fails\n");
                        }
                    }
                }
                else
                {
                    CXConnectionSession * pSession = (CXConnectionSession *)pCon->GetSession();
                    DWORD dwMessageCode = pMes->dwMesCode;
                    switch (dwMessageCode)
                    {
                    case CX_SESSION_LOGIN_CODE:
                        break;
                    case CX_SESSION_LOGOUT_CODE:
                        iRet = m_pSessionLevelProcess->SessionLogout(pMes, *pSession);
                        if (iRet != RETURN_SUCCEED)
                        {
                            printf("SessionLogout fails\n");
                            if (iRet == -2)
                            {
                                pCon->Lock();
                                ProcessConnectionError(pCon);
                                pCon->UnLock();
                            }
                        }
                        break;
                    case CX_SESSION_SETITING_CODE:
                        
                        iRet = m_pSessionLevelProcess->SessionSetting(pMes, *pSession);
                        if (iRet != RETURN_SUCCEED)
                        {
                            printf("SessionSetting fails\n");
                            if (iRet == -2)
                            {
                                pCon->Lock();
                                ProcessConnectionError(pCon);
                                pCon->UnLock();
                            }
                        }
                        break;
                    default:
                        if (m_pUserMessageProcess == NULL)
                        {
                            m_pUserMessageProcess = new CXUserMessageProcess();
                        }
                        iRet = m_pUserMessageProcess->OnReceivedMessage(pMes, pCon, pSession);
                        if (iRet != RETURN_SUCCEED)
                        {
                            printf("Process message fails\n");
                            if (iRet == -2)
                            {
                                pCon->Lock();
                                ProcessConnectionError(pCon);
                                pCon->UnLock();
                            }
                        }

                        break;
                    }
                }
                pCon->FreeBuffer(pMes);

                //<Process the closing event of this socket>
                //pCon->Lock();
                pCon->ReduceReceivedPacketNumber();
                if (pCon->GetState() >= 3)//closing
                {
                    //ProcessConnectionError(pCon);
                    CXCommunicationServer *pServer = (CXCommunicationServer *)pCon->GetServer();
                    pServer->CloseConnection(*pCon);
                }
                //pCon->UnLock();

                pMes = (PCXMessageData)pQueue->GetMessage();
                /*
                while (pMes==NULL && iLoopNum<1000)
                {
                    pMes = (PCXBufferObj)pQueue->GetMessage();
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
    m_threadProcess.Wait();
}

int CXMessageProcessLevelBase::ProcessConnectionError(CXConnectionObject * pCon)
{
    CXCommunicationServer *pServer = (CXCommunicationServer *)pCon->GetServer();
    CXConnectionsManager & connectionsManager = pServer->GetConnectionManager();
    pServer->CloseConnection(*pCon,false);
    return 0;
}
