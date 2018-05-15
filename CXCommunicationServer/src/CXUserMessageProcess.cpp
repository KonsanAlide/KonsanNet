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

Description£º
*****************************************************************************/
#include "CXUserMessageProcess.h"
#include "CXPacketCodeDefine.h"
#include "CXFilePacketStructure.h"
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

#include "CXFile64.h"

using namespace CXCommunication;

CXUserMessageProcess::CXUserMessageProcess()
{
}


CXUserMessageProcess::~CXUserMessageProcess()
{
}

//return value:==-2 need to close the connection
int CXUserMessageProcess::ProcessPacket(PCXBufferObj pBuf, CXConnectionObject * pCon,
    CXConnectionSession *pSession)
{
    if (pBuf != NULL)
    {
        pCon->AddProcessPacketNumber();
        //printf("####Process packet ,connection id = %I64i,datalen = %d,packet number :%I64i\n",
        //    pCon->GetConnectionIndex(), pBuf->wsaBuf.len, pCon->GetProcessPacketNumber());
    }

    PCXPacketHeader pTcpHeader = (PCXPacketHeader)pBuf->wsaBuf.buf;
    PCXPacketData pPacket = (PCXPacketData)(pBuf->wsaBuf.buf);
    DWORD dwMessageCode = pPacket->dwMesCode;
    int iReceiveDataLen = pBuf->wsaBuf.len;

    CXFile64 *pFile = NULL;
    switch (dwMessageCode)
    {
    case CX_FILE_OPEN_CODE:
    {
        PCXBufferObj pBufSend = pCon->GetBuffer();
        PCXCommonMessageReply pReply = (PCXCommonMessageReply)(pBufSend->wsaBuf.buf + sizeof(CXPacketData)-1);

        while (true)
        {
            PCXFileOpenFile pMes = (PCXFileOpenFile)pPacket->buf;
            if (pMes->dwFilePathLen <= 0 || pMes->dwFilePathLen>iReceiveDataLen)
            {
                pReply->dwReplyCode = 201;
                break;
            }
            bool bHaveFileObject = false;
            pFile = (CXFile64 *)pSession->GetData("file");
            if (pFile ==NULL)
            {
                pFile = new CXFile64();
            }
            else
            {
                if (pFile->IsOpen())
                {
                    pFile->Close();
                }
                bHaveFileObject = true;
            }
            if (!pFile->Open(pMes->szFilePath,CXFile64::modeRead))
            {
                pReply->dwReplyCode = 202;

                char *pszError = strerror(errno);
                strcpy(pReply->szData, pszError);
                pReply->dwDataLen = strlen(pszError) + 1;
                break;
            }
            if(!bHaveFileObject)
                pSession->SetData("file", (void*)pFile);

            pReply->dwReplyCode = 200;
            break;
        }
        int iRet = SendCommonMessageReply(pCon, CX_FILE_OPEN_CODE + 1, pBufSend);
        pCon->FreeBuffer(pBufSend);
        if(iRet==-2)
        {
            return -2;
        }
        break;
    }
    case CX_FILE_SEEK_CODE:
        break;
    case CX_FILE_READ_CODE:
    {
        if (dwMessageCode != pPacket->dwMesCode)
        {
            int k = 0;
        }
        PCXBufferObj pBufSend = pCon->GetBuffer();
        int iBufLen = pBufSend->wsaBuf.len;
        PCXFileReadReply pReply = (PCXFileReadReply)(pBufSend->wsaBuf.buf + sizeof(CXPacketData) - 1);
        int iLeftBufLen = iBufLen - (sizeof(CXPacketHeader) + sizeof(DWORD))-sizeof(CXFileReadReply)+1;
        bool bNotReadFromFle = false;
        while (true)
        {

            PCXFileRead pMes = (PCXFileRead)pPacket->buf;
            if (pMes->dwReadLen <= 0)
            {
                pReply->dwReplyCode = 201;
                bNotReadFromFle = true;
                break;
            }

            pFile = (CXFile64 *)pSession->GetData("file");
            if (pFile == NULL)
            {
                pReply->dwReplyCode = 202;
                strcpy(pReply->szData, "The file had been close by other process!");
                pReply->dwDataLen = strlen(pReply->szData) + 1;
                bNotReadFromFle = true;
                break;
            }
            int iNeedRead = pMes->dwReadLen;
            int iTotalReadLen = 0;
            bool bReadFails = false;
            bool bReadToEnd = false;
            while (iTotalReadLen<pMes->dwReadLen)
            {
                iNeedRead = iLeftBufLen;
                if (iNeedRead > (pMes->dwReadLen- iTotalReadLen))
                {
                    iNeedRead = pMes->dwReadLen - iTotalReadLen;
                }
                DWORD dwReadLen = 0;
                if (pFile->Read((byte*)pReply->szData, iNeedRead, dwReadLen))
                {
                    if (dwReadLen != iNeedRead)
                    {
                        pReply->dwReplyCode = 204;
                        iTotalReadLen += dwReadLen;
                        pReply->dwDataLen = dwReadLen;
                        bReadToEnd = true;
                        pReply->dwReplyCode = 203;
                        
                    }
                    else
                    {
                        pReply->dwDataLen = iNeedRead;
                        pReply->dwReplyCode = 200;
                        iTotalReadLen += dwReadLen;
                    }
                }
                else
                {
                    pReply->dwReplyCode = 205;
                    char *pszError = strerror(errno);
                    strcpy(pReply->szData, pszError);
                    pReply->dwDataLen = strlen(pszError) + 1;
                    bReadFails = true;
                }

                int iRet = SendFileReadReply(pCon, CX_FILE_OPEN_REPLY_CODE, pBufSend);
                //pCon->FreeBuffer(pBufSend);
                memset(pBufSend->wsaBuf.buf,0, iBufLen);
                if (iRet == -2)
                {
                    pCon->FreeBuffer(pBufSend);
                    return -2;
                }

                if (bReadFails|| bReadToEnd)
                {
                    break;
                }
            }
            break;
        }

        if (bNotReadFromFle)
        {
            int iRet = SendFileReadReply(pCon, CX_FILE_OPEN_CODE + 1, pBufSend);
            pCon->FreeBuffer(pBufSend);
            if (iRet == -2)
            {
                return -2;
            }
        }
        else
        {
            pCon->FreeBuffer(pBufSend);
        }


        break;
    }
        break;
    case CX_FILE_WRITE_CODE:
        break;
    case CX_FILE_CLOSE_CODE:
    {
        PCXBufferObj pBufSend = pCon->GetBuffer();
        PCXCommonMessageReply pReply = (PCXCommonMessageReply)(pBufSend->wsaBuf.buf + sizeof(CXPacketData) - 1);

        while (true)
        {
            PCXFileClose pMes = (PCXFileClose)pPacket->buf;

            pFile = (CXFile64 *)pSession->GetData("file");
            if (pFile == NULL)
            {
                pReply->dwReplyCode = 202;
                strcpy(pReply->szData, "The file had been close by other process!");
                pReply->dwDataLen = strlen(pReply->szData) + 1;
                break;
            }

            pFile->Close();
            //pSession->RemoveData("file");

            pReply->dwReplyCode = 200;
            break;
        }
        int iRet = SendCommonMessageReply(pCon, CX_FILE_CLOSE_CODE + 1, pBufSend);
        pCon->FreeBuffer(pBufSend);
        if (iRet == -2)
        {
            return -2;
        }
        break;
    }
    case CX_FILE_GET_LENGTH_CODE:
    {
        PCXBufferObj pBufSend = pCon->GetBuffer();
        PCXCommonMessageReply pReply = (PCXCommonMessageReply)(pBufSend->wsaBuf.buf + sizeof(CXPacketData) - 1);

        while (true)
        {
            pFile = (CXFile64 *)pSession->GetData("file");
            if (pFile == NULL)
            {
                pReply->dwReplyCode = 202;
                strcpy(pReply->szData, "The file had been close by other process!");
                pReply->dwDataLen = strlen(pReply->szData) + 1;
                break;
            }

            uint64 uiFileLen = 0;
            if (pFile->GetFileLength(uiFileLen))
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
        int iRet = SendCommonMessageReply(pCon, CX_FILE_GET_LENGTH_REPLY_CODE, pBufSend);
        pCon->FreeBuffer(pBufSend);
        if (iRet == -2)
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

int CXUserMessageProcess::SendCommonMessageReply(CXConnectionObject * pCon,
    DWORD deMessageCode, PCXBufferObj pBuf)
{
    PCXCommonMessageReply pReply = (PCXCommonMessageReply)(pBuf->wsaBuf.buf+ sizeof(CXPacketData) - 1);
    int iPacketBodyLen = sizeof(DWORD) + sizeof(CXCommonMessageReply) - 1 + pReply->dwDataLen;
    byte *pBodyData = (byte*)(pBuf->wsaBuf.buf + sizeof(CXPacketHeader));


    DWORD dwCheckSum = 0;
    for (int i = 0; i<iPacketBodyLen; i++)
    {
        dwCheckSum += (unsigned int)pBodyData[i];
    }

    //send the header
    PCXPacketHeader pTcpHeader = (PCXPacketHeader)pBuf->wsaBuf.buf;

    pTcpHeader->wDataLen = iPacketBodyLen;
    pTcpHeader->byReserve = 0x15;
    pTcpHeader->byType = 2;
    pTcpHeader->dwFlag = 0x25f7;
    pTcpHeader->dwCheckSum = dwCheckSum;
    pTcpHeader->dwPacketNum = 0;

    PCXPacketData pPacket = (PCXPacketData)(pBuf->wsaBuf.buf);
    pPacket->dwMesCode = deMessageCode;
    int iPacketLen = iPacketBodyLen + sizeof(CXPacketHeader);
    pBuf->wsaBuf.len = iPacketLen;

    DWORD dwSendLen = 0;
    int iRet = pCon->PostSendBlocking(pBuf, dwSendLen);
    if (iRet != 0)
    {
        return -2;
    }
    return RETURN_SUCCEED;
}

int CXUserMessageProcess::SendFileReadReply(CXConnectionObject * pCon,
    DWORD deMessageCode, PCXBufferObj pBuf)
{
    PCXFileReadReply pReply = (PCXFileReadReply)(pBuf->wsaBuf.buf + sizeof(CXPacketData) - 1);
    int iPacketBodyLen = sizeof(DWORD)+sizeof(CXFileReadReply) - 1 + pReply->dwDataLen;
    byte *pBodyData = (byte*)(pBuf->wsaBuf.buf + sizeof(CXPacketHeader));


    DWORD dwCheckSum = 0;
    for (int i = 0; i<iPacketBodyLen; i++)
    {
        dwCheckSum += (unsigned int)pBodyData[i];
    }

    //send the header
    PCXPacketHeader pTcpHeader = (PCXPacketHeader)pBuf->wsaBuf.buf;

    pTcpHeader->wDataLen = iPacketBodyLen;
    pTcpHeader->byReserve = 0x15;
    pTcpHeader->byType = 2;
    pTcpHeader->dwFlag = 0x25f7;
    pTcpHeader->dwCheckSum = dwCheckSum;
    pTcpHeader->dwPacketNum = 0;

    PCXPacketData pPacket = (PCXPacketData)(pBuf->wsaBuf.buf);
    pPacket->dwMesCode = deMessageCode;
    int iPacketLen = iPacketBodyLen + sizeof(CXPacketHeader);
    pBuf->wsaBuf.len = iPacketLen;

    DWORD dwSendLen = 0;
    int iRet = pCon->PostSendBlocking(pBuf, dwSendLen);
    if (iRet != 0)
    {
        return -2;
    }
    return RETURN_SUCCEED;
}
