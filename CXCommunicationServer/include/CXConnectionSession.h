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
#include <unordered_map>
#include <string>
#include <list>
using namespace std;
#include "CXConnectionObject.h"
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

            int RemoveConnection(CXConnectionObject &conObj);
            string GetSessionGuid() { return m_strSessionGuid; }
            void   SetSesssionGuid(string strGuid) { m_strSessionGuid= strGuid; }
            void   SetTemporaryVerificationCode(string strCode) { m_strTemporaryVerificationCode= strCode; }
            string GetTemporaryVerificationCode() { return m_strTemporaryVerificationCode; }

            void Destroy();

            void SetData(string strKey,void *pData);
            void *GetData(string strKey);
            void RemoveData(string strKey);

            int  GetConnectionNumber();
    
        protected:
        private:
            CXConnectionObject * m_pMmainConnetion;
            list<CXConnectionObject*> m_lstMessageConnections;
            list<CXConnectionObject*> m_lstDataConnections;
            CXSpinLock m_lock;
            string m_strSessionGuid;
            string m_strTemporaryVerificationCode;
            unordered_map<string, void*> m_mapData;
            
    };
}
#endif // CXCONNECTIONSESSION_H
