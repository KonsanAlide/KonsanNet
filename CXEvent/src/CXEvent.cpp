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
#include "CXEvent.h"

CXEvent::CXEvent(BOOL bManualReset, BOOL bInitialState)
{
#ifdef WIN32
    m_hEvent = ::CreateEvent(NULL, bManualReset,bInitialState,NULL);
#else
    pthread_mutex_init(&m_mutexEvent, NULL);
    pthread_cond_init(&m_condEvent, NULL);
#endif // WIN32

}

CXEvent::~CXEvent()
{
#ifdef WIN32
    ::CloseHandle(m_hEvent);
#else
    pthread_mutex_destroy(&m_mutexEvent);
    pthread_cond_destroy(&m_condEvent);
#endif // WIN32
  
}

void CXEvent::SetEvent()
{
#ifdef WIN32
    ::SetEvent(m_hEvent);
#else
    pthread_mutex_lock(&m_mutexEvent);
    pthread_cond_signal(&m_condEvent);
    pthread_mutex_unlock(&m_mutexEvent);
#endif // WIN32
    
}

DWORD CXEvent::WaitForSingleObject(DWORD dwMilliseconds)
{
    DWORD dwRet = 0;
#ifdef WIN32
    dwRet = ::WaitForSingleObject(m_hEvent, dwMilliseconds);
#else
    dwRet = WAIT_FAILED;

    struct timespec timerWait;
    timerWait.tv_sec = time(NULL) + dwMilliseconds / 1000;
    timerWait.tv_nsec = dwMilliseconds % 1000;
    int nRet = 0;
    pthread_mutex_lock(&m_mutexEvent);
    int nRet = pthread_cond_timedwait(&m_condEvent, &m_mutexEvent, &timer);
    pthread_mutex_unlock(&m_mutexEvent);

    switch (nRet)
    {
    case 0://succeed
        dwRet = WAIT_OBJECT_0;
        break;
    case EINVAL:
        dwRet = WAIT_FAILED;
        break;
    case ETIMEDOUT:
        dwRet = WAIT_TIMEOUT;
        break;
        
    default:
        break;
    }

#endif // WIN32
    
    return dwRet;
}
