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

#ifndef __CXSOCKETDEFINE_H__
#define __CXSOCKETDEFINE_H__


#ifdef WIN32

#include <winsock2.h>
typedef SOCKET cxsocket;
#pragma warning(disable : 4786)
#pragma warning(disable : 4996)

typedef int socklen_t;

#else //WIN32

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/tcp.h>

typedef int cxsocket;
#define SOCKET_ERROR  (-1)
#define INVALID_SOCKET (-1)
#define SD_SEND     SHUT_RD
#define SD_RECEIVE  SHUT_WR
#define SD_BOTH     SHUT_RDWR

#ifndef closesocket
#define closesocket close
#else //closesocket
#endif //closesocket


#endif //WIN32

#endif //__CXSOCKETDEFINE_H__

