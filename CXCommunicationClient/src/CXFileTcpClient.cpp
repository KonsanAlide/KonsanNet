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

    if (m_cxTcpClient.Create(false)!= RETURN_SUCCEED)
    {
        return -3;
    }

    CXSocketAddress addr(m_strRemoteIP.c_str(), m_unRemotePort);
    if (m_cxTcpClient.Connect(addr)!= RETURN_SUCCEED)
    {
        return -4;
    }

    if (m_cxTcpClient.Login(m_strRemoteUser, m_strRemotePassword) != RETURN_SUCCEED)
    {
        return -5;
    }


    byte *pData = m_byPacketData;
    if (pData == NULL)
    {
        return -2;
    }

    memset(pData, 0, CLIENT_BUF_SIZE);

    PCXFileOpenFile pMes = (PCXFileOpenFile)(pData + sizeof(CXPacketHeader) + sizeof(DWORD));
    pMes->byOpenType = type;
    pMes->dwFilePathLen = strRemoteFilePath.length()+1;
    memcpy(pMes->szFilePath, strRemoteFilePath.c_str(), pMes->dwFilePathLen);

    int iPacketLen = sizeof(CXFileOpenFile) - 1 + sizeof(CXPacketData) - 1;
    iPacketLen += pMes->dwFilePathLen;

    m_cxTcpClient.BuildHeader(pData, iPacketLen, CX_FILE_OPEN_CODE);

    int iTransDataLen = 0;
    int iTransRet = m_cxTcpClient.Send(pData, iPacketLen, iTransDataLen);
    if (iTransRet != iPacketLen)
    {
        m_cxTcpClient.Close();
        return -6;
    }
    memset(pData, 0, iPacketLen);
    iTransRet = m_cxTcpClient.RecvPacket(pData, CLIENT_BUF_SIZE, iTransDataLen);
    if (iTransRet != 0)
    {
        m_cxTcpClient.Close();
        return -7;
    }

    PCXPacketHeader pTcpHeader = (PCXPacketHeader)pData;
    PCXPacketData pPacket = (PCXPacketData)(pData);
    PCXCommonMessageReply pReply = (PCXCommonMessageReply)(pData + sizeof(CXPacketHeader) + sizeof(DWORD));
    if (pReply->dwReplyCode != 200)
    {
        m_cxTcpClient.Close();
        return -8;
    }
    /*
    if (m_cxDataClient.Create(false) != ERROR_SUCCESS)
    {
        return -9;
    }

    if (m_cxDataClient.Connect(addr) != ERROR_SUCCESS)
    {
        return -10;
    }
    */
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
    int iPacketLen = sizeof(CXFileRead) + sizeof(DWORD) + sizeof(CXPacketHeader);
    byte *pData = m_byPacketData;
    if (pData == NULL)
    {
        return -3;
    }

    memset(pData, 0, iPacketLen);

    m_cxTcpClient.BuildHeader(pData, iPacketLen, CX_FILE_READ_CODE);

    PCXFileRead pMes = (PCXFileRead)(pData + sizeof(CXPacketHeader) + sizeof(DWORD));
    pMes->dwSeekType = CXFileTcpClient::current;
    pMes->dwReadLen = iWantReadLen;
    pMes->uiBeginPos = 0;
    int iTransDataLen = 0;
    int iTransRet = m_cxTcpClient.Send(pData, iPacketLen, iTransDataLen);
    if (iTransRet != iPacketLen)
    {
        m_cxTcpClient.Close();
        m_bIsOpened = false;
        return -4;
    }
    memset(pData, 0, iPacketLen);

    int iNeedReadLen = sizeof(CXPacketHeader) + sizeof(DWORD) + sizeof(CXFileReadReply)-1;
    iTransRet = m_cxTcpClient.Recv(pData, iNeedReadLen, iTransDataLen);
    if (iTransRet != iNeedReadLen)
    {
        m_cxTcpClient.Close();
        m_bIsOpened = false;
        return -5;
    }

    PCXPacketHeader pTcpHeader = (PCXPacketHeader)pData;
    PCXPacketData pPacket = (PCXPacketData)(pData);
    PCXFileReadReply pReply = (PCXFileReadReply)(pData + sizeof(CXPacketHeader) + sizeof(DWORD));
    if (pReply->dwReplyCode != 200 && pReply->dwReplyCode != 203 && pReply->dwReplyCode != 204)
    {
        m_cxTcpClient.Close();
        m_bIsOpened = false;
        return -6;
    }

    if (iNeedReadLen == 0)//read to file end
    {
        *piReadLen = 0;
        return RETURN_SUCCEED;
    }
    iNeedReadLen = pReply->dwDataLen;
    if (iNeedReadLen > iWantReadLen)
    {
        m_cxTcpClient.Close();
        m_bIsOpened = false;
        return -7;
    }
    iTransDataLen = 0;
    iTransRet = m_cxTcpClient.Recv(pBuf, iNeedReadLen, iTransDataLen);
    if (iTransRet != iNeedReadLen)
    {
        m_cxTcpClient.Close();
        m_bIsOpened = false;
        return -8;
    }

    if (pReply->dwReplyCode == 203 ) //read to file end
    {
        return -9;
    }
    else if(pReply->dwReplyCode == 204) //some error occurs in reading file, only read part of data
    {
        return -10;
    }


    *piReadLen = iTransDataLen;
    return iRet;
}

int  CXFileTcpClient::Write(const byte* pBuf, int iBufLen, int *piWrittenLen)
{
    int iRet = RETURN_SUCCEED;
    if (!IsOpen())
    {
        return -2;
    }

    int iPacketLen = sizeof(CXFileWrite) + sizeof(DWORD) + sizeof(CXPacketHeader)-1+ iBufLen;
    byte *pData = m_byPacketData;
    if (pData == NULL)
    {
        return -3;
    }

    memset(pData, 0, iPacketLen);

    m_cxTcpClient.BuildHeader(pData, iPacketLen, CX_FILE_WRITE_CODE);

    int iNeedTransLen = iPacketLen - iBufLen;

    PCXFileWrite pMes = (PCXFileWrite)(pData + sizeof(CXPacketHeader) + sizeof(DWORD));
    pMes->dwSeekType = CXFileTcpClient::current;
    pMes->dwDataLen = iBufLen;
    pMes->uiBeginPos = 0;
    int iTransDataLen = 0;
    int iTransRet = m_cxTcpClient.Send(pData, iNeedTransLen, iTransDataLen);
    if (iTransRet != iNeedTransLen)
    {
        m_cxTcpClient.Close();
        m_bIsOpened = false;
        return -6;
    }

    iTransDataLen = 0;
    iTransRet = m_cxTcpClient.Send(pBuf, iBufLen, iTransDataLen);
    if (iTransRet != iBufLen)
    {
        m_cxTcpClient.Close();
        m_bIsOpened = false;
        return -7;
    }

    memset(pData, 0, iPacketLen);
    iTransRet = m_cxTcpClient.RecvPacket(pData, CLIENT_BUF_SIZE, iTransDataLen);
    if (iTransRet != 0)
    {
        m_cxTcpClient.Close();
        m_bIsOpened = false;
        return -7;
    }

    PCXPacketHeader pTcpHeader = (PCXPacketHeader)pData;
    PCXPacketData pPacket = (PCXPacketData)(pData);
    PCXCommonMessageReply pReply = (PCXCommonMessageReply)(pData + sizeof(CXPacketHeader) + sizeof(DWORD));
    if (pReply->dwReplyCode != 200)
    {
        m_cxTcpClient.Close();
        m_bIsOpened = false;
        return -8;
    }
    return RETURN_SUCCEED;
}

int  CXFileTcpClient::Seek(uint64 pos, SEEKTYPE type)
{
    if (!IsOpen())
    {
        return -2;
    }
    int iPacketLen = sizeof(CXFileSeek) + sizeof(DWORD) + sizeof(CXPacketHeader);
    byte *pData = m_byPacketData;
    if (pData == NULL)
    {
        return -3;
    }

    memset(pData, 0, iPacketLen);

    m_cxTcpClient.BuildHeader(pData, iPacketLen, CX_FILE_SEEK_CODE);

    PCXFileSeek pMes = (PCXFileSeek)(pData + sizeof(CXPacketHeader) + sizeof(DWORD));
    pMes->dwSeekType = type;
    pMes->uiSeekPos = pos;

    int iTransDataLen = 0;
    int iTransRet = m_cxTcpClient.Send(pData, iPacketLen, iTransDataLen);
    if (iTransRet != iPacketLen)
    {
        m_cxTcpClient.Close();
        m_bIsOpened = false;
        return -4;
    }
    memset(pData, 0, iPacketLen);
    iTransRet = m_cxTcpClient.RecvPacket(pData, CLIENT_BUF_SIZE, iTransDataLen);
    if (iTransRet != 0)
    {
        m_cxTcpClient.Close();
        m_bIsOpened = false;
        return -5;
    }

    PCXPacketHeader pTcpHeader = (PCXPacketHeader)pData;
    PCXPacketData pPacket = (PCXPacketData)(pData );
    PCXCommonMessageReply pReply = (PCXCommonMessageReply)(pData + sizeof(CXPacketHeader) + sizeof(DWORD));
    if (pReply->dwReplyCode != 200)
    {
        m_cxTcpClient.Close();
        m_bIsOpened = false;
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
    int iPacketLen = sizeof(CXFileClose) + sizeof(DWORD) + sizeof(CXPacketHeader);
    byte *pData = m_byPacketData;
    if (pData == NULL)
    {
        return -2;
    }

    memset(pData, 0, iPacketLen);

    m_cxTcpClient.BuildHeader(pData, iPacketLen, CX_FILE_CLOSE_CODE);

    PCXFileClose pMes = (PCXFileClose)(pData + sizeof(CXPacketHeader) + sizeof(DWORD));
    pMes->dwType = 1;
    pMes->dwDataLen = 0;

    int iTransDataLen = 0;
    int iTransRet = m_cxTcpClient.Send(pData, iPacketLen, iTransDataLen);
    if (iTransRet != iPacketLen)
    {
        m_cxTcpClient.Close();
        m_bIsOpened = false;
        return -6;
    }
    memset(pData, 0, iPacketLen);
    iTransRet = m_cxTcpClient.RecvPacket(pData, CLIENT_BUF_SIZE, iTransDataLen);
    if (iTransRet != 0)
    {
        m_cxTcpClient.Close();
        m_bIsOpened = false;
        return -7;
    }

    PCXPacketHeader pTcpHeader = (PCXPacketHeader)pData;
    PCXPacketData pPacket = (PCXPacketData)(pData );
    PCXCommonMessageReply pReply = (PCXCommonMessageReply)(pData + sizeof(CXPacketHeader) + sizeof(DWORD));
    if (pReply->dwReplyCode != 200)
    {
        m_cxTcpClient.Close();
        m_bIsOpened = false;
        return -8;
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
}

int CXFileTcpClient::GetFileLength(uint64 & uiFileLength)
{
    if (!IsOpen())
    {
        return RETURN_SUCCEED;
    }
    int iPacketLen = sizeof(CXPacketData)+ sizeof(CXPacketHeader);
    byte *pData = m_byPacketData;
    if (pData == NULL)
    {
        return -2;
    }

    memset(pData, 0, iPacketLen);

    m_cxTcpClient.BuildHeader(pData, iPacketLen, CX_FILE_GET_LENGTH_CODE);

    int iTransDataLen = 0;
    int iTransRet = m_cxTcpClient.Send(pData, iPacketLen, iTransDataLen);
    if (iTransRet != iPacketLen)
    {
        m_cxTcpClient.Close();
        m_bIsOpened = false;
        return -6;
    }
    memset(pData, 0, iPacketLen);
    iTransRet = m_cxTcpClient.RecvPacket(pData, CLIENT_BUF_SIZE, iTransDataLen);
    if (iTransRet != 0)
    {
        m_cxTcpClient.Close();
        m_bIsOpened = false;
        return -7;
    }

    PCXPacketHeader pTcpHeader = (PCXPacketHeader)pData;
    PCXPacketData pPacket = (PCXPacketData)(pData);
    PCXCommonMessageReply pReply = (PCXCommonMessageReply)(pData + sizeof(CXPacketHeader) + sizeof(DWORD));
    if (pReply->dwReplyCode != 200)
    {
        m_cxTcpClient.Close();
        m_bIsOpened = false;
        return -8;
    }

    uiFileLength = pReply->dwValue1;
    uiFileLength <<= 32;
    uiFileLength += pReply->dwValue2;

    return RETURN_SUCCEED;
}
