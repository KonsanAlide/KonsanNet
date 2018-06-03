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
#include "CXQueue.h"
#ifndef WIN32
#include <string.h>
#endif

using namespace CXCommunication;
CXQueue::CXQueue()
{

    //the head pointer of the empty memory object list
    m_pEmptyListHead = NULL;
    //the end pointer of the empty memory object list
    m_pEmptyListEnd = NULL;

    //the head pointer of the memory block list
    m_pListHead = NULL;
    //the end pointer of the memory block list
    m_pListEnd = NULL;

    m_plstBlocksHead = NULL;
    m_plstBlocksEnd = NULL;

    m_dwNumberOfListNodes=0;
    m_dwNumberOfEmptyNodes = 0;
    m_dwNumberOfBlocks = 0;
    m_bInited = false;
    m_dwInitSize = 10240;
}

CXQueue::~CXQueue()
{
    Destroy();
}
//initialize the first memory block
int CXQueue::Init(DWORD dwInitSize)
{
    if(m_bInited)
    {
        return 0;
    }

    DWORD dwObjectSize = sizeof(cx_list_node);
    m_dwInitSize = dwInitSize;

    int nRet = AllocateMemoryBlock();
    if(nRet==0)
    {
        m_bInited = true;
    }

    return nRet;
}

void CXQueue::Destroy()
{
    if (m_bInited)
    {
        cx_list_header * pHeader = m_plstBlocksHead;
        cx_list_header * pNext = NULL;
        while (pHeader != NULL)
        {
            pNext = pHeader->pNext;
            delete[] (byte*)pHeader;
        }

        //the head pointer of the empty list
        m_pEmptyListHead = NULL;
        //the end pointer of the emptylist
        m_pEmptyListEnd = NULL;

        //the head pointer of the list
        m_pListHead = NULL;
        //the end pointer of the list
        m_pListEnd = NULL;

        m_plstBlocksHead = NULL;
        m_plstBlocksEnd = NULL;

        m_dwNumberOfListNodes = 0;
        m_dwNumberOfEmptyNodes = 0;
        m_dwNumberOfBlocks = 0;
        m_bInited = false;
        m_dwInitSize = 1024;
    }

}

//allocate a new memory block, add to the end of the m_pBlockList
//return value: ==0 succeed ,==-1 allocate a memory block failed
int  CXQueue::AllocateMemoryBlock()
{
    int iRet = 0;
    int iObjectStructSize = sizeof(cx_list_node);
    int iBufferSize = iObjectStructSize *m_dwInitSize + sizeof(cx_list_header);
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

    memset(pBuf, 0, iBufferSize);
    cx_list_header * pListHeader = (cx_list_header*)pBuf;

    byte *pListData = pBuf + sizeof(cx_list_header);
    cx_list_node * pObjectList = (cx_list_node*)pListData;

    if (m_plstBlocksHead == NULL)
    {
        m_plstBlocksEnd = m_plstBlocksHead = pListHeader;
    }
    else
    {
        m_plstBlocksEnd->pNext = pListHeader;
    }

    int i = 1;
    cx_list_node * pPrevNode = pObjectList;
    pObjectList->pPrev = NULL;
    pObjectList->pNext = (cx_list_node*)(pListData + iObjectStructSize);
    pObjectList = pObjectList->pNext;
    while (++i < m_dwInitSize)
    {
        pObjectList->pNext = (cx_list_node*)(pListData + iObjectStructSize *i);
        pObjectList->pPrev = pPrevNode;
        pPrevNode = pObjectList;
        pObjectList = pObjectList->pNext;
    }
    pObjectList->pNext = NULL;
    pObjectList->pPrev = pPrevNode;
    m_dwNumberOfBlocks++;

    if (m_pEmptyListHead == NULL)
    {
        m_pEmptyListHead = (cx_list_node*)pListData;
    }
    else
    {
        m_pEmptyListEnd->pNext = (cx_list_node*)pListData;
        m_pEmptyListEnd->pNext->pPrev = m_pEmptyListEnd;
    }
    m_pEmptyListEnd = pObjectList;

    return 0;
}

bool  CXQueue::Push(void *pData)
{
    if (!m_bInited)
    {
        if (0!= Init())
        {
            return false;
        }
    }
    cx_list_node *pNode = GetNode();
    if (pNode)
    {
        pNode->pData = pData;

        if (m_pListEnd != NULL)
        {
            pNode->pPrev = m_pListEnd;
            m_pListEnd->pNext = pNode;
            m_pListEnd = pNode;
        }
        else
        {
            m_pListEnd = m_pListHead = pNode;
        }
        ++m_dwNumberOfListNodes;
        return true;
    }
    return false;
}
void  CXQueue::Pop()
{
    if (m_pListHead != NULL)
    {
        cx_list_node *pNode = m_pListHead;
        if (m_pListHead->pNext == NULL)
        {
            m_pListHead= m_pListEnd = NULL;
            --m_dwNumberOfListNodes;
            FreeNode(pNode);
            return;
        }
        m_pListHead = pNode->pNext;
        m_pListHead->pPrev = NULL;
        --m_dwNumberOfListNodes;
        FreeNode(pNode);
    }
}

void* CXQueue::Front()
{
    if (m_pListHead != NULL)
    {
        return m_pListHead->pData;
    }
    else
    {
        return NULL;
    }
}

CXQueue::cx_list_node * CXQueue::GetNode()
{
    if (!m_bInited)
    {
        return NULL;
    }

    cx_list_node *pObj = NULL;

    if (m_pEmptyListHead != NULL)
    {
        pObj = m_pEmptyListHead;
        m_pEmptyListHead = m_pEmptyListHead->pNext;
        if (m_pEmptyListHead == NULL)
        {
            m_pEmptyListHead = NULL;
        }
        else
        {
            m_pEmptyListHead->pPrev = NULL;
        }
    }
    else
    {
        int iRet = AllocateMemoryBlock();
        if (iRet == 0)
        {
            pObj = m_pEmptyListHead;
            m_pEmptyListHead = m_pEmptyListHead->pNext;
            m_pEmptyListHead->pPrev = NULL;
        }
        else
        {
            return NULL;
        }

    }
    pObj->pNext = NULL;
    pObj->pData = NULL;
    pObj->pPrev = NULL;
    --m_dwNumberOfEmptyNodes;
    return pObj;
}

void CXQueue::FreeNode(void*pObjFree)
{
    if (!m_bInited)
    {
        return ;
    }

    if (pObjFree == NULL)
        return;

    cx_list_node *pObj = (cx_list_node *)pObjFree;
    pObj->pNext = NULL;

    if (m_pEmptyListEnd != NULL)
    {
        pObj->pPrev = m_pEmptyListEnd;
        m_pEmptyListEnd->pNext = pObj;
        m_pEmptyListEnd = pObj;
    }
    else
    {
        m_pEmptyListHead = m_pEmptyListHead = pObj;
        pObj->pPrev = NULL;
    }
    ++m_dwNumberOfEmptyNodes;
    return ;
}

void  CXQueue::Clear()
{
    if (m_pEmptyListHead == NULL)
    {
        m_pEmptyListHead = m_pListHead;
    }
    else
    {
        m_pEmptyListEnd->pNext = m_pListHead;
        m_pEmptyListEnd->pNext->pPrev = m_pEmptyListEnd;
    }
    m_pEmptyListEnd = m_pListEnd;
    m_pListHead = m_pListEnd = NULL;
    m_dwNumberOfEmptyNodes += m_dwNumberOfListNodes;
    m_dwNumberOfListNodes = 0;
}
