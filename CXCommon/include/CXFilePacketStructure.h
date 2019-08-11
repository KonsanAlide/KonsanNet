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

Description£ºThe structure of the data packet used in the file transmission comnunication.
             eg: open file,read data, write data,close file, and so on.
*****************************************************************************/
#ifndef __CXFILEPACKETSTRUCTURE_H__
#define __CXFILEPACKETSTRUCTURE_H__

#include "PlatformDataTypeDefine.h"
#pragma pack(1)

#ifndef _CX_FILE_OPEN_FILE
typedef struct _CX_FILE_OPEN_FILE
{
    byte  byOpenType;
    byte  byReserve[3];
    DWORD dwFilePathLen;
    char  szFilePath[1];
}CXFileOpenFile, *PCXFileOpenFile;
#endif


#ifndef _CX_FILE_SEEK
typedef struct _CX_FILE_SEEK
{
    DWORD dwSeekType;
    int64 iSeekPos;
}CXFileSeek, *PCXFileSeek;
#endif

#ifndef _CX_FILE_READ
typedef struct _CX_FILE_READ
{
    DWORD dwSeekType;
    int64 iBeginPos;
    DWORD dwReadLen;
}CXFileRead, *PCXFileRead;
#endif

#ifndef _CX_FILE_READ_REPLY
typedef struct _CX_FILE_READ_REPLY
{
    DWORD dwReplyCode;
    DWORD dwDataLen;
    char  szData[1];
}CXFileReadReply, *PCXFileReadReply;
#endif

#ifndef _CX_FILE_WRITE
typedef struct _CX_FILE_WRITE
{
    DWORD dwSeekType;
    int64 iBeginPos;
    DWORD dwDataLen;
    char  szData[1];
}CXFileWrite, *PCXFileWrite;
#endif

#ifndef _CX_FILE_CLOSE
typedef struct _CX_FILE_CLOSE
{
    DWORD  dwType;
    DWORD  dwDataLen;
    char   szData[1];
}CXFileClose, *PCXFileClose;
#endif


#pragma pack()

#endif //__CXFILEPACKETSTRUCTURE_H__