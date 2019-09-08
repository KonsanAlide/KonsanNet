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

Description£ºThis file contain the related definitions about the socket,
             the definitions will use in the windows platform and linux platform projects.
*****************************************************************************/


#ifndef __PLATFORMDATATYPEDEFINE_H__
#define __PLATFORMDATATYPEDEFINE_H__

#include "CXErrorCode.h"

#define CX_BUF_SIZE 4096
#define CLIENT_BUF_SIZE 4096
#define CX_GUID_LEN 16

//10MB
#define CX_MAX_CHACHE_SIZE 1048576

#ifdef WIN32
#ifndef byte
typedef unsigned char      byte;
#endif
typedef __int64            int64;
typedef unsigned __int64   uint64;

#define RETURN_SUCCEED     0
#define INVALID_PARAMETER  -1
//this function had enter a asynchronous process,
//this asynchronous process will return in other way.
#define ASYNC_CALLBACK     -1000

#else
#include <inttypes.h>


#ifndef BOOL
typedef int32_t    BOOL;
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    1
#endif

#ifndef byte
typedef uint8_t    byte;
#endif

#ifndef byte
typedef uint8_t    BYTE;
#endif

#ifndef DWORD
typedef uint32_t    DWORD;
#endif

#ifndef WORD
typedef uint16_t    WORD;
#endif

#ifndef int64
typedef int64_t    int64;
#endif

#ifndef uint64
typedef uint64_t    uint64;
#endif

#define RETURN_SUCCEED 0
#define INVALID_PARAMETER -1
#define ASYNC_CALLBACK     -1000

#ifndef NULL
#define NULL 0
#endif

#ifndef INFINITE
#define INFINITE (DWORD)0xFFFFFFFF
#endif

#ifndef WAIT_ABANDONED
#define WAIT_ABANDONED 0x00000080L
#endif

#ifndef WAIT_OBJECT_0
#define WAIT_OBJECT_0 0x00000000L
#endif

#ifndef WAIT_TIMEOUT
#define WAIT_TIMEOUT 0x00000102L
#endif

#ifndef WAIT_FAILED
#define WAIT_FAILED (DWORD)0xFFFFFFFF
#endif


#endif


#endif


