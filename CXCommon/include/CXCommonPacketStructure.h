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

Description£ºThe structure of the data packet used in the common comnunication.
             contain the header of all packet,common packet and reply message.
*****************************************************************************/
#ifndef __CXCOMMONPACKETSTRUCTURE_H__
#define __CXCOMMONPACKETSTRUCTURE_H__

#include "PlatformDataTypeDefine.h"

#pragma pack(1)

#ifndef _CX_PACKET_HEADER
typedef struct _CX_PACKET_HEADER
{
    WORD  wDataLen;
    byte  byType;
    byte  byReserve;
    DWORD dwCheckSum;
    DWORD dwFlag;
    DWORD dwPacketNum;
}CXPacketHeader, *PCXPacketHeader;
#endif

#ifndef _CX_PACKET_DATA
typedef struct _CX_PACKET_DATA
{
    CXPacketHeader header;
    DWORD dwMesCode;
    byte buf[1];
}CXPacketData, *PCXPacketData;
#endif

#ifndef _CX_COMMON_MESSAGE
typedef struct _CX_COMMON_MESSAGE
{
    byte  byType;
    byte  byReserve[3];
    DWORD dwValue1;
    DWORD dwValue2;
    DWORD dwDataLen;
    char  szData[1];
}CXCommonMessage, *PCXCommonMessage;
#endif

#ifndef _CX_COMMON_MESSAGE_REPLY
typedef struct _CX_COMMON_MESSAGE_REPLY
{
    DWORD dwReplyCode;
    DWORD dwValue1;
    DWORD dwValue2;
    DWORD dwDataLen;
    char  szData[1];
}CXCommonMessageReply, *PCXCommonMessageReply;
#endif


#pragma pack()

#endif //__CXCOMMONPACKETSTRUCTURE_H__