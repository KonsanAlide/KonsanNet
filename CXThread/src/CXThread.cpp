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

Description：
*****************************************************************************/
#include "CXThread.h"
#include "PlatformFunctionDefine.h"
#ifndef WIN32
#include <string.h>
#include <pthread.h>
#endif
#include "CXThreadPool.h"

CXThread::CXThread()
{
	m_dwThreadID = 0;
    m_hThread = NULL;
#ifndef WIN32
    memset(&m_ThreadAttr,0,sizeof(m_ThreadAttr));
#endif
	m_bStarted = false;
    m_funRun = NULL;
	m_pTaskPara = NULL;
    m_bRunning = false;

	m_dwThreadResult = 0;

	m_pbyFirstCache = NULL;
	m_dwFirstCacheSize=0;

	m_pbySecondCache = NULL;
	m_dwSecondCacheSize = 0;
}

CXThread::~CXThread()
{
	Destroy();

    if (NULL != m_hThread)
    {
#ifdef WIN32
        CloseHandle(m_hThread);
#else
        pthread_attr_destroy(&m_ThreadAttr);
#endif

    }

	if (m_pbyFirstCache != NULL)
	{
		delete[]m_pbyFirstCache;
	}
	if (m_pbySecondCache != NULL)
	{
		delete[]m_pbySecondCache;
	}

	m_pbyFirstCache = NULL;
	m_dwFirstCacheSize = 0;

	m_pbySecondCache = NULL;
	m_dwSecondCacheSize = 0;

    m_hThread = NULL;
    m_bRunning = false;
}

//create thread
bool  CXThread::Create()
{
	bool bRet = true;
    //in 32bit program, if the stack size is too large,
    //and the number of the threads is also too more,
    //that maybe case an error failed to allocated memory.
    //because the threads had used too more virtual address of the process space.
	//int iStackSize = 1024 * 1024 * 5;//5MB
    //thre default size of the thread stack is 1MB in windows
    //thre default size of the thread stack is 8092KB in linux
    int iStackSize = 0;

	m_bStarted = true;
#ifdef WIN32
	m_hThread = (HANDLE)_beginthreadex(NULL, iStackSize, ThreadFunction, this, 0, (unsigned int*)&m_dwThreadID);
	if (NULL == m_hThread)
	{
		bRet = false;
		m_bStarted = false;
	}
#else
	//pthread_attr_setscope (&pCurThread->threadAttr, PTHREAD_SCOPE_SYSTEM);
	//pthread_attr_setstacksize( &pCurThread->threadAttr, 5*1024 );
	//pthread_attr_setdetachstate (&pCurThread->threadAttr, PTHREAD_CREATE_DETACHED);
	pthread_attr_init(&m_ThreadAttr);
	int iRet = pthread_create(&m_hThread, &m_ThreadAttr, ThreadFunction, this);
	if (0 != iRet)
	{
		m_hThread = 0;
		pthread_attr_destroy(&m_ThreadAttr);
		bRet = false;
		m_bStarted = false;
	}
	else
	{
		m_dwThreadID = (DWORD)m_hThread;
	}
#endif

	return bRet;
}

//stop the thread task and terminal the thread
void CXThread::Destroy()
{
	m_bStarted = false;
	if (m_bRunning)
	{
		Stop();
	}
}

// start the thread
int CXThread::Start(RunFun funTask, void *pTaskPara)
{
	if (m_bRunning)
	{
		return -2;
	}

	if (!m_bStarted)
	{
		if (!Create())
		{
			m_bRunning = false;
			return -1;
		}
	}
    m_funRun = funTask;
	m_pTaskPara = pTaskPara;
	m_eveWaitRunning.SetEvent();
	m_bRunning = true;

    return 0;
}

