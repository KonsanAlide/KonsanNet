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
#include "CXMessageQueue.h"
using namespace CXCommunication;

CXMessageQueue::CXMessageQueue()
{
}

CXMessageQueue::~CXMessageQueue()
{
    //dtor
}

void CXMessageQueue::PushMessage(void*pMes)
{
    m_lock.Lock();
    m_queue.push(pMes);
    m_event.SetEvent();
    m_lock.Unlock();
}

void* CXMessageQueue::GetMessage()
{
    m_lock.Lock();
    if (m_queue.size() > 0)
    {
        void*pMes = m_queue.front();
        m_queue.pop();
        m_lock.Unlock();
        return pMes;
    }
    
    m_lock.Unlock();
    return NULL;
}

DWORD CXMessageQueue::Wait(DWORD dwMilliseconds)
{
    return m_event.WaitForSingleObject(dwMilliseconds);
}

