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

Description：
*****************************************************************************/
#include "CXFile64.h"
#include <time.h>
#ifndef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE
#endif

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#endif


CXFile64::CXFile64()
{
    m_strFilePath = "";
    m_bIsOpened = false;
#ifdef WIN32
    m_file = (HANDLE)-1;
#else
    m_file = -1;
#endif

}

CXFile64::~CXFile64()
{

}

bool CXFile64::Open(string strFilePath, OPENTYPE type, DWORD dwFlagsAndAttributes, BOOL bOnlyOpenExistingFile)
{

#ifdef WIN32
    DWORD dwDesiredAccess = 0;

    //if this file not exist , create it and open
    DWORD dwCreateFlag = OPEN_ALWAYS;
    DWORD dwShareFlag = FILE_SHARE_READ | FILE_SHARE_WRITE;
    bool bTruncateFile = true;

    if (dwFlagsAndAttributes == 0)
        dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;

    if (bOnlyOpenExistingFile)
    {
        dwCreateFlag = OPEN_EXISTING;
    }

    switch (type)
    {
    case CXFile64::modeRead:
        dwDesiredAccess = GENERIC_READ;
        //only open the existing file
        dwCreateFlag = OPEN_EXISTING;
        bTruncateFile = false;
        break;
    case CXFile64::modeWrite:
        dwDesiredAccess = GENERIC_WRITE;
        break;
    case CXFile64::modeReadWrite:
        dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
        break;
    case CXFile64::modeNoTruncateWrite:
        dwDesiredAccess = GENERIC_WRITE;
        bTruncateFile = false;
        break;
    case CXFile64::modeNoTruncateReadWrite:
        dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
        bTruncateFile = false;
        break;
    default:
        break;
    }

    // if the file exists,and need to open file to write,
    // the default mode is that truncate the file to zero size.
    DWORD dwAttrib = GetFileAttributes(strFilePath.c_str());

    // if this path indicate a file and this file exists, if need to truncate it,
    // add the TRUNCATE_EXISTING bit to the dwCreateFlag .
    if (INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
    {
        if (bTruncateFile && (dwDesiredAccess&GENERIC_WRITE))
        {
             dwCreateFlag |= TRUNCATE_EXISTING;
        }
        /*
        else
        {
            dwCreateFlag = OPEN_EXISTING;
        }
        */
    }



    // if the file not exists, create it and open.
    m_file = ::CreateFile(strFilePath.c_str(), dwDesiredAccess, dwShareFlag, NULL,
        dwCreateFlag, dwFlagsAndAttributes, NULL);
    if (m_file == INVALID_HANDLE_VALUE)
    {
        return false;
    }
#else

    int iOpenFlags = 0;
    mode_t iOpenMode= 0644;

    DWORD dwDesiredAccess = 0;
    DWORD dwCreateFlag = 0;
    DWORD dwShareFlag = 0;
    bool bTruncateFile = true;

    //if (dwFlagsAndAttributes == 0)
    //    dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;



    switch (type)
    {
    case CXFile64::modeRead:
        iOpenFlags = O_RDONLY;
        break;
    case CXFile64::modeWrite:
        iOpenFlags = O_WRONLY;
        iOpenFlags |= O_TRUNC;
        iOpenFlags |= O_CREAT;
        break;
    case CXFile64::modeReadWrite:
        iOpenFlags = O_RDWR;
        iOpenFlags |= O_TRUNC;
        iOpenFlags |= O_CREAT;
        break;
    case CXFile64::modeNoTruncateWrite:
        iOpenFlags = O_WRONLY;
        iOpenFlags |= O_CREAT;
        break;
    case CXFile64::modeNoTruncateReadWrite:
        iOpenFlags = O_RDWR;
        iOpenFlags |= O_CREAT;
        break;
    default:
        break;
    }

    // if the file exists,and need to open file to write,
    // the default mode is that truncate the file to zero size.
    //DWORD dwAttrib = GetFileAttributes(strFilePath.c_str());


    m_file = open(strFilePath.c_str(), iOpenFlags, 0644);
    if (m_file <= 0) {
        return false;
    }

#endif // WIN32

    m_strFilePath = strFilePath;
    m_bIsOpened = true;
    return true;
}
bool  CXFile64::Read(byte* pBuf, DWORD dwWantReadLen, DWORD &dwReadLen)
{
    dwReadLen = 0;
#ifdef WIN32
    if (m_file == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    BOOL bRet = ::ReadFile(m_file, pBuf, dwWantReadLen, &dwReadLen,NULL);
    if (bRet)
    {
        return true;
    }
#else
    if (m_file <=0)
    {
        return false;
    }
    dwReadLen = ::read(m_file, pBuf, dwWantReadLen);
    if (dwReadLen>0)
    {
        return true;
    }


#endif // WIN32
    return false;
}
bool CXFile64::Seek(int64 pos, SEEKTYPE type)
{
    uint64  uiFileLength = 0;

#ifdef WIN32
    if (m_file == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    //in windows system , if the file is a disk or a volume,
    //this function will return error
    if (GetFileLength(uiFileLength))
    {
        if (pos > uiFileLength)
        {
            return false;
        }
    }

    LARGE_INTEGER liPos;
    liPos.QuadPart = 0;

    //the new position
    LARGE_INTEGER liCurPos;
    liCurPos.QuadPart = 0;

    DWORD dwMoveMethod = FILE_CURRENT;
    //get the current position of the file position
    if (!::SetFilePointerEx(m_file, liPos, &liCurPos, dwMoveMethod))
    {
        return false;
    }


    switch(type)
    {
    case begin:
        liPos.QuadPart = pos;
        dwMoveMethod = FILE_BEGIN;
        break;
    case current:
        liPos.QuadPart = pos;
        dwMoveMethod = FILE_CURRENT;
        break;
    case end:
        liPos.QuadPart = pos;
        dwMoveMethod = FILE_END;
        break;
    default:
        break;
    }


    //the new position
    LARGE_INTEGER liNewPos;
    liNewPos.QuadPart = 0;
    if (!::SetFilePointerEx(m_file, liPos, &liNewPos, dwMoveMethod))
    {
        //if this function fails, return the file pointer to the old position.
        if (liNewPos.QuadPart != liCurPos.QuadPart)
        {
            liNewPos.QuadPart = 0;
            ::SetFilePointerEx(m_file, liCurPos, &liNewPos, FILE_BEGIN);
        }
        return false;
    }
#else

    if (!GetFileLength(uiFileLength))
    {
        return false;
    }

    if (pos > uiFileLength)
    {
        return false;
    }

    uint64 uiOldPos = ::lseek(m_file, SEEK_CUR, 0);
    int iWhence = 0;

    switch (type)
    {
    case begin:
        iWhence = SEEK_SET;
        break;
    case current:
        iWhence = SEEK_CUR;
        break;
    case end:
        iWhence = SEEK_END;
        break;
    default:
        break;
    }

    uint64 uiNewPos = lseek(m_file, iWhence, pos);
    if (uiNewPos == -1)
    {
        return false;
        //uiOldPos
    }

#endif // WIN32
    return true;
}
bool CXFile64::GetCurrentPosition(uint64 & uiPos)
{
#ifdef WIN32
    if (m_file == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    LARGE_INTEGER liPos;
    liPos.QuadPart = 0;

    //the new position
    LARGE_INTEGER liNewPos;
    liNewPos.QuadPart = 0;

    DWORD dwMoveMethod = FILE_CURRENT;

    if (!SetFilePointerEx(m_file, liPos, &liNewPos, dwMoveMethod))
    {
        return false;
    }
    uiPos = liNewPos.QuadPart;
    return true;
#else
    if (m_file <= 0)
    {
        return false;
    }
    uiPos = ::lseek(m_file, SEEK_CUR, 0);
    return true;
#endif // WIN32

}
bool  CXFile64::Write(const byte* pBuf, DWORD dwBufLen, DWORD &dwWrittenLen)
{
    dwWrittenLen = 0;
#ifdef WIN32
    if (m_file == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    BOOL bRet = ::WriteFile(m_file, pBuf, dwBufLen, &dwWrittenLen, NULL);
    if (bRet)
    {
        return true;
    }
#else
    if (m_file <= 0)
    {
        return false;
    }
    dwWrittenLen = ::write(m_file, pBuf, dwBufLen);
    if (dwWrittenLen>0)
    {
        return true;
    }
#endif // WIN32
    return false;
}
bool  CXFile64::Flush()
{
#ifdef WIN32
    if (m_file == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    if (::FlushFileBuffers(m_file))
    {
        return true;
    }
    return false;
#else
    //flush(m_file);
#endif // WIN32
}

bool CXFile64::TruncateFile(uint64 uiFileLen)
{
#ifdef WIN32
    if (m_file == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    if (Seek(uiFileLen, CXFile64::begin))
    {
        BOOL bRet = ::SetEndOfFile(m_file);
        if (bRet)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

#else
    if (m_file <= 0)
    {
        return false;
    }

    if (ftruncate64(m_file, uiFileLen) < 0)
    //if (ftruncate(m_file, uiFileLen) == 0)
    {
        return true;
    }
#endif // WIN32
    return false;

}

bool CXFile64::Close()
{
    if (!m_bIsOpened)
    {
        return true;
    }

#ifdef WIN32
    if (m_file == INVALID_HANDLE_VALUE)
    {
        return true;
    }
    if (!::CloseHandle(m_file))
    {
        return false;
    }
    m_file = INVALID_HANDLE_VALUE;

#else
    if (m_file <= 0)
    {
        return true;
    }
    close(m_file);
    m_file = -1;
    
#endif // WIN32

    m_bIsOpened = false;
    return true;

}

bool CXFile64::GetFileLength(uint64 & uiFileLength)
{
#ifdef WIN32
    if (m_file == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    LARGE_INTEGER liSize;
    liSize.QuadPart = 0;
    BOOL bRet = ::GetFileSizeEx(m_file,&liSize);
    if (!bRet)
    {
        DWORD dwError = GetLastError();
        return false;
    }
    uiFileLength = liSize.QuadPart;
#else
    if (m_file <= 0)
    {
        return false;
    }

    //if want to read 64bit large file, add the flags -D_FILE_OFFSET_BITS=64 when compile the program
    struct stat fileStat;
    memset(&fileStat,0,sizeof(fileStat));
    int iRet = ::stat(m_strFilePath.c_str(), &fileStat);
    if (iRet!=0)
    {
        return false;
    }

    uiFileLength = fileStat.st_size;

#endif // WIN32

    return true;
}

bool  CXFile64::GetLastModifiedTime(time_t & tmFileModified)
{
    tmFileModified = 0;

#ifdef WIN32
    if (m_file == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    WIN32_FILE_ATTRIBUTE_DATA fileAttrData;
    memset(&fileAttrData,0,sizeof(WIN32_FILE_ATTRIBUTE_DATA));
    if (!::GetFileAttributesEx(m_strFilePath.c_str(), GetFileExInfoStandard,&fileAttrData))
    {
        return false;
    }
    tmFileModified = fileAttrData.ftLastWriteTime.dwHighDateTime;
    tmFileModified <<= 8;
    tmFileModified += fileAttrData.ftLastWriteTime.dwLowDateTime;
    return true;
#else
    if (m_file <= 0)
    {
        return false;
    }


    //if want to read 64bit large file, add the flags -D_FILE_OFFSET_BITS=64 when compile the program
    struct stat fileStat;
    memset(&fileStat, 0, sizeof(fileStat));
    int iRet = ::stat(m_strFilePath.c_str(), &fileStat);
    if (iRet != 0)
    {
        return false;
    }
    tmFileModified = fileStat.st_ctime;
    return errno;
#endif
}

DWORD CXFile64::GetErrorCode()
{
#ifdef WIN32
    return GetLastError();
#else
    return errno;
#endif
}

string CXFile64::GetErrorDescription()
{
#ifdef WIN32
    DWORD dwErrorCode = GetLastError();
    char szErrDesc[1024] = { 0 };


    LPVOID lpDescBuf = NULL;
    DWORD dwLen = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwErrorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR)&lpDescBuf,
        0,
        NULL
    );
    if (dwLen == 0)
    {
        DWORD dwFmtErrCode = GetLastError(); //FormatMessage 引起的错误代码
        sprintf_s(szErrDesc, "FormatMessage failed with %u\n", dwFmtErrCode);
    }

    if (lpDescBuf)
    {
        sprintf_s(szErrDesc, "Error Code = %u, description = %s",
            dwErrorCode, (LPCTSTR)lpDescBuf);

    }

    if (lpDescBuf)
    {
        // Free the buffer.
        LocalFree(lpDescBuf);
        lpDescBuf = NULL;
    }

    return szErrDesc;
#else
    string strDesc = "";
    char *pszErr = strerror(errno);
    if (pszErr != NULL)
        strDesc = strerror(errno);
    return strDesc;
#endif

}
