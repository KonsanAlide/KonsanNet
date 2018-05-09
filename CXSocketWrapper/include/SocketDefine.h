/***************************************************************************************
*Description:
*           This file contain the related definitions about the socket,
*           the definitions will use in the windows platform and linux platform projects.
*Writer:    Cheng.Yang
*Date  :    2017.11.30
****************************************************************************************/


#ifndef __CXSOCKETDEFINE_H__
#define __CXSOCKETDEFINE_H__


#ifdef WIN32

#include <winsock2.h>
typedef SOCKET cxsocket;
#pragma warning(disable : 4786)
#pragma warning(disable : 4996)

#else

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

typedef int cxsocket;
typedef SOCKET cxsocket;
#define SOCKET_ERROR  (-1)  
#define INVALID_SOCKET (-1)
#define SD_SEND     SHUT_RD
#define SD_RECEIVE  SHUT_WR
#define SD_BOTH     SHUT_RDWR
#endif

#endif

