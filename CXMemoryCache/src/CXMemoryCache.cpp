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
#include "CXMemoryCache.h"
#ifndef WIN32
#include <string.h>
#endif

CXMemoryCache::CXMemoryCache()
{
    //the head pointer of the used memory object list
    //m_pUsedObjectList = NULL;
    //the end pointer of the used memory object list
    //m_pUsedObjectListEnd = NULL;

    //the head pointer of the empty memory object list
    m_pEmptyObjectListHead = NULL;
    //the end pointer of the empty memory object list
    m_pEmptyObjectListEnd = NULL;

    //the head pointer of the memory block list
    m_pBlockListHead = NULL;
    //the end pointer of the memory block list
    m_pBlockListEnd = NULL;

    //the number of the memory blocks
    m_nMemoryBlocksNumber = 0;

    //the number of the objects in a memory block
    m_iObjectNumInMB = 0;
    m_iObjectSize = 0;
    m_bInited = false;
}

CXMemoryCache::~CXMemoryCache()
{
    Destroy();
}
//initialize the first memory block
int CXMemoryCache::Initialize(int iObjectSize, int iObjectNumber)
{
    if(m_bInited)
    {
        return 0;
    }

    m_iObjectNumInMB = iObjectNumber;
    m_iObjectSize = iObjectSize;
    int nRet = AllocateMemoryBlock();
    if(nRet==0)
    {
        m_bInited = true;
    }

    return nRet;
}

void CXMemoryCache::Destroy()
{
    m_lock.Lock();
    if (m_bInited)
    {
        cx_cache_obj * pObj = NULL;
        std::list<cx_cache_obj*>::iterator it = m_lstBlocks.begin();
        for (; it != m_lstBlocks.end();)
        {
            pObj = *it;
            m_lstBlocks.pop_front();
            it = m_lstBlocks.begin();
            delete[] (byte*)pObj;
        }

        m_nMemoryBlocksNumber = 0;
        m_pBlockListHead = NULL;
        m_pBlockListEnd = NULL;
        m_pEmptyObjectListHead = NULL;
        m_pEmptyObjectListEnd = NULL;
    }

    m_lock.Unlock();
}

//allocate a new memory block, add to the end of the m_pBlockList
//return value: ==0 succeed ,==-1 allocate a memory block failed
int  CXMemoryCache::AllocateMemoryBlock()
{
    int iRet = 0;
    int iObjectStructSize = sizeof(cx_cache_obj) + m_iObjectSize;
    int iBufferSize = iObjectStructSize *m_iObjectNumInMB;
    byte *pBuf = NULL;
    try
    {
        pBuf = new byte[iBufferSize];
        if (pBuf == NULL)
        {
            return -1;
        }
    }
    catch (const bad_alloc& e)
    {
       return -1;
    }

    memset(pBuf,0, iBufferSize);
    cx_cache_obj * pObjectList = (cx_cache_obj*)pBuf;
    int i = 0;
    while (++i<m_iObjectNumInMB)
    {
        pObjectList->pThis = this;
        pObjectList->pNext = (cx_cache_obj*)(pBuf + iObjectStructSize *i);
        pObjectList->pData = ((byte*)pObjectList + sizeof(cx_cache_obj));
        pObjectList = pObjectList->pNext;
    }
    pObjectList->pThis = this;
    pObjectList->pNext = NULL;
    pObjectList->pData = ((byte*)pObjectList + sizeof(cx_cache_obj));
    m_lstBlocks.push_back((cx_cache_obj*)pBuf);
    m_nMemoryBlocksNumber++;

    m_pEmptyObjectListHead = (cx_cache_obj*)pBuf;
    m_pEmptyObjectListEnd = pObjectList;

    //printf("AllocateMemoryBlock Memory list head :%x, list end:%x\n",(void*)m_pEmptyObjectListHead, (void*)m_pEmptyObjectListEnd);

    return 0;
}

void * CXMemoryCache::GetObject()
{
    if (!m_bInited)
    {
        return NULL;
    }

    cx_cache_obj *pObj = NULL;

    m_lock.Lock();
    if (m_pEmptyObjectListHead != NULL)
    {
        pObj = m_pEmptyObjectListHead;
        m_pEmptyObjectListHead = m_pEmptyObjectListHead->pNext;
        if (m_pEmptyObjectListHead == NULL)
        {
            m_pEmptyObjectListEnd = NULL;
        }
    }
    else
    {
        int iRet = AllocateMemoryBlock();
        if (iRet == 0)
        {
            pObj = m_pEmptyObjectListHead;
            m_pEmptyObjectListHead = m_pEmptyObjectListHead->pNext;
        }
        else
        {
            m_lock.Unlock();
            return NULL;
        }

    }
    pObj->pNext = NULL;
    m_lock.Unlock();

    memset(pObj->pData, 0, m_iObjectSize);

    return pObj->pData;
}

void CXMemoryCache::FreeObject(void*pObjFree)
{
    if (!m_bInited)
    {
        return ;
    }

    if (pObjFree == NULL)
        return;

    cx_cache_obj *pObj = (cx_cache_obj *)((byte*)pObjFree - sizeof(cx_cache_obj));
    pObj->pNext = NULL;


    m_lock.Lock();
    if (m_pEmptyObjectListEnd != NULL)
    {
        m_pEmptyObjectListEnd->pNext = pObj;
        m_pEmptyObjectListEnd = pObj;
    }
    else
    {
        m_pEmptyObjectListHead = m_pEmptyObjectListEnd = pObj;
    }
    m_lock.Unlock();

    return ;
}
