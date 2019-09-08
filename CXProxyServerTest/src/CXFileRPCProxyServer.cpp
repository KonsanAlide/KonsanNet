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
#include "CXFileRPCProxyServer.h"
#include "CXPacketCodeDefine.h"
#include "PlatformFunctionDefine.h"
#include "CXTaskPool.h"
#include "CXRPCObjectManager.h"
#include "CXCommunicationServer.h"
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
typedef int (CXFileRPCProxyServer::*FileReplyFun)(PCXMessageData, int, byte*, int);
CXFileRPCProxyServer::CXFileRPCProxyServer()
{
	m_dwTimeoutSecs = 3;
    GetClassGuid();
	m_strCurrentFilePath = "";
	m_bFileOpened = false;
	m_bContinuousObject = true;
	m_pStorageObj = NULL;

    RegisterEntryFun(CX_FILE_OPEN_CODE, std::bind(&CXFileRPCProxyServer::OpenFile, this, placeholders::_1, placeholders::_2));
    RegisterEntryFun(CX_FILE_CLOSE_CODE, std::bind(&CXFileRPCProxyServer::CloseFile, this, placeholders::_1, placeholders::_2));
    RegisterEntryFun(CX_FILE_WRITE_CODE, std::bind(&CXFileRPCProxyServer::Write, this, placeholders::_1, placeholders::_2));
    RegisterEntryFun(CX_FILE_READ_CODE, std::bind(&CXFileRPCProxyServer::Read, this, placeholders::_1, placeholders::_2));
    RegisterEntryFun(CX_FILE_SEEK_CODE, std::bind(&CXFileRPCProxyServer::Seek, this, placeholders::_1, placeholders::_2));
    RegisterEntryFun(CX_FILE_GET_LENGTH_CODE, std::bind(&CXFileRPCProxyServer::GetFileLength, this, placeholders::_1, placeholders::_2));
    RegisterEntryFun(CX_FILE_GET_CUR_POS_CODE, std::bind(&CXFileRPCProxyServer::GetCurrentFilePosition, this, placeholders::_1, placeholders::_2));


    RegisterStepFun(CX_INTER_FILE_OPEN_REPLY, std::bind(&CXFileRPCProxyServer::OpenFileReply, this, placeholders::_1, placeholders::_2,
        placeholders::_3));
    RegisterStepFun(CX_INTER_FILE_READ_REPLY, std::bind(&CXFileRPCProxyServer::ReadReply, this, placeholders::_1, placeholders::_2,
        placeholders::_3));
    RegisterStepFun(CX_INTER_FILE_WRITE_REPLY, std::bind(&CXFileRPCProxyServer::WriteReply, this, placeholders::_1, placeholders::_2,
        placeholders::_3));
    RegisterStepFun(CX_INTER_FILE_SEEK_REPLY, std::bind(&CXFileRPCProxyServer::SeekReply, this, placeholders::_1, placeholders::_2,
        placeholders::_3));
    RegisterStepFun(CX_INTER_FILE_CLOSE_REPLY, std::bind(&CXFileRPCProxyServer::CloseFileReply, this, placeholders::_1, placeholders::_2,
        placeholders::_3));
    RegisterStepFun(CX_INTER_FILE_GET_FILE_LEN_REPLY, std::bind(&CXFileRPCProxyServer::GetFileLengthReply, this, placeholders::_1, placeholders::_2,
        placeholders::_3));
    RegisterStepFun(CX_INTER_FILE_GET_FILE_POS_REPLY, std::bind(&CXFileRPCProxyServer::GetCurrentFilePositionReply, this, placeholders::_1, placeholders::_2,
        placeholders::_3));
}


CXFileRPCProxyServer::~CXFileRPCProxyServer()
{
	Destroy();
}

//return value:==-2 need to close the connection
int CXFileRPCProxyServer::DispatchMes(PCXMessageData pMes)
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
        iProcessRet = OpenFile(pMes, pCon);
		break;
    case CX_FILE_SEEK_CODE:
		iProcessRet = Seek(pMes, pCon);
        break;
    case CX_FILE_READ_CODE:
		iProcessRet = Read(pMes, pCon);
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


CXRPCObjectServer* CXFileRPCProxyServer::CreateObject()
{
    return (CXRPCObjectServer*)new CXFileRPCProxyServer;
}

int CXFileRPCProxyServer::SendData(CXConnectionObject * pCon, const byte *pbyData,
    DWORD dwDataLen)
{
    return RETURN_SUCCEED;
}

void CXFileRPCProxyServer::Destroy()
{
	m_strCurrentFilePath = "";
	m_bFileOpened = false;
    if (m_pStorageObj != NULL)
    {
        if (m_pComServer != NULL)
        {
            CXRPCObjectManager *pRPCObjPool = ((CXCommunicationServer*)m_pComServer)->GetRPCObjectManager();
            if (pRPCObjPool != NULL)
            {
                m_pStorageObj->Destroy();
                pRPCObjPool->FreeObject(m_pStorageObj);
            }
            m_pStorageObj = NULL;
        }
    }
	Reset();
}

void CXFileRPCProxyServer::MessageToString(PCXMessageData pMes, string &strMes)
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

int CXFileRPCProxyServer::OpenFile(PCXMessageData pMes,CXConnectionObject *pCon)
{
	PCXFileOpenFile pData = (PCXFileOpenFile)pMes->bodyData.buf;
	DWORD dwDataLen = pMes->dwDataLen - (sizeof(CXPacketBodyData) - 1);
	DWORD dwSendMesLen = sizeof(CXCommonMessageReply);
	byte  bySendBuf[1024] = { 0 };
    char  szInfo[1024] = { 0 };
	int   iProcessRet = 0;

	PCXCommonMessageReply pReply = (PCXCommonMessageReply)bySendBuf;
	DWORD dwLeftDataLen = 1024 - sizeof(CXCommonMessageReply) - 1;

	while (true)
	{
		if (pData->dwFilePathLen <= 0 || pData->dwFilePathLen > dwDataLen)
		{
			pReply->dwReplyCode = 400;
			pReply->dwDataLen = 0;
			break;
		}

        m_strCurrentFilePath = pData->szFilePath;

		string strRemoteObjID = "{D2753C32-37A1-47F7-A1E0-7F4CAC074B47}";
		if (m_pStorageObj == NULL)
		{
			CXRPCObjectManager *pRPCObjPool = (CXRPCObjectManager *)pCon->GetRPCObjectManager();
			if (pRPCObjPool != NULL)
			{
				//CXStorageTcpV1
				m_pStorageObj = (CXStorageRPCClient *)pRPCObjPool->GetFreeObject(strRemoteObjID);
			}
		}

		if (m_pStorageObj == NULL )
		{
			pReply->dwReplyCode = 404;
			sprintf_s(pReply->szData, dwLeftDataLen, "Failed to create the remote storage object '%s'",
				strRemoteObjID.c_str());
			pReply->dwDataLen = strlen(pReply->szData) + 1;
			m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
			break;
		}

		m_pStorageObj->SetCommunicationServer(pCon->GetServer());
		//iProcessRet = m_pStorageObj->Create("127.0.0.1", 4354);
        iProcessRet = m_pStorageObj->Create("192.168.0.118", 4354);
		if (iProcessRet != CXRET_SUCCEED)
		{
			pReply->dwReplyCode = 404;
			sprintf_s(pReply->szData, dwLeftDataLen, "Failed to create the remote storage object '%s'",
				strRemoteObjID.c_str());
			pReply->dwDataLen = strlen(pReply->szData) + 1;
			m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
			break;
		}
		m_pStorageObj->GetAttachConnection()->SetEncryptType(pCon->GetEncryptType());
		m_pStorageObj->GetAttachConnection()->SetCompressType(pCon->GetCompressType());

		if (m_pStorageObj->IsOpen())
		{
			pReply->dwReplyCode = 200;
			sprintf_s(pReply->szData, dwLeftDataLen, "The file '%s' had been opened, not need to reopen it",
				m_strCurrentFilePath.c_str());
			pReply->dwDataLen = strlen(pReply->szData) + 1;
			m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
			break;
		}
		else
		{
            string strRequestID = GetPacketGuid(pMes);
			CXRPCRequest request(strRequestID, CX_INTER_FILE_OPEN_REPLY, this->GetObjectID(), 
                pMes->bodyData.byRequestID, (void*)this);

			bool bOpenExisted = false;
			if (pData->byReserve[0] == 1)
			{
				bOpenExisted = true;
			}

			PCXStateStepInfo pStepInfo = (PCXStateStepInfo)pCon->GetBuffer(sizeof(CXStateStepInfo));
			pStepInfo->pMes = pMes;
			pStepInfo->iCurTime = GetCurrentTimeMS();
			pStepInfo->dwNextState = CX_INTER_FILE_OPEN_REPLY;
			PushState(strRequestID, pStepInfo);
            iProcessRet = m_pStorageObj->Open(request, m_strCurrentFilePath, (CXStorageRPCClient::OPENTYPE)pData->byOpenType, bOpenExisted);
            
            if (iProcessRet != CXRET_SUCCEED)
            {
                pReply->dwReplyCode = 502;
                sprintf_s(pReply->szData, dwLeftDataLen, "Failed to notify the third party to open file '%s'",
                    m_strCurrentFilePath.c_str());
                pReply->dwDataLen = strlen(pReply->szData) + 1;
                m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
				PopState(strRequestID);
            }
            else
            {
                return CXRET_ASYNC_CALLBACK;
            }
		}

		break;
	}

	dwSendMesLen = sizeof(CXCommonMessageReply) - 1 + pReply->dwDataLen;
	bool bSentRet = pCon->SendPacket(bySendBuf, dwSendMesLen, CX_FILE_OPEN_REPLY_CODE, pMes);
	if (!bSentRet)
	{
		iProcessRet =-2;
	}
	return iProcessRet;
}

int CXFileRPCProxyServer::OpenFileReply(PCXMessageData pMes, CXConnectionObject *pCon, const CXRPCReply & rpcReply)
{
    return ReplyClient(pMes, CX_FILE_OPEN_REPLY_CODE, pCon, rpcReply);
}

int CXFileRPCProxyServer::CloseFile(PCXMessageData pMes,CXConnectionObject *pCon)
{
	PCXFileClose pData = (PCXFileClose)pMes->bodyData.buf;
	DWORD dwDataLen = pMes->dwDataLen - (sizeof(CXPacketBodyData) - 1);

	DWORD dwSendMesLen = sizeof(CXCommonMessageReply);
	byte  bySendBuf[1024] = { 0 };

	PCXCommonMessageReply pReply = (PCXCommonMessageReply)bySendBuf;

    DWORD dwLeftDataLen = 1024 - sizeof(CXCommonMessageReply) - 1;

    int iProcessRet = CXRET_SUCCEED;
    if (m_pStorageObj==NULL || (m_pStorageObj != NULL && !m_pStorageObj->IsOpen()))
    {
        pReply->dwReplyCode = 200;
        sprintf_s(pReply->szData, dwLeftDataLen, "The file '%s' had been closed, not need to reclose it",
            m_strCurrentFilePath.c_str());
        pReply->dwDataLen = strlen(pReply->szData) + 1;
        m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
    }
    else
    {
        string strRequestID = GetPacketGuid(pMes);
        CXRPCRequest request(strRequestID, CX_INTER_FILE_CLOSE_REPLY, this->GetObjectID(), 
            pMes->bodyData.byRequestID,(void*)this);

		PCXStateStepInfo pStepInfo = (PCXStateStepInfo)pCon->GetBuffer(sizeof(CXStateStepInfo));
		pStepInfo->pMes = pMes;
		pStepInfo->iCurTime = GetCurrentTimeMS();
		pStepInfo->dwNextState = CX_INTER_FILE_CLOSE_REPLY;
		PushState(strRequestID, pStepInfo);

		iProcessRet = m_pStorageObj->Close(request);

		//CXRET_ASYNC_CALLBACK
		if (iProcessRet != CXRET_SUCCEED)
		{
			pReply->dwReplyCode = 502;
			sprintf_s(pReply->szData, dwLeftDataLen, "Failed to notify the third party to close file '%s'",
				m_strCurrentFilePath.c_str());
			pReply->dwDataLen = strlen(pReply->szData) + 1;
			m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
			PopState(strRequestID);
		}
		else
		{
			return CXRET_ASYNC_CALLBACK;
		}
    }

	char  szInfo[1024] = { 0 };
	sprintf_s(szInfo, 1024, "Close file,connection index is %lld,file object pointer is %x\n",
		pCon->GetConnectionIndex(), (void*)this);
	//m_pLogHandle->Log(CXLog::CXLOG_INFO, szInfo);
	
    dwSendMesLen = sizeof(CXCommonMessageReply) - 1 + pReply->dwDataLen;
	bool bSentRet = pCon->SendPacket(bySendBuf, dwSendMesLen, CX_FILE_CLOSE_REPLY_CODE, pMes);
	if (!bSentRet)
	{
		return -2;
	}
	return RETURN_SUCCEED;
}

