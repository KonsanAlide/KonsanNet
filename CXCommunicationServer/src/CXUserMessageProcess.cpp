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
int CXUserMessageProcess::OnReceivedMessage(const PCXMessageData pMes, CXConnectionObject* pCon,
    CXConnectionSession *pSession)
{
    DWORD dwMessageCode = pMes->bodyData.dwMesCode;
    int iReceiveDataLen = pMes->dwDataLen;

    DWORD dwSendMesLen = sizeof(CXCommonMessageReply);
    byte  bySendBuf[1024] = { 0 };

    CXFile64 *pFile = NULL;
    switch (dwMessageCode)
    {
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
            if (!pFile->Open(pData->szFilePath,CXFile64::modeRead))
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

        dwSendMesLen = sizeof(CXCommonMessageReply);
        bool bRet = pCon->SendPacket(bySendBuf,dwSendMesLen,CX_FILE_OPEN_REPLY_CODE);
        if(!bRet)
        {
            return -2;
        }
        break;
    }
    case CX_FILE_SEEK_CODE:
        break;
    case CX_FILE_READ_CODE:
    {
        byte *pBufSend = NULL;
        bool bNotReadFromFle = false;
        PCXFileReadReply pReply = (PCXFileReadReply)bySendBuf;
        dwSendMesLen = sizeof(CXFileReadReply)-1;

        while (true)
        {
            PCXFileRead pData = (PCXFileRead)pMes->bodyData.buf;
            if (pData->dwReadLen <= 0 || pData->dwReadLen>(1024*1023))
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

            int iBufLen = pData->dwReadLen + sizeof(CXFileReadReply)-1;
            pBufSend = (byte*)pCon->GetBuffer(iBufLen);
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
                if (iNeedRead > (pData->dwReadLen- iTotalReadLen))
                {
                    iNeedRead = pData->dwReadLen - iTotalReadLen;
                }
                DWORD dwReadLen = 0;
                if (pFile->Read((byte*)pReply->szData, iNeedRead, dwReadLen))
                {
                    iTotalReadLen += dwReadLen;
                    pReply->dwDataLen += dwReadLen;

                    if (dwReadLen != iNeedRead)//read to file end
                    {
                        pReply->dwReplyCode = 204; 
                        break;
                    }
                }
                else
                {
                    pReply->dwReplyCode = 205;
                    char *pszError = strerror(errno);
                    strcpy(pReply->szData, pszError);
                    pReply->dwDataLen = strlen(pszError) + 1;
                    break;
                }
            }

            dwSendMesLen = pReply->dwDataLen +sizeof(CXFileReadReply) - 1;
            bool bRet = pCon->SendPacket(pBufSend, dwSendMesLen, CX_FILE_READ_REPLY_CODE);
            pCon->FreeBuffer(pBufSend);
            if (!bRet)
            {
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

int CXUserMessageProcess::SendData(CXConnectionObject * pCon, const byte *pbyData, DWORD dwDataLen)
{
    return RETURN_SUCCEED;
}

