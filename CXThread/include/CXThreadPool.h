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
#ifndef __CXTHREADPOOL_H__
#define __CXTHREADPOOL_H__

#include "CXThread.h"
#include "CXSpinLock.h"
#include <map>
#include <list>
using namespace std;


class CXThreadPool
{
public:
	CXThreadPool();
	virtual ~CXThreadPool();

	//create thread pool 
	bool CreatePool(DWORD dwThreadNum);
	bool AddPoolSize(DWORD dwAddNumber);
	void Destroy();

	CXThread* FindThread(DWORD dwThreadID);

	//get a free thread
	CXThread* GetFreeThread();

	//release thread to the free list
	void ReleaseThread(CXThread*pThread);

	//remove thread from the thread pool
	void RemoveThread(CXThread*pThread);

	//waitting to the all thread exit
	void WaitAllExited();

private:

	//the key is thread id
	map<DWORD, CXThread*>m_mapThreadPool;

	//the key is thread id
	//the free thread pool
	list<CXThread*>m_lstFreePool;

	//the maximum number of the threads
	DWORD m_dwMaximumNum;

	//the total number of the threads in the pool
	DWORD m_dwThreadsNum;

	//the free number of the threads in the pool
	DWORD m_dwFreeThreadsNum;

	//the using number of the threads in the pool
	DWORD m_dwUsingThreadsNum;

	bool  m_bCreated;

	CXSpinLock m_lock;
};
#endif //__CXTHREADPOOL_H__ 

