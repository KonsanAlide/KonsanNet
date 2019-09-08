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
#include "CXDataParserImpl.h"
#include <map>
#include <chrono>
#include <ctime>
#ifdef WIN32
#else
#include <string.h>
#endif
#include "PlatformFunctionDefine.h"
using namespace std;

namespace CXCommunication
{
    CXDataParserImpl::CXDataParserImpl()
    {
        //ctor
    }

    CXDataParserImpl::~CXDataParserImpl()
    {
        //dtor
    }

    //pszTimeString: if not NULL,will save the time string , the format is: 2019-07-31_15:39:29
    int64 CXDataParserImpl::GetCurrentTimeMS(char *pszTimeString)
    {
        typedef chrono::time_point<chrono::system_clock, chrono::milliseconds> MsClockType;
        MsClockType tp = chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now());

        if (pszTimeString != NULL)
        {
            time_t timeCur = chrono::system_clock::to_time_t(tp);
            std::strftime(pszTimeString, 60, "%Y-%m-%d_%H:%M:%S", std::localtime(&timeCur));
            char szMSTime[10] = { 0 };
            sprintf_s(szMSTime, 10, ".%03d", (int)(tp.time_since_epoch().count() % 1000));
            strcat(pszTimeString, szMSTime);
        }

        return (int64)tp.time_since_epoch().count();
    }
}
