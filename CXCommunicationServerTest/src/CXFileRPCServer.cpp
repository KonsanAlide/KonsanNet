/****************************************************************************
Copyright (c) 2018-2019 Charles Yang

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
#include "CXFileRPCServer.h"
#include "CXPacketCodeDefine.h"
#include "CXFilePacketStructure.h"
#include "PlatformFunctionDefine.h"
#include <time.h>
#include <fcntl.h>
#ifdef WIN32
#include <io.h>
#else // WIN32
#include <errno.h>
#include<string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#endif


using namespace CXCommunication;

CXFileRPCServer::CXFileRPCServer()
{
    GetObjectGuid();
}


CXFileRPCServer::~CXFileRPCServer()
{
}

//return value:==-2 need to close the connection
int CXFileRPCServer::ProcessMessage(PCXMessageData pMes)
{
    DWORD dwMessageCode = pMes->bodyData.dwMesCode;
    int iReceiveDataLen = pMes->dwDataLen;
    CXConnectionObject *pCon = (CXConnectionObject*)pMes->pConObj;
    if (pCon == NULL)
    {
        return -1;
    }
    CXConnectionSession *pSession = (CXConnectionSession*)pCon->GetSession();
    if (pSession == NULL)
    {
        return -1;
    }

    DWORD dwSendMesLen = sizeof(CXCommonMessageReply);
    byte  bySendBuf[1024] = { 0 };

    CXFile64 *pFile = NULL;
    switch (dwMessageCode)
    {
    case CX_HEAERT_BEAT_CODE:
    {
        PCXCommonMessageReply pReply = (PCXCommonMessageReply)bySendBuf;
        pReply->dwReplyCode = 200;
        dwSendMesLen = sizeof(CXCommonMessageReply);
        bool bRet = pCon->SendPacket(bySendBuf, dwSendMesLen, CX_HEAERT_BEAT_REPLY_CODE);
        if (!bRet)
        {
            return -2;
        }
        break;
    }
    case CX_FILE_OPEN_CODE:
    {
        PCXCommonMessageReply pReply = (PCXCommonMessageReply)bySendBuf;

        while (true)
        {
            PCXFileOpenFile pData = (PCXFileOpenFile)pMes->bodyData.buf;
            if (pData->dwFilePathLen <= 0 || pData->dwFilePathLen>iReceiveDataLen)
            {
                pReply->dwReplyCode = 201;
                break;
            }

            if (m_file.IsOpen())
            {
                m_file.Close();
            }
            
            if (!m_file.Open(pData->szFilePath, CXFile64::modeRead))
            {
                pReply->dwReplyCode = 202;

                char *pszError = strerror(errno);
                strcpy(pReply->szData, pszError);
                pReply->dwDataLen = strlen(pszError) + 1;
                break;
            }

            char  szInfo[1024] = { 0 };
            sprintf_s(szInfo, 1024, "Open file %s,connection index is %lld, file handle is %x,session is %x,guid is %s\n",
                pData->szFilePath,pCon->GetConnectionIndex(), pFile, pSession, pSession->GetSessionGuid().c_str());
            //pCon->GetLogHandle()->Log(CXLog::CXLOG_INFO, szInfo);

            pReply->dwReplyCode = 200;
            break;
        }

        dwSendMesLen = sizeof(CXCommonMessageReply);
        bool bRet = pCon->SendPacket(bySendBuf, dwSendMesLen, CX_FILE_OPEN_REPLY_CODE);
        if (!bRet)
        {
            return -2;
        }
        break;
    }
    case CX_FILE_SEEK_CODE:
    {
        PCXFileSeek pData = (PCXFileSeek)pMes->bodyData.buf;
        DWORD dwSendMesLen = sizeof(CXCommonMessageReply);
        char  szInfo[1024] = { 0 };

        PCXCommonMessageReply pReply = (PCXCommonMessageReply)bySendBuf;
        DWORD dwLeftDataLen = 1024 - sizeof(CXCommonMessageReply) - 1;

        while (true)
        {
            if (!m_file.IsOpen())
            {
                pReply->dwReplyCode = 202;
                strcpy(pReply->szData, "The file had been closed!\n");
                pReply->dwDataLen = strlen(pReply->szData) + 1;
                pCon->GetLogHandle()->Log(CXLog::CXLOG_ERROR, pReply->szData);
                break;
            }


            DWORD dwWrittenLen = 0;

            //need to seek a other position
            if (!((pData->dwSeekType == (DWORD)CXFile64::current) && pData->iSeekPos == 0))
            {
                if (!m_file.Seek(pData->iSeekPos, (CXFile64::SEEKTYPE)pData->dwSeekType))
                {
                    pReply->dwReplyCode = 501;
                    sprintf_s(pReply->szData, dwLeftDataLen, "Failed to seek to a new position %lld, seek type is %d!",
                        pData->iSeekPos, pData->dwSeekType);
                    pReply->dwDataLen = strlen(pReply->szData) + 1;

                    pCon->GetLogHandle()->Log(CXLog::CXLOG_ERROR, pReply->szData);
                    break;
                }
            }

            pReply->dwReplyCode = 200;
            break;
        }

        bool bRet = pCon->SendPacket(bySendBuf, dwSendMesLen, CX_FILE_SEEK_REPLY_CODE);
        if (!bRet)
        {
            return -2;
        }
    }
        break;
    case CX_FILE_READ_CODE:
    {
        bool bNotReadFromFle = false;
        PCXFileReadReply pReply = (PCXFileReadReply)bySendBuf;
        dwSendMesLen = sizeof(CXFileReadReply) - 1;

        while (true)
        {
            PCXFileRead pData = (PCXFileRead)pMes->bodyData.buf;
            if (pData->dwReadLen <= 0 || pData->dwReadLen>(1024 * 10240))
            {
                pReply->dwReplyCode = 201;
                bNotReadFromFle = true;
                break;
            }

            if (!m_file.IsOpen())
            {
                pReply->dwReplyCode = 202;
                strcpy(pReply->szData, "The file had been closed!\n");
                pReply->dwDataLen = strlen(pReply->szData) + 1;
                pCon->GetLogHandle()->Log(CXLog::CXLOG_ERROR, pReply->szData);
                bNotReadFromFle = true;
                break;
            }

            int iBufLen = pData->dwReadLen + sizeof(CXFileReadReply) - 1;
            byte *pBufSend = (byte*)pCon->GetBuffer(iBufLen);
            if (pBufSend == NULL)
            {
                pReply->dwReplyCode = 203;
                bNotReadFromFle = true;
                break;
            }
            pReply = (PCXFileReadReply)pBufSend;
            pReply->dwReplyCode = 200;

            int iNeedRead = pData->dwReadLen;
            int iTotalReadLen = 0;
            int iLeftBufLen = pData->dwReadLen;

            while (iTotalReadLen<pData->dwReadLen)
            {
                iNeedRead = pData->dwReadLen;
                if (iNeedRead >(pData->dwReadLen - iTotalReadLen))
                {
                    iNeedRead = pData->dwReadLen - iTotalReadLen;
                }
                DWORD dwReadLen = 0;
                if (m_file.Read((byte*)(pReply->szData+ iTotalReadLen), iNeedRead, dwReadLen))
                {
                    iTotalReadLen += dwReadLen;
                    pReply->dwDataLen += dwReadLen;

                    if (dwReadLen != iNeedRead)//read to file end
                    {
                        DWORD dwEr = GetLastError();
                        uint64 uiFileSize = 0;
                        m_file.GetFileLength(uiFileSize);
                        uint64 uiCurPos = 0;
                        m_file.GetCurrentPosition(uiCurPos);
                        pReply->dwReplyCode = 204;
                        char  szInfo[1024] = { 0 };
                        sprintf_s(szInfo, 1024, "Failed to read data, need read is %d,had read %d,"
                            "current position is %lld,file is %lld,connection index is %lld,"
                            "file handle is %x,session is %x,guid is %s\n",
                            pData->dwReadLen, iTotalReadLen, uiCurPos, uiFileSize,
                            pCon->GetConnectionIndex(), pFile,pSession,pSession->GetSessionGuid().c_str());
                        pCon->GetLogHandle()->Log(CXLog::CXLOG_ERROR, szInfo);
                        break;
                    }
                }
                else
                {
                    pReply->dwReplyCode = 205;

                    uint64 uiFileSize = 0;
                    m_file.GetFileLength(uiFileSize);
                    uint64 uiCurPos = 0;
                    m_file.GetCurrentPosition(uiCurPos);

                    DWORD dwEr = GetLastError();
                    char *pszError = strerror(errno);
                    strcpy(pReply->szData, pszError);
                    pReply->dwDataLen = strlen(pszError) + 1;

                    char  szInfo[1024] = { 0 };
                    sprintf_s(szInfo, 1024, "Failed to read data, need read is %d,had read %d,"
                        "current position is %lld,file is %lld,connection index is %lld,file handle is %x\n",
                        pData->dwReadLen, iTotalReadLen, uiCurPos, uiFileSize,
                        pCon->GetConnectionIndex(), pFile);
                    pCon->GetLogHandle()->Log(CXLog::CXLOG_ERROR, szInfo);
                    break;
                }
            }

            dwSendMesLen = pReply->dwDataLen + sizeof(CXFileReadReply) - 1;
            bool bRet = pCon->SendPacket(pBufSend, dwSendMesLen, CX_FILE_READ_REPLY_CODE);
            pCon->FreeBuffer(pBufSend);
            if (!bRet)
            {
                char  szInfo[1024] = { 0 };
                sprintf_s(szInfo, 1024, "Failed to sent data, need send is %d,had sent %d,connection index is %lld\n",
                    dwSendMesLen, iTotalReadLen,
                    pCon->GetConnectionIndex());
                pCon->GetLogHandle()->Log(CXLog::CXLOG_ERROR, szInfo);
                return -2;
            }
            break;
        }

        if (bNotReadFromFle)
        {
            dwSendMesLen = sizeof(CXFileReadReply);
            bool bRet = pCon->SendPacket(bySendBuf, dwSendMesLen, CX_FILE_OPEN_REPLY_CODE);
            if (!bRet)
            {
                return -2;
            }
        }

        break;
    }
    break;
    case CX_FILE_WRITE_CODE:
        break;
    case CX_FILE_CLOSE_CODE:
    {
        PCXCommonMessageReply pReply = (PCXCommonMessageReply)bySendBuf;
        while (true)
        {
            PCXFileClose pData = (PCXFileClose)pMes->bodyData.buf;

            if (m_file.IsOpen())
            {
                m_file.Close();
            }

            char  szInfo[1024] = { 0 };
            sprintf_s(szInfo, 1024, "Close file,connection index is %lld,file handle is %x\n",
                pCon->GetConnectionIndex(), pFile);
            pCon->GetLogHandle()->Log(CXLog::CXLOG_INFO, szInfo);

            pReply->dwReplyCode = 200;
            break;
        }
        dwSendMesLen = sizeof(CXCommonMessageReply);
        bool bRet = pCon->SendPacket(bySendBuf, dwSendMesLen, CX_FILE_CLOSE_REPLY_CODE);
        if (!bRet)
        {
            return -2;
        }
        break;
    }
    case CX_FILE_GET_LENGTH_CODE:
    {
        PCXCommonMessageReply pReply = (PCXCommonMessageReply)bySendBuf;
        while (true)
        {
            if (!m_file.IsOpen())
            {
                pReply->dwReplyCode = 202;
                strcpy(pReply->szData, "The file had been closed!\n");
                pReply->dwDataLen = strlen(pReply->szData) + 1;
                pCon->GetLogHandle()->Log(CXLog::CXLOG_ERROR, pReply->szData);
                break;
            }

            uint64 uiFileLen = 0;
            if (m_file.GetFileLength(uiFileLen))
            {
                pReply->dwValue1 = uiFileLen >> 32;
                pReply->dwValue2 = uiFileLen & 0xffffffff;
                pReply->dwReplyCode = 200;
            }
            else
            {
                pReply->dwReplyCode = 201;
            }

            break;
        }
        dwSendMesLen = sizeof(CXCommonMessageReply);
        bool bRet = pCon->SendPacket(bySendBuf, dwSendMesLen, CX_FILE_GET_LENGTH_REPLY_CODE);
        if (!bRet)
        {
            return -2;
        }
        break;
    }
    default:
        break;
    }

    return RETURN_SUCCEED;
}

CXRPCObjectServer* CXFileRPCServer::CreateObject()
{
    return (CXRPCObjectServer*)new CXFileRPCServer;
}

int CXFileRPCServer::SendData(CXConnectionObject * pCon, const byte *pbyData, DWORD dwDataLen)
{
    return RETURN_SUCCEED;
}

void CXFileRPCServer::Destroy()
{
    if (!m_file.IsOpen())
    {
        m_file.Close();
    }
}

void CXFileRPCServer::RecordSlowOps(PCXMessageData pMes)
{

}
