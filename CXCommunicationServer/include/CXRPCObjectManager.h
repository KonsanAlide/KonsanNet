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
#include <list>

using namespace std;

namespace CXCommunication
{
    class CXRPCObjectManager
    {
    public:
        CXRPCObjectManager();
        virtual ~CXRPCObjectManager();

		//register a class of object to the generator
		//strObjectClassGuid: the object's class guid 
        void Register(const string &strObjectClassGuid, CXRPCObjectServer *pObj);

		//get a free RPC object according to the object's class guid 
        CXRPCObjectServer * GetFreeObject(const string &strObjectClassGuid);

		//get the static rpc object, this object cann't delete or attach to the session,
		//only use to convern the message to string
		CXRPCObjectServer * GetStaticObject(const string &strObjectClassGuid);

		// free a object to the free pool
		void                FreeObject(CXRPCObjectServer * pObj); 


		CXRPCObjectServer * FindUsingObject(const string &strObjectID);

        void Lock();
        void UnLock();

		void TravelTimeoutRequest();

    protected:
    private:
        unordered_map<string, CXRPCObjectServer *> m_mapRegisterObject;

		//key: the class guid of the object
		unordered_map<string, list<CXRPCObjectServer *>> m_mapFreeObjs;

		//key: the guid of the object
		//value: the list of the objects in this session
		unordered_map<string, CXRPCObjectServer *> m_mapUsingObjs;

		uint64 m_uiUsingObjects;
		uint64 m_uiFreeObjects;

        CXSpinLock m_lock;
    };
}
#endif //CXRPCOBJECTMANAGER_H

