/****************************************************************************
Copyright (c) 2018 Chance Yang

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
#pragma once
#include "PlatformDataTypeDefine.h"
#include <string>
using std::string;
class CXFileTcpClientTest
{
public:
    CXFileTcpClientTest();
    ~CXFileTcpClientTest();

    int Download(int iNumber);
    int Upload(int iNumber);
    bool CompareFile(string strFile1,string strFile2);

    bool CompareFile(string strFile1, byte *pFileData, int iDataLen);
};

