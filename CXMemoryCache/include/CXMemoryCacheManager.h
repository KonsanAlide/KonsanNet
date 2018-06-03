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
#ifdef WIN32
#include <unordered_map>
using namespace std;
#else
#define GCC_VERSION (__GNUC__ * 10000 \
    + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

#if GCC_VERSION >= 40300
#include <tr1/unordered_map>
using namespace std::tr1;
#define hash_map unordered_map

#else
#include <ext/hash_map>
using namespace __gnu_cxx;
#endif
#endif

#include<map>
using namespace std;
//The maximum number of a class of cache
#define MAX_CACHE_NUMBER_IN_ONE_KIND 10
class CXMemoryCacheManager
{
public:
    //iCacheNumber: The initialized number of a class of cache
    //uiMaxMemorySize: the maximum available memory size,the unit is byte
    CXMemoryCacheManager(DWORD dwInitCacheNumber, uint64 uiMaxMemorySize, DWORD dwMaxObjectSize);
    CXMemoryCacheManager();

    virtual ~CXMemoryCacheManager();

    //int  Init(DWORD dwObjectSize, DWORD dwObjectNumber);
    int  Init(map<DWORD,DWORD>mapCacheConfig);

    void Destory();

    //get a memory cache object,this memory cache object have contains
    //a lot of continuous small memory blocks of the same size,
    //the size of the small memory blocks had been set to iObjectSize
    CXMemoryCache* GetMemoryCache(DWORD dwObjectSize);

    //free a CXMemoryCache object to the cache manager
    void FreeMemoryCache(CXMemoryCache *pObj);

    void * GetBuffer(DWORD dwObjectSize);
    void   FreeBuffer(void *pBuf);

    //Parameter: pDestBuf: the pointer of the destination buffer that must be allocated by a CXMemoryCacheManager object,otherwise, the program will crash 
    //           the pDestBuf must be the beginning pointer of the object allocated from the CXMemoryCacheManager object
    //           iOffset : the beginning offset of the destination buffer
    //           pSrc    : the pointer of the source buffer
    //           iCopyLen: the length needed to copy from the source buffer to the destination buffer, the unit is bytes    
    //Return : ==true succeed
    //         ==false , not have enough space to save the bytes from the source buffer
    bool   MemcpyBytes(byte *pDestBuf,int iOffset, byte *pSrc,int iCopyLen);

    //Parameter: pBuf    : the pointer of the buffer that must be allocated by a CXMemoryCacheManager object,otherwise, the program will crash 
    //           the pBuf must be the beginning pointer of the object allocated from the CXMemoryCacheManager object
    //           iLen    : the length needed to memset, the unit is bytes
    void   MemsetZero(byte *pBuf, int iLen);

    void   SetMaxObjectSize(DWORD dwSize) { m_dwMaxObjectSize = dwSize; }
    DWORD  GetMaxObjectSize() {return m_dwMaxObjectSize;}

protected:
    //add a memory cache
    //iObjectSize: the object size in the memory cache
    //iObjectNumber: the object number in the memory cache
    //iIndex: the index of the m_ppCaches, ==-1 show that need to parse the index from the dwObjectSize
    //return value:==0 succeed 
    //             ==-1 invalid parameter
    //             ==-2 failed to allocate memory 
    //             ==-3 the number of this kind cache is more than  MAX_CACHE_NUMBER_IN_ONE_KIND
    int AddCache(DWORD dwObjectSize, DWORD dwObjectNumber,int iIndex=-1 );

private:
    int CalculateIndex(DWORD dwObjectSize);


private:
    //the map of the cache array,
    //this manager has 12 kinds of memory cache,
    //the object size of in the memory cache range as : 256B,512B,1KB,2KB,4KB,8KB,16KB,32KB,64KB,128KB,256KB,512KB,1MB
    //1MB=256*£¨2^12£©bytes
    //the maximum size of the object is 1MB
    CXMemoryCache** m_ppCaches[13];

    //the map of the cache array,
    //the key is the small memory blocks size of the memory cache,
    //the value is the memory cache array pointers
    unordered_map<int, CXMemoryCache**> m_mapCaches;
    CXSpinLock m_lock;

    //The initialized number of a class of cache
    DWORD m_dwInitCacheNumber;

    //the maximum available memory size, the unit is byte
    uint64 m_uiMaxUsableMemorySize;

    // the maximum size of the object in the memory cache
    DWORD  m_dwMaxObjectSize;

};
#endif // CXMEMORYCACHEMANAGER_H
