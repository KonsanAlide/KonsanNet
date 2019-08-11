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

#include "CXGuidObject.h"
#ifdef WIN32
#include <objbase.h>
#else
#include <uuid/uuid.h>
#endif // WIN32
#include "PlatformFunctionDefine.h"

#include <stdio.h>
#include <string.h>

struct cxguid{
		unsigned long Data1;
		unsigned short Data2;
		unsigned short Data3;
		unsigned char Data4[8];
} ;

CXGuidObject::CXGuidObject(bool bGenerateGuid)
{
	m_bGenerated = false;
	memset(m_byGuid,0, CX_GUID_LEN);

	if (bGenerateGuid)
	{
		m_strGuid = GenerateNewGuid(m_byGuid);
		if (m_strGuid != "")
		{
			m_bGenerated = true;
		}
	}
}


CXGuidObject::~CXGuidObject()
{
}

string CXGuidObject::GenerateNewGuid(byte *pbyGuid)
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
    }

#else
    uuid_t guid;
    int i;
    uuid_generate(guid);

	uuid_unparse_upper(guid, buf+1);
	buf[0] = '{';
	buf[strlen(buf)] = '}';

    byte *pBytes = (byte*)&guid;
	ConvertBytes(pBytes,sizeof(DWORD));
	pBytes+=sizeof(DWORD);

	ConvertBytes(pBytes,sizeof(WORD));
	pBytes+=sizeof(WORD);

	ConvertBytes(pBytes,sizeof(WORD));

	/*
    for (i = 0; i<16; i++)
    {
        sprintf_s(buf, sizeof(buf), "%02X-", uu[i]);
        strGuid += buf;
    }
	*/
#endif//WIN32
	memcpy(pbyGuid, &guid, CX_GUID_LEN);
	strGuid = buf;
    return strGuid;
}

string CXGuidObject::GenerateNewGuid()
{
	byte byGuid[CX_GUID_LEN] = { 0 };
	return GenerateNewGuid(byGuid);
}

string CXGuidObject::ConvertGuid(const byte *pbyGuid)
{
	string strGuid = "";
	char buf[64] = { 0 };
#ifdef WIN32
	GUID guid;
	memcpy(&guid, pbyGuid, CX_GUID_LEN);
	_snprintf(buf, sizeof(buf)
		, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"
		, guid.Data1
		, guid.Data2
		, guid.Data3
		, guid.Data4[0], guid.Data4[1]
		, guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5]
		, guid.Data4[6], guid.Data4[7]
	);
#else
    uuid_t guid;
    memcpy(&guid,pbyGuid,CX_GUID_LEN);

    byte *pBytes = (byte*)&guid;
	ConvertBytes(pBytes,sizeof(DWORD));
	pBytes+=sizeof(DWORD);

	ConvertBytes(pBytes,sizeof(WORD));
	pBytes+=sizeof(WORD);

	ConvertBytes(pBytes,sizeof(WORD));
	//uuid_unparse_upper(guid, buf);
	uuid_unparse_upper(guid, buf+1);
	buf[0] = '{';
	buf[strlen(buf)] = '}';

#endif//WIN32

	strGuid = buf;
	return strGuid;
}

void CXGuidObject::ConvertBytes(byte*pBytes,int iLen)
{
    byte byTemp = 0;
    for(int i=0;i<iLen/2;i++)
    {
        byTemp = pBytes[i];
        pBytes[i] = pBytes[iLen-i-1];
        pBytes[iLen-i-1] = byTemp;
    }
}


void CXGuidObject::ConvertGuid(const string &strGuid, byte *pbyGuid)
{
	unsigned int p0=0;
	unsigned int p1, p2;
	unsigned int p3, p4, p5, p6, p7, p8, p9, p10;
	p1=p2=p3=p4 = p5 = p6 = p7 = p8 = p9 = p10=0;


#ifdef WIN32
	GUID guid;
    int err = sscanf_s(strGuid.c_str(), "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
		&p0, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10);

#else
	int err = sscanf(strGuid.c_str(), "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
		&p0, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10);

    cxguid guid;

#endif//WIN32

    guid.Data1 = p0;
	guid.Data2 = p1;
	guid.Data3 = p2;
	guid.Data4[0] = p3;
	guid.Data4[1] = p4;
	guid.Data4[2] = p5;
	guid.Data4[3] = p6;
	guid.Data4[4] = p7;
	guid.Data4[5] = p8;
	guid.Data4[6] = p9;
	guid.Data4[7] = p10;

    byte *pHeader = pbyGuid;
	memcpy(pbyGuid, &guid.Data1, sizeof(DWORD));
	pbyGuid+=sizeof(DWORD);
	memcpy(pbyGuid, &guid.Data2, sizeof(WORD));
	pbyGuid+=sizeof(WORD);
	memcpy(pbyGuid, &guid.Data3, sizeof(WORD));
	pbyGuid+=sizeof(WORD);
	memcpy(pbyGuid, &guid.Data4, 8);
/*
#ifdef WIN32
#else
	ConvertBytes(pHeader,sizeof(DWORD));
	pHeader+=sizeof(DWORD);

	ConvertBytes(pHeader,sizeof(WORD));
	pHeader+=sizeof(WORD);

	ConvertBytes(pHeader,sizeof(WORD));
#endif//WIN32
*/

}

bool CXGuidObject::GetGuidBytes(byte *pbyGuid)
{
	if (!m_bGenerated)
		return false;
	memcpy(pbyGuid, m_byGuid, CX_GUID_LEN);
	return true;
}
