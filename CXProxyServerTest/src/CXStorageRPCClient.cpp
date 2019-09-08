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
#include "CXStorageRPCClient.h"
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

CXStorageRPCClient::CXStorageRPCClient()
{
    GetClassGuid();
	m_strCurrentFilePath = "";
	m_bContinuousObject = true;
	m_bIsOpened = false;
}


CXStorageRPCClient::~CXStorageRPCClient()
{
	m_strCurrentFilePath = "";
}

//return value:==-2 need to close the connection
int CXStorageRPCClient::DispatchMes(PCXMessageData pMes)
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
    case CX_FILE_OPEN_REPLY_CODE:
		iProcessRet = OnOpen(pMes, pCon);
		break;
    case CX_FILE_SEEK_REPLY_CODE:
		iProcessRet = OnSeek(pMes, pCon);
		break;
    case CX_FILE_READ_REPLY_CODE:
		iProcessRet = OnRead(pMes, pCon);
		break;
    case CX_FILE_WRITE_REPLY_CODE:
		iProcessRet = OnWrite(pMes, pCon);
		break;
    case CX_FILE_CLOSE_REPLY_CODE:
		iProcessRet = OnClose(pMes, pCon);
		break;
    case CX_FILE_GET_LENGTH_REPLY_CODE:
        iProcessRet = OnGetFileLength(pMes, pCon);
        break;
	case CX_FILE_GET_CUR_POS_REPLY_CODE:
		iProcessRet = OnGetCurrentPosition(pMes, pCon);
		break;
    default:
        break;
    }

    return iProcessRet;
}


CXRPCObjectServer* CXStorageRPCClient::CreateObject()
{
    return (CXRPCObjectServer*)new CXStorageRPCClient();
}

int CXStorageRPCClient::SendData(CXConnectionObject * pCon, const byte *pbyData,
    DWORD dwDataLen)
{
    return RETURN_SUCCEED;
}

void CXStorageRPCClient::Destroy()
{
	m_strCurrentFilePath = "";
    UnCreate();
	Reset();
}

