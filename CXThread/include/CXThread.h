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
#ifndef __CXTHREAD_H__
#define __CXTHREAD_H__

#ifdef WIN32  
#include <Windows.h>  
#include <process.h>  
#else  //WIN32  
#include <pthread.h>  
#endif  //WIN32  

#include "PlatformDataTypeDefine.h" 
#include "CXEvent.h"

//the thread main runing function
typedef DWORD (*RunFun)(void *);
class CXThread
{
public:
	enum CXTHREAD_STATE
	{
		CXTHREAD_STATE_FREE = 0,
		CXTHREAD_STATE_RUNNING,
		CXTHREAD_STATE_DESTROYED,
		CXTHREAD_STATE_DEAD,
	};
public:
    CXThread();
    virtual ~CXThread();

	//create thread
	bool  Create();
	//stop the thread task and terminal the thread
	void  Destroy();

    //start the thread task 
    int   Start(RunFun funTask, void *pTaskPara);

    //stop the thread task and terminal the thread
    void  Stop();

	// wait the thread task to finish
    DWORD Wait();

	//get the task parameters
    void *GetTaskPara() { return m_pTaskPara; }

	//get the result of the thread task
	DWORD GetThreadResult() { return m_dwThreadResult; }

	bool  AllocateFirstCache(DWORD dwSize);

	bool  AllocateSecondCache(DWORD dwSize);

	byte* GetFirstCache() { return m_pbyFirstCache; }
	byte* GetSecondCache() { return m_pbySecondCache; }

	DWORD GetFirstCacheSize() { return m_dwFirstCacheSize; }
	DWORD GetSecondCacheSize() { return m_dwSecondCacheSize; }

	bool IsRunning() { return m_bRunning; }

	DWORD GetThreadID() { return m_dwThreadID; }

	void SetThreadPool(void *pPool) { m_pThreadPool = pPool; }
	void SetThreadState(CXTHREAD_STATE state) { m_stateThread = state; }
	CXTHREAD_STATE GetThreadState() { return m_stateThread; }

protected:
#ifdef WIN32  
	static unsigned __stdcall ThreadFunction(void* arg);
#else  
	static void* ThreadFunction(void* arg);
#endif  

	//the main logic
	void ThreadLoop();
private:
	DWORD   m_dwThreadID;
#ifdef WIN32  
    HANDLE    m_hThread;
#else  
    pthread_t m_hThread;
    pthread_attr_t m_ThreadAttr;
#endif
	bool   m_bStarted;

    RunFun m_funRun;
    void * m_pTaskPara;
    bool   m_bRunning;
	//==-2 terminaled
	DWORD  m_dwThreadResult;

	byte * m_pbyFirstCache;
	DWORD  m_dwFirstCacheSize;

	byte * m_pbySecondCache;
	DWORD  m_dwSecondCacheSize;

	CXEvent m_eveWaitRunning;
	CXEvent m_eveFinish;
	void  * m_pThreadPool;
	CXTHREAD_STATE m_stateThread;
};

#endif //__CXTHREAD_H__ 