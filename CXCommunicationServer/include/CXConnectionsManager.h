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
#ifndef CXCONNECTIONSMANAGER_H
#define CXCONNECTIONSMANAGER_H

#include "CXConnectionObject.h"
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

#include <map>
#include <queue>
#include "CXSpinLock.h"

using namespace std;


namespace CXCommunication
{
    class CXConnectionsManager
    {
        public:
            CXConnectionsManager();
            virtual ~CXConnectionsManager();
            CXConnectionObject * FindConnectionInPendingMap(uint64 uiConIndex);
            CXConnectionObject * GetFreeConnectionObj();
            int  AddPendingConnection(CXConnectionObject * pObj);
            void RemovePendingConnection(CXConnectionObject * pObj);
            void RemovePendingConnection(uint64 uiConIndex);
            void AddFreeConnectionObj(CXConnectionObject * pObj);

            int  AddUsingConnection(CXConnectionObject * pObj);
            void RemoveUsingConnection(CXConnectionObject * pObj);
            void RemoveUsingConnection(uint64 uiConIndex);
            uint64 GetCurrentConnectionIndex();
            void Destroy();

        protected:
        private:
            unordered_map<uint64, CXConnectionObject *> m_mapPendingConnections;
            unordered_map<uint64, CXConnectionObject *> m_mapUsingConnections;
            queue<CXConnectionObject *> m_queueFreeConnections;
            uint64 m_uiCurrentConnectionIndex;
            CXSpinLock m_lockFreeConnections;
            CXSpinLock m_lockUsingConnections;
            CXSpinLock m_lockPendingConnections;

    };
}

#endif // CXCONNECTIONSMANAGER_H
