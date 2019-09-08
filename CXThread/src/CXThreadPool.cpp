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
#include "CXThreadPool.h"



CXThreadPool::CXThreadPool()
{
	m_dwMaximumNum = 1024;
	m_dwThreadsNum = 0;
	m_bCreated = false;
	m_dwFreeThreadsNum = 0;
	m_dwUsingThreadsNum = 0;
}


CXThreadPool::~CXThreadPool()
{
}

//create thread pool 
bool      CXThreadPool::CreatePool(DWORD dwThreadNum)
{
	bool bRet = true;
	if (dwThreadNum > m_dwMaximumNum)
		return false;
    if (m_bCreated)
        return true;
    int i = 0;
	try
	{
		for (i=0;i< dwThreadNum;i++)
		{
			CXThread *pThread = new CXThread();
			if (pThread != NULL)
			{
				if (pThread->Create())
				{
					m_lstFreePool.push_back(pThread);
					m_mapThreadPool.insert(make_pair(pThread->GetThreadID(), pThread));
					m_dwFreeThreadsNum ++;
				}
				else
				{
					bRet = false;
					delete pThread;
					break;
				}
			}
			else
			{
				bRet = false;
				break;
			}
		}
	}
	catch (...)
	{
		bRet = false;
	}

	if (!bRet)
	{
		Destroy();
	}
	else
	{
		m_bCreated = true;
		m_dwThreadsNum = dwThreadNum;
	}
	
	return bRet;
}

bool CXThreadPool::AddPoolSize(DWORD dwAddNumber)
{
	if (dwAddNumber == 0)
		return true;

	bool bRet = true;
	int i = 0;
	for (i = 0; i < dwAddNumber; i++)
	{
		CXThread *pThread = new CXThread();
		if (pThread != NULL)
		{
			if (pThread->Create())
			{
				m_lock.Lock();
				m_lstFreePool.push_back(pThread);
				m_mapThreadPool.insert(make_pair(pThread->GetThreadID(), pThread));
				m_dwFreeThreadsNum++;
				m_dwThreadsNum++;
				m_lock.Unlock();
			}
			else
			{
				bRet = false;
				delete pThread;
				break;
			}
		}
		else
		{
			bRet = false;
			break;
		}
	}

	return bRet;
}
void CXThreadPool::Destroy()
{
	m_lock.Lock();
	map<DWORD, CXThread*>::iterator it= m_mapThreadPool.begin();
	for (; it != m_mapThreadPool.end();)
	{
		it->second->Destroy();
		it = m_mapThreadPool.erase(it);
	}

	m_lstFreePool.clear();
	m_dwFreeThreadsNum = 0;
	m_dwUsingThreadsNum = 0;
	m_lock.Unlock();
}

CXThread* CXThreadPool::FindThread(DWORD dwThreadID)
{
	CXThread* pThread = NULL;
	m_lock.Lock();
	map<DWORD, CXThread*>::iterator it = m_mapThreadPool.find(dwThreadID);
	if (it != m_mapThreadPool.end() )
	{
		pThread = it->second;
	}
	m_lock.Unlock();
	return pThread;
}

CXThread* CXThreadPool::GetFreeThread()
{
	CXThread* pThread = NULL;
	m_lock.Lock();
	if (m_lstFreePool.size() > 0)
	{
		pThread = m_lstFreePool.front();
		m_lstFreePool.pop_front();
		m_dwFreeThreadsNum--;
		m_dwUsingThreadsNum++;
	}
	m_lock.Unlock();
	return pThread;
}
void  CXThreadPool::ReleaseThread(CXThread*pThread)
{
	if (pThread->GetThreadState() != CXThread::CXTHREAD_STATE_FREE)
		return;

	m_lock.Lock();
	
	bool bFind = false;
	list<CXThread*>::iterator it = m_lstFreePool.begin();
	for (; it != m_lstFreePool.end(); ++it)
	{
		if (*it == pThread)
		{
			bFind = true;
			break;
		}
	}

	if (!bFind)
	{
		m_lstFreePool.push_back(pThread);
		m_dwFreeThreadsNum++;
		m_dwUsingThreadsNum--;
	}

	m_lock.Unlock();
}

//remove thread from the thread pool
void CXThreadPool::RemoveThread(CXThread*pThread)
{
	m_lock.Lock();
	list<CXThread*>::iterator it = m_lstFreePool.begin();
	for (; it != m_lstFreePool.end(); ++it)
	{
		if (*it == pThread)
		{
			it = m_lstFreePool.erase(it);
			break;
		}		
	}
	
	map<DWORD, CXThread*>::iterator itPool = m_mapThreadPool.find(pThread->GetThreadID());
	if (itPool != m_mapThreadPool.end())
	{
		itPool = m_mapThreadPool.erase(itPool);
		delete[]itPool->second;
	}
	
	m_lock.Unlock();
}

//waitting to the all thread exit
void CXThreadPool::WaitAllExited()
{
	map<DWORD, CXThread*>::iterator it = m_mapThreadPool.begin();
	for (; it != m_mapThreadPool.end(); ++it)
	{
		it->second->Wait();
	}
}