int CXFileRPCProxyServer::CloseFileReply(PCXMessageData pMes, CXConnectionObject *pCon, const CXRPCReply & rpcReply)
{
    int iRet = ReplyClient(pMes, CX_FILE_CLOSE_REPLY_CODE, pCon, rpcReply);

    if (m_pStorageObj != NULL)
    {
        if (m_pComServer != NULL)
        {
            m_pStorageObj->SetNeedToDestroy(true);
            m_pStorageObj = NULL;
        }
    }
    m_strCurrentFilePath = "";
    m_bFileOpened = false;
    return iRet;
}


int CXFileRPCProxyServer::Write(PCXMessageData pMes, CXConnectionObject *pCon)
{
	PCXFileWrite pData = (PCXFileWrite)pMes->bodyData.buf;
	DWORD dwDataLen = pMes->dwDataLen - (sizeof(CXPacketBodyData) - 1);

	DWORD dwSendMesLen = sizeof(CXCommonMessageReply);
	byte  bySendBuf[1024] = { 0 };

	PCXCommonMessageReply pReply = (PCXCommonMessageReply)bySendBuf;

	DWORD dwLeftDataLen = 1024 - sizeof(CXCommonMessageReply) - 1;

	int iProcessRet = CXRET_SUCCEED;
	if (m_pStorageObj == NULL || (m_pStorageObj != NULL && !m_pStorageObj->IsOpen()))
	{
		pReply->dwReplyCode = 200;
		sprintf_s(pReply->szData, dwLeftDataLen, "The file '%s' had been closed, not need to reclose it",
			m_strCurrentFilePath.c_str());
		pReply->dwDataLen = strlen(pReply->szData) + 1;
		m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
	}
	else
	{
        string strRequestID = GetPacketGuid(pMes);
        CXRPCRequest request(strRequestID, CX_INTER_FILE_WRITE_REPLY, this->GetObjectID(), 
            pMes->bodyData.byRequestID, (void*)this);


		PCXStateStepInfo pStepInfo = (PCXStateStepInfo)pCon->GetBuffer(sizeof(CXStateStepInfo));
		pStepInfo->pMes = pMes;
		pStepInfo->iCurTime = GetCurrentTimeMS();
		pStepInfo->dwNextState = CX_INTER_FILE_WRITE_REPLY;
		PushState(strRequestID, pStepInfo);

		iProcessRet = m_pStorageObj->Write(request, (byte*)pData->szData, pData->dwDataLen, 
			pData->iBeginPos, (CXStorageRPCClient::SEEKTYPE)pData->dwSeekType);

		//CXRET_ASYNC_CALLBACK
		if (iProcessRet != CXRET_SUCCEED)
		{
			pReply->dwReplyCode = 502;
			sprintf_s(pReply->szData, dwLeftDataLen, "Failed to notify the third party to write data to file '%s'",
				m_strCurrentFilePath.c_str());
			pReply->dwDataLen = strlen(pReply->szData) + 1;
			m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
			PopState(strRequestID);
		}
		else
		{
			return CXRET_ASYNC_CALLBACK;
		}
	}

	dwSendMesLen = sizeof(CXCommonMessageReply) - 1 + pReply->dwDataLen;
	bool bSentRet = pCon->SendPacket(bySendBuf, dwSendMesLen, CX_FILE_WRITE_REPLY_CODE, pMes);
	if (!bSentRet)
	{
		return -2;
	}
	return RETURN_SUCCEED;
}

int CXFileRPCProxyServer::WriteReply(PCXMessageData pMes, CXConnectionObject *pCon, const CXRPCReply & rpcReply)
{
    return ReplyClient(pMes, CX_FILE_WRITE_REPLY_CODE, pCon, rpcReply);
}


