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

#include <stdio.h>
#include <list>

using namespace std;
namespace CXCommunication
{
    class CXQueue
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
        CXQueue();
        virtual ~CXQueue();
        //initialize the first memory block
        int  Init(DWORD dwInitSize= 10240);
        

        bool  Push(void *pData);
        void  Pop();
        void* Front();
        int   Size() { return m_dwNumberOfListNodes; }
        void  Clear();

    private:
        //allocate a new memory block, add to the end of the m_pBlockList
        //return value: ==0 succeed ,==-1 allocate a memory block failed
        int  AllocateMemoryBlock();

        //get a free object
        cx_list_node * GetNode();

        //free a object
        void FreeNode(void*pObject);

        //destroy all momory blocks
        void Destroy();

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

        bool  m_bInited;
        DWORD m_dwNumberOfListNodes;
        DWORD m_dwNumberOfEmptyNodes;
        DWORD m_dwNumberOfBlocks;
        DWORD m_dwInitSize;
    };
}
#endif // CXQUEUE_H
