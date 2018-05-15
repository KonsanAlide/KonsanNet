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
#ifndef CXSESSIONSSMANAGER_H
#define CXSESSIONSSMANAGER_H

#include "CXConnectionSession.h"

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
    class CXSessionsManager
    {
    public:
        CXSessionsManager();
        virtual ~CXSessionsManager();

        void AddFreeSession(CXConnectionSession * pObj);
        CXConnectionSession * GetFreeSession();

        int  AddUsingSession(CXConnectionSession * pObj);
        void RemoveUsingSession(CXConnectionSession * pObj);
        void RemoveUsingSession(string strSessionGuid);
        string BuildSessionGuid();

        CXConnectionSession * FindUsingSession(string strSessionGuid);

        void Destroy();

    protected:
    private:
        unordered_map<string, CXConnectionSession *> m_mapUsingSessions;
        queue<CXConnectionSession*> m_queueFreeSessions;
        CXSpinLock m_lockFreeSessions;
        CXSpinLock m_lockUsingSessions;
    };
}
#endif //CXSESSIONSSMANAGER_H