void CXStorageRPCClient::MessageToString(PCXMessageData pMes, string &strMes)
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
	case CX_FILE_READ_REPLY_CODE:
	{
		PCXFileReadReply pData = (PCXFileReadReply)pMes->bodyData.buf;
		sprintf_s(szInfo, 1024, ",op:read_reply,reply_code:%d,,data_len:%d",
			pData->dwReplyCode, pData->dwDataLen);
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
	case CX_FILE_OPEN_REPLY_CODE:
	case CX_FILE_SEEK_REPLY_CODE:
	case CX_FILE_WRITE_REPLY_CODE:
	case CX_FILE_CLOSE_REPLY_CODE:
	case CX_FILE_GET_LENGTH_REPLY_CODE:
	{
		PCXCommonMessageReply pData = (PCXCommonMessageReply)pMes->bodyData.buf;
		sprintf_s(szInfo, 1024, ",op:read_reply,reply_code:%d,value1:%d,value2:%d,data_len:%d,content:",
			pData->dwReplyCode, pData->dwValue1, pData->dwValue2, pData->dwDataLen);
		if (pData->dwDataLen > 0)
		{
			strcat(szInfo, pData->szData);
		}
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

int  CXStorageRPCClient::Close(CXRPCRequest request)
{
	if (m_pAttachedConnection == NULL || !request.IsParametersValid())
	{
		return CXRET_INVALID_PARAMETER;
	}

    int iBufLen = 0;

    byte *       pBufSend = NULL;
    PCXBufferObj pCXBuf = NULL;
    DWORD dwSendBufLen = iBufLen + sizeof(CXFileCommand);
    if (CXRET_SUCCEED != GetSendBuffer(&pCXBuf, &pBufSend, dwSendBufLen))
    {
        return CXRET_ALLOCATE_MEMORY_FAILED;
    }

	DWORD dwSendMesLen = sizeof(CXFileCommand);
    return SendPacketData(request, pCXBuf, dwSendMesLen, CX_FILE_CLOSE_CODE);
}

int  CXStorageRPCClient::Open(CXRPCRequest request,string &strRemoteFilePath, OPENTYPE type, bool bOpenExisted)
{
	if (m_pAttachedConnection == NULL || strRemoteFilePath.length() == 0
		|| !request.IsParametersValid())
	{
		return CXRET_INVALID_PARAMETER;
	}

    int iBufLen = strRemoteFilePath.length() + 1;

    byte *       pBufSend = NULL;
    PCXBufferObj pCXBuf = NULL;
    DWORD dwSendBufLen = iBufLen + (sizeof(CXFileOpenFile) - 1);
    if (CXRET_SUCCEED != GetSendBuffer(&pCXBuf, &pBufSend, dwSendBufLen))
    {
        return CXRET_ALLOCATE_MEMORY_FAILED;
    }

	PCXFileOpenFile pMes = (PCXFileOpenFile)pBufSend;
	pMes->byOpenType = type;
	pMes->dwFilePathLen = iBufLen;
	memcpy(pMes->szFilePath, strRemoteFilePath.c_str(), iBufLen);
	if (bOpenExisted)
	{
		pMes->byReserve[0] = 1;
	}
	DWORD dwSendMesLen = sizeof(CXFileOpenFile) - 1 + pMes->dwFilePathLen;

    return SendPacketData(request, pCXBuf, dwSendMesLen, CX_FILE_OPEN_CODE);
}
int  CXStorageRPCClient::Read(CXRPCRequest request,int iWantReadLen,uint64 uiOffset, SEEKTYPE type)
{
	if (m_pAttachedConnection == NULL || iWantReadLen == 0
		|| !request.IsParametersValid())
	{
		return CXRET_INVALID_PARAMETER;
	}

    int iBufLen = 0;

    byte *       pBufSend = NULL;
    PCXBufferObj pCXBuf = NULL;
    DWORD dwSendBufLen = iBufLen + sizeof(CXFileRead);
    if (CXRET_SUCCEED != GetSendBuffer(&pCXBuf, &pBufSend, dwSendBufLen))
    {
        return CXRET_ALLOCATE_MEMORY_FAILED;
    }
    PCXFileRead pMes = (PCXFileRead)pBufSend;
    pMes->dwSeekType = (DWORD)type;
    pMes->dwReadLen = iWantReadLen;
    pMes->iBeginPos = uiOffset;

    return SendPacketData(request, pCXBuf, dwSendBufLen, CX_FILE_READ_CODE);
}

int  CXStorageRPCClient::Seek(CXRPCRequest request,int64 pos, SEEKTYPE type)
{
	if (m_pAttachedConnection == NULL || !request.IsParametersValid())
	{
		return CXRET_INVALID_PARAMETER;
	}

    int iBufLen = 0;

    byte *       pBufSend = NULL;
    PCXBufferObj pCXBuf = NULL;
    DWORD dwSendBufLen = iBufLen + sizeof(CXFileSeek);
    if (CXRET_SUCCEED != GetSendBuffer(&pCXBuf, &pBufSend, dwSendBufLen))
    {
        return CXRET_ALLOCATE_MEMORY_FAILED;
    }
    PCXFileSeek pMes = (PCXFileSeek)pBufSend;
    pMes->dwSeekType = type;
    pMes->iSeekPos = pos;

    return SendPacketData(request, pCXBuf, dwSendBufLen, CX_FILE_SEEK_CODE);
}
int  CXStorageRPCClient::Write(CXRPCRequest request,const byte* pBuf, int iBufLen,
	uint64 uiOffset, SEEKTYPE type)
{
	if (NULL==m_pAttachedConnection|| pBuf == 0 || iBufLen==0|| !request.IsParametersValid())
	{
		return CXRET_INVALID_PARAMETER;
	}

	DWORD dwSendMesLen = 0;
	byte  bySendBuf[CX_BUF_SIZE] = { 0 };
	
	byte *       pBufSend = NULL;
    PCXBufferObj pCXBuf = NULL;
    DWORD dwSendBufLen = iBufLen + (sizeof(CXFileWrite) - 1);
    if (CXRET_SUCCEED != GetSendBuffer(&pCXBuf,&pBufSend, dwSendBufLen))
    {
        return CXRET_ALLOCATE_MEMORY_FAILED;
    }

    PCXFileWrite pMes = (PCXFileWrite)(pBufSend);
	pMes->dwSeekType = type;
	pMes->dwDataLen = iBufLen;
	pMes->iBeginPos = uiOffset;
	memcpy(pMes->szData, pBuf, iBufLen);

    return SendPacketData(request,pCXBuf, dwSendBufLen, CX_FILE_WRITE_CODE);
}

int  CXStorageRPCClient::GetFileLength(CXRPCRequest request)
{
	if (m_pAttachedConnection == NULL || !request.IsParametersValid())
	{
		return CXRET_INVALID_PARAMETER;
	}

    int iBufLen = 0;

    byte *       pBufSend = NULL;
    PCXBufferObj pCXBuf = NULL;
    DWORD dwSendBufLen = iBufLen + sizeof(CXFileCommand);
    if (CXRET_SUCCEED != GetSendBuffer(&pCXBuf, &pBufSend, dwSendBufLen))
    {
        return CXRET_ALLOCATE_MEMORY_FAILED;
    }

    return SendPacketData(request, pCXBuf, dwSendBufLen, CX_FILE_GET_LENGTH_CODE);
}
int  CXStorageRPCClient::GetCurrentPosition(CXRPCRequest request)
{
	if (m_pAttachedConnection == NULL || !request.IsParametersValid())
	{
		return CXRET_INVALID_PARAMETER;
	}
    int iBufLen = 0;

    byte *       pBufSend = NULL;
    PCXBufferObj pCXBuf = NULL;
    DWORD dwSendBufLen = iBufLen + sizeof(CXFileCommand);
    if (CXRET_SUCCEED != GetSendBuffer(&pCXBuf, &pBufSend, dwSendBufLen))
    {
        return CXRET_ALLOCATE_MEMORY_FAILED;
    }

    return SendPacketData(request, pCXBuf, dwSendBufLen, CX_FILE_GET_CUR_POS_CODE);
}

int  CXStorageRPCClient::OnOpen(PCXMessageData pMes, CXConnectionObject *pCon)
{
	CXGuidObject guidObj(false);
	string strRequestID = guidObj.ConvertGuid(pMes->bodyData.byRequestID);
	CXRPCRequest request = PopRequest(strRequestID);
	if (request.GetRequestID().length() > 0)
	{
		PCXCommonMessageReply pReply = (PCXCommonMessageReply)pMes->bodyData.buf;
		DWORD dwDataLen = pMes->dwDataLen - (sizeof(CXPacketBodyData) - 1);

		CXRPCReply reply("", pReply->dwReplyCode, (byte*)pReply->szData, pReply->dwDataLen, 
            pReply->dwValue1, pReply->dwValue2);

		if (pReply->dwReplyCode != 200 && pReply->dwDataLen>0 
			&& pReply->dwDataLen==dwDataLen-(sizeof(CXCommonMessageReply)-1))
		{
			pReply->szData[pReply->dwDataLen-1] = '\0';
			reply.SetReplyContent(pReply->szData);
		}
		if (pReply->dwReplyCode==200)
		{
			m_bIsOpened = true;
		}

		return ReplyRequester(request, reply);
	}
	
	return CXRET_SUCCEED;
}
int  CXStorageRPCClient::OnRead(PCXMessageData pMes, CXConnectionObject *pCon)
{
	CXGuidObject guidObj(false);
	string strRequestID = guidObj.ConvertGuid(pMes->bodyData.byRequestID);
	CXRPCRequest request = PopRequest(strRequestID);
	if (request.GetRequestID().length() > 0)
	{
		PCXFileReadReply pReply = (PCXFileReadReply)pMes->bodyData.buf;
		DWORD dwDataLen = pMes->dwDataLen - (sizeof(CXPacketBodyData) - 1);

        CXRPCReply reply("", pReply->dwReplyCode, (byte*)pReply->szData, pReply->dwDataLen,0, 0);
		if (pReply->dwReplyCode != 200 && pReply->dwDataLen > 0
			&& pReply->dwDataLen == dwDataLen - (sizeof(CXFileReadReply) - 1))
		{
			pReply->szData[pReply->dwDataLen - 1] = '\0';
			reply.SetReplyContent(pReply->szData);
		}

        return ReplyRequester(request, reply);
	}
	return CXRET_SUCCEED;
}
int  CXStorageRPCClient::OnSeek(PCXMessageData pMes, CXConnectionObject *pCon)
{
	CXGuidObject guidObj(false);
	string strRequestID = guidObj.ConvertGuid(pMes->bodyData.byRequestID);
	CXRPCRequest request = PopRequest(strRequestID);
	if (request.GetRequestID().length() > 0)
	{
		PCXCommonMessageReply pReply = (PCXCommonMessageReply)pMes->bodyData.buf;
		DWORD dwDataLen = pMes->dwDataLen - (sizeof(CXPacketBodyData) - 1);
        CXRPCReply reply("", pReply->dwReplyCode, (byte*)pReply->szData, pReply->dwDataLen,
            pReply->dwValue1, pReply->dwValue2);

		if (pReply->dwReplyCode != 200 && pReply->dwDataLen > 0
			&& pReply->dwDataLen == dwDataLen - (sizeof(CXCommonMessageReply) - 1))
		{
			pReply->szData[pReply->dwDataLen - 1] = '\0';
			reply.SetReplyContent(pReply->szData);
		}

        return ReplyRequester(request, reply);
	}
	return CXRET_SUCCEED;
}
int  CXStorageRPCClient::OnWrite(PCXMessageData pMes, CXConnectionObject *pCon)
{
	CXGuidObject guidObj(false);
	string strRequestID = guidObj.ConvertGuid(pMes->bodyData.byRequestID);
	CXRPCRequest request = PopRequest(strRequestID);
	if (request.GetRequestID().length() > 0)
	{
		PCXCommonMessageReply pReply = (PCXCommonMessageReply)pMes->bodyData.buf;
		DWORD dwDataLen = pMes->dwDataLen - (sizeof(CXPacketBodyData) - 1);
        CXRPCReply reply("", pReply->dwReplyCode, (byte*)pReply->szData, pReply->dwDataLen,
            pReply->dwValue1, pReply->dwValue2);
		if (pReply->dwReplyCode != 200 && pReply->dwDataLen > 0
			&& pReply->dwDataLen == dwDataLen - (sizeof(CXCommonMessageReply) - 1))
		{
			pReply->szData[pReply->dwDataLen - 1] = '\0';
			reply.SetReplyContent(pReply->szData);
		}

        return ReplyRequester(request, reply);
	}
	return CXRET_SUCCEED;
}
int  CXStorageRPCClient::OnGetFileLength(PCXMessageData pMes, CXConnectionObject *pCon)
{
	CXGuidObject guidObj(false);
	string strRequestID = guidObj.ConvertGuid(pMes->bodyData.byRequestID);
	CXRPCRequest request = PopRequest(strRequestID);
	if (request.GetRequestID().length() > 0)
	{
		PCXCommonMessageReply pReply = (PCXCommonMessageReply)pMes->bodyData.buf;
		DWORD dwDataLen = pMes->dwDataLen - (sizeof(CXPacketBodyData) - 1);
        CXRPCReply reply("", pReply->dwReplyCode, (byte*)pReply->szData, pReply->dwDataLen,
            pReply->dwValue1, pReply->dwValue2);
		if (pReply->dwReplyCode != 200 && pReply->dwDataLen > 0
			&& pReply->dwDataLen == dwDataLen - (sizeof(CXCommonMessageReply) - 1))
		{
			pReply->szData[pReply->dwDataLen - 1] = '\0';
			reply.SetReplyContent(pReply->szData);
		}

        return ReplyRequester(request, reply);
	}
	return CXRET_SUCCEED;
}
int  CXStorageRPCClient::OnGetCurrentPosition(PCXMessageData pMes, CXConnectionObject *pCon)
{
	CXGuidObject guidObj(false);
	string strRequestID = guidObj.ConvertGuid(pMes->bodyData.byRequestID);
	CXRPCRequest request = PopRequest(strRequestID);
	if (request.GetRequestID().length() > 0)
	{
		PCXCommonMessageReply pReply = (PCXCommonMessageReply)pMes->bodyData.buf;
		DWORD dwDataLen = pMes->dwDataLen - (sizeof(CXPacketBodyData) - 1);
        CXRPCReply reply("", pReply->dwReplyCode, (byte*)pReply->szData, pReply->dwDataLen,
            pReply->dwValue1, pReply->dwValue2);
		if (pReply->dwReplyCode != 200 && pReply->dwDataLen > 0
			&& pReply->dwDataLen == dwDataLen - (sizeof(CXCommonMessageReply) - 1))
		{
			pReply->szData[pReply->dwDataLen - 1] = '\0';
			reply.SetReplyContent(pReply->szData);
		}

        return ReplyRequester(request, reply);
	}
	return CXRET_SUCCEED;
}

int CXStorageRPCClient::OnClose(PCXMessageData pMes, CXConnectionObject *pCon)
{
	CXGuidObject guidObj(false);
	string strRequestID = guidObj.ConvertGuid(pMes->bodyData.byRequestID);
	CXRPCRequest request = PopRequest(strRequestID);
	if (request.GetRequestID().length() > 0)
	{
		PCXCommonMessageReply pReply = (PCXCommonMessageReply)pMes->bodyData.buf;
		DWORD dwDataLen = pMes->dwDataLen - (sizeof(CXPacketBodyData) - 1);
        CXRPCReply reply("", pReply->dwReplyCode, (byte*)pReply->szData, pReply->dwDataLen,
            pReply->dwValue1, pReply->dwValue2);
		if (pReply->dwReplyCode != 200 && pReply->dwDataLen > 0
			&& pReply->dwDataLen == dwDataLen - (sizeof(CXCommonMessageReply) - 1))
		{
			pReply->szData[pReply->dwDataLen - 1] = '\0';
			reply.SetReplyContent(pReply->szData);
		}
        if(pReply->dwReplyCode == 200)
            m_bIsOpened = false;

        return ReplyRequester(request, reply);
	}
	return CXRET_SUCCEED;
}


