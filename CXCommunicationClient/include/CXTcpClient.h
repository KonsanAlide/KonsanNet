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
        byte *   m_pbyPacketData;
        int      m_iBufferLen;
        CXSocketAddress m_addressRemote;

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
        virtual int SendPacket(PCXPacketData bpBuf, int iWantSendLen, int &iSentBytes);
        virtual int Close();

        //login to the server by the user name ,password
        //Received value: ==RETURN_SUCCEED the socket was created successfully 
        //                ==INVALID_PARAMETER invalid inputed parameters
        //                ==-2 this socket object had not created
        //                ==-3 failed to connect the peer 
        virtual int Login(const string &strUserName, const string &strPassword, 
            int iUserType=1, int iSessionType=1,
            string strSessionGuid="",string strVerifyCode="");

        virtual void BuildHeader(byte *pData, int iDataLen, DWORD dwMesCode);

        virtual int RecvPacket(byte *pData, int iDataLen, int &iReadBytes);

        bool SetBufferSize(int iBufSize);
        byte *GetBuffer(int &iBufSize);


    };
}
#endif
