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
#ifndef CXSESSIONLEVLBASE_H
#define CXSESSIONLEVLBASE_H

#include "CXServerStructDefine.h"
#include "CXSessionsManager.h"
namespace CXCommunication
{
    class CXSessionLevelBase
    {
    public:
        CXSessionLevelBase();
        virtual ~CXSessionLevelBase();

        virtual int SessionLogin(PCXBufferObj pBuf, CXConnectionSession ** ppSession);
        virtual int SessionLogout(PCXBufferObj pBuf, CXConnectionSession &session);
        virtual int SessionSetting(PCXBufferObj pBuf, CXConnectionSession &session);
    };
}
#endif // CXSESSIONLEVLBASE_H
