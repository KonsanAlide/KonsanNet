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
#ifndef CXEVENT_H
#define CXEVENT_H

#ifdef WIN32
#include <windows.h>
#else
#include  <unistd.h>
#include  <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#endif // WIN32

#include "../../CXCommon/include/PlatformDataTypeDefine.h"
#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>


using namespace std;

class CXEvent
{
    public:
        CXEvent(BOOL bManualReset=FALSE, BOOL bInitialState= FALSE);
        virtual ~CXEvent();

        void SetEvent();
        void ResetEvent();

        //return value:
        //==0 the event has been set,
        //==ETIMEDOUT
        //==otherwise, error value
        //dwMilliseconds:
        DWORD WaitForSingleObject(DWORD dwMilliseconds);
    protected:
    private:
#ifdef WIN32
        HANDLE m_hEvent;
#else
        sem_t  m_hEvent;
#endif // WIN32



};

#endif // CXEVENT_H
