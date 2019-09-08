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
#include "CXTaskPool.h"
using namespace CXCommunication;
CXTaskPool::CXTaskPool()
{
	m_lpCacheObj = NULL;
    m_bRunning   = false;
    m_dwPoolSize = 128;
    m_dwInitedQueueSize = 10000;
	m_dwReducedPoolSize = 128;
	m_bReducingPoolSize = false;
}


CXTaskPool::~CXTaskPool()
{
	Destroy();
}

//register a class of task to the pool
void CXTaskPool::RegisterTaskClass(CXTaskBase *pTask)
{
	m_lock.Lock();
	m_mapRegisteredTaskClass[pTask->GetTaskType()] = pTask;
	m_lock.Unlock();
}

int CXTaskPool::Create(int iPoolSize, int iTaskQueueSize, CXMemoryCacheManager *pCacheObj,
    bool bAllocateThreadCache, DWORD dwThreadCacheSize)
{
    if (iPoolSize < 2 || iTaskQueueSize < 100 || iTaskQueueSize>1000000 || pCacheObj ==NULL)
    {
        return INVALID_PARAMETER;
    }

    if (!m_threadTasks.CreatePool(iPoolSize))
    {
        return -2;
    }

	m_bRunning = true;
	bool bSucceed = true;
	CXThread *pThread = NULL;
	for (int i = 0; i < iPoolSize; i++)
	{
		pThread = m_threadTasks.GetFreeThread();
        if (bAllocateThreadCache && dwThreadCacheSize>0 && dwThreadCacheSize<10485760)
        {
            pThread->AllocateFirstCache(dwThreadCacheSize);
        }
		if (pThread != NULL && (0 != pThread->Start(CXTaskPool::Run, (void*)this)))
		{
			bSucceed = false;
			break;
		}
	}

	if (!bSucceed)
	{
		m_bRunning = false;
		m_threadTasks.Destroy();
		return -3;
	}

    
    if (iTaskQueueSize > m_dwInitedQueueSize)
        m_dwInitedQueueSize = iTaskQueueSize;

	if (0 != m_queueOutstandingTask.Init(m_dwInitedQueueSize))
	{
		m_bRunning = false;
		m_threadTasks.Destroy();
		return -4;
	}
	m_dwPoolSize = iPoolSize;
	m_dwReducedPoolSize = m_dwPoolSize;
	m_lpCacheObj = pCacheObj;
    return RETURN_SUCCEED;
}

bool CXTaskPool::ResizePool(int iPoolSize)
{
	if (iPoolSize < 2)
	{
		return false;
	}


	bool bSucceed = true;

	m_lock.Lock();
	if (iPoolSize < m_dwPoolSize)
	{
		m_bReducingPoolSize = true;
		m_dwReducedPoolSize = iPoolSize;

		//notify the threads to exit
		for (int i=0;i<(m_dwPoolSize- iPoolSize);i++)
		{
			m_event.SetEvent();
		}

		bSucceed = false;
		DWORD dwWaitRet = 0;
		bool  bWaitFailed = false;
		while (true)
		{
			if (!m_bRunning)
			{
				break;
			}
			bWaitFailed = false;
			dwWaitRet = m_event.WaitForSingleObject(100);
			switch (dwWaitRet)
			{
			case WAIT_OBJECT_0:
				bSucceed = true;
				break;
			case WAIT_TIMEOUT:
				continue;
			case WAIT_FAILED:
			default:
				bWaitFailed = true;
				break;
			}

			if (bSucceed || bWaitFailed)
			{
				break;
			}
		}
	}
	else
	{
		
		CXThread *pThread = NULL;
		int iAddNumber = 0;
		for (int i = 0; i < iPoolSize; i++)
		{
			pThread = m_threadTasks.GetFreeThread();
			if (pThread != NULL)
			{
				if(0 == pThread->Start(CXTaskPool::Run, (void*)this))
				{
					iAddNumber++;
					m_dwPoolSize++;
				}
				else
				{
					bSucceed = false;
					break;
				}
			}
		}

		if (bSucceed && m_threadTasks.AddPoolSize(iPoolSize- iAddNumber))
		{
			for (int i = 0; i < iPoolSize - iAddNumber; i++)
			{
				pThread = m_threadTasks.GetFreeThread();
				if (pThread != NULL)
				{
					if (0 == pThread->Start(CXTaskPool::Run, (void*)this))
					{
						iAddNumber++;
						m_dwPoolSize++;
					}
					else
					{
						bSucceed = false;
						break;
					}
				}
			}
		}
		else
		{
			bSucceed = false;
		}

	}
	m_lock.Unlock();

    return bSucceed;
}
void CXTaskPool::Destroy()
{
	m_bRunning = false;
	//notify the threads to exit
	for (int i = 0; i < m_dwPoolSize; i++)
	{
		m_event.SetEvent();
	}
	m_threadTasks.WaitAllExited();
	m_threadTasks.Destroy();

	map<int, CXTaskBase*>::iterator it = m_mapRegisteredTaskClass.begin();
	for (; it != m_mapRegisteredTaskClass.end();)
	{
		it = m_mapRegisteredTaskClass.erase(it);
	}
}

//push a new task to the waitting queue, wait to process
void CXTaskPool::PushTask(CXTaskBase *pTask)
{
	pTask->SetMemoryCache(this->GetMemoryCache());
    m_lock.Lock();
    m_queueOutstandingTask.Push((void*)pTask);
    m_event.SetEvent();
    m_lock.Unlock();
}

//stop the task if the task is running.
void CXTaskPool::StopTask(CXTaskBase *pTask)
{
    if (!m_bRunning)
        return;
    if (pTask != NULL)
    {
        pTask->StopTask();
    }
}

CXTaskBase* CXTaskPool::GetFreeTaskObject(CXTaskBase::CX_RUNING_TASK_TYPE type)
{
    CXTaskBase* pTask = NULL;
    m_lock.Lock();
	map<int, list<CXTaskBase*>>::iterator it = m_mapFreeTaskObj.find(type);
	if (it != m_mapFreeTaskObj.end())
	{
		if (it->second.size() > 0)
		{
			pTask = it->second.front();
			it->second.pop_front();
		}
	}

	if (pTask == NULL)
	{
		map<int, CXTaskBase*>::iterator itTask = m_mapRegisteredTaskClass.find(type);
		if (itTask != m_mapRegisteredTaskClass.end())
		{
			pTask = itTask->second->CreateObject();
		}
	}
	
    m_lock.Unlock();
    return pTask;
}

void CXTaskPool::FreeTaskObject(CXTaskBase *pTask)
{
	m_lock.Lock();
	map<int, list<CXTaskBase*>>::iterator it = m_mapFreeTaskObj.find(pTask->GetTaskType());
	if (it != m_mapFreeTaskObj.end())
	{
		if (it->second.size() > 0)
		{
			pTask = it->second.front();
			it->second.pop_front();
		}
	}
	else
	{
		list<CXTaskBase*> lstFreeTask;
		lstFreeTask.push_back(pTask);
		m_mapFreeTaskObj.insert(make_pair(pTask->GetTaskType(), lstFreeTask));
	}
	m_lock.Unlock();
}

DWORD CXTaskPool::Run(void *pParas)
{
    DWORD dwRet = 0;
    CXTaskPool* pThis = (CXTaskPool*)pParas;
    if (pThis == NULL)
    {
        return DWORD(-1);
    }
    dwRet = pThis->ProcessTasks();

    return dwRet;
}

DWORD CXTaskPool::ProcessTasks()
{
	DWORD dwRet = 0;
    DWORD dwWaitRet = 0;
    bool  bWaitFailed = false;
	bool  bExit = false;

	CXTaskBase* pTask = NULL;
    while (m_bRunning)
    {
		bExit = false;
        bWaitFailed = false;
        dwWaitRet = m_event.WaitForSingleObject(INFINITE);
        if (!m_bRunning)
        {
            break;
        }
        switch (dwWaitRet)
        {
        case WAIT_OBJECT_0:
            break;
        case WAIT_TIMEOUT:
            continue;
		case WAIT_FAILED:
        default:
			bWaitFailed = true;
            break;
        }
        if (bWaitFailed)
        {
            break;
        }
		m_lock.Lock();
		if (m_bReducingPoolSize)
		{
			if (m_dwPoolSize > m_dwReducedPoolSize)
			{
				m_dwPoolSize--;
				bExit = true;
			}
			else
			{
				m_bReducingPoolSize = false;
				m_eventResizeFinished.SetEvent();
			}
		}
		m_lock.Unlock();

		if (bExit) //release the thread to the thread pool
		{
			break;
		}

#ifdef WIN32
		InterlockedIncrement(&m_lRunningTaskNum);
#else
		__sync_fetch_and_add(&m_lRunningTaskNum, 1);
#endif
		while (m_bRunning)
		{
			m_lock.Lock();
			if (m_queueOutstandingTask.Size() > 0)
			{
				pTask = (CXTaskBase*)m_queueOutstandingTask.PopFront();
			}
			m_lock.Unlock();
			if (pTask != NULL)
			{
				pTask->Run();
				FreeTaskObject(pTask);
				pTask = NULL;
				//try again
				continue;
			}
			else
			{
				break;
			}
		}

#ifdef WIN32
		InterlockedDecrement(&m_lRunningTaskNum);
#else
		__sync_fetch_and_sub(&m_lRunningTaskNum, 1);
#endif

    }

	DWORD dwThreadID = 0;
#ifdef WIN32
	dwThreadID = GetCurrentThreadId();
#else
	dwThreadID = pthread_self();
#endif

	CXThread *pThread = m_threadTasks.FindThread(dwThreadID);
	if (pThread != NULL)
	{
		m_threadTasks.ReleaseThread(pThread);
	}

	return dwRet;
}