//stop the thread
void  CXThread::Stop()
{
	if (NULL != m_hThread)
	{
		m_bStarted = false;
		m_bRunning = false;
		m_eveWaitRunning.SetEvent();
		
#ifdef WIN32
        TerminateThread(m_hThread,-1);
		WaitForSingleObject(m_hThread, INFINITE);
		if (NULL != m_hThread)
		{
			CloseHandle(m_hThread);
		}
#else
		pthread_cancel(m_hThread);
		pthread_join(m_hThread, NULL);
#endif // WIN32
		m_hThread = NULL;
		m_dwThreadResult = -1;
	}
}

// wait the thread task to finish
DWORD CXThread::Wait()
{
	if (NULL == m_hThread)
	{
		return WAIT_OBJECT_0;
	}
	if (!m_bRunning || !m_bStarted)
	{
		return WAIT_OBJECT_0;
	}
	return m_eveFinish.WaitForSingleObject(INFINITE);
}

#ifdef WIN32
unsigned __stdcall CXThread::ThreadFunction(void* arg)
#else
void* CXThread::ThreadFunction(void* arg)
#endif
{
#ifdef WIN32
#else
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    //异步取消， 线程接到取消信号后，立即退出
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
#endif //WIN32

    CXThread *pThis = (CXThread*)arg;
	if (pThis == NULL)
#ifdef WIN32
        return DWORD(-1);
#else
        return (void*)-1;
#endif //WIN32

    pThis->ThreadLoop();
    
	return 0;
}

//the main logic
void CXThread::ThreadLoop()
{
	DWORD dwRet = 0;
	DWORD dwWaitRet = 0;
	bool  bWaitFailed = false;
	while (m_bStarted)
	{
		dwWaitRet = m_eveWaitRunning.WaitForSingleObject(INFINITE);
		if (!m_bStarted)
		{
			break;
		}
		switch (dwWaitRet)
		{
		case WAIT_OBJECT_0:
			break;
		case WAIT_FAILED:
			bWaitFailed = true;
			break;
		case WAIT_TIMEOUT:
			continue;
		default:
			continue;
		}
		if (bWaitFailed)
		{
			break;
		}
		m_dwThreadResult = (DWORD)m_funRun(m_pTaskPara);
		m_bRunning = false;
		m_eveFinish.SetEvent();
		if (m_pThreadPool != NULL)
		{
			((CXThreadPool*)m_pThreadPool)->ReleaseThread(this);
		}
	}

#ifdef WIN32
	m_hThread = NULL;
#else
#endif //WIN32
}

bool   CXThread::AllocateFirstCache(DWORD dwSize)
{
	bool bRet = true;

	if (dwSize <= m_dwFirstCacheSize)
	{
		return true;
	}

	byte *pData = NULL;
	try
	{
		pData = new byte[dwSize];
		if (pData == NULL)
		{
			return false;
		}

		if (m_pbyFirstCache != NULL)
		{
			delete[]m_pbyFirstCache;
			m_pbyFirstCache = NULL;
		}
		m_pbyFirstCache = pData;
		m_dwFirstCacheSize = dwSize;
		return true;
	}
	catch (const bad_alloc& e)
	{
        DWORD dwError = GetLastError();
        string strEr = strerror(dwError);
		return false;
	}

	return bRet;
}

bool   CXThread::AllocateSecondCache(DWORD dwSize)
{
	bool bRet = true;

	if (dwSize <= m_dwSecondCacheSize)
	{
		return true;
	}

	byte *pData = NULL;
	try
	{
		pData = new byte[dwSize];
		if (pData == NULL)
		{
			return false;
		}

		if (m_pbySecondCache != NULL)
		{
			delete[]m_pbySecondCache;
			m_pbySecondCache = NULL;
		}
		m_pbySecondCache = pData;
		m_dwSecondCacheSize = dwSize;
		return true;
	}
	catch (const bad_alloc& e)
	{
        DWORD dwError = GetLastError();
        string strEr = strerror(dwError);
		return false;
	}
	return bRet;
}
