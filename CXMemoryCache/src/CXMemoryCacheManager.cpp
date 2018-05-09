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

Description£ºThis class manage some memory cache object,a memory cache object
maybe have a different structure size with the other cache object,
use to save data blocks of different size.
*****************************************************************************/
#include "CXMemoryCacheManager.h"

CXMemoryCacheManager::CXMemoryCacheManager(int iCacheNumber, int iMaxMemorySize)
{
    m_iInitCacheNumber = 1;
    if (iCacheNumber > MAX_CACHE_NUMBER_IN_ONE_KIND)
    {
        m_iInitCacheNumber = iCacheNumber;
    }
    //m_iMaxUsableMemorySize = 1024 * 1024 * 1792;
    m_iMaxUsableMemorySize = iMaxMemorySize;
}

CXMemoryCacheManager::CXMemoryCacheManager()
{
    m_iInitCacheNumber = 1;
    m_iMaxUsableMemorySize = 1024 * 1024 * 1024;
}


CXMemoryCacheManager::~CXMemoryCacheManager()
{
}

int  CXMemoryCacheManager::Init(int iObjectSize, int iObjectNumber)
{
    m_lock.Lock();
    CXMemoryCache ** ppArray = m_mapCaches[iObjectSize];
    if (ppArray == NULL)
    {
        for (int i=0;i<MAX_CACHE_NUMBER_IN_ONE_KIND;i++)
        {
            int iRet = AddCache(iObjectSize, iObjectNumber);
            if (iRet != 0)
            {
                m_lock.Unlock();
                return -2;
            }
        }
    }

    m_lock.Unlock();

    return RETURN_SUCCEED;
}

//
//return value:==-2 failed to allocate memory 
//             ==-3 the number of this kind cache is more than  MAX_CACHE_NUMBER_IN_ONE_KIND
int CXMemoryCacheManager::AddCache(int iObjectSize, int iObjectNumber)
{
    int iRet = 0;
    try
    {
        CXMemoryCache ** ppArray = m_mapCaches[iObjectSize];
        if (ppArray == NULL)
        {
            //allocate the memory caches that its structure size is 4kb 
            ppArray = new CXMemoryCache*[MAX_CACHE_NUMBER_IN_ONE_KIND];
            if (ppArray == NULL)
            {
                return -2;
            }
            for (int i = 0; i<MAX_CACHE_NUMBER_IN_ONE_KIND; i++)
            {
                ppArray[i] = NULL;
            }

            bool bSucceed = true;
            for (int i = 0; i<m_iInitCacheNumber; i++)
            {
                ppArray[i] = new CXMemoryCache();
                if (ppArray[i] == NULL)
                {
                    bSucceed = false;
                    break;
                }
                else
                {
                    iRet = ppArray[i]->Initialize(iObjectSize, iObjectNumber);
                    if (iRet != 0)
                    {
                        bSucceed = false;
                        break;
                    }
                }
            }
            if (!bSucceed)
            {
                for (int i = 0; i<m_iInitCacheNumber; i++)
                {
                    if (ppArray[i] != NULL)
                    {
                        ppArray[i]->Destroy();
                        delete[]ppArray[i];
                        ppArray[i] = NULL;
                    }
                }
                return -2;
            }
            m_mapCaches[iObjectSize] = ppArray;
        }
        else
        {
            bool bFindNullPointer = false;
            for (int i = 0; i<MAX_CACHE_NUMBER_IN_ONE_KIND; i++)
            {
                if (ppArray[i] == NULL)
                {
                    bFindNullPointer = true;
                    ppArray[i] = new CXMemoryCache();
                    if (ppArray[i] == NULL)
                    {
                        return -2;
                    }
                    else
                    {
                        iRet = ppArray[i]->Initialize(iObjectSize, iObjectNumber);
                        if (iRet != 0)
                        {
                            ppArray[i]->Destroy();
                            delete[]ppArray[i];
                            ppArray[i] = NULL;
                            return -4;
                        }
                    }
                    break;
                }
            }
            if (!bFindNullPointer)
            {
                return -3;
            }
        }
    }
    catch (const bad_alloc& e)
    {
        return -2;
    }

    return 0;
}

void CXMemoryCacheManager::Destory()
{

}

//get a memory cache object,this memory cache object have contains 
//a lot of continuous small memory blocks of the same size,
//the size of the small memory blocks had been set to iObjectSize
CXMemoryCache* CXMemoryCacheManager::GetMemoryCache(int iObjectSize)
{
    m_lock.Lock();
    CXMemoryCache ** ppArray = m_mapCaches[iObjectSize];
    if (ppArray == NULL)
    {
        int iRet = AddCache(iObjectSize,500);
        if (iRet!=0)
        {
            m_lock.Unlock();
            return NULL;
        }

        ppArray = m_mapCaches[iObjectSize];
    }

    if (ppArray != NULL)
    {
        int i = rand()% MAX_CACHE_NUMBER_IN_ONE_KIND;
        if (ppArray[i] != NULL)
        {
            m_lock.Unlock();
            return ppArray[i];
        }
        for (int i = 0; i<MAX_CACHE_NUMBER_IN_ONE_KIND; i++)
        {
            if (ppArray[i] != NULL)
            {
                if (ppArray[i]->IsHaveObject())
                {
                    m_lock.Unlock();
                    return ppArray[i];
                }
            }
        }
    }

    m_lock.Unlock();

    return NULL;

}

//free a CXMemoryCache object to the cache manager
void CXMemoryCacheManager::FreeMemoryCache(CXMemoryCache *pObj)
{

}