int CXFileRPCProxyServer::Read(PCXMessageData pMes, CXConnectionObject *pCon)
{	
	PCXFileRead pData = (PCXFileRead)pMes->bodyData.buf;
    DWORD dwDataLen = pMes->dwDataLen - (sizeof(CXPacketBodyData) - 1);

	byte  bySendBuf[1024] = { 0 };
	PCXFileReadReply pReply = (PCXFileReadReply)bySendBuf;
	DWORD dwSendMesLen = sizeof(CXFileReadReply) - 1;

	DWORD dwLeftDataLen = 1024 - sizeof(CXFileReadReply) - 1;
	bool bNotReadFromFle = false;

    int iProcessRet = CXRET_SUCCEED;
	if (m_pStorageObj == NULL || (m_pStorageObj != NULL && !m_pStorageObj->IsOpen()))
    {
        pReply->dwReplyCode = 501;
        sprintf_s(pReply->szData, dwLeftDataLen, "The file '%s' had been closed",
            m_strCurrentFilePath.c_str());
        pReply->dwDataLen = strlen(pReply->szData) + 1;
        m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
    }
    else
    {
        string strRequestID = GetPacketGuid(pMes);
        CXRPCRequest request(strRequestID, CX_INTER_FILE_READ_REPLY, this->GetObjectID(), 
            pMes->bodyData.byRequestID, (void*)this);

		PCXStateStepInfo pStepInfo = (PCXStateStepInfo)pCon->GetBuffer(sizeof(CXStateStepInfo));
		pStepInfo->pMes = pMes;
		pStepInfo->iCurTime = GetCurrentTimeMS();
		pStepInfo->dwNextState = CX_INTER_FILE_READ_REPLY;
		PushState(strRequestID, pStepInfo);

		iProcessRet = m_pStorageObj->Read(request, pData->dwReadLen, pData->iBeginPos, (CXStorageRPCClient::SEEKTYPE)pData->dwSeekType);

        if (iProcessRet != CXRET_SUCCEED)
        {
            pReply->dwReplyCode = 502;
            sprintf_s(pReply->szData, dwLeftDataLen, "Failed to notify the third party to read file '%s'",
                m_strCurrentFilePath.c_str());
            pReply->dwDataLen = strlen(pReply->szData) + 1;
            m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
			PopState(strRequestID);
        }
        else
        {
			return CXRET_ASYNC_CALLBACK;
        }
    }

    dwSendMesLen = sizeof(CXFileReadReply) - 1 + pReply->dwDataLen;
	bool bSentRet = pCon->SendPacket(bySendBuf, dwSendMesLen, CX_FILE_READ_REPLY_CODE, pMes);
	if (!bSentRet)
	{
		return CXRET_SEND_DATA_FAILED;
	}
	

	return RETURN_SUCCEED;
}

