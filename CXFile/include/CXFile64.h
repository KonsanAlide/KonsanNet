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
#ifndef CXFILE64_H
#define CXFILE64_H

#ifdef WIN32  
#include <Windows.h>  
#else  //WIN32  
#endif  //WIN32  
#include <string>

#include "PlatformDataTypeDefine.h" 

using namespace std;
class CXFile64
{
public:
    enum SEEKTYPE
    {
        begin = 0,
        current,
        end
    };
    enum OPENTYPE
    {
        modeRead=0,
        modeWrite,
        modeReadWrite,
        modeNoTruncateWrite,
        modeNoTruncateReadWrite
    };

public:
    CXFile64();
    virtual ~CXFile64();

    bool Open(string strFilePath,OPENTYPE type, DWORD dwFlagsAndAttributes=0,BOOL bOnlyOpenExistingFile=FALSE);

    //return value : ==true, if read to end of the file, the dwReadLen will be set to zero.
    bool Read(byte* pBuf, DWORD dwWantReadLen, DWORD &dwReadLen);

    bool Seek(int64 pos, SEEKTYPE type);
    bool GetCurrentPosition(uint64 & uiPos);
    bool Write(const byte* pBuf, DWORD dwBufLen, DWORD &dwWrittenLen);
    bool Flush();
    bool TruncateFile(uint64 uiFileLen);
    bool Close();
    bool IsOpen() { return m_bIsOpened; }
    string GetFilePathName() { return m_strFilePath; }
    bool GetFileLength(uint64 & uiFileLength);
    bool GetLastModifiedTime(time_t & tmFileModified);
    DWORD GetErrorCode();
    string GetErrorDescription();


private:
    string  m_strFilePath;
    bool    m_bIsOpened;
#ifdef WIN32
    HANDLE m_file;
#else
    int    m_file;
#endif
};

#endif //CXFILE64_H 