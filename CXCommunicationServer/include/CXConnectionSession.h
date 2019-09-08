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
#ifndef CXCONNECTIONSESSION_H
#define CXCONNECTIONSESSION_H

#include "PlatformDataTypeDefine.h"
#include "SocketDefine.h"
#include "CXSpinLock.h"
#include "CXServerStructDefine.h"
#include <string>
#include <list>
using namespace std;
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

namespace CXCommunication
{
    class CXConnectionSession
    {
        public:
            CXConnectionSession();
            virtual ~CXConnectionSession();
            void Lock();
            void UnLock();
            int AddMainConnection(CXConnectionObject &conObj);
            int AddMessageConnection(CXConnectionObject &conObj);
            int AddDataConnection(CXConnectionObject &conObj);
            int AddObjectConnection(CXConnectionObject &conObj);

            int    RemoveConnection(CXConnectionObject &conObj);
            string GetSessionGuid() { return m_strSessionGuid; }
            void   SetSesssionGuid(string strGuid) { m_strSessionGuid= strGuid; }
            string GetVerificationCode() { return m_strVerificationCode; }

            void   Destroy();

			//add a object in the session
            void   AddObject(string strObjectID,void *pObj,bool bLockBySelf=true);
			//get the object in the session
            void*  FindObject(string strObjectID,bool bLockBySelf=true);
			//remove a object
            void   RemoveObject(string strObjectID,bool bLockBySelf=true);

            int    GetConnectionNumber();

            void   ResetVerificationInfo();

            //verify the verification code
            //==0 succeed
            //==-1 the verification code not match
            //==-2 the verification code is time out
            int   VerifyCode(string strCode);

			void  SetObjectPool(void *pPool) { m_pObjectsPool = pPool; }
			void* GetObjectPool() { return m_pObjectsPool; }

        protected:
        private:
            CXConnectionObject * m_pMmainConnetion;
            list<CXConnectionObject*> m_lstMessageConnections;
            list<CXConnectionObject*> m_lstDataConnections;
            list<CXConnectionObject*> m_lstObjectConnections;
            CXSpinLock m_lock;
            string     m_strSessionGuid;
            string     m_strVerificationCode;
			//key: the object id
			//value: object pointer
            unordered_map<string, void*> m_mapObjects;
            time_t     m_tmBeginOfVerification;
            int        m_iTimeOutSecondsOfVerification;
			void      *m_pObjectsPool;

    };
}
#endif // CXCONNECTIONSESSION_H
