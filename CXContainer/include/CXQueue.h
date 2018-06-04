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

Description£ºThis class manage a list of some large memory blocks, 
             a large memory block is a continuous memory block,
             the large memory block will been divided into many small structures,
             the size of the structure is able to custom ,etc 4kb,8kb,1MB,and so on.
             a structure will been used to save a user's buffer. 
             This class have a spinlock to solve the problem of thread synchronization.
*****************************************************************************/

#ifndef CXQUEUE_H
#define CXQUEUE_H

#ifdef WIN32
#include <windows.h>  
#include <stdio.h>  
#else
#include  <unistd.h>
#include  <sys/types.h>
#include <iostream>
#include <stdlib.h>
#include <errno.h>
#endif

#include "PlatformDataTypeDefine.h"
#include "CXQueueData.h"
#include <stdio.h>
#include <list>

using namespace std;
namespace CXCommunication
{
    class CXQueue
    {
    public:
        CXQueue();
        virtual ~CXQueue();
        //initialize the first memory block
        int   Init(DWORD dwInitSize= 10240); 

        bool  Push(void *pData);
        
        void  Pop();
        void* Front();

        //pop the front node, and get the data in the front node
        void* PopFront();

        int   Size() { return m_firstQueueData.Size()+ m_secondQueueData.Size(); }
        void  Clear();
        bool  CompactMemory();
        void  SetAutoCompactMemory(bool bSet) { m_bAutoCompactMemory = bSet; }
        bool  IsAutoCompactMemory() { return m_bAutoCompactMemory; }


    private:
        CXQueueData *m_pQueueData;
        CXQueueData *m_pQueueDataBak;
        CXQueueData m_firstQueueData;
        CXQueueData m_secondQueueData;
        
        DWORD m_dwInitSize;

        bool  m_bInited;
        bool  m_bAutoCompactMemory;
        bool  m_isSwappingQueue;
    };
}
#endif // CXQUEUE_H
