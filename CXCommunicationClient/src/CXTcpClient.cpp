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
#include "CXTcpClient.h"
#include "CXCommonPacketStructure.h"
#include "CXSessionPacketStructure.h"
#include "CXPacketCodeDefine.h"

#ifdef WIN32
#else
#include <string.h>
#endif

namespace CXCommunication
{
    CXTcpClient::CXTcpClient()
    {
        m_bCreated = false;
        m_bConnected = false;
        m_bClosing = false;
        m_pSocket = NULL;
        m_iPacketNumber = 0;
        try
        {
            m_pbyPacketData = new byte[CLIENT_BUF_SIZE];
            memset(m_pbyPacketData, 0, CLIENT_BUF_SIZE);
            m_iBufferLen = CLIENT_BUF_SIZE;
        }
        catch (const bad_alloc& e)
        {
            //DWORD dwEr = GetLastError();
            m_pbyPacketData = NULL;
            m_iBufferLen = 0;
        }

        
        
    }


    CXTcpClient::~CXTcpClient()
    {
    }

    //create a socket,set the address and flags
    //Received value: ==RETURN_SUCCEED the socket was created successfully
    //                ==INVALID_PARAMETER invalid inputed parameters
    //                ==-2 socket creation failed
    //                ==-3 socket creation failed, a error accured when allocated memory
    int CXTcpClient::Create(bool bAccepted, cxsocket sock)
    {
        if (bAccepted && sock <= 0)
        {
            return INVALID_PARAMETER;
        }

        if (m_pSocket==NULL)
        {
            m_pSocket = new CXSocketImpl;
            if (m_pSocket == NULL)
            {
                return -3;
            }

            if (bAccepted)
            {
                if (m_pSocket->CreateByAccepted(sock) == 0)
                {
                    m_bConnected = true;
                    m_bCreated = true;
                }
            }
            else
            {
                if (m_pSocket->Create() == 0)
                {
                    m_bCreated = true;
                }
            }

            if (!m_bCreated)
            {
                delete m_pSocket;
                m_pSocket = NULL;
                return -2;
            }
            else
            {
                m_pSocket->SetNoDelay(true);
                return RETURN_SUCCEED;
            }
        }
        return RETURN_SUCCEED;
    }

    //connect to the peer by the ip address
    //Received value: ==RETURN_SUCCEED the socket was created successfully
    //                ==INVALID_PARAMETER invalid inputed parameters
    //                ==-2 this socket object had not been created
    //                ==-3 failed to connect the peer
    int CXTcpClient::Connect(const CXSocketAddress& address)
    {
        if (IsConnected())
        {
            return RETURN_SUCCEED;
        }
        if (!IsCreated())
        {
            return -2;
        }
        int iRet = m_pSocket->Connect(address);
        if (iRet == 0)
        {
            m_pSocket->SetNoDelay(true);
            m_bConnected = true;
            m_addressRemote.SetAddress(address.GetAddress());
            return RETURN_SUCCEED;
        }
        else
        {
            Close();
            //DWORD dwEr = GetLastError();
            return -3;
        }
    }

    int CXTcpClient::Recv(byte *bpBuf, int iWantRecvLen, int &iReceivedBytes)
    {
        if (bpBuf==NULL || iWantRecvLen <= 0 )
        {
            return INVALID_PARAMETER;
        }
        if (!IsConnected() || !IsCreated()|| IsClosing())
        {
            return -2;
        }

        int iRecv = 0;
        unsigned int iOffset = 0;
        bool bRecvFailed = false;
        while (iOffset < iWantRecvLen)
        {
            if (IsClosing())
            {
                bRecvFailed = true;
                break;
            }

            iRecv = m_pSocket->RecvBytes(bpBuf+ iOffset, iWantRecvLen- iOffset);

            if (iRecv != SOCKET_ERROR)
            {
                if (iRecv == 0)
                {
                    Close();
                    break;
                }
                iOffset += iRecv;
            }
            else
            {
                bRecvFailed = true;
                Close();
                break;
            }

        }

        iReceivedBytes = iOffset;

        if (bRecvFailed)
        {
            return -3;
        }
        else if (iRecv == 0) // read to the end , this socket had been closed
        {
            return 0;
        }
        else
        {
            return iOffset;
        }
    }

    int CXTcpClient::Send(const byte *bpBuf, int iWantSendLen, int &iSentBytes)
    {
        if (bpBuf == NULL || iWantSendLen <= 0)
        {
            return INVALID_PARAMETER;
        }
        if (!IsConnected() || !IsCreated() || IsClosing())
        {
            return -2;
        }

        int iSent = 0;
        unsigned int iOffset = 0;
        bool bSendFailed = false;
        do
        {
            if (IsClosing())
            {
                bSendFailed = true;
                break;
            }

            iSent = m_pSocket->SendBytes(bpBuf + iOffset, iWantSendLen - iOffset);

            if (iSent != SOCKET_ERROR)
            {
                if (iSent == 0)
                    break;
                iOffset += iSent;
            }
            else
            {
                bSendFailed = true;
                break;
            }

        } while (iOffset < iWantSendLen);

        iSentBytes = iOffset;

        if (bSendFailed)
        {
            return -3;
        }
        else if (iSent == 0) // read to the end , this socket had been closed
        {
            return 0;
        }
        else
        {
            return iOffset;
        }
    }

    int CXTcpClient::SendPacket(PCXPacketData bpBuf, int iWantSendLen, int &iSentBytes)
    {
        if (bpBuf == NULL || iWantSendLen <= 0)
        {
            return INVALID_PARAMETER;
        }
        if (!IsConnected() || !IsCreated() || IsClosing())
        {
            return -2;
        }

        int iPacketLen = sizeof(CXPacketData)-1+ iWantSendLen;
        byte *pData = m_pbyPacketData;
        if (pData == NULL)
        {
            return -2;
        }

        memset(pData, 0, iPacketLen);
        memcpy(pData+ sizeof(CXPacketData) - 1, bpBuf->buf, iWantSendLen);

        BuildHeader(pData, iPacketLen, 10003);

        int iTransDataLen = 0;
        int iTransRet = Send(pData, iPacketLen, iTransDataLen);
        if (iTransRet != iPacketLen)
        {
            Close();
            return -6;
        }
        return iTransRet;

        /*
        int iSent = 0;
        unsigned int iOffset = 0;
        DWORD dwCheckSum = 0;
        for (int i=0;i<iWantSendLen;i++)
        {
            dwCheckSum += (unsigned int)bpBuf->buf[i];
        }

        //send the header
        CXPacketHeader tcpHeader;
        memset(&tcpHeader, 0, sizeof(CXPacketHeader));
        tcpHeader.wDataLen = iWantSendLen;
        tcpHeader.byReserve = 0x15;
        tcpHeader.byType = 2;
        tcpHeader.dwFlag = 0x25f7;
        tcpHeader.dwCheckSum = dwCheckSum;
        tcpHeader.dwPacketNum = ++m_iPacketNumber;
        byte * pData = (byte*)&tcpHeader;//reinterpret_cast<const byte * *>(&addr);

        int iNeedSend = sizeof(CXPacketHeader);
        bool bSendFailed = false;
        do
        {
            if (IsClosing())
            {
                bSendFailed = true;
                break;
            }

            iSent = m_pSocket->SendBytes(pData + iOffset, iNeedSend - iOffset);

            if (iSent != SOCKET_ERROR)
            {
                if (iSent == 0)
                {
                    return 0;
                }
                iOffset += iSent;
            }
            else
            {
                bSendFailed = true;
                return -3;
            }

        } while (iOffset < iNeedSend);


        iSent = 0;
        iOffset = 0;
        bSendFailed = false;
        pData = bpBuf->buf;
        do
        {
            if (iOffset > 0)
            {
                int l = 0;
            }
            if (IsClosing())
            {
                bSendFailed = true;
                break;
            }

            iSent = m_pSocket->SendBytes(pData + iOffset, iWantSendLen - iOffset);

            if (iSent != SOCKET_ERROR)
            {
                if (iSent == 0)
                    break;
                iOffset += iSent;
            }
            else
            {
                bSendFailed = true;
                break;
            }

        } while (iOffset < iWantSendLen);

        if (iOffset > iWantSendLen)
        {
            int l = 0;
        }

        iSentBytes = iOffset;

        if (bSendFailed)
        {
            return -3;
        }
        else if (iSent == 0) // read to the end , this socket had been closed
        {
            return 0;
        }
        else
        {
            return iOffset;
        }
        */
    }

    int CXTcpClient::Close()
    {
        if (m_pSocket == NULL)
        {
            return 0;
        }
        if (IsConnected())
        {
            m_pSocket->SetNoDelay(true);
            m_pSocket->SetLinger(true,1);
            m_bClosing = true;
            m_pSocket->Close();
            //m_pSocket->Shutdown();
        }
        else
        {
            if (IsCreated())
            {
                m_pSocket->Close();
            }
        }

        m_bClosing = false;
        m_bConnected = false;

        delete m_pSocket;
        m_pSocket = NULL;

        return 0;
    }

    void CXTcpClient::BuildHeader(byte *pData, int iDataLen, DWORD dwMesCode)
    {
        int iPacketBodyLen = iDataLen - sizeof(CXPacketHeader);
        byte *pBodyData = pData+ sizeof(CXPacketHeader);
        
        DWORD dwCheckSum = 0;
        for (int i = 0; i<iPacketBodyLen; i++)
        {
            dwCheckSum += (unsigned int)pBodyData[i];
        }

        //send the header
        CXPacketHeader &tcpHeader = *((PCXPacketHeader)pData);
        memset(&tcpHeader, 0, sizeof(CXPacketHeader));
        tcpHeader.wDataLen = iPacketBodyLen;
        tcpHeader.byReserve = 0x15;
        tcpHeader.byType = 2;
        tcpHeader.dwFlag = 0x25f7;
        tcpHeader.dwCheckSum = dwCheckSum;
        tcpHeader.dwPacketNum = ++m_iPacketNumber;

        PCXPacketData pPacket = (PCXPacketData)(pData);
        pPacket->dwMesCode = dwMesCode;
    }

    int CXTcpClient::RecvPacket(byte *pData, int iDataLen,int &iReadBytes)
    {
        if (pData == NULL || iDataLen <= sizeof(CXPacketHeader)+sizeof(CXPacketData))
        {
            return INVALID_PARAMETER;
        }
        if (!IsConnected() || !IsCreated() || IsClosing())
        {
            return -2;
        }
        memset(pData, 0, iDataLen);


        int iTransRet = 0;
        int iTransLen = 0;
        iReadBytes = 0;

        int iNeedSend = sizeof(CXPacketHeader);

        iTransRet = Recv(pData, iNeedSend, iTransLen);

        if (iTransRet <=0 )
        {
            return -3;
        }
        iReadBytes += iTransLen;

        //receive the header
        PCXPacketHeader pTcpHeader = (PCXPacketHeader)pData;

        iTransRet = 0;
        iTransLen = 0;
        iNeedSend = pTcpHeader->wDataLen;
        iTransRet = Recv(pData+ sizeof(CXPacketHeader), iNeedSend, iTransLen);

        if (iTransRet <= 0)
        {
            return -4;
        }
        iReadBytes += iTransLen;

        return RETURN_SUCCEED;
    }


    //login to the server by the ip address, user name ,password
    //Received value: ==RETURN_SUCCEED the socket was created successfully
    //                ==INVALID_PARAMETER invalid inputed parameters
    //                ==-2 this socket object had not created
    //                ==-3 failed to connect the peer
    int CXTcpClient::Login(const string &strUserName, const string &strPassword,
        int iUserType, int iSessionType,
        string strSessionGuid, string strVerifyCode)
    {
        int iRet = RETURN_SUCCEED;
        if (strUserName == "" || strUserName.length()>128 || strPassword == ""|| strPassword.length()>256)
        {
            return INVALID_PARAMETER;
        }

        byte *pData = m_pbyPacketData;
        if (pData == NULL)
        {
            return -2;
        }

        memset(pData, 0, m_iBufferLen);

        PCXSessionLogin pMes = (PCXSessionLogin)(pData + sizeof(CXPacketHeader) + sizeof(DWORD));
        pMes->dwSessionType = iSessionType;
        pMes->byUserType = iUserType;
        pMes->byConnectionType = 1;

        string strUserData = strUserName + "\r" + strPassword;
        pMes->dwUserDataLen = strUserData.length()+1;
        memcpy(pMes->szUserData, strUserData.c_str(), pMes->dwUserDataLen);

        int iPacketLen = sizeof(CXSessionLogin)-1 + sizeof(CXPacketData)-1;
        iPacketLen += pMes->dwUserDataLen;

        BuildHeader(pData, iPacketLen, CX_SESSION_LOGIN_CODE);

        int iTransDataLen = 0;
        int iTransRet = Send(pData, iPacketLen, iTransDataLen);
        if (iTransRet != iPacketLen)
        {
            Close();
            return -3;
        }
        memset(pData, 0, iPacketLen);
        iTransRet = RecvPacket(pData, m_iBufferLen, iTransDataLen);
        if (iTransRet != 0)
        {
            Close();
            return -4;
        }

        PCXPacketHeader pTcpHeader = (PCXPacketHeader)pData;
        PCXPacketData pPacket = (PCXPacketData)(pData);
        PCXCommonMessageReply pReply = (PCXCommonMessageReply)(pData + sizeof(CXPacketHeader) + sizeof(DWORD));
        if (pReply->dwReplyCode != 200)
        {
            Close();
            return -5;
        }

        return RETURN_SUCCEED;
    }

    bool CXTcpClient::SetBufferSize(int iBufSize)
    {
        if (iBufSize > 0 && iBufSize == m_iBufferLen)
        {
            return true;
        }
        

        byte *pData = NULL;
        try
        {
            pData = new byte[iBufSize];
            if (pData == NULL)
            {
                return false;
            }
            if (m_pbyPacketData != NULL)
            {
                delete[]m_pbyPacketData;
                m_pbyPacketData = NULL;
            }
            m_pbyPacketData = pData;
            m_iBufferLen = iBufSize;
            return true;
        }
        catch (const bad_alloc& e)
        {
            return false;
        }
    }

    byte *CXTcpClient::GetBuffer(int &iBufSize)
    {
        iBufSize = m_iBufferLen;
        return m_pbyPacketData;
    }
}
