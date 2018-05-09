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
#ifndef CXMUTEXLOCK_H
#define CXMUTEXLOCK_H

#ifdef WIN32
	#include <Windows.h>
#else
	#include <pthread.h>
#endif
#include <string>
using namespace std;

class CXMutexLock
{
    public:
        CXMutexLock(bool bLock=false,string strMutexName="");
        void Lock();
        void Unlock();
        virtual ~CXMutexLock();
    protected:
    private:
#ifdef WIN32
		HANDLE m_mutex;
#else
		pthread_mutex_t m_mutex;
        pthread_mutexattr_t m_mattr;
#endif
        bool m_bInitLocked;
};

#endif // CRLOCK_H
