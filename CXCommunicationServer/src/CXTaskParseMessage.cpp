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

#include "CXTaskParseMessage.h"
using namespace CXCommunication;
CXTaskParseMessage::CXTaskParseMessage()
{
	m_taskType = CX_RUNING_TASK_TYPE_PARSE_DATA;
	Reset();
}


CXTaskParseMessage::~CXTaskParseMessage()
{
}


CXTaskBase* CXTaskParseMessage::CreateObject()
{
	CXTaskBase* pTask = NULL;
	try
	{
		pTask = new CXTaskParseMessage();
	}
	catch (...)
	{
	}

	return pTask;
}

void CXTaskParseMessage::SetTaskInfo(PCXMessageData  pMessageData, CXPacketHeader &packetHeader)
{
	m_pMessageData = pMessageData;
	memcpy(&m_packetHeader,&packetHeader,sizeof(CXPacketHeader));
	if (m_pMessageData != NULL)
	{
		m_bSetParas = true;
	}
}
void CXTaskParseMessage::Reset()
{
	m_pMessageData = NULL;
	memset(&m_packetHeader, 0, sizeof(CXPacketHeader));
}


DWORD CXTaskParseMessage::ProcessTask()
{
	DWORD dwRet = 0;
	CXConnectionObject *pCon = (CXConnectionObject*)m_pMessageData->pConObj;
	if (!pCon->ParseMessage(m_pMessageData, m_packetHeader))
	{
		dwRet = -2;
		pCon->CallbackTaskProcess(dwRet, (int)m_taskType, m_pMessageData);
	}
		
	return dwRet;
}
