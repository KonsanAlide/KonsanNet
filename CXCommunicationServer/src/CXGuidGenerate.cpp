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

#include "CXGuidGenerate.h"
#ifdef WIN32
#include <objbase.h>
#else
#include <uuid/uuid.h>
#endif // WIN32
#include "PlatformFunctionDefine.h"

#include <stdio.h>

CXGuidGenerate::CXGuidGenerate()
{
#ifdef WIN32
    CoInitialize(NULL);
#endif
}


CXGuidGenerate::~CXGuidGenerate()
{
#ifdef WIN32
    CoUninitialize();
#endif
}

string CXGuidGenerate::GenerateGuid()
{
    string strGuid = "";
    char buf[64] = { 0 };
#ifdef WIN32

    GUID guid;
    if (S_OK == ::CoCreateGuid(&guid))
    {
        _snprintf(buf, sizeof(buf)
            , "{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}"
            , guid.Data1
            , guid.Data2
            , guid.Data3
            , guid.Data4[0], guid.Data4[1]
            , guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5]
            , guid.Data4[6], guid.Data4[7]
        );
        strGuid = buf;
    }

#else
    uuid_t uu;
    int i;
    uuid_generate(uu);

    for (i = 0; i<16; i++)
    {
        sprintf_s(buf, sizeof(buf), "%02X-", uu[i]);
        strGuid += buf;
    }
#endif//WIN32
    return strGuid;
}
