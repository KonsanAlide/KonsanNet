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
#ifndef CXSERVERSTRUCTDEFINE_H
#define CXSERVERSTRUCTDEFINE_H

#include "PlatformDataTypeDefine.h"
#ifdef WIN32
#include <winsock2.h>
#else
#include  <unistd.h>
#include  <sys/types.h>       /* basic system data types */
#include  <sys/socket.h>      /* basic socket definitions */
#include  <netinet/in.h>      /* sockaddr_in{} and other Internet defns */
#include  <arpa/inet.h>       /* inet(3) functions */
#include <sys/epoll.h> /* epoll function */
#include <sys/resource.h> /*setrlimit */
#include <pthread.h>
#include <fcntl.h>     /* nonblocking */
#endif

namespace CXCommunication
{
    
#ifndef WIN32
    typedef struct _WSABUF {
        DWORD len; 
        char *buf;
    } WSABUF, *LPWSABUF;
#endif

    typedef struct _CX_BUFFER_OBJ
    {
#ifdef WIN32
        OVERLAPPED ol;
#endif
        WSABUF wsaBuf;

        void *pConObj;
        
        //the size of the data in the buffer
        int  iBufSize;

        //the start point of the left data 
        int nCurDataPointer;

        //the sequence number of this buffer
        uint64 nSequenceNum;

        #define OP_ACCEPT  1
        #define OP_READ    2
        #define OP_WRITE   3
        int nOperate;

        _CX_BUFFER_OBJ * pNext;
        char buf[BUF_SIZE];
    }CXBufferObj, *PCXBufferObj;


    
}
#endif // CXSERVERSTRUCTDEFINE_H

