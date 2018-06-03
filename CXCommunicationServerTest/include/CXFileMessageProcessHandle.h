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
#ifndef CXFILEMESSAGEPROCESSHANDLE_H
#define CXFILEMESSAGEPROCESSHANDLE_H

#include "CXConnectionObject.h"
#include "CXConnectionSession.h"
#include "CXCommonPacketStructure.h"
#include "CXUserMessageProcessBase.h"
namespace CXCommunication
{
    class CXFileMessageProcessHandle : public CXUserMessageProcessBase
    {
    public:
        CXFileMessageProcessHandle();
        ~CXFileMessageProcessHandle();

        int OnReceivedMessage(const PCXMessageData pMes, CXConnectionObject * pCon,
            CXConnectionSession *pSession);

        int SendData(CXConnectionObject * pCon, const byte *pbyData, DWORD dwDataLen);
    };
}
#endif //CXFILEMESSAGEPROCESSHANDLE_H

