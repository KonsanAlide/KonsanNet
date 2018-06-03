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
#ifndef CXADVANCEDSESSIONMESSAGEPROCESSHANDLE_H
#define CXADVANCEDSESSIONMESSAGEPROCESSHANDLE_H

#include "CXSessionLevelBase.h"

namespace CXCommunication
{
    class CXAdvancedSessionMessageProcessHandle :public CXSessionLevelBase
    {
    public:
        CXAdvancedSessionMessageProcessHandle();
        ~CXAdvancedSessionMessageProcessHandle();

        int SessionLogin(PCXMessageData pMes, CXConnectionSession ** ppSession);
        int SessionLogout(PCXMessageData pMes, CXConnectionSession &session);
        int SessionSetting(PCXMessageData pMes, CXConnectionSession &session);

    };
}
#endif //CXADVANCEDSESSIONMESSAGEPROCESSHANDLE_H

