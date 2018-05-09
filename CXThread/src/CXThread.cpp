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

CXThread::CXThread()
{

    m_hThread = NULL;
#ifndef WIN32
    memset(&m_ThreadAttr,0,sizeof(m_ThreadAttr));
#endif

    m_funRun = NULL;
    m_pThreadPara = NULL;
    m_bRunning = false;
}

CXThread::~CXThread()
{
    if (m_bRunning)
    {
        Stop();
        Wait();
    }

    if (NULL != m_hThread)
    {
#ifdef WIN32
        CloseHandle(m_hThread);
#else
        pthread_attr_destroy(&m_ThreadAttr);
#endif
        
    }

    m_hThread = NULL;
    m_bRunning = false;
}

// start the thread
int CXThread::Start(RunFun funThread, void *pThreadPara)
{
    int iRet = 0;
    int iStackSize = 1024 * 1024 * 5;//5MB
#ifdef WIN32
    m_hThread = (HANDLE)_beginthreadex(NULL, iStackSize, ThreadFunction, this, 0, NULL);
    if (NULL == m_hThread)
    {
        iRet = -1;
    }
#else
    //pthread_attr_setscope (&pCurThread->threadAttr, PTHREAD_SCOPE_SYSTEM);
    //pthread_attr_setstacksize( &pCurThread->threadAttr, 5*1024 );
    //pthread_attr_setdetachstate (&pCurThread->threadAttr, PTHREAD_CREATE_DETACHED);
    pthread_attr_init(&m_ThreadAttr);
    iRet = pthread_create(&m_hThread, m_ThreadAttr, ThreadFunction, this);
    if (0 == iRet)
    {
        iRet = 0;
    }
    else
    {
        m_hThread = 0;
        pthread_attr_destroy(&m_ThreadAttr);
        iRet = -1;
    }
#endif
    m_funRun = funThread;
    m_pThreadPara = pThreadPara;
    return iRet;
}

//stop the thread 
void  CXThread::Stop()
{
#ifdef WIN32
    if (NULL != m_hThread)
    {
        TerminateThread(m_hThread,-1);
    }

#else
    pthread_cancel(m_hThread);
#endif // WIN32

}

// wait the thread to exit
DWORD CXThread::Wait()
{
#ifdef WIN32
    WaitForSingleObject(m_hThread, INFINITE);
    if (NULL != m_hThread)
    {
        CloseHandle(m_hThread);
    }
    
#else
    pthread_join(m_hThread, NULL);
#endif // WIN32

    m_hThread = NULL;
    return 0;
}

#ifdef WIN32
unsigned __stdcall CXThread::ThreadFunction(void* arg)
#else
void* base_thread::ThreadFunction(void* arg)
#endif
{
#ifdef WIN32
#else
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    //异步取消， 线程接到取消信号后，立即退出
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
#endif //WIN32
    
    void *pRet = NULL;
    CXThread *pThis = (CXThread*)arg;
    pThis->m_bRunning = true;
    if (pThis->m_funRun != NULL)
    {
        pRet = pThis->m_funRun(pThis->GetThreadPara());
    }
    pThis->m_bRunning = false;
    pThis->m_hThread = NULL;
    return (unsigned int)pRet;
}