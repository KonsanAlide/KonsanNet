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
#ifndef __CXSESSIONLOGINRPCSERVER_H__
#define __CXSESSIONLOGINRPCSERVER_H__

#include "CXSessionLevelBase.h"

namespace CXCommunication
{
    class CXSessionLoginRPCServer :public CXSessionLevelBase
    {
    public:
        CXSessionLoginRPCServer();
        ~CXSessionLoginRPCServer();

        virtual string GetObjectName() { return "CXSessionLoginV10001"; }

        virtual CXRPCObjectServer* CreateObject();

        virtual void Destroy();

        virtual void MessageToString(PCXMessageData pMes);

    };
}
#endif //__CXSESSIONLOGINRPCSERVER_H__

