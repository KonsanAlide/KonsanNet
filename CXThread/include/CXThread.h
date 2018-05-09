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
#ifndef CXEPOLLSERVERKERNEL_H
#define CXEPOLLSERVERKERNEL_H

#ifdef WIN32  
#include <Windows.h>  
#include <process.h>  
#else  //WIN32  
#include <pthread.h>  
#endif  //WIN32  

#include "../../CXCommon/include/PlatformDataTypeDefine.h" 

//the thread main runing function
typedef void* (*RunFun)(void *);
class CXThread
{
public:
    CXThread();
    virtual ~CXThread();

    //start the thread
    int   Start(RunFun funThread, void *pThreadPara);

    //stop the thread 
    void  Stop();

    // wait the thread to exit
    //not used the DWORD dwMilliseconds
    DWORD Wait();

    void *GetThreadPara() { return m_pThreadPara; }

#ifdef WIN32  
    static unsigned __stdcall ThreadFunction(void* arg);
#else  
    static void* ThreadFunction(void* arg);
#endif  

protected:
private:
#ifdef WIN32  
    HANDLE    m_hThread;
#else  
    pthread_t m_hThread;
    pthread_attr_t m_ThreadAttr;
#endif

    RunFun m_funRun;
    void * m_pThreadPara;
    bool   m_bRunning;
};

#endif //CXEPOLLSERVERKERNEL_H 