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
#include "CXMutexLock.h"
#ifndef WIN32
#include <errno.h>
#include<string.h>
#include <stdio.h>
#include <stdlib.h>
#endif

CXMutexLock::CXMutexLock(bool bLock,string strMutexName)
{
    m_bInitLocked = bLock;

#ifdef WIN32
    m_mutex = NULL;
    m_mutex = ::CreateMutex(NULL,FALSE,strMutexName.c_str());
#else
    m_mutex = PTHREAD_MUTEX_INITIALIZER;
    int iRet = pthread_mutexattr_init(&m_mattr);
    if (iRet != 0)
    {
        printf("create mutex attribute error. msg:%s", strerror(errno));
        exit(1);
    }
    pthread_mutexattr_settype(&m_mattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_mutex, &m_mattr);

#endif

    if (bLock)
    {
        Lock();
    }

}

CXMutexLock::~CXMutexLock()
{

#ifdef WIN32
    if (m_mutex != NULL)
    {
        ::CloseHandle(m_mutex);
    }
#else
    if (m_bInitLocked)
        pthread_mutex_unlock(&m_mutex);
    pthread_mutex_destroy(&m_mutex);
    pthread_mutexattr_destroy(&m_mattr);
#endif

}

void CXMutexLock::Lock()
{

#ifdef WIN32
    if (m_mutex != NULL)
    {
        ::WaitForSingleObject(m_mutex, INFINITE);
    }
#else
    pthread_mutex_lock(&m_mutex);
#endif

}
void CXMutexLock::Unlock()
{

#ifdef WIN32
    if (m_mutex != NULL)
    {
        ::ReleaseMutex(m_mutex);
    }
#else
    pthread_mutex_unlock(&m_mutex);
#endif

}