int CXFileRPCProxyServer::ReadReply(PCXMessageData pMes, CXConnectionObject *pCon, const CXRPCReply & rpcReply)
{
    int iProcessRet = RETURN_SUCCEED;
	DWORD dwSendMesLen = 0;
	byte  bySendBuf[CX_BUF_SIZE] = { 0 };
	DWORD dwLeftDataLen = CX_BUF_SIZE - sizeof(CXFileReadReply) - 1;
	PCXFileReadReply pReply = (PCXFileReadReply)bySendBuf;
	do
	{
		if (rpcReply.GetReplyCode() == 503) //failed to notify the third party
		{
			pReply->dwReplyCode = 404;
			sprintf_s(pReply->szData, dwLeftDataLen, rpcReply.GetReplyContent().c_str());
			pReply->dwDataLen = strlen(pReply->szData) + 1;
			m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
			iProcessRet = CXRET_NOTIFY_REMOTE_FAILED;
			break;
		}
		else
		{
			byte *pBufSend = NULL;
			int iSendBufLen = rpcReply.GetReplyDataLen() + (sizeof(CXFileReadReply) - 1);
			PCXBufferObj pCXBuf = pCon->GetSendCXBufferObj(iSendBufLen);
			if (pCXBuf != NULL)
			{
				pBufSend = pCon->GetWritableBuffer(pCXBuf);
				if (pBufSend == NULL)
				{
					pCon->FreeSendCXBufferObj(pCXBuf);
					return CXRET_ALLOCATE_MEMORY_FAILED;
				}
			}

			if (pBufSend == NULL)
			{
				pReply->dwReplyCode = 203;
				sprintf_s(pReply->szData, dwLeftDataLen, "Failed to read data,an error occur in allocating memory %d bytes", iSendBufLen);
				pReply->dwDataLen = strlen(pReply->szData) + 1;
				m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
				iProcessRet = CXRET_ALLOCATE_MEMORY_FAILED;
				break;
			}

			pReply = (PCXFileReadReply)(pBufSend);
			pReply->dwReplyCode = rpcReply.GetReplyCode();
			pReply->dwDataLen = rpcReply.GetReplyDataLen();
			if (pReply->dwDataLen > 0)
			{
				memcpy(pReply->szData, rpcReply.GetReplyData(), rpcReply.GetReplyDataLen());
			}

			dwSendMesLen = pReply->dwDataLen + (sizeof(CXFileReadReply) - 1);
			bool bSentRet = pCon->SendPacket(pCXBuf, dwSendMesLen, CX_FILE_READ_REPLY_CODE, pMes);
			if (!bSentRet)
			{
				return CXRET_SEND_DATA_FAILED;
			}

			return RETURN_SUCCEED;
		}

	} while (false);

	dwSendMesLen = sizeof(CXFileReadReply) - 1 + pReply->dwDataLen;
	bool bSentRet = pCon->SendPacket(bySendBuf, dwSendMesLen, CX_FILE_READ_REPLY_CODE, pMes);
	if (!bSentRet)
	{
		iProcessRet = CXRET_SEND_DATA_FAILED;
	}
    

    return iProcessRet;
}

int CXFileRPCProxyServer::Seek(PCXMessageData pMes, CXConnectionObject *pCon)
{
	PCXFileSeek pData = (PCXFileSeek)pMes->bodyData.buf;
	DWORD dwDataLen = pMes->dwDataLen - (sizeof(CXPacketBodyData) - 1);

	DWORD dwSendMesLen = sizeof(CXCommonMessageReply);
	byte  bySendBuf[1024] = { 0 };

	PCXCommonMessageReply pReply = (PCXCommonMessageReply)bySendBuf;

	DWORD dwLeftDataLen = 1024 - sizeof(CXCommonMessageReply) - 1;

	int iProcessRet = CXRET_SUCCEED;
	if (m_pStorageObj == NULL || (m_pStorageObj != NULL && !m_pStorageObj->IsOpen()))
	{
		pReply->dwReplyCode = 501;
		sprintf_s(pReply->szData, dwLeftDataLen, "The file '%s' had been closed",
			m_strCurrentFilePath.c_str());
		pReply->dwDataLen = strlen(pReply->szData) + 1;
		m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
	}
	else
	{
        string strRequestID = GetPacketGuid(pMes);
        CXRPCRequest request(strRequestID, CX_INTER_FILE_SEEK_REPLY, this->GetObjectID(), 
            pMes->bodyData.byRequestID, (void*)this);

		PCXStateStepInfo pStepInfo = (PCXStateStepInfo)pCon->GetBuffer(sizeof(CXStateStepInfo));
		pStepInfo->pMes = pMes;
		pStepInfo->iCurTime = GetCurrentTimeMS();
		pStepInfo->dwNextState = CX_INTER_FILE_SEEK_REPLY;
		PushState(strRequestID, pStepInfo);

		iProcessRet = m_pStorageObj->Seek(request, pData->iSeekPos, (CXStorageRPCClient::SEEKTYPE)pData->dwSeekType);

		if (iProcessRet != CXRET_SUCCEED)
		{
			pReply->dwReplyCode = 502;
			sprintf_s(pReply->szData, dwLeftDataLen, "Failed to notify the third party to seek file '%s'",
				m_strCurrentFilePath.c_str());
			pReply->dwDataLen = strlen(pReply->szData) + 1;
			m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
			PopState(strRequestID);
		}
		else
		{
			return CXRET_ASYNC_CALLBACK;
		}
	}

	dwSendMesLen = sizeof(CXCommonMessageReply) - 1 + pReply->dwDataLen;
	bool bSentRet = pCon->SendPacket(bySendBuf, dwSendMesLen, CX_FILE_SEEK_REPLY_CODE, pMes);
	if (!bSentRet)
	{
		return CXRET_SEND_DATA_FAILED;
	}
	return CXRET_SUCCEED;
}

