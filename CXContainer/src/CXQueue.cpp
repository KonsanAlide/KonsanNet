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
    m_pQueueData = &m_firstQueueData;
    m_pQueueDataBak = &m_secondQueueData;

    m_bInited = false;
    m_dwInitSize = 10240;
    m_bAutoCompactMemory = true;
    m_isSwappingQueue = false;
}

CXQueue::~CXQueue()
{
    m_firstQueueData.Destroy();
    m_secondQueueData.Destroy();
}
//initialize the first memory block
int CXQueue::Init(DWORD dwInitSize)
{
    if(m_bInited)
    {
        return 0;
    }

    int iRet = m_firstQueueData.Init(dwInitSize);
    if (iRet == 0)
    {
        m_bInited = true;
    }
    m_dwInitSize = dwInitSize;

    return iRet;
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
    if (m_bAutoCompactMemory)
    {
        CompactMemory();
    }
    if (m_isSwappingQueue)
    {
        if (m_pQueueDataBak->GetNumberOfUsingNodes() == 0)
        {
            m_pQueueDataBak->Destroy();
            m_isSwappingQueue = false;
        }
    }


    return m_pQueueData->Push(pData);
}

void  CXQueue::Pop()
{
    if (m_bAutoCompactMemory)
    {
        CompactMemory();
    }

    if (m_isSwappingQueue)
    {
        if (m_pQueueDataBak->GetNumberOfUsingNodes() == 0)
        {
            m_pQueueDataBak->Destroy();
            m_isSwappingQueue = false;
        }
    }
    if (m_isSwappingQueue)
    {
        m_pQueueDataBak->PopFront();
    }
    else
    {
        m_pQueueData->PopFront();
    }
}

void* CXQueue::Front()
{
    if (m_isSwappingQueue)
    {
        return m_pQueueDataBak->Front();
    }
    else
    {
        return m_pQueueData->Front();
    }
}

void* CXQueue::PopFront()
{
    if (m_bAutoCompactMemory)
    {
        CompactMemory();
    }
    if (m_isSwappingQueue)
    {
        if (m_pQueueDataBak->GetNumberOfUsingNodes() == 0)
        {
            m_pQueueDataBak->Destroy();
            m_isSwappingQueue = false;
        }
    }
    if (m_isSwappingQueue)
    {
        return m_pQueueDataBak->PopFront();
    }
    else
    {
        return m_pQueueData->PopFront();
    }
}


void  CXQueue::Clear()
{
    m_pQueueDataBak->Clear();
    m_pQueueData->Clear();
    if (m_isSwappingQueue)
    {
        m_pQueueDataBak->Destroy();
        m_isSwappingQueue = false;
    }
}

bool CXQueue::CompactMemory()
{
    if (!m_isSwappingQueue)
    {
        if (m_pQueueData->GetNumberOfBlocks() >= 10
            && m_pQueueDataBak->GetNumberOfUsingNodes() == 0)
        {
            // the number of the empty nodes is double that of using nodes
            // swap the pointer of the m_pQueueData and m_pQueueDataBak
            if ((m_pQueueData->GetNumberOfEmptyNodes() / m_pQueueData->GetNumberOfUsingNodes())>2)
            {
                CXQueueData *pTemp = m_pQueueData;
                m_pQueueData = m_pQueueDataBak;
                m_pQueueDataBak = pTemp;
                m_isSwappingQueue = true;
            }
        }
    }
    return true;
}
