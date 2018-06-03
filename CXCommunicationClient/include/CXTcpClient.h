/****************************************************************************
Copyright (c) 2018 Chance Yang

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
#ifndef __CXTCPCLIENT_H__
#define __CXTCPCLIENT_H__

#include "CXSocketImpl.h"
#include "CXCommonPacketStructure.h"
#include "CXDataParserImpl.h"

namespace CXCommunication
{
    class CXSocketImpl;
    class CXTcpClient
    {
    private:
        bool     m_bBlocking;
        bool     m_bCreated;
        bool     m_bClosing;
        bool     m_bConnected;
        CXSocketImpl *m_pSocket;
        int      m_iPacketNumber;

        byte *   m_pbySendBuffer;
        DWORD    m_dwSendBufferLen;

        byte *   m_pbyRealSendBuffer;
        DWORD    m_dwRealSendBufferLen;

        byte *   m_pbyRecvBuffer;
        DWORD    m_dwRecvBufferLen;

        byte *   m_pbyParserBuffer;
        DWORD    m_dwParserBufferLen;
        
        CXSocketAddress m_addressRemote;
        CXDataParserImpl *m_pDataParserHandle;

        bool     m_bCompressData;
        bool     m_bEncryptData;

        DWORD    m_dwLeftDataBeginPos;
        DWORD    m_dwLeftRecvDataLen;

    public:

        CXTcpClient();
        ~CXTcpClient();

        inline bool IsCreated() { return m_bCreated; }
        inline bool IsConnected() { return m_bConnected; }
        inline bool IsClosing() { return m_bClosing; }


        //create a socket,set the address and flags
        //Received value: ==RETURN_SUCCEED the socket was created successfully 
        //                ==INVALID_PARAMETER invalid inputed parameters
        //                ==-2 socket creation failed
        //                ==-3 socket creation failed, a error accured when allocated memory
        virtual int Create(bool bAccepted = false, cxsocket sock = -1);

        //connect to the peer by the ip address
        //Received value: ==RETURN_SUCCEED the socket was created successfully 
        //                ==INVALID_PARAMETER invalid inputed parameters
        //                ==-2 this socket object had not created
        //                ==-3 failed to connect the peer 
        virtual int Connect(const CXSocketAddress& address);

        virtual int Recv(byte *bpBuf, int iWantRecvLen, int &iReceivedBytes);
        virtual int Send(const byte *bpBuf, int iWantSendLen, int &iSentBytes);
        virtual int Close();
        //send a packet to the peer
        virtual int SendPacket(const byte* pbData, DWORD dwLen, DWORD dwMesCode);

        virtual int RecvPacket(byte *pData, int iDataLen, int &iReadBytes,DWORD &dwMesCode);

        virtual bool ReadLeftData(byte *pData, int iDataLen, int &iReadBytes);

        bool   SetSendBufferSize(DWORD dwBufSize);
        byte * GetSendBuffer(DWORD &dwBufSize);
        DWORD  GetSendBufferSize() { return m_dwSendBufferLen; }

        bool   SetRecvBufferSize(DWORD dwBufSize);
        byte * GetRecvBuffer(DWORD &dwBufSize);
        DWORD  GetRecvBufferSize() { return m_dwRecvBufferLen; }

        //get the parse buffer
        byte * GetRealRecvBuffer(DWORD &dwBufSize);

        void SetDataParserHandle(CXDataParserImpl * handle) { m_pDataParserHandle = handle; }
        CXDataParserImpl *GetDataParserHandle() { return m_pDataParserHandle; }

        void SetCompressData(bool bSet) { m_bCompressData = bSet; }
        void SetEncryptData(bool bSet) { m_bEncryptData = bSet; }
        bool IsCompressData() { return m_bCompressData; }
        bool IsEncryptData() { return m_bEncryptData; }
    };
}
#endif
