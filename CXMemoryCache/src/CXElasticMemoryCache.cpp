/****************************************************************************
Copyright (c) 2018-2019 Charles Yang

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
#include "CXElasticMemoryCache.h"
#ifndef WIN32
#include <string.h>
#endif

CXElasticMemoryCache::CXElasticMemoryCache()
{
	m_pCacheObj = &m_firstObj;
	m_pCacheObjBak = &m_secondObj;
	m_firstObj.SetElasticObj((void*)this);
	m_secondObj.SetElasticObj((void*)this);
	m_bAutoCompactMemory = true;
	m_isSwapping = false;
}

CXElasticMemoryCache::~CXElasticMemoryCache()
{
    Destroy();
}
//initialize the first memory block
int CXElasticMemoryCache::Initialize(int iObjectSize, int iObjectNumber, bool bInitAtOnce)
{
	m_secondObj.Initialize(iObjectSize, iObjectNumber, false);
	return m_firstObj.Initialize(iObjectSize, iObjectNumber, bInitAtOnce);
}

void CXElasticMemoryCache::Destroy()
{
	m_firstObj.Destroy();
	m_secondObj.Destroy();
}

void * CXElasticMemoryCache::GetObject()
{
	if (m_bAutoCompactMemory)
	{
		CompactMemory();
	}
	
	return m_pCacheObj->GetObject();
}

void CXElasticMemoryCache::FreeObject(void*pObjFree)
{
    if (pObjFree == NULL)
        return;


	if (m_bAutoCompactMemory)
	{
		CompactMemory();
	}

	CXMemoryCache::cx_cache_obj *pObj = (CXMemoryCache::cx_cache_obj *)((byte*)pObjFree - sizeof(CXMemoryCache::cx_cache_obj));
	if(pObj!=NULL)
		pObj->pThis->FreeObject(pObjFree);
    
    return ;
}

bool CXElasticMemoryCache::CompactMemory()
{
	m_lock.Lock();
	if (!m_isSwapping)
	{
		if (m_pCacheObj->GetNumberOfBlocks() >= 3
			&& m_pCacheObjBak->GetUsingNodesNum() == 0)
		{
			// the number of the empty nodes is double that of using nodes
			// swap the pointer of the m_pCacheObj and m_pCacheObjBak
			if ((m_pCacheObj->GetTotalNodesNum() / m_pCacheObj->GetUsingNodesNum()) >= 3)
			{
				CXMemoryCache *pTemp = m_pCacheObj;
				m_pCacheObj = m_pCacheObjBak;
				m_pCacheObjBak = pTemp;
				m_isSwapping = true;
                m_pCacheObj->RecordUsedTime();
			}
		}
	}

	if (m_isSwapping)
	{
		if (m_pCacheObjBak->GetUsingNodesNum() == 0)
		{
			m_pCacheObjBak->Destroy();
			m_isSwapping = false;
		}
	}
	m_lock.Unlock();
	return true;
}

//get the unused seconds for this cache
int64 CXElasticMemoryCache::GetUnusedState(DWORD &dwUsingNodes)
{ 
    m_lock.Lock();
    int64 iInterval = 0;
    dwUsingNodes = 0;
    if (!m_isSwapping)
    {
        DWORD dwUsingNodesBak = m_pCacheObjBak->GetUsingNodesNum();
        iInterval = m_pCacheObj->GetUnusedState(dwUsingNodes);
        dwUsingNodes += dwUsingNodesBak;
    }
    
    m_lock.Unlock();
    return iInterval;
}
