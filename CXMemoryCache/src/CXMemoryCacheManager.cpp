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

Description��This class manage some memory cache object,a memory cache object
maybe have a different structure size with the other cache object,
use to save data blocks of different size.
*****************************************************************************/
#include "CXMemoryCacheManager.h"
#ifndef WIN32
#include <string.h>
#include <math.h>
#endif

CXMemoryCacheManager::CXMemoryCacheManager(DWORD dwInitCacheNumber, 
    uint64 uiMaxMemorySize, DWORD dwMaxObjectSize)
{
    m_dwInitCacheNumber = 1;
    if (dwInitCacheNumber > MAX_CACHE_NUMBER_IN_ONE_KIND)
    {
        dwInitCacheNumber = MAX_CACHE_NUMBER_IN_ONE_KIND;
    }
    m_dwInitCacheNumber = dwInitCacheNumber;

    m_uiMaxUsableMemorySize = uiMaxMemorySize;

    m_dwMaxObjectSize = dwMaxObjectSize;

    memset(m_ppCaches, 0, sizeof(m_ppCaches));
}

CXMemoryCacheManager::CXMemoryCacheManager()
{
    m_dwInitCacheNumber = 1;
    m_uiMaxUsableMemorySize = 1024 * 1024 * 1024;
    m_dwMaxObjectSize = 1024 * 1024;
    memset(m_ppCaches, 0, sizeof(m_ppCaches));
}


CXMemoryCacheManager::~CXMemoryCacheManager()
{
}
int  CXMemoryCacheManager::Init(map<DWORD, DWORD>mapCacheConfig)
//int  CXMemoryCacheManager::Init(DWORD dwObjectSize, DWORD dwObjectNumber)
{
    DWORD dwObjectSize = 0;
    DWORD dwObjectNumber = 0;

    m_lock.Lock();
    map<DWORD, DWORD>::iterator it = mapCacheConfig.begin();
    for (; it!= mapCacheConfig.end();it++)
    {
        dwObjectSize = it->first;
        dwObjectNumber = it->second;
        int iIndex = CalculateIndex(dwObjectSize);

        CXMemoryCache ** ppArray = (CXMemoryCache **)m_ppCaches[iIndex];
        //CXMemoryCache ** ppArray = m_mapCaches[dwObjectSize];
        if (ppArray == NULL)
        {
            for (int i = 0; i<MAX_CACHE_NUMBER_IN_ONE_KIND; i++)
            {
                int iRet = AddCache(dwObjectSize, dwObjectNumber, iIndex);
                if (iRet != 0)
                {
                    m_lock.Unlock();
                    return -2;
                }
            }
        }
    }

    m_lock.Unlock();

    return RETURN_SUCCEED;
}

