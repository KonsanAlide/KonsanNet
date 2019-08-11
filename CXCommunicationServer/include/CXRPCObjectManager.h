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

Description£º
*****************************************************************************/
#ifndef CXRPCOBJECTMANAGER_H
#define CXRPCOBJECTMANAGER_H

#include "CXRPCObjectServer.h"

#ifdef WIN32
#include <unordered_map>
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

using namespace std;
#endif

#include "CXSpinLock.h"
#include <queue>

using namespace std;

namespace CXCommunication
{
    class CXRPCObjectManager
    {
    public:
        CXRPCObjectManager();
        virtual ~CXRPCObjectManager();

        void Register(const string &strObjectGuid, CXRPCObjectServer *pObj);
        CXRPCObjectServer * GetRPCObject(const string &strObjectGuid);
        void Lock();
        void UnLock();

    protected:
    private:
        unordered_map<string, CXRPCObjectServer *> m_mapRegisterObject;
        CXSpinLock m_lock;
    };
}
#endif //CXRPCOBJECTMANAGER_H

