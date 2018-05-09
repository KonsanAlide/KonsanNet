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
#ifndef CXMEMORYCACHEMANAGER_H
#define CXMEMORYCACHEMANAGER_H
#include "../../CXCommon/include/PlatformDataTypeDefine.h"
#include "../../CXLock/include/CXSpinLock.h"
#include "CXMemoryCache.h"
#include <unordered_map>
using std::unordered_map;

//The maximum number of a class of cache
#define MAX_CACHE_NUMBER_IN_ONE_KIND 10
class CXMemoryCacheManager
{
public:
    //iCacheNumber: The initialized number of a class of cache 
    //iMaxMemorySize: the maximum available memory size,the unit is byte
    CXMemoryCacheManager(int iCacheNumber, int iMaxMemorySize);
    CXMemoryCacheManager();

    virtual ~CXMemoryCacheManager();

    int  Init(int iObjectSize, int iObjectNumber);

    void Destory();

    //get a memory cache object,this memory cache object have contains 
    //a lot of continuous small memory blocks of the same size,
    //the size of the small memory blocks had been set to iObjectSize
    CXMemoryCache* GetMemoryCache(int iObjectSize);

    //free a CXMemoryCache object to the cache manager
    void FreeMemoryCache(CXMemoryCache *pObj);

protected:
    //add a memory cache
    //iObjectSize: the object size in the memory cache
    //iObjectNumber: the object number in the memory cache
    int AddCache(int iObjectSize, int iObjectNumber);

private:
    //the map of the cache array,
    //the key is the small memory blocks size of the memory cache,
    //the value is the memory cache array pointers
    unordered_map<int, CXMemoryCache**> m_mapCaches;
    CXSpinLock m_lock;

    //The initialized number of a class of cache 
    int m_iInitCacheNumber;

    //the maximum available memory size, the unit is byte
    INT64 m_iMaxUsableMemorySize;
    
};
#endif // CXMEMORYCACHEMANAGER_H
