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
#include "CXTaskPrepareMessage.h"
#include "CXGuidObject.h"
#include "CXRPCObjectClientInServer.h"
#include "CXConnectionObject.h"
#include "CXConnectionSession.h"
#include "PlatformFunctionDefine.h"
using namespace CXCommunication;
CXTaskPrepareMessage::CXTaskPrepareMessage()
{
	m_taskType = CX_RUNING_TASK_TYPE_PREPARE_DATA;
	Reset();
}


CXTaskPrepareMessage::~CXTaskPrepareMessage()
{
	
}

CXTaskBase* CXTaskPrepareMessage::CreateObject()
{
	CXTaskBase* pTask = NULL;
	try
	{
		pTask = new CXTaskPrepareMessage();
	}
	catch (...)
	{
	}

	return pTask;
}

void CXTaskPrepareMessage::SetTaskInfo(CXConnectionObject* pCon, PCXMessageData  pMessageData, PCXBufferObj pBuf)
{
	m_pCon = pCon;
	m_pSendBuf = pBuf;
	m_pMessageData = pMessageData;
	if (m_pCon != NULL && m_pSendBuf!= NULL && m_pMessageData!=NULL)
	{
		m_bSetParas = true;
	}
}
void CXTaskPrepareMessage::Reset()
{
	m_pCon = NULL;
	m_pSendBuf = NULL;
	m_pMessageData = NULL;
}


DWORD CXTaskPrepareMessage::ProcessTask()
{
	int iRet = 0;
    CXGuidObject guidObj(false);
    PCXPacketBodyData pBodyData = (PCXPacketBodyData)(m_pSendBuf->wsaBuf.buf + sizeof(CXPacketHeader));
    string strRequestID = guidObj.ConvertGuid(pBodyData->byRequestID);
	string strObjectID = guidObj.ConvertGuid(pBodyData->byObjectGuid);
	m_pCon->LockSend();
	if (!m_pCon->PreparedAndSendPacket(m_pSendBuf))
	{
        m_pCon->UnlockSend();

		iRet = CXRET_SEND_DATA_FAILED;
		if (m_pMessageData ==NULL) //send data to remote object
		{
			CXConnectionSession* pSession = (CXConnectionSession*)m_pCon->GetSession();
			if (pSession != NULL)
			{
				CXRPCObjectClientInServer *pObj = (CXRPCObjectClientInServer*)pSession->FindObject(strObjectID);
				if (pObj != NULL)
				{
                    string strError = "Failed to send request data to the remote object ";
                    strError += strObjectID;
                    pObj->OnSendDataError(strRequestID, strError);
				}
				else
				{
                    //the request objec id is a class id
                    if (pObj == NULL && m_pCon->IsProxyConnection())
                    {
                        //the server generate a new object for this proxy request
                        pObj = (CXRPCObjectClientInServer *)m_pCon->PopRPCObj(strRequestID);
                        if (pObj != NULL)
                        {
                            string strError = "Failed to send request data to the remote object ";
                            strError += strObjectID;
                            return  pObj->OnSendDataError(strRequestID, strError);
                        }
                    }
                    

					char  szInfo[1024] = { 0 };
					sprintf_s(szInfo, 1024, "Failed to reply third-party data to the requested object, the object '%s' may have been closed.",
						strObjectID.c_str());
					m_pCon->GetLogHandle()->Log(CXLog::CXLOG_ERROR, szInfo);
					return CXRET_NOT_FOUND_REQUESTED_OBJ;
				}
			}
			else
			{
				char  szInfo[1024] = { 0 };
				sprintf_s(szInfo, 1024, "Failed to get the session of the connection attached on the object '%s'.",
					strObjectID.c_str());
				m_pCon->GetLogHandle()->Log(CXLog::CXLOG_ERROR, szInfo);
				return CXRET_NOT_FOUND_REQUESTED_OBJ;
			}
		}
	}
    else
    {
        m_pCon->UnlockSend();
    }

	if (m_pMessageData != NULL)
	{
		m_pMessageData->iProcessedRet = iRet;
		m_pCon->CallbackTaskProcess(iRet, (int)m_taskType, m_pMessageData, strRequestID);
	}
	
	return (DWORD)iRet;
}
