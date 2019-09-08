#include "CThreadPoolTest.h"
#include <iostream>
#include "CXThreadPool.h"

using namespace std;


CThreadPoolTest::CThreadPoolTest()
{
}


CThreadPoolTest::~CThreadPoolTest()
{
}


DWORD ThreadTest(void *pVoid)
{

#ifdef WIN32
    Sleep(3000);
	DWORD dwThreadID = GetCurrentThreadId();
#else
	DWORD dwThreadID = pthread_self();
#endif
	return dwThreadID;
}
void CThreadPoolTest::Test()
{
	CXThreadPool threadPool;
	if (!threadPool.CreatePool(128))
	{
		printf("failed to create thread pool \n");
		return;
	}
	CXThread *pThread = threadPool.GetFreeThread();
	byte *pbyData = new byte[1024];
	if (0!=pThread->Start(ThreadTest, (void*)pbyData))
	{
		delete[]pbyData;
		printf("failed to start thread \n");
	}
	else
	{
		bool bRunning = pThread->IsRunning();
		byte *pBuf = pThread->GetFirstCache();
		DWORD dwBufSize = pThread->GetFirstCacheSize();
		if (pBuf == NULL)
		{
			if (pThread->AllocateFirstCache(1024))
			{
				pBuf = pThread->GetFirstCache();
				dwBufSize = pThread->GetFirstCacheSize();
			}
		}

		pBuf = pThread->GetSecondCache();
		dwBufSize = pThread->GetSecondCacheSize();
		if (pBuf == NULL)
		{
			if (pThread->AllocateSecondCache(2048))
			{
				pBuf = pThread->GetSecondCache();
				dwBufSize = pThread->GetSecondCacheSize();
			}
		}

		// if stop this thread, must remove it from the pool
		// because this thread had been destroyed
		//pThread->Stop();
		//threadPool.RemoveThread(pThread);

		DWORD dwThreadID = pThread->GetThreadID();
		CXThread *pFindThread = threadPool.FindThread(dwThreadID);
		if (pFindThread != pThread)
		{
			printf("an error occur in thread pool \n");
		}
		pThread->Wait();
		DWORD dwRet = pThread->GetThreadResult();
		if (pThread->GetThreadID() != dwRet)
		{
			printf("failed to get the thread id \n");
		}
		//threadPool.ReleaseThread(pThread);
	}

	return ;
}

