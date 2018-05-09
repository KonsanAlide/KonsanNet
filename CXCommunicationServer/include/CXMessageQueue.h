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
#ifndef CXMESSAGEQUEUE_H
#define CXMESSAGEQUEUE_H

#include "CXConnectionObject.h"
#include "CXSpinLock.h"
#include "CXEvent.h"
#include <queue>
using std::queue;

namespace CXCommunication
{
    class CXMessageQueue
    {
    public:
        CXMessageQueue();
        virtual ~CXMessageQueue();
        void PushMessage(void*pMes);
        void* GetMessage();
        DWORD Wait(DWORD dwMilliseconds);

    private:
        queue<void*> m_queue;
        CXSpinLock   m_lock;
        CXEvent      m_event;
    };
}

#endif // CXMESSAGEQUEUE_H