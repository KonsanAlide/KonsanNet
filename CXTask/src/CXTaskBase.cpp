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
#include "CXTaskBase.h"
using namespace CXCommunication;
CXTaskBase::CXTaskBase()
{
	m_lpCacheObj = NULL;
	m_bRunning = false;
	m_bSetParas = false;
	m_dwResult = 0;
}


CXTaskBase::~CXTaskBase()
{
	
}

void CXTaskBase::StopTask()
{
	if(!m_bRunning)
		return ;
	m_bRunning = false;
}

// wait the thread task to finish
//return value:
//==0 the event has been set,
//==ETIMEDOUT
//==otherwise, error value 
DWORD CXTaskBase::WaitFinished(DWORD dwMilliseconds)
{
	if (!m_bRunning )
	{
		return 0;
	}
	return m_event.WaitForSingleObject(dwMilliseconds);
}

DWORD CXTaskBase::Run()
{
	DWORD dwRet = 0;
	m_bRunning = true;
	m_dwResult= ProcessTask();
	m_event.SetEvent();
	m_bRunning = false;
	return dwRet;
}
