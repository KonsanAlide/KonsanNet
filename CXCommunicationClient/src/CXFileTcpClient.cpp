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
#include "CXFileTcpClient.h"
#include "CXCommonPacketStructure.h"
#include "CXFilePacketStructure.h"
#include "CXPacketCodeDefine.h"

#ifdef WIN32
#else
#include <string.h>
#endif


using namespace CXCommunication;
CXFileTcpClient::CXFileTcpClient()
{
    m_strRemoteIP="";
    m_unRemotePort=0;
    m_strRemoteFilePath = "";
    m_strRemoteUser = "";
    m_strRemotePassword = "";
    m_bIsOpened = false;
}


CXFileTcpClient::~CXFileTcpClient()
{
    if (m_bIsOpened)
    {
        Close();
    }
}

int CXFileTcpClient::Open(string strRemoteFilePath, OPENTYPE type)
{
    int iRet = RETURN_SUCCEED;
    if (strRemoteFilePath=="")
    {
        return INVALID_PARAMETER;
    }

    if (m_strRemoteIP == "" || m_unRemotePort==0)
    {
        return -2;
    }
    if (m_strRemoteUser == "" || m_strRemotePassword == "")
    {
        return -2;
    }

    if (IsOpen())
    {
        if (strRemoteFilePath.compare(GetFilePathName())==0)
        {
            return RETURN_SUCCEED;
        }
        else
        {
            Close();
        }
    }
    m_bIsOpened = false;

    if(m_cxSession.OpenChannel(m_cmmClient, CXClientSocketChannel::MAJOR_MES_CONNECTION) != RETURN_SUCCEED)
    {
        return -3;
    }

    if (m_cxSession.OpenChannel(m_dataClient, CXClientSocketChannel::DATA_CONNECTION) != RETURN_SUCCEED)
    {
        m_cmmClient.Close();
        return -4;
    }

    byte *pData = m_byPacketData;
    memset(pData, 0, CLIENT_BUF_SIZE);

    PCXFileOpenFile pMes = (PCXFileOpenFile)(pData);
    pMes->byOpenType = type;
    pMes->dwFilePathLen = strRemoteFilePath.length()+1;
    memcpy(pMes->szFilePath, strRemoteFilePath.c_str(), pMes->dwFilePathLen);

    int iMesLen = sizeof(CXFileOpenFile) - 1 + pMes->dwFilePathLen;

    int iTransDataLen = 0;
    int iTransRet = m_cmmClient.SendPacket(pData, iMesLen, CX_FILE_OPEN_CODE);
    if (iTransRet != 0)
    {
        m_cmmClient.Close();
        m_dataClient.Close();
        return -5;
    }

    memset(pData, 0, CLIENT_BUF_SIZE);
    DWORD dwMesCode = 0;
    iTransRet = m_cmmClient.RecvPacket(pData, CLIENT_BUF_SIZE, iTransDataLen, dwMesCode);
    if (iTransRet != 0)
    {
        m_cmmClient.Close();
        m_dataClient.Close();
        return -6;
    }
    if (dwMesCode != CX_FILE_OPEN_REPLY_CODE)
    {
        m_cmmClient.Close();
        m_dataClient.Close();
        return -7;
    }

    PCXCommonMessageReply pReply = (PCXCommonMessageReply)(pData);
    if (pReply->dwReplyCode != 200)
    {
        m_cmmClient.Close();
        m_dataClient.Close();
        return -8;
    }
    
    m_bIsOpened = true;
    return RETURN_SUCCEED;
}

//return value: ==ERROR_SUCCESS, if the data length received data is less than iWantReadLen, show that had read to the file end
//              ==-9  read to file end, have read some data
//              ==-10 some error occurs in reading file, only read part of data
int  CXFileTcpClient::Read(byte* pBuf, int iWantReadLen, int *piReadLen)
{
    int iRet = RETURN_SUCCEED;
    if (!IsOpen())
    {
        return -2;
    }
  
    byte *pData = m_byPacketData;
    memset(pData, 0, CLIENT_BUF_SIZE);

    PCXFileRead pMes = (PCXFileRead)(pData);
    pMes->dwSeekType = CXFileTcpClient::current;
    pMes->dwReadLen = iWantReadLen;
    pMes->uiBeginPos = 0;

    int iMesLen = sizeof(CXFileRead);

    int iTransDataLen = 0;
    int iTransRet = m_dataClient.SendPacket(pData, iMesLen, CX_FILE_READ_CODE);
    if (iTransRet != 0)
    {
        Close();
        return -3;
    }

    memset(pData, 0, CLIENT_BUF_SIZE);

    DWORD dwMesCode = 0;
    iTransRet = m_dataClient.RecvPacket(pData, sizeof(CXFileReadReply)-1, iTransDataLen, dwMesCode);
    if (iTransRet != 0 && iTransRet != -8)
    {
        Close();
        return -4;
    }
    if (dwMesCode != CX_FILE_READ_REPLY_CODE)
    {
        Close();
        return -5;
    }

    PCXFileReadReply pReply = (PCXFileReadReply)(pData);
    if (pReply->dwReplyCode != 200 && pReply->dwReplyCode != 205 && pReply->dwReplyCode != 204)
    {
        Close();
        return -6;
    }

    if (pReply->dwDataLen > iWantReadLen)
    {
        Close();
        return -7;
    }

    if (pReply->dwDataLen > 0)
    {
        if (!m_dataClient.ReadLeftData(pBuf, iWantReadLen, *piReadLen))
        {
            Close();
            return -8;
        }
    }

    if (pReply->dwReplyCode == 203 ) //read to file end
    {
        return -9;
    }
    else if(pReply->dwReplyCode == 204) //some error occurs in reading file, only read part of data
    {
        return -10;
    }

    return iRet;
}

int  CXFileTcpClient::Write(const byte* pBuf, int iBufLen, int *piWrittenLen)
{
    int iRet = RETURN_SUCCEED;
    if (!IsOpen())
    {
        return -2;
    }

    DWORD dwBufferSize = 0;
    byte *pData = m_dataClient.GetSendBuffer(dwBufferSize);
    PCXFileWrite pMes = (PCXFileWrite)(pData);

    // not use the socket buffer to save data
    if (pBuf < pData || pBuf > (pData + dwBufferSize))
    {
        if (iBufLen > dwBufferSize)
        {
            if (!m_dataClient.SetSendBufferSize(iBufLen + sizeof(CXFileWrite) - 1))
            {
                return -7;
            }
            pData = m_dataClient.GetSendBuffer(dwBufferSize);
        }
        pMes = (PCXFileWrite)(pData);
        memcpy(pMes->szData, pBuf, iBufLen);
    }

    pMes->dwSeekType = CXFileTcpClient::current;
    pMes->dwDataLen = iBufLen;
    pMes->uiBeginPos = 0;
    
    int iMesLen = sizeof(CXFileWrite) - 1 + iBufLen;

    int iTransDataLen = 0;
    int iTransRet = m_dataClient.SendPacket(pData, iMesLen, CX_FILE_WRITE_CODE);
    if (iTransRet != 0)
    {
        Close();
        return -3;
    }

    DWORD dwMesCode = 0;
    iTransRet = m_dataClient.RecvPacket(pData, dwBufferSize, iTransDataLen, dwMesCode);
    if (iTransRet != 0)
    {
        Close();
        return -4;
    }
    if (dwMesCode != CX_FILE_WRITE_REPLY_CODE)
    {
        Close();
        return -5;
    }
    PCXCommonMessageReply pReply = (PCXCommonMessageReply)(pData);
    if (pReply->dwReplyCode != 200)
    {
        Close();
        return -6;
    }

    return RETURN_SUCCEED;
}

int  CXFileTcpClient::Seek(uint64 pos, SEEKTYPE type)
{
    if (!IsOpen())
    {
        return -2;
    }

    byte *pData = m_byPacketData;
    memset(pData, 0, CLIENT_BUF_SIZE);
	PCXFileSeek pMes = (PCXFileSeek)(pData);
    pMes->dwSeekType = type;
    pMes->uiSeekPos = pos;
    int iMesLen = sizeof(CXFileSeek);

    int iTransDataLen = 0;
    int iTransRet = m_cmmClient.SendPacket(pData, iMesLen, CX_FILE_SEEK_CODE);
    if (iTransRet != 0)
    {
        Close();
        return -3;
    }

    memset(pData, 0, CLIENT_BUF_SIZE);

    DWORD dwMesCode = 0;
    iTransRet = m_cmmClient.RecvPacket(pData, CLIENT_BUF_SIZE, iTransDataLen, dwMesCode);
    if (iTransRet != 0)
    {
        Close();
        return -4;
    }
    if (dwMesCode != CX_FILE_SEEK_REPLY_CODE)
    {
        Close();
        return -5;
    }
    PCXCommonMessageReply pReply = (PCXCommonMessageReply)(pData);
    if (pReply->dwReplyCode != 200)
    {
        Close();
        return -6;
    }

    return RETURN_SUCCEED;
}

int CXFileTcpClient::Close()
{
    if (!IsOpen())
    {
        return RETURN_SUCCEED;
    }

    byte *pData = m_byPacketData;
    memset(pData, 0, CLIENT_BUF_SIZE);

    int iMesLen = sizeof(CXFileClose);

    int iTransDataLen = 0;
    int iTransRet = m_cmmClient.SendPacket(pData, iMesLen, CX_FILE_CLOSE_CODE);
    if (iTransRet != 0)
    {
        m_cmmClient.Close();
        m_dataClient.Close();
        m_bIsOpened = false;
        return -3;
    }

    memset(pData, 0, CLIENT_BUF_SIZE);

    DWORD dwMesCode = 0;
    iTransRet = m_cmmClient.RecvPacket(pData, CLIENT_BUF_SIZE, iTransDataLen, dwMesCode);
    if (iTransRet != 0)
    {
        m_cmmClient.Close();
        m_dataClient.Close();
        m_bIsOpened = false;
        return -4;
    }
    if (dwMesCode != CX_FILE_CLOSE_REPLY_CODE)
    {
        m_cmmClient.Close();
        m_dataClient.Close();
        m_bIsOpened = false;
        return -5;
    }
    PCXCommonMessageReply pReply = (PCXCommonMessageReply)(pData);
    if (pReply->dwReplyCode != 200)
    {
        m_cmmClient.Close();
        m_dataClient.Close();
        m_bIsOpened = false;
        return -6;
    }

    m_bIsOpened = false;
    return RETURN_SUCCEED;
}

void CXFileTcpClient::SetRemoteServerInfo(string strRemoteIP, unsigned short unRemotePort,
    string strRemoteUser, string strRemotePassword)
{
    m_strRemoteIP = strRemoteIP;
    m_unRemotePort = unRemotePort;
    m_strRemoteUser = strRemoteUser;
    m_strRemotePassword = strRemotePassword;
    m_cxSession.SetUserInfo(m_strRemoteUser, m_strRemotePassword);
    CXSocketAddress addr(m_strRemoteIP.c_str(), m_unRemotePort);
    m_cxSession.SetRemoteAddress(addr);
}

int CXFileTcpClient::GetFileLength(uint64 & uiFileLength)
{
    if (!IsOpen())
    {
        return RETURN_SUCCEED;
    }

    byte *pData = m_byPacketData;
    memset(pData, 0, CLIENT_BUF_SIZE);

    int iMesLen = sizeof(CXFileSeek);

    int iTransDataLen = 0;
    int iTransRet = m_cmmClient.SendPacket(pData, iMesLen, CX_FILE_GET_LENGTH_CODE);
    if (iTransRet != 0)
    {
        Close();
        return -3;
    }

    memset(pData, 0, CLIENT_BUF_SIZE);

    DWORD dwMesCode = 0;
    iTransRet = m_cmmClient.RecvPacket(pData, CLIENT_BUF_SIZE, iTransDataLen, dwMesCode);
    if (iTransRet != 0)
    {
        Close();
        return -4;
    }
    if (dwMesCode != CX_FILE_GET_LENGTH_REPLY_CODE)
    {
        Close();
        return -5;
    }
    PCXCommonMessageReply pReply = (PCXCommonMessageReply)(pData);
    if (pReply->dwReplyCode != 200)
    {
        Close();
        return -6;
    }

    uiFileLength = pReply->dwValue1;
    uiFileLength <<= 32;
    uiFileLength += pReply->dwValue2;

    return RETURN_SUCCEED;
}

byte * CXFileTcpClient::GetSendBuffer(DWORD &dwBufSize)
{
    DWORD dwBufferSize = 0;
    byte *pData = m_dataClient.GetSendBuffer(dwBufferSize);
    memset(pData, 0, dwBufferSize);
    dwBufSize = dwBufferSize - sizeof(CXFileWrite);
    return (pData+ sizeof(CXFileWrite));
}

byte * CXFileTcpClient::GetRecvBuffer(DWORD &dwBufSize)
{
    DWORD dwBufferSize = 0;
    byte *pData = m_dataClient.GetRealRecvBuffer(dwBufferSize);
    memset(pData, 0, dwBufferSize);
    dwBufSize = dwBufferSize - sizeof(CXFileReadReply);
    return (pData + sizeof(CXFileReadReply));
}

