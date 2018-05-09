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
#ifndef CXCOMUNICATIONUSER_H
#define CXCOMUNICATIONUSER_H
#include "PlatformDataTypeDefine.h"
#include <queue>
#include <string>
using namespace std;
namespace CXCommunication
{
    class CXComunicationUser
    {
        public:
            CXComunicationUser();
            virtual ~CXComunicationUser();
            void PushReceivedData();

        protected:
        private:
            queue<char *>m_queueDataRecv;
            uint64 m_uiConnectionIndex;
            string m_strUniqueID;
            string m_strUserName;
            string m_strUserDescription;
            string m_strIPAddr;
            int    m_nPort;
            int    m_iUserType;
    };
}

#endif // CXCOMUNICATIONUSER_H
