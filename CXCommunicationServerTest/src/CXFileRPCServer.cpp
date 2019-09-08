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
    GetClassGuid();
	m_strCurrentFilePath = "";
	m_bContinuousObject = true;
}


CXFileRPCServer::~CXFileRPCServer()
{
	m_strCurrentFilePath = "";
}

//return value:==-2 need to close the connection
int CXFileRPCServer::DispatchMes(PCXMessageData pMes)
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

	int iProcessRet = RETURN_SUCCEED;
    switch (dwMessageCode)
    {
    case CX_HEAERT_BEAT_CODE:
    {
        PCXCommonMessageReply pReply = (PCXCommonMessageReply)bySendBuf;
        pReply->dwReplyCode = 200;
        dwSendMesLen = sizeof(CXCommonMessageReply);
        bool bRet = pCon->SendPacket(bySendBuf,dwSendMesLen, CX_HEAERT_BEAT_REPLY_CODE, pMes);
        if (!bRet)
        {
			iProcessRet =-2;
        }
        break;
    }
    case CX_FILE_OPEN_CODE:
		iProcessRet = OpenFile(pMes,pCon);
		break;
    case CX_FILE_SEEK_CODE:
		iProcessRet = Seek(pMes, pCon);
        break;
    case CX_FILE_READ_CODE:
		iProcessRet = Read(pMes, pCon);
		break;
    break;
    case CX_FILE_WRITE_CODE:
		iProcessRet = Write(pMes, pCon);
        break;
    case CX_FILE_CLOSE_CODE:
		iProcessRet = CloseFile(pMes, pCon);
        break;
    case CX_FILE_GET_LENGTH_CODE:
		iProcessRet = GetFileLength(pMes,pCon);
    default:
        break;
    }

    return iProcessRet;
}


CXRPCObjectServer* CXFileRPCServer::CreateObject()
{
    return (CXRPCObjectServer*)new CXFileRPCServer();
}

int CXFileRPCServer::SendData(CXConnectionObject * pCon, const byte *pbyData,
    DWORD dwDataLen)
{
    return RETURN_SUCCEED;
}

void CXFileRPCServer::Destroy()
{
    if (!m_file.IsOpen())
    {
        m_file.Close();
    }
	m_strCurrentFilePath = "";
	Reset();
}

void CXFileRPCServer::MessageToString(PCXMessageData pMes, string &strMes)
{
	char  szInfo[1024] = { 0 };
	CXGuidObject guidObj(false);
	string strRequestID = guidObj.ConvertGuid(pMes->bodyData.byRequestID);

	sprintf_s(szInfo, 1024, "file message, packet_guid:%s,message_code:%04d,message_length:%d",
        strRequestID.c_str(), pMes->bodyData.dwMesCode, pMes->dwDataLen);
	strMes = szInfo;

	switch (pMes->bodyData.dwMesCode)
	{
	case CX_HEAERT_BEAT_CODE:
		strMes += ",op:heart_beat";
		break;
	case CX_FILE_OPEN_CODE:
	{
		PCXFileOpenFile pData = (PCXFileOpenFile)pMes->bodyData.buf;
		if (pData->dwFilePathLen <= 0 || pData->dwFilePathLen > pMes->dwDataLen)
		{
			pData->szFilePath[0] = '\0';
		}
		else
		{
			pData->szFilePath[pData->dwFilePathLen-1]='\0';
		}
		sprintf_s(szInfo, 1024, ",op:open, open_type:%d,reserve:%06x,path_len:%d,path:%s",
			pData->byOpenType, *((DWORD*)pData->byReserve), pData->dwFilePathLen,pData->szFilePath);
		strMes += szInfo;
		break;
	}	
	case CX_FILE_SEEK_CODE:
	{
		PCXFileSeek pData = (PCXFileSeek)pMes->bodyData.buf;
		sprintf_s(szInfo, 1024, ",op:seek,seek_type:%d,seek_pos:%lld",
			pData->dwSeekType, pData->iSeekPos);
		strMes += szInfo;
		break;
	}
	case CX_FILE_READ_CODE:
	{
		PCXFileRead pData = (PCXFileRead)pMes->bodyData.buf;
		sprintf_s(szInfo, 1024, ",op:read,seek_type:%d,seek_pos:%lld,need_len:%d",
			pData->dwSeekType, pData->iBeginPos, pData->dwReadLen);
		strMes += szInfo;
		break;
	}
	case CX_FILE_WRITE_CODE:
	{
		PCXFileWrite pData = (PCXFileWrite)pMes->bodyData.buf;
		sprintf_s(szInfo, 1024, ",op:write,seek_type:%d,seek_pos:%lld,write_len:%d",
			pData->dwSeekType, pData->iBeginPos, pData->dwDataLen);
		strMes += szInfo;
		break;
	}
	case CX_FILE_CLOSE_CODE:
	{
		PCXFileClose pData = (PCXFileClose)pMes->bodyData.buf;
		sprintf_s(szInfo, 1024, ",op:close,close_type:%d",
			pData->dwType);
		strMes += szInfo;
		break;
	}
	case CX_FILE_GET_LENGTH_CODE:
	{
		PCXFileClose pData = (PCXFileClose)pMes->bodyData.buf;
		sprintf_s(szInfo, 1024, ",op:get file length");
		strMes += szInfo;
		break;
	}
	default:
	{
		strMes += ",unknown packet";
		break;
	}
	}
}


int CXFileRPCServer::OpenFile(PCXMessageData pMes,CXConnectionObject *pCon)
{
	PCXFileOpenFile pData = (PCXFileOpenFile)pMes->bodyData.buf;
	DWORD dwDataLen = pMes->dwDataLen - (sizeof(CXPacketBodyData) - 1);
	DWORD dwSendMesLen = sizeof(CXCommonMessageReply);
	byte  bySendBuf[1024] = { 0 };
	CXFile64 *pFile = NULL;
    char  szInfo[1024] = { 0 };

	PCXCommonMessageReply pReply = (PCXCommonMessageReply)bySendBuf;
	DWORD dwLeftDataLen = 1024 - sizeof(CXCommonMessageReply) - 1;

	while (true)
	{
		if (pData->dwFilePathLen <= 0 || pData->dwFilePathLen > dwDataLen)
		{
			pReply->dwReplyCode = 201;
			pReply->dwDataLen = 0;
			break;
		}

		if (m_file.IsOpen())
		{
			m_file.Close();
		}
		
		m_strCurrentFilePath = "";
        bool bOpenExisted = false;
        if (pData->byReserve[0] == 1)
        {
            bOpenExisted = true;
        }

		if (!m_file.Open(pData->szFilePath, (CXFile64::OPENTYPE)pData->byOpenType,0, bOpenExisted))
		{
			pReply->dwReplyCode = 202;
            DWORD dwEr = GetLastError();
			char *pszError = strerror(errno);
			strcpy(pReply->szData, pszError);
			pReply->dwDataLen = strlen(pszError) + 1;

            sprintf_s(szInfo, dwLeftDataLen, "Failed to open file %s , open type is %d,error code is %d, error description is %s\n",
                pData->szFilePath, pData->byOpenType,dwEr, pszError);
            m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);

			break;
		}
		m_strCurrentFilePath = pData->szFilePath;

		pReply->dwReplyCode = 200;

        sprintf_s(szInfo, 1024, "Success in open file %s\n",pData->szFilePath);
        //m_pLogHandle->Log(CXLog::CXLOG_INFO, szInfo);

		break;
	}

	dwSendMesLen = sizeof(CXCommonMessageReply)-1+ pReply->dwDataLen;
	bool bSentRet = pCon->SendPacket(bySendBuf, dwSendMesLen, CX_FILE_OPEN_REPLY_CODE,pMes);
	if (!bSentRet)
	{
		return -2;
	}
	return RETURN_SUCCEED;
}
int CXFileRPCServer::CloseFile(PCXMessageData pMes,CXConnectionObject *pCon)
{
	PCXFileClose pData = (PCXFileClose)pMes->bodyData.buf;
    DWORD dwDataLen = pMes->dwDataLen - (sizeof(CXPacketBodyData) - 1);

	DWORD dwSendMesLen = sizeof(CXCommonMessageReply);
	byte  bySendBuf[1024] = { 0 };

	PCXCommonMessageReply pReply = (PCXCommonMessageReply)bySendBuf;

	if (m_file.IsOpen())
	{
		m_file.Close();
	}

	char  szInfo[1024] = { 0 };
	sprintf_s(szInfo, 1024, "Close file,connection index is %lld,file object pointer is %x\n",
		pCon->GetConnectionIndex(), (void*)this);
	//m_pLogHandle->Log(CXLog::CXLOG_INFO, szInfo);

	pReply->dwReplyCode = 200;
	dwSendMesLen = sizeof(CXCommonMessageReply) + pReply->dwDataLen;
	bool bSentRet = pCon->SendPacket(bySendBuf, dwSendMesLen, CX_FILE_CLOSE_REPLY_CODE, pMes);
	if (!bSentRet)
	{
		return -2;
	}
	return RETURN_SUCCEED;
}
int CXFileRPCServer::Write(PCXMessageData pMes, CXConnectionObject *pCon)
{
	PCXFileWrite pData = (PCXFileWrite)pMes->bodyData.buf;
    DWORD dwDataLen = pMes->dwDataLen - (sizeof(CXPacketBodyData) - 1);

	DWORD dwSendMesLen = sizeof(CXCommonMessageReply);
	byte  bySendBuf[1024] = { 0 };

	PCXCommonMessageReply pReply = (PCXCommonMessageReply)bySendBuf;

	DWORD dwLeftDataLen = 1024-sizeof(CXCommonMessageReply)-1;

	while (true)
	{
		if (!m_file.IsOpen())
		{
			strcpy(pReply->szData, "The file is closed!\n");
			pReply->dwDataLen = strlen(pReply->szData) + 1;
			m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
			pReply->dwReplyCode = 412;
			break;
		}

		DWORD dwWrittenLen = 0;

		//need to seek a other position
		if (!((pData->dwSeekType == (DWORD)CXFile64::current) && pData->iBeginPos == 0))
		{
			if (!m_file.Seek(pData->iBeginPos, (CXFile64::SEEKTYPE)pData->dwSeekType))
			{
				pReply->dwReplyCode = 501;
				sprintf_s(pReply->szData, dwLeftDataLen, "Failed to seek to a new position %lld, seek type is %d!",
					pData->iBeginPos, pData->dwSeekType);
				pReply->dwDataLen = strlen(pReply->szData) + 1;

                DWORD dwEr = GetLastError();
                char szInfo[1024] = { 0 };
                sprintf_s(szInfo, 1024, "%s,error code is %d\n", pReply->szData, dwEr);
                m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);

				break;
			}
		}
			
		if (!m_file.Write((byte*)pData->szData, pData->dwDataLen, dwWrittenLen) || dwWrittenLen != pData->dwDataLen)
		{
            DWORD dwError = GetLastError();
			uint64 uiPos = 0;
			m_file.GetCurrentPosition(uiPos);
			pReply->dwReplyCode = 502;
			sprintf_s(pReply->szData, dwLeftDataLen, 
				"Failed to write data to file, in position %lld, "
				"the size needed to write is %d, had written %d,error code=%d!",
				uiPos, pData->dwDataLen, dwWrittenLen, dwError);
			pReply->dwDataLen = strlen(pReply->szData) + 1;

            m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);

			break;
		}

		pReply->dwReplyCode = 200;
		break;
	}
	dwSendMesLen = sizeof(CXCommonMessageReply) + pReply->dwDataLen;
	bool bSentRet = pCon->SendPacket(bySendBuf, dwSendMesLen, CX_FILE_WRITE_REPLY_CODE, pMes);
	if (!bSentRet)
	{
		return -2;
	}
	return RETURN_SUCCEED;
}
int CXFileRPCServer::Read(PCXMessageData pMes, CXConnectionObject *pCon)
{	
	PCXFileRead pData = (PCXFileRead)pMes->bodyData.buf;
    DWORD dwDataLen = pMes->dwDataLen - (sizeof(CXPacketBodyData) - 1);

	byte  bySendBuf[1024] = { 0 };
	PCXFileReadReply pReply = (PCXFileReadReply)bySendBuf;
	DWORD dwSendMesLen = sizeof(CXFileReadReply) - 1;

	DWORD dwLeftDataLen = 1024 - sizeof(CXFileReadReply) - 1;
	bool bNotReadFromFle = false;

	while (true)
	{
		if (pData->dwReadLen <= 0 || pData->dwReadLen > (1024 * 10240))
		{
			pReply->dwReplyCode = 201;
			bNotReadFromFle = true;
			sprintf_s(pReply->szData, dwLeftDataLen, "Failed to read data,the need read length %d is a invalid value", pData->dwReadLen);
			pReply->dwDataLen = strlen(pReply->szData) + 1;

			m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
			break;
		}

		if (!m_file.IsOpen())
		{
			pReply->dwReplyCode = 202;
			strcpy(pReply->szData, "The file had been closed!\n");
			pReply->dwDataLen = strlen(pReply->szData) + 1;
			m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
			bNotReadFromFle = true;
			break;
		}

		int iBufLen = pData->dwReadLen + sizeof(CXFileReadReply) - 1;
        bool bGetBuf = false;
        byte *pBufSend = NULL;
		//this buffer not need to free in this function, that will be free by the SendPacket function
        PCXBufferObj pCXBuf = pCon->GetSendCXBufferObj(iBufLen);
        if (pCXBuf != NULL)
        {
            pBufSend = pCon->GetWritableBuffer(pCXBuf);
            if (pBufSend != NULL)
            {
                bGetBuf = true;
            }
        }
		if (!bGetBuf)
		{
			pReply->dwReplyCode = 203;
			bNotReadFromFle = true;
			sprintf_s(pReply->szData, dwLeftDataLen, "Failed to read data,an error occur in allocating memory %d bytes", iBufLen);
			pReply->dwDataLen = strlen(pReply->szData) + 1;
			m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
			break;
		}
		pReply = (PCXFileReadReply)pBufSend;
		pReply->dwReplyCode = 200;

		int iNeedRead = pData->dwReadLen;
		int iTotalReadLen = 0;
		int iLeftBufLen = pData->dwReadLen;

		while (iTotalReadLen < pData->dwReadLen)
		{
			iNeedRead = pData->dwReadLen;
			if (iNeedRead > (pData->dwReadLen - iTotalReadLen))
			{
				iNeedRead = pData->dwReadLen - iTotalReadLen;
			}
			DWORD dwReadLen = 0;
			if (m_file.Read((byte*)(pReply->szData + iTotalReadLen), iNeedRead, dwReadLen))
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
						"file object handle is %x,session is %x,guid is %s\n",
						pData->dwReadLen, iTotalReadLen, uiCurPos, uiFileSize,
						pCon->GetConnectionIndex(), (void*)this, pCon->GetSession(), 
						((CXConnectionSession*)pCon->GetSession())->GetSessionGuid().c_str());
					m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
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
					"current position is %lld,file size is %lld,connection index is %lld,file object handle is %x\n",
					pData->dwReadLen, iTotalReadLen, uiCurPos, uiFileSize,
					pCon->GetConnectionIndex(), (void*)this);
				m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
				break;
			}
		}

		dwSendMesLen = pReply->dwDataLen + sizeof(CXFileReadReply) - 1;
		bool bSentRet = pCon->SendPacket(pCXBuf, dwSendMesLen, CX_FILE_READ_REPLY_CODE, pMes);
		if (!bSentRet)
		{
			char  szInfo[1024] = { 0 };
			sprintf_s(szInfo, 1024, "Failed to sent data, need send is %d,had sent %d,connection index is %lld\n",
				dwSendMesLen, iTotalReadLen,
				pCon->GetConnectionIndex());
			m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
			return -2;
		}
		break;
	}

	if (bNotReadFromFle)
	{
		dwSendMesLen = pReply->dwDataLen + sizeof(CXFileReadReply)-1;
		bool bSentRet = pCon->SendPacket(bySendBuf, dwSendMesLen, CX_FILE_READ_REPLY_CODE, pMes);
		if (!bSentRet)
		{
			return -2;
		}
	}

	return RETURN_SUCCEED;
}

