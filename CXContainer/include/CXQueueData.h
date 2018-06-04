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

Description: 
*****************************************************************************/

#ifndef CXQUEUEDATA_H
#define CXQUEUEDATA_H

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

#include <stdio.h>
#include <list>

using namespace std;
namespace CXCommunication
{
    class CXQueueData
    {
    public:
        struct cx_list_node
        {
            cx_list_node *pNext;
            cx_list_node *pPrev;
            void *pData;
        };

        struct cx_list_header
        {
            cx_list_header *pNext;
            cx_list_node *pList;  
        };

    public:
        CXQueueData();
        virtual ~CXQueueData();
        //initialize the first memory block
        int  Init(DWORD dwInitSize= 10240);
        
        bool  Push(void *pData);
        
        void* Front();

        //pop the front node, and get the data in the front node
        void* PopFront();

        int   Size() { return m_dwNumberOfUsingNodes; }
        void  Clear();

        DWORD GetNumberOfUsingNodes() { return m_dwNumberOfUsingNodes; }
        DWORD GetNumberOfEmptyNodes() { return m_dwNumberOfEmptyNodes; }
        DWORD GetNumberOfBlocks() { return m_dwNumberOfBlocks; }
        bool  IsInited() { return m_bInited; }

        //destroy all momory blocks
        void Destroy();

    private:
        //allocate a new memory block, add to the end of the m_pBlockList
        //return value: ==0 succeed ,==-1 allocate a memory block failed
        int  AllocateMemoryBlock();

        //get a free object
        cx_list_node * GetNode();

        //free a object
        void FreeNode(void*pObject);

    private:

        //the head pointer of the list
        cx_list_node *m_pListHead;
        //the end pointer of the the list
        cx_list_node *m_pListEnd;

        //the head pointer of the empty list
        cx_list_node *m_pEmptyListHead;
        //the end pointer of the empty list
        cx_list_node *m_pEmptyListEnd;

        cx_list_header *m_plstBlocksHead;
        cx_list_header *m_plstBlocksEnd;

        
        DWORD m_dwNumberOfUsingNodes;
        DWORD m_dwNumberOfEmptyNodes;
        DWORD m_dwNumberOfBlocks;
        DWORD m_dwInitSize;

        bool  m_bInited;
    };
}
#endif // CXQUEUEDATA_H
