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

#ifndef CXELASTICMEMORYCACHE_H
#define CXELASTICMEMORYCACHE_H

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
#include "CXMemoryCache.h"
#include <stdio.h>
#include <list>
#include "CXSpinLock.h"

using namespace std;

class CXElasticMemoryCache
{
public:

public:
	CXElasticMemoryCache();
    virtual ~CXElasticMemoryCache();

    //initialize the first memory block
    //iObjectSize: the object size in the memory cache
    //iObjectNumber: the object number in the memory cache
	//bInitAtOnce: ==true allocate memory for the cache at once
    int  Initialize(int iObjectSize, int iObjectNumber,bool bInitAtOnce = true);

    //destroy all momory blocks
    void Destroy();
    //get a free object
    void * GetObject();
    //free a object
    void FreeObject(void*pObject);

    bool IsHaveObject() { return m_pCacheObj->IsHaveObject(); }

    int  GetObjectSize() { return m_pCacheObj->GetObjectSize(); }

	void  SetAutoCompactMemory(bool bSet) { m_bAutoCompactMemory = bSet; }
	bool  IsAutoCompactMemory() { return m_bAutoCompactMemory; }

    //get the unused seconds for this cache
    int64 GetUnusedState(DWORD &dwUsingNodes);

private:
	bool  CompactMemory();

private:
	CXMemoryCache *m_pCacheObj;
	CXMemoryCache *m_pCacheObjBak;
	CXMemoryCache m_firstObj;
	CXMemoryCache m_secondObj;

	bool  m_bAutoCompactMemory;
	bool  m_isSwapping;
	CXSpinLock m_lock;
};

#endif // CXELASTICMEMORYCACHE_H