int CXFileRPCProxyServer::SeekReply(PCXMessageData pMes, CXConnectionObject *pCon, const CXRPCReply & rpcReply)
{
    return ReplyClient(pMes, CX_FILE_SEEK_REPLY_CODE, pCon, rpcReply);
}

int CXFileRPCProxyServer::GetFileLength(PCXMessageData pMes,CXConnectionObject *pCon)
{
	DWORD dwSendMesLen = sizeof(CXCommonMessageReply);
	byte  bySendBuf[1024] = { 0 };

	PCXCommonMessageReply pReply = (PCXCommonMessageReply)bySendBuf;

	DWORD dwLeftDataLen = 1024 - sizeof(CXCommonMessageReply) - 1;

	int iProcessRet = CXRET_SUCCEED;
	if (m_pStorageObj == NULL || (m_pStorageObj != NULL && !m_pStorageObj->IsOpen()))
	{
		pReply->dwReplyCode = 501;
		sprintf_s(pReply->szData, dwLeftDataLen, "The file '%s' had been closed",
			m_strCurrentFilePath.c_str());
		pReply->dwDataLen = strlen(pReply->szData) + 1;
		m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
	}
	else
	{
        string strRequestID = GetPacketGuid(pMes);
        CXRPCRequest request(strRequestID, CX_INTER_FILE_GET_FILE_LEN_REPLY, this->GetObjectID(), 
            pMes->bodyData.byRequestID, (void*)this);
		

		PCXStateStepInfo pStepInfo = (PCXStateStepInfo)pCon->GetBuffer(sizeof(CXStateStepInfo));
		pStepInfo->pMes = pMes;
		pStepInfo->iCurTime = GetCurrentTimeMS();
		pStepInfo->dwNextState = CX_INTER_FILE_GET_FILE_LEN_REPLY;
		PushState(strRequestID, pStepInfo);

		iProcessRet = m_pStorageObj->GetFileLength(request);

		if (iProcessRet != CXRET_SUCCEED)
		{
			pReply->dwReplyCode = 502;
			sprintf_s(pReply->szData, dwLeftDataLen, "Failed to notify the third party to get length of the file '%s'",
				m_strCurrentFilePath.c_str());
			pReply->dwDataLen = strlen(pReply->szData) + 1;
			m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
			PopState(strRequestID);
		}
		else
		{
			return CXRET_ASYNC_CALLBACK;
		}
	}

	dwSendMesLen = sizeof(CXCommonMessageReply) - 1 + pReply->dwDataLen;
	bool bSentRet = pCon->SendPacket(bySendBuf, dwSendMesLen, CX_FILE_GET_LENGTH_REPLY_CODE, pMes);
	if (!bSentRet)
	{
		return CXRET_SEND_DATA_FAILED;
	}

	return CXRET_SUCCEED;
}

int CXFileRPCProxyServer::GetFileLengthReply(PCXMessageData pMes, CXConnectionObject *pCon, const CXRPCReply & rpcReply)
{
    return ReplyClient(pMes, CX_FILE_GET_LENGTH_REPLY_CODE, pCon, rpcReply);
}

int CXFileRPCProxyServer::GetCurrentFilePosition(PCXMessageData pMes,CXConnectionObject *pCon)
{
	DWORD dwSendMesLen = sizeof(CXCommonMessageReply);
	byte  bySendBuf[1024] = { 0 };

	PCXCommonMessageReply pReply = (PCXCommonMessageReply)bySendBuf;

	DWORD dwLeftDataLen = 1024 - sizeof(CXCommonMessageReply) - 1;

	int iProcessRet = CXRET_SUCCEED;
	if (m_pStorageObj == NULL || (m_pStorageObj != NULL && !m_pStorageObj->IsOpen()))
	{
		pReply->dwReplyCode = 501;
		sprintf_s(pReply->szData, dwLeftDataLen, "The file '%s' had been closed",
			m_strCurrentFilePath.c_str());
		pReply->dwDataLen = strlen(pReply->szData) + 1;
		m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
	}
	else
	{
        string strRequestID = GetPacketGuid(pMes);
        CXRPCRequest request(strRequestID, CX_INTER_FILE_GET_FILE_POS_REPLY, this->GetObjectID(), 
            pMes->bodyData.byRequestID, (void*)this);

		PCXStateStepInfo pStepInfo = (PCXStateStepInfo)pCon->GetBuffer(sizeof(CXStateStepInfo));
		pStepInfo->pMes = pMes;
		pStepInfo->iCurTime = GetCurrentTimeMS();
		pStepInfo->dwNextState = CX_INTER_FILE_GET_FILE_POS_REPLY;
		PushState(strRequestID, pStepInfo);

		iProcessRet = m_pStorageObj->GetFileLength(request);

		if (iProcessRet != CXRET_SUCCEED)
		{
			pReply->dwReplyCode = 502;
			sprintf_s(pReply->szData, dwLeftDataLen, "Failed to notify the third party to get current position of the file '%s'",
				m_strCurrentFilePath.c_str());
			pReply->dwDataLen = strlen(pReply->szData) + 1;
			m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
			PopState(strRequestID);
		}
		else
		{
			return CXRET_ASYNC_CALLBACK;
		}
	}

	dwSendMesLen = sizeof(CXCommonMessageReply) - 1 + pReply->dwDataLen;
	bool bSentRet = pCon->SendPacket(bySendBuf, dwSendMesLen, CX_FILE_GET_CUR_POS_REPLY_CODE, pMes);
	if (!bSentRet)
	{
		return CXRET_SEND_DATA_FAILED;
	}

	return CXRET_SUCCEED;
}


int CXFileRPCProxyServer::GetCurrentFilePositionReply(PCXMessageData pMes, CXConnectionObject *pCon, const CXRPCReply & rpcReply)
{
    return ReplyClient(pMes, CX_FILE_GET_CUR_POS_REPLY_CODE, pCon, rpcReply);
}

int CXFileRPCProxyServer::ReplyClient(PCXMessageData pMes, DWORD dwReplyCode, CXConnectionObject *pCon, const CXRPCReply & rpcReply)
{
    int   iProcessRet = RETURN_SUCCEED;

	DWORD dwSendMesLen = 0;
	byte  bySendBuf[CX_BUF_SIZE] = { 0 };
	DWORD dwLeftDataLen = CX_BUF_SIZE - sizeof(CXCommonMessageReply) - 1;
	PCXCommonMessageReply pReply = (PCXCommonMessageReply)bySendBuf;
	do
	{
		if (rpcReply.GetReplyCode() == 503) //failed to notify the third party
		{
			pReply->dwReplyCode = 404;
			sprintf_s(pReply->szData, dwLeftDataLen, rpcReply.GetReplyContent().c_str());
			pReply->dwDataLen = strlen(pReply->szData) + 1;
			m_pLogHandle->Log(CXLog::CXLOG_ERROR, pReply->szData);
			iProcessRet = CXRET_NOTIFY_REMOTE_FAILED;
			break;
		}
		else
		{
			if (CX_FILE_OPEN_REPLY_CODE == dwReplyCode && !m_bFileOpened)
			{
				if (rpcReply.GetReplyCode() == 200)
					m_bFileOpened = true;
			}
			pReply->dwReplyCode = rpcReply.GetReplyCode();
			pReply->dwDataLen = rpcReply.GetReplyDataLen();
			pReply->dwValue1 = rpcReply.GetReserveValue1();
			pReply->dwValue2 = rpcReply.GetReserveValue2();
			memcpy(pReply->szData, rpcReply.GetReplyData(), rpcReply.GetReplyDataLen());
			break;
		}

	} while (false);

	dwSendMesLen = sizeof(CXCommonMessageReply) - 1 + pReply->dwDataLen;
	bool bSentRet = pCon->SendPacket(bySendBuf, dwSendMesLen, dwReplyCode, pMes);
	if (!bSentRet)
	{
		iProcessRet = CXRET_SEND_DATA_FAILED;
	}

    return iProcessRet;
}
