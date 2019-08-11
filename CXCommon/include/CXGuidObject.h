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
#ifndef __CXGUIDOBJECT_H__
#define __CXGUIDOBJECT_H__

#include "PlatformDataTypeDefine.h"
#include <string>
using namespace std;
class CXGuidObject
{
public:
	CXGuidObject(bool bGenerateGuid=true);
    ~CXGuidObject();

    string GenerateNewGuid(byte *pbyGuid);
	string GenerateNewGuid();
	string ConvertGuid(const byte *pbyGuid);
	void   ConvertGuid(const string &strGuid, byte *pbyGuid);

	string GetGuidString() { return m_strGuid; }
	bool   GetGuidBytes(byte byGuid[CX_GUID_LEN]);

	void  ConvertBytes(byte*pBytes,int iLen);

private:
	string m_strGuid;
	byte   m_byGuid[CX_GUID_LEN];
	bool   m_bGenerated;

};
#endif //__CXGUIDOBJECT_H__