//add a memory cache
//iObjectSize: the object size in the memory cache
//iObjectNumber: the object number in the memory cache
//iIndex: the index of the m_ppCaches, ==-1 show that need to parse the index from the dwObjectSize
//return value:==0 succeed 
//             ==-1 invalid parameter
//             ==-2 failed to allocate memory 
//             ==-3 the number of this kind cache is more than  MAX_CACHE_NUMBER_IN_ONE_KIND
int CXMemoryCacheManager::AddCache(DWORD dwObjectSize, DWORD dwObjectNumber, int iIndex)
{
    if (iIndex > 13 || dwObjectSize>m_uiMaxUsableMemorySize)
    {
        return INVALID_PARAMETER;
    }
    int iRet = 0;
    try
    {
        //CXMemoryCache ** ppArray = m_mapCaches[dwObjectSize];
        if (iIndex == -1)//need to parse the index
        {
            iIndex = CalculateIndex(dwObjectSize);
        }

        CXMemoryCache ** ppArray = (CXMemoryCache **)m_ppCaches[iIndex];

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
            for (int i = 0; i<m_dwInitCacheNumber; i++)
            {
                ppArray[i] = new CXMemoryCache();
                if (ppArray[i] == NULL)
                {
                    bSucceed = false;
                    break;
                }
                else
                {
                    iRet = ppArray[i]->Initialize(dwObjectSize, dwObjectNumber);
                    if (iRet != 0)
                    {
                        bSucceed = false;
                        break;
                    }
                }
            }
            if (!bSucceed)
            {
                for (int i = 0; i<m_dwInitCacheNumber; i++)
                {
                    if (ppArray[i] != NULL)
                    {
                        ppArray[i]->Destroy();
                        delete []ppArray[i];
                        ppArray[i] = NULL;
                    }
                }
                delete [] ppArray;
                return -2;
            }
            //m_mapCaches[dwObjectSize] = ppArray;
            m_ppCaches[iIndex] = ppArray;
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
                        iRet = ppArray[i]->Initialize(dwObjectSize, dwObjectNumber);
                        if (iRet != 0)
                        {
                            ppArray[i]->Destroy();
                            delete[]ppArray[i];
                            ppArray[i] = NULL;
                            return -2;
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

int CXMemoryCacheManager::CalculateIndex(DWORD dwObjectSize)
{
    int iIndex = 0;
    int iValue = dwObjectSize / 256;
    if (iValue > 0)
    {
        while (iValue>1)
        {
            iValue >>= 1;
            iIndex++;
        }
        if (dwObjectSize % 256 != 0)
        {
            iIndex++;
        }
    }
    return iIndex;
}

void CXMemoryCacheManager::Destory()
{

}

//get a memory cache object,this memory cache object have contains 
//a lot of continuous small memory blocks of the same size,
//the size of the small memory blocks had been set to iObjectSize
CXMemoryCache* CXMemoryCacheManager::GetMemoryCache(DWORD dwObjectSize)
{
    //CXMemoryCache ** ppArray = m_mapCaches[dwObjectSize];
    int iIndex = CalculateIndex(dwObjectSize);
    dwObjectSize = pow(2,iIndex) * 256;
    
    CXMemoryCache ** ppArray = (CXMemoryCache **)m_ppCaches[iIndex];
    if (ppArray == NULL)
    {
        m_lock.Lock();
        //ppArray = m_mapCaches[dwObjectSize];
        ppArray = (CXMemoryCache **)m_ppCaches[iIndex];
        if (ppArray == NULL)
        {
            int iRet = AddCache(dwObjectSize, 512);
            if (iRet != 0)
            {
                m_lock.Unlock();
                return NULL;
            }

            //ppArray = m_mapCaches[dwObjectSize];
            ppArray = (CXMemoryCache **)m_ppCaches[iIndex];
        }
        m_lock.Unlock();  
    }

    if (ppArray != NULL)
    {
        int j = rand()% MAX_CACHE_NUMBER_IN_ONE_KIND;
        if (ppArray[j] != NULL)
        {
            return ppArray[j];
        }
        else
        {
            m_lock.Lock();
            ppArray[j] = new CXMemoryCache();
            if (ppArray[j] == NULL)
            {
                return NULL;
            }
            else
            {
                int iRet = ppArray[j]->Initialize(dwObjectSize, 512);
                if (iRet != 0)
                {
                    ppArray[j]->Destroy();
                    delete[]ppArray[j];
                    ppArray[j] = NULL;
                    return NULL;
                }
            }
            m_lock.Unlock();
        }
        for (int i = 0; i<MAX_CACHE_NUMBER_IN_ONE_KIND; i++)
        {
            if (ppArray[i] != NULL)
            {
                if (ppArray[i]->IsHaveObject())
                {
                    return ppArray[i];
                }
            }
        }
    }

    return NULL;

}

//free a CXMemoryCache object to the cache manager
void CXMemoryCacheManager::FreeMemoryCache(CXMemoryCache *pObj)
{

}

void * CXMemoryCacheManager::GetBuffer(DWORD dwObjectSize)
{
    if (dwObjectSize > m_dwMaxObjectSize)
    {
        return NULL;
    }

    int iIndex = CalculateIndex(dwObjectSize);
    dwObjectSize = pow(2, iIndex) * 256;

    CXMemoryCache ** ppArray = (CXMemoryCache **)m_ppCaches[iIndex];
    if (ppArray == NULL)
    {
        m_lock.Lock();
        //ppArray = m_mapCaches[dwObjectSize];
        ppArray = (CXMemoryCache **)m_ppCaches[iIndex];
        if (ppArray == NULL)
        {
            int iRet = AddCache(dwObjectSize, 512);
            if (iRet != 0)
            {
                m_lock.Unlock();
                return NULL;
            }

            //ppArray = m_mapCaches[dwObjectSize];
            ppArray = (CXMemoryCache **)m_ppCaches[iIndex];
        }
        m_lock.Unlock();
    }

    if (ppArray != NULL)
    {
        void *pObj = NULL;
        int j = rand() % MAX_CACHE_NUMBER_IN_ONE_KIND;
        if (ppArray[j] == NULL)
        {
            m_lock.Lock();
            ppArray[j] = new CXMemoryCache();
            if (ppArray[j] == NULL)
            {
                pObj = NULL;
            }
            else
            {
                int iRet = ppArray[j]->Initialize(dwObjectSize, 512);
                if (iRet != 0)
                {
                    ppArray[j]->Destroy();
                    delete[]ppArray[j];
                    ppArray[j] = NULL;
                }
            }
            m_lock.Unlock();
        }

        if (ppArray[j] != NULL)
        {
            pObj = ppArray[j]->GetObject();
            if (pObj)
            {
                return pObj;
            }
        }

        for (int i = 0; i<MAX_CACHE_NUMBER_IN_ONE_KIND; i++)
        {
            if (ppArray[i] != NULL)
            {
                if (ppArray[i]->IsHaveObject())
                {
                    pObj = ppArray[i]->GetObject();
                    if (pObj)
                        return pObj;
                }
            }
        }
    }

    return NULL;
}
void   CXMemoryCacheManager::FreeBuffer(void *pBuf)
{
    if (pBuf == NULL)
        return;
    CXMemoryCache::cx_cache_obj *pObj = (CXMemoryCache::cx_cache_obj *)((byte*)pBuf - sizeof(CXMemoryCache::cx_cache_obj));
    if (pObj != NULL)
    {
        if (pObj->pThis != NULL)
        {
            pObj->pThis->FreeObject(pBuf);
        }
    }
}

//Parameter: pDestBuf: the pointer of the destination buffer that must be allocated by a CXMemoryCacheManager object,otherwise, the program will crash 
//           the pDestBuf must be the beginning pointer of the object allocated from the CXMemoryCacheManager object
//           iOffset : the beginning offset of the destination buffer
//           pSrc    : the pointer of the source buffer
//           iCopyLen: the length needed to copy from the source buffer to the destination buffer, the unit is bytes    
//Return : ==true succeed
//         ==false , not have enough space to save the bytes from the source buffer    
bool  CXMemoryCacheManager::MemcpyBytes(byte *pDestBuf, int iOffset, byte *pSrc, int iCopyLen)
{
    if (pDestBuf == NULL || pSrc==NULL)
        return false;
    CXMemoryCache::cx_cache_obj *pObj = (CXMemoryCache::cx_cache_obj *)((byte*)pDestBuf - sizeof(CXMemoryCache::cx_cache_obj));
    if (pObj != NULL)
    {
        if (pObj->pThis != NULL)
        {
            if (pObj->pThis->GetObjectSize() >= (iOffset + iCopyLen))
            {
                memcpy(pDestBuf+ iOffset, pSrc, iCopyLen);
                return true;
            }
        }
    }
    return false;
}

//Parameter: pBuf    : the pointer of the buffer that must be allocated by a CXMemoryCacheManager object,otherwise, the program will crash 
//           the pBuf must be the beginning pointer of the object allocated from the CXMemoryCacheManager object
//           iLen    : the length needed to memset, the unit is bytes
void CXMemoryCacheManager::MemsetZero(byte *pBuf, int iLen)
{
    if (pBuf == NULL)
        return ;
    CXMemoryCache::cx_cache_obj *pObj = (CXMemoryCache::cx_cache_obj *)((byte*)pBuf - sizeof(CXMemoryCache::cx_cache_obj));
    if (pObj != NULL)
    {
        if (pObj->pThis != NULL)
        {
            if (pObj->pThis->GetObjectSize() >= iLen)
            {
                memset(pBuf, 0, iLen);
            }
        }
    }
}