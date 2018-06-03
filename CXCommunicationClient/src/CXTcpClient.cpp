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


        m_pbySendBuffer = NULL;
        m_dwSendBufferLen = 0;

        m_pbyRealSendBuffer = NULL;
        m_dwRealSendBufferLen = 0;

        m_pbyRecvBuffer = NULL;
        m_dwRecvBufferLen = 0;

        m_pbyParserBuffer = NULL;
        m_dwParserBufferLen = 0;

        m_pDataParserHandle = NULL;

        SetSendBufferSize(CLIENT_BUF_SIZE);
        SetRecvBufferSize(CLIENT_BUF_SIZE);

        m_bCompressData = false;
        m_bEncryptData = false;

        m_dwLeftDataBeginPos = 0;
        m_dwLeftRecvDataLen = 0; 
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

    int CXTcpClient::Close()
    {
        if (m_pSocket == NULL)
        {
            return 0;
        }
        if (IsConnected())
        {
            m_pSocket->SetNoDelay(true);
            m_pSocket->SetLinger(true,0);
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

    //send a packet to the peer
    int  CXTcpClient::SendPacket(const byte* pbData, DWORD dwLen, DWORD dwMesCode)
    {
        if (pbData==NULL || dwLen > 1024 * 1024)
        {
            return INVALID_PARAMETER;
        }

        byte *pbyBodyData = m_pbyRealSendBuffer+sizeof(CXPacketHeader);
        DWORD dwPacketLen = dwLen + sizeof(CXPacketData) - 1;
        DWORD dwOrignDataLen = dwPacketLen;

        if (!SetSendBufferSize(dwPacketLen))
        {
            return -2;
        }

        if (m_pDataParserHandle != NULL)
        {
            DWORD dwSrcLen = dwLen+ sizeof(CXPacketBodyData) - 1;
            DWORD dwDestLen = m_dwRealSendBufferLen- sizeof(CXPacketHeader);

            PCXPacketBodyData pBodyData = (PCXPacketBodyData)m_pbySendBuffer;
            pBodyData->dwMesCode = dwMesCode;
            pBodyData->dwPacketNum = ++m_iPacketNumber;

            // not use the socket buffer to save data
            if (pbData < m_pbySendBuffer || pbData >(m_pbySendBuffer + m_dwSendBufferLen))
            {
                if (dwLen>0)
                {
                    memcpy(pBodyData->buf, pbData, dwLen);
                }
            }

            if (!m_pDataParserHandle->PrepareData(m_bEncryptData, m_bCompressData,
                (const byte*)pBodyData, dwSrcLen, 
                pbyBodyData, dwDestLen, dwDestLen))
            {
                return -3;
            }
            dwPacketLen = dwDestLen + sizeof(CXPacketHeader);
        }
        else
        {
            PCXPacketBodyData pBodyData = (PCXPacketBodyData)(m_pbyRealSendBuffer + sizeof(CXPacketHeader));
            pBodyData->dwMesCode = dwMesCode;
            pBodyData->dwPacketNum = ++m_iPacketNumber;

            // not use the socket buffer to save data
            if (pbData < m_pbyRealSendBuffer || pbData >(m_pbyRealSendBuffer + m_dwRealSendBufferLen))
            {
                if (dwLen>0)
                {
                    memcpy(pBodyData->buf, pbData, dwLen);
                }
            }
        }

        int iPacketBodyLen = dwPacketLen - sizeof(CXPacketHeader);

        DWORD dwCheckSum = 0;
        for (int i = 0; i<iPacketBodyLen; i++)
        {
            dwCheckSum += (unsigned int)pbyBodyData[i];
        }

        //build the header
        PCXPacketHeader pTcpHeader = (PCXPacketHeader)m_pbyRealSendBuffer;
        pTcpHeader->dwDataLen = iPacketBodyLen;
        pTcpHeader->byReserve = 0x15;
        pTcpHeader->byType = 2;
        pTcpHeader->wFlag = 0x25f7;
        pTcpHeader->dwCheckSum = dwCheckSum;
        pTcpHeader->dwOrignDataLen = dwOrignDataLen;
       

        int iTransDataLen = 0;
        int iTransRet = Send(m_pbyRealSendBuffer, dwPacketLen, iTransDataLen);
        if (iTransRet != dwPacketLen)
        {
            Close();
            return -6;
        }
        return 0;
    }

    int CXTcpClient::RecvPacket(byte *pData, int iDataLen,int &iReadBytes, DWORD &dwMesCode)
    {
        if (pData == NULL || iDataLen < sizeof(CXPacketBodyData)-1)
        {
            return INVALID_PARAMETER;
        }
        if (!IsConnected() || !IsCreated() || IsClosing())
        {
            return -2;
        }
        memset(pData, 0, iDataLen);
        m_dwLeftDataBeginPos = 0;
        m_dwLeftRecvDataLen = 0;


        int iTransRet = 0;
        int iTransLen = 0;
        iReadBytes = 0;
        CXPacketHeader header;
        memset(&header, 0, sizeof(CXPacketHeader));

        int iNeedSend = sizeof(CXPacketHeader);

        iTransRet = Recv((byte*)&header, iNeedSend, iTransLen);

        if (iTransRet <=0 )
        {
            return -3;
        }

        if (header.dwDataLen>1024*1024)
        {
            Close();
            return -4;
        }

        if (header.wFlag != 0x25f7)
        {
            Close();
            return -5;
        }

        if (!SetRecvBufferSize(header.dwDataLen))
        {
            return -6;
        }

        iTransRet = 0;
        iTransLen = 0;
        iNeedSend = header.dwDataLen;
        iTransRet = Recv(m_pbyRecvBuffer, iNeedSend, iTransLen);

        if (iTransRet != iNeedSend)
        {
            return -7;
        }

        if (m_pDataParserHandle != NULL)
        {
            DWORD dwDestLen = m_dwParserBufferLen;

            if (!m_pDataParserHandle->ParseData(m_bEncryptData, m_bCompressData,
                m_pbyRecvBuffer, iTransLen,
                m_pbyParserBuffer, dwDestLen, dwDestLen))
            {
                return -3;
            }
            PCXPacketBodyData pBodyData = (PCXPacketBodyData)m_pbyParserBuffer;
            dwMesCode = pBodyData->dwMesCode;
            DWORD dwRealDataLen = dwDestLen - (sizeof(CXPacketBodyData) - 1);
            if (iDataLen < dwRealDataLen) //left some data
            {
                memcpy(pData, pBodyData->buf, iDataLen);
                m_dwLeftRecvDataLen = dwRealDataLen - iDataLen;
                m_dwLeftDataBeginPos = iDataLen;
                return -8;
            }
            
            if(dwRealDataLen>0)
                memcpy(pData, pBodyData->buf, dwRealDataLen);
            iReadBytes = dwRealDataLen;
        }
        else
        {
            PCXPacketBodyData pBodyData = (PCXPacketBodyData)m_pbyRecvBuffer;
            dwMesCode = pBodyData->dwMesCode;
            DWORD dwRealDataLen = iTransLen - (sizeof(CXPacketBodyData) - 1);
            if (iDataLen < dwRealDataLen)
            {
                memcpy(pData, pBodyData->buf, iDataLen);
                m_dwLeftRecvDataLen = dwRealDataLen - iDataLen;
                m_dwLeftDataBeginPos = iDataLen;
                return -8;
            }

            if (dwRealDataLen>0)
                memcpy(pData, pBodyData->buf, dwRealDataLen);
            iReadBytes = dwRealDataLen;
            
        }

        return RETURN_SUCCEED;
    }

    bool CXTcpClient::ReadLeftData(byte *pData, int iDataLen, int &iReadBytes)
    {
        if (m_dwLeftRecvDataLen == 0)
            return true;

        iReadBytes = 0;
        if (iDataLen < m_dwLeftRecvDataLen) //left some data
        {
            return false;
        }
        else
        {
            PCXPacketBodyData pBodyData = (PCXPacketBodyData)m_pbyRecvBuffer;
            if (m_pDataParserHandle != NULL)
                pBodyData = (PCXPacketBodyData)m_pbyParserBuffer;
            memcpy(pData, pBodyData->buf + m_dwLeftDataBeginPos, m_dwLeftRecvDataLen);
            iReadBytes = m_dwLeftRecvDataLen;
            return true;
        }
    }


    bool CXTcpClient::SetSendBufferSize(DWORD dwBufSize)
    {
        dwBufSize += sizeof(CXPacketHeader);
        if (dwBufSize > 0 && dwBufSize <= m_dwSendBufferLen)
        {
            return true;
        }

        byte *pData = NULL;
        byte *pDataReal = NULL;
        try
        {
            pData = new byte[dwBufSize];
            if (pData == NULL)
            {
                return false;
            }
            pDataReal = new byte[dwBufSize*2];
            if (pDataReal == NULL)
            {
                delete[]pData;
                return false;
            }
            
            
            if (m_pbySendBuffer != NULL)
            {
                delete[]m_pbySendBuffer;
                m_pbySendBuffer = NULL;
            }
            m_pbySendBuffer = pData;
            m_dwSendBufferLen = dwBufSize;

            if (m_pbyRealSendBuffer != NULL)
            {
                delete[]m_pbyRealSendBuffer;
                m_pbyRealSendBuffer = NULL;
            }
            m_pbyRealSendBuffer = pDataReal;
            m_dwRealSendBufferLen = dwBufSize*2;

            return true;
        }
        catch (const bad_alloc& e)
        {
            return false;
        }
    }

    byte *CXTcpClient::GetSendBuffer(DWORD &dwBufSize)
    {
        dwBufSize = m_dwSendBufferLen- sizeof(CXPacketHeader);
        return (m_pbySendBuffer+sizeof(CXPacketHeader));
    }

    bool CXTcpClient::SetRecvBufferSize(DWORD dwBufSize)
    {
        dwBufSize+= sizeof(CXPacketHeader);
        if (dwBufSize > 0 && dwBufSize <= m_dwRecvBufferLen)
        {
            return true;
        }

        byte *pData = NULL;
        byte *pDataParser = NULL;
        try
        {
            pData = new byte[dwBufSize];
            if (pData == NULL)
            {
                return false;
            }

            pDataParser = new byte[dwBufSize*10];
            if (pDataParser == NULL)
            {
                delete[]pData;
                return false;
            }

            if (m_pbyRecvBuffer != NULL)
            {
                delete[]m_pbyRecvBuffer;
                m_pbyRecvBuffer = NULL;
            }
            m_pbyRecvBuffer = pData;
            m_dwRecvBufferLen = dwBufSize;
 
            if (m_pbyParserBuffer != NULL)
            {
                delete[]m_pbyParserBuffer;
                m_pbyParserBuffer = NULL;
            }

            m_pbyParserBuffer = pDataParser;
            m_dwParserBufferLen = dwBufSize * 10;

            return true;
        }
        catch (const bad_alloc& e)
        {
            return false;
        }
    }

    byte *CXTcpClient::GetRecvBuffer(DWORD &dwBufSize)
    {
        dwBufSize = m_dwRecvBufferLen - sizeof(CXPacketHeader);
        return (m_pbyRecvBuffer + sizeof(CXPacketHeader));
    }

    //get the parse buffer
    byte * CXTcpClient::GetRealRecvBuffer(DWORD &dwBufSize)
    {
        dwBufSize = m_dwParserBufferLen - sizeof(CXPacketHeader);
        return (m_pbyParserBuffer + sizeof(CXPacketHeader));
    }
}
