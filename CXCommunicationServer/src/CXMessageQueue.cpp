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
    m_queue.Push(pMes);
    //if(m_queue.size()==1)
       m_event.SetEvent();
    m_lock.Unlock();
}

void* CXMessageQueue::GetMessage()
{
    if (m_queue.Size() > 0)
    {
        m_lock.Lock();
        void*pMes = NULL;
        if (m_queue.Size() > 0)
        {
            pMes = m_queue.PopFront();
        }

        m_lock.Unlock();
        return pMes;
    }
    
    
    return NULL;
}

DWORD CXMessageQueue::Wait(DWORD dwMilliseconds)
{
    return m_event.WaitForSingleObject(dwMilliseconds);
}

