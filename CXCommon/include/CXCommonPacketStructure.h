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

#define CX_PACKET_HEADER_FLAG 0x25F7
#pragma pack(1)

//the header structure of the packet transferd between client and server
#ifndef _CX_PACKET_HEADER
typedef struct _CX_PACKET_HEADER
{
    //packet flag: CX_PACKET_HEADER_FLAG
    WORD  wFlag;
    //compress data,if the first bit is 1, the packet is compressed,the left 7 bit is the compressed type
    byte  byCompressFlag;
    //encrypt data,if the first bit is 1, the packet is encrypted,the left 7 bit is the encrypted type
    byte  byEncryptFlag;

    DWORD dwDataLen;
    //check sum
    DWORD dwCheckSum;
    
    DWORD dwOrignDataLen;
}CXPacketHeader, *PCXPacketHeader;
#endif


//the body data structure of the packet transferd between client and server
#ifndef _CX_PACKET_BODY_DATA
typedef struct _CX_PACKET_BODY_DATA
{
	byte  byObjectGuid[16];
	byte  byPacketGuid[16];
    DWORD dwMesCode;
    DWORD dwPacketNum;
    byte  buf[1];
}CXPacketBodyData, *PCXPacketBodyData;
#endif

//the message structure used inner of the server
#ifndef _CX_MESSAGE_DATA
typedef struct _CX_MESSAGE_DATA
{
    DWORD dwType;
    DWORD dwDataLen;
    void  *pConObj;
    CXPacketBodyData bodyData;
}CXMessageData, *PCXMessageData;
#endif

//a packet structure transferd between client and server
#ifndef _CX_PACKET_DATA
typedef struct _CX_PACKET_DATA
{
    CXPacketHeader header;
    CXPacketBodyData bodyData;
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