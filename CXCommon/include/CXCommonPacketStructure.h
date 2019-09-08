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
    //==0x0  not compress
	//==0x81 zlib
	//==0x82 snappy
    byte  byCompressFlag;
	//==0x0  not encrypt
	//==0x81 RSA
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
	byte  byObjectGuid[CX_GUID_LEN];
	byte  byRequestID[CX_GUID_LEN];
    DWORD dwMesCode;
    DWORD dwPacketNum;
    byte  buf[1];
}CXPacketBodyData, *PCXPacketBodyData;
#endif

//the message structure used inner of the server
#ifndef _CX_MESSAGE_DATA
typedef struct _CX_MESSAGE_DATA
{
	//==1 normal message
	//==2 this message is not need to process, 
	//    only call the pConObj->ProcessUnpackedMessage(NULL) to process the left message;
    DWORD dwType;
    DWORD dwDataLen;
	int64 iBeginTime;
	int64 iBeginProcessTime;
	//the sequence number of the received packet 
	int64 iSequenceNum;
    void  *pConObj;
    //CX_RESULT define the value
	int   iProcessedRet;
	_CX_MESSAGE_DATA *pNext;
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