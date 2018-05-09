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

#ifndef CXMEMORYCACHE_H
#define CXMEMORYCACHE_H

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
#include "CXSpinLock.h"

#include <stdio.h>
#include <list>

using namespace std;

class CXMemoryCache
{
    struct cx_cache_obj
    {
        //cx_cache_obj *pPrev;
        cx_cache_obj *pNext;
        byte *pData;
    };

    struct cx_block_cache_head
    {
        //cx_cache_obj *pPrev;
        cx_block_cache_head *pNext;
        cx_cache_obj *pCache;
    };

public:
    CXMemoryCache();
    virtual ~CXMemoryCache();
    //initialize the first memory block
    //iObjectSize: the object size in the memory cache
    //iObjectNumber: the object number in the memory cache
    int  Initialize(int iObjectSize, int iObjectNumber);
    //destroy all momory blocks
    void Destroy();
    //get a free object
    void * GetObject();
    //free a object
    void FreeObject(void*pObject);

    bool IsHaveObject() { return m_pEmptyObjectListHead!=NULL; }

protected:
    //allocate a new memory block, add to the end of the m_pBlockList
    //return value: ==0 succeed ,==-1 allocate a memory block failed
    int  AllocateMemoryBlock();
private:
    //the head pointer of the used memory object list
    //cx_cache_obj * m_pUsedObjectList;
    //the end pointer of the used memory object list
    //cx_cache_obj * m_pUsedObjectListEnd;

    //the head pointer of the empty memory object list
    cx_cache_obj *m_pEmptyObjectListHead;
    //the end pointer of the empty memory object list
    cx_cache_obj *m_pEmptyObjectListEnd;

    //the head pointer of the memory block list
    cx_cache_obj *m_pBlockListHead;
    //the end pointer of the memory block list
    cx_cache_obj *m_pBlockListEnd;

    //the number of the memory blocks
    int   m_nMemoryBlocksNumber;

    //the number of the objects in a memory block
    int   m_iObjectNumInMB;
    int   m_iObjectSize;

    CXSpinLock m_lock;
    bool  m_bInited;

    list<cx_cache_obj*> m_lstBlocks;
};

#endif // CXMEMORYCACHE_H