int CXFileRPCServer::Seek(PCXMessageData pMes, CXConnectionObject *pCon)
{
	PCXFileSeek pData = (PCXFileSeek)pMes->bodyData.buf;
    DWORD dwDataLen = pMes->dwDataLen - (sizeof(CXPacketBodyData) - 1);

	DWORD dwSendMesLen = sizeof(CXCommonMessageReply);
	byte  bySendBuf[1024] = { 0 };
    char  szInfo[1024] = { 0 };

	PCXCommonMessageReply pReply = (PCXCommonMessageReply)bySendBuf;
	CXFile64 *pFile = NULL;

	DWORD dwLeftDataLen = 1024 - sizeof(CXCommonMessageReply) - 1;

	while (true)
	{
		if (!m_file.IsOpen())
		{
			strcpy(pReply->szData, "The file is closed!\n");
			pReply->dwDataLen = strlen(pReply->szData) + 1;
			m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
			pReply->dwReplyCode = 412;
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

                m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
				break;
			}
		}

		pReply->dwReplyCode = 200;
		break;
	}
	dwSendMesLen = pReply->dwDataLen + sizeof(CXCommonMessageReply)-1;
	bool bSentRet = pCon->SendPacket(bySendBuf, dwSendMesLen, CX_FILE_SEEK_REPLY_CODE, pMes);
	if (!bSentRet)
	{
		return -2;
	}
	return RETURN_SUCCEED;
}

int CXFileRPCServer::GetFileLength(PCXMessageData pMes,CXConnectionObject *pCon)
{
	DWORD dwSendMesLen = sizeof(CXCommonMessageReply);
	byte  bySendBuf[1024] = { 0 };

	CXFile64 *pFile = NULL;
	PCXCommonMessageReply pReply = (PCXCommonMessageReply)bySendBuf;
	while (true)
	{
		if (!m_file.IsOpen())
		{
			strcpy(pReply->szData, "The file is closed!\n");
			pReply->dwDataLen = strlen(pReply->szData) + 1;
			m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
			pReply->dwReplyCode = 412;
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

	dwSendMesLen = pReply->dwDataLen + sizeof(CXCommonMessageReply)-1;
	bool bSentRet = pCon->SendPacket(bySendBuf, dwSendMesLen, CX_FILE_GET_LENGTH_REPLY_CODE, pMes);
	if (!bSentRet)
	{
		return -2;
	}

	return RETURN_SUCCEED;
}

int CXFileRPCServer::GetCurrentFilePosition(PCXMessageData pMes,CXConnectionObject *pCon)
{
    DWORD dwSendMesLen = sizeof(CXCommonMessageReply);
    byte  bySendBuf[1024] = { 0 };

    CXFile64 *pFile = NULL;
    PCXCommonMessageReply pReply = (PCXCommonMessageReply)bySendBuf;
    DWORD dwLeftDataLen = 1024 - sizeof(CXCommonMessageReply) - 1;

    while (true)
    {
		if (!m_file.IsOpen())
		{
			strcpy(pReply->szData, "The file is closed!\n");
			pReply->dwDataLen = strlen(pReply->szData) + 1;
			m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
			pReply->dwReplyCode = 412;
			break;
		}
  
        //need to seek a other position
        uint64 uiFilePos = 0;
        if (!m_file.GetCurrentPosition(uiFilePos))
        {
            DWORD dwEr = GetLastError();
            pReply->dwReplyCode = 201;
            sprintf_s(pReply->szData, dwLeftDataLen, "Failed to get the current position of the file,error code is %d!",
                dwEr);
            pReply->dwDataLen = strlen(pReply->szData) + 1;

            m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
            break;
        }
        else
        {
            pReply->dwValue1 = uiFilePos >> 32;
            pReply->dwValue2 = uiFilePos & 0xffffffff;
            pReply->dwReplyCode = 200;
        }
           

        break;
    }
	dwSendMesLen = pReply->dwDataLen + sizeof(CXCommonMessageReply)-1;
	bool bSentRet = pCon->SendPacket(bySendBuf, dwSendMesLen, CX_FILE_GET_CUR_POS_REPLY_CODE, pMes);
    if (!bSentRet)
    {
        return -2;
    }

    return RETURN_SUCCEED;
}

