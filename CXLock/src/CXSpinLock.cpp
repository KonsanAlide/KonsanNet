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
#include "CXSpinLock.h"

CXSpinLock::CXSpinLock(bool bLock)
{
	m_bInitLocked = bLock;
	m_lLockRefNum = 0;
	
#ifdef WIN32
	InitializeCriticalSection(&m_lock);
	if (m_bInitLocked)
	{
		EnterCriticalSection(&m_lock);
		m_lLockRefNum = 1;
	}
#else
	m_lock = SPINLOCK_INITIALIZER;
	if (m_bInitLocked)
	{
		spin_lock(&m_lock);
		m_lLockRefNum = 1;
	}
#endif
}

CXSpinLock::~CXSpinLock()
{
    if(m_bInitLocked)
    {
#ifdef WIN32
		LeaveCriticalSection(&m_lock);
		InterlockedDecrement(&m_lLockRefNum);
#else
		spin_unlock(&m_lock);
		__sync_fetch_and_sub(&m_lLockRefNum, 1);
#endif
 
    }

#ifdef WIN32
	DeleteCriticalSection(&m_lock);
#endif
}

void CXSpinLock::Lock()
{
    if(m_bInitLocked)
    {
        return  ;
    }
#ifdef WIN32
	EnterCriticalSection(&m_lock);
	InterlockedIncrement(&m_lLockRefNum);
#else
	spin_lock(&m_lock);
	__sync_fetch_and_add(&m_lLockRefNum, 1);
#endif
	
}
void CXSpinLock::Unlock()
{
    if(m_bInitLocked)
    {
        return  ;
    }
#ifdef WIN32
	LeaveCriticalSection(&m_lock);
	InterlockedDecrement(&m_lLockRefNum);
#else
	spin_unlock(&m_lock);
	__sync_fetch_and_sub(&m_lLockRefNum, 1);
#endif
}

bool CXSpinLock::TryLock()
{
    bool bRet = false;
#ifdef WIN32
    if (TryEnterCriticalSection(&m_lock))
    {
		InterlockedIncrement(&m_lLockRefNum);
        bRet= true;
    }
#else
    spin_trylock(&m_lock);
	__sync_fetch_and_add(&m_lLockRefNum, 1);
#endif

    return bRet;
}
