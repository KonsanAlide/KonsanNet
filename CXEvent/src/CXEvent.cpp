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
    sem_init(&m_hEvent,0,0);
#endif // WIN32

}

CXEvent::~CXEvent()
{
#ifdef WIN32
    ::CloseHandle(m_hEvent);
#else
    sem_destroy(&m_hEvent);
#endif // WIN32

}

void CXEvent::SetEvent()
{
#ifdef WIN32
    ::SetEvent(m_hEvent);
#else
    sem_post(&m_hEvent);
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
    if(INFINITE!=dwMilliseconds)
    {
        clock_gettime(CLOCK_REALTIME, &timerWait);
        timerWait.tv_sec += dwMilliseconds / 1000;
        timerWait.tv_nsec += (dwMilliseconds%1000) * 1000000;
    }

    int nRet = 0;
    while(true)
    {
        if(INFINITE==dwMilliseconds)
        {
            nRet = sem_wait(&m_hEvent);
        }
        else
        {
            nRet = sem_timedwait(&m_hEvent, &timerWait);
        }

        if(nRet!=0)
        {
            switch (errno)
            {
            case EINTR://The call was interrupted by a signal handler
                continue;
            //The call was interrupted by a signal handler,
            //or The  value  of  abs_timeout.tv_nsecs  is less than 0, or greater than or equal to 1000 million.
            case EINVAL:
                dwRet = WAIT_FAILED;
                break;
            case ETIMEDOUT:
                dwRet = WAIT_TIMEOUT;
                break;

            default:
                break;
            }
        }
        else
        {
            dwRet = WAIT_OBJECT_0;
        }

        break;
    }

#endif // WIN32

    return dwRet;
}
