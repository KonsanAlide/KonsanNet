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

Description:
*****************************************************************************/

#include <memory.h>
#include <time.h>
#include "CXConnectionObject.h"
#include "CXLog.h"
#include "PlatformFunctionDefine.h"
#ifndef WIN32
#include<sys/time.h>
#endif

#include <chrono>
#include <ctime>
#include "CXGuidObject.h"
#include "CXConnectionSession.h"
#include "CXPacketCodeDefine.h"
#include "CXCommunicationServer.h"
#include "CXRPCObjectManager.h"
#include "CXRPCObjectServer.h"
#include "CXTaskParseMessage.h"
#include "CXTaskPool.h"
#include "CXTaskPrepareMessage.h"
//#include "CXSocketServerKernel.h"
using namespace std;

//extern CXSpinLock g_lock;
//extern int g_iTotalMessage ;

namespace CXCommunication
{
    CXConnectionObject::CXConnectionObject()
    {
		m_pLogHandle = NULL;
		m_pIOStatHandle = NULL;
		m_pJournalLogHandle = NULL;
		m_pRPCObjectManager = NULL;
		m_pSocketKernel = NULL;
		m_pTaskPool = NULL;
        Reset();
    }

    CXConnectionObject::~CXConnectionObject()
    {
        //dtor
    }

    void CXConnectionObject::Reset()
    {
        m_lockRead.Lock();
        m_sock = 0;
        m_iConnectionType = 1;

        m_tmAcceptTime = 0;

        m_uiConnectionIndex = 0;

        m_uiRecvBufferIndex = 0;
        m_uiSendBufferIndex = 0;
        m_uiLastProcessBufferIndex = 0;

        m_nState = INITIALIZED;

        //last packet time
        m_tmLastPacketTime = 0;
        m_strRemoteIP = "";
        m_wRemotePort = 0;

        m_lpServer = NULL;
        m_lpCacheObj = NULL;

        m_iObjectSizeInCache = 0;
        m_iDispacherQueueIndex = 0;

        m_pListReadBuf=NULL;
        m_pListReadBufEnd = NULL;
        m_pListSendBuf = NULL;
        m_pListSendBufEnd = NULL;
        m_pfnProcessOnePacket = NULL;

        // the number of the received buffers in the receiving buffer list
        m_uiNumberOfReceivedBufferInList = 0;

        // the number of the received packets in the message queue
        m_uiNumberOfReceivedPacketInQueue = 0;

		m_uiLastProcessedMessageSequenceNum = 0;

        // the number of the buffer by posting to iocp model
        m_uiPostedSentBuffers = 0;

        m_uiPostedRecvBuffers = 0;
 
		m_iLastUnpackedMessageSequenceNum = 0;

        m_pSessionsManager = NULL;

        m_pSession = NULL;

        m_pDataParserHandle = NULL;

        //60 seconds
        m_dwTimeOutMSeconds = 60000;

        m_tmReleasedTime = 0;

		// the  header of the unpacked message list
		m_plstMessageHead =NULL;

		// the end of the unpacked message list
		m_plstMessageEnd=NULL;

		m_uiLastSentPacketSequenceNum = 0;

		m_uiCurSendingPacketSequenceNum = 1;

        m_encryptType = CXDataParserImpl::CXENCRYPT_TYPE_NONE;
        m_compressType = CXDataParserImpl::CXCOMPRESS_TYPE_NONE;

        m_bProxyConnection = false;

		// the header of the inner message list
		m_plstInnerMessageHead = NULL;

		// the end of the inner message list
		m_plstInnerMessageEnd = NULL;

		m_bAsyncParseData = false;

		//use task pool to process the data needed to send, compress and encrypt it asynchronously
		m_bAsyncPrepareData = false;

		// this message had been processed asynchronously
		m_bAsynchronouslyMes = false;

		m_bContinuousProcessing = false;

		memset(m_byObjectID,0, CX_GUID_LEN);
		memset(m_byRequestID, 0, CX_GUID_LEN);
        m_lockRead.Unlock();
    }

    void CXConnectionObject::Lock()
    {
        m_lock.Lock();
    }
    void CXConnectionObject::UnLock()
    {
        m_lock.Unlock();
    }

    void CXConnectionObject::Build(cxsocket sock,uint64 uiConnIndex, const string &strRemoteIp, WORD wRemotePort)
    {
        Lock();
        Reset();
        m_sock = sock;

        m_tmAcceptTime = GetCurrentTimeMS();

        m_uiConnectionIndex = uiConnIndex;

		if (sock != 0) //this socket have not been created, is creating
		{
			m_nState = PENDING;
		}

        m_strRemoteIP = strRemoteIp;
        m_wRemotePort = wRemotePort;
        UnLock();
    }

	void CXConnectionObject::OnCreated(cxsocket sock)
	{
		Lock();
		m_sock = sock;
		m_tmAcceptTime = GetCurrentTimeMS();
		m_nState = ESTABLISHED;
		UnLock();
	}

    void CXConnectionObject::RecordCurrentPacketTime()
    {
        m_tmLastPacketTime = GetCurrentTimeMS();
    }

    //push the received buffer to the end of the read buffer list
	bool CXConnectionObject::PushReceivedBuffer(PCXBufferObj pBufObj)
    {
        if (pBufObj != NULL)
        {
            PCXBufferObj pCurBuf = NULL;
            PCXBufferObj pPrevBuf = NULL;

			//record the time of the last received packet
            RecordCurrentPacketTime();

			pBufObj->pNext = NULL;
			

			//if the read buffer list is empty, add the current received packet to the end;
			if (m_pListReadBuf == NULL && m_pListReadBufEnd == NULL)
			{
				m_pListReadBuf = m_pListReadBufEnd = pBufObj;
				pBufObj->pNext = NULL;
			}
			//if the sequence number of the current received packet is larger than the sequence number 
			//of the last packet in the read list, add the current received packet to the end of the read list;
			else if (m_pListReadBufEnd != NULL && (m_pListReadBufEnd->nSequenceNum < pBufObj->nSequenceNum))
			{
				m_pListReadBufEnd->pNext = pBufObj;
				m_pListReadBufEnd = pBufObj;
				if (m_pListReadBuf == NULL)
				{
					char szInfo[1024] = { 0 };
					sprintf_s(szInfo, 1024, "The read list had an error, the end pointer is not empty, but the header pointer is empty\n");
					m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
					return false;
				}
			}
			//if the sequence number of the current received packet is less than the sequence number 
			//of the first packet in the read list, add the current received packet to the header of the read list;
            else if (m_pListReadBuf != NULL && (m_pListReadBuf->nSequenceNum > pBufObj->nSequenceNum))
            {
                pBufObj->pNext = m_pListReadBuf;
                m_pListReadBuf = pBufObj;
                if (m_pListReadBufEnd == NULL)
                {
					char szInfo[1024] = { 0 };
					sprintf_s(szInfo, 1024, "The read list had an error, the header pointer is not empty, but the end pointer is empty\n");
					m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
					return false;
                }
            }
            else //add the current received packet to the middle of the read list
            {
                pCurBuf = m_pListReadBuf->pNext;
                pPrevBuf = m_pListReadBuf;
                while (pCurBuf)
                {
                    if (pCurBuf->nSequenceNum > pBufObj->nSequenceNum)
                    {
                        pPrevBuf->pNext = pBufObj;
                        pBufObj->pNext = pCurBuf;
                        break;
                    }
                    pPrevBuf = pCurBuf;
                    pCurBuf = pCurBuf->pNext;
                }
            }

			//increase the packet number in the read list,
			//reduce the packet number that has been posted.
            m_lockRead.Lock();
            m_uiNumberOfReceivedBufferInList++;
            m_uiPostedRecvBuffers--;
            m_lockRead.Unlock();
			return true;
        }
		else
		{
			return false;
		}
    }

    
    //receive a buffer, add it to the read buffer list
    //parse the read buffer list, restructure the message packet
	//return value: ==-2 the read list has some error
	//              ==-3 occur error when allocat memory
	//              ==-4 received a incorrect packet
    int CXConnectionObject::RecvPacket(PCXBufferObj pBufObj, DWORD dwTransDataOfBytes,bool bLockBySelf)
    {
		if (pBufObj == NULL || m_lpServer ==NULL )
		{
			return INVALID_PARAMETER;
		}

        if(bLockBySelf)
            m_lock.Lock();

        //sprintf_s(szInfo, 1024, "Receive buffer,buffer index = %I64i,datalen = %d,pBufObj=%x\n", pBufObj->nSequenceNum,
        //    dwTransDataOfBytes, pBufObj);
        //m_pLogHandle->Log(CXLog::CXLOG_INFO, szInfo);
        //printf_s(szInfo);

		if (!PushReceivedBuffer(pBufObj))
		{
			if (bLockBySelf)
				m_lock.Unlock();
			return -2;
		}

        //save the header data
        CXPacketHeader packetHeader;
        memset(&packetHeader, 0, sizeof(CXPacketHeader));
        byte *pPacketHeader = (byte*)&packetHeader;
        const int iHeaderLen = sizeof(CXPacketHeader);

        PCXMessageData pMessageData = NULL;


		PCXBufferObj pCurBuf = NULL;
		PCXBufferObj pPrevBuf = NULL;

		bool bAllcateMemoryFails = false;
		char szInfo[1024] = { 0 };
        bool bFindErrorPacket = false;
		int  iUsedLen = 0;
        uint64 uiPacketIndex = m_uiLastProcessBufferIndex;
        pCurBuf = m_pListReadBuf;

        //byte byLastByte = 0;
        while (pCurBuf)
        {
            if (pCurBuf->nSequenceNum != (uiPacketIndex+1))//is not the next data buffer
            {
                break;
            }

			int iLeftLen = pCurBuf->wsaBuf.len;
			int iPacketLen = 0;
			int iNeedCopyLen = 0;
			int iBeginPointInBuffer = pCurBuf->nCurDataPointer;

            if (iLeftLen<=0) //the empty buffer
            {
                uiPacketIndex++;
                pCurBuf = pCurBuf->pNext;
                continue;
            }

			//copy the header data to pPacketHeader
            if(iUsedLen<iHeaderLen)
            {
                iNeedCopyLen = iHeaderLen - iUsedLen;
                if (iLeftLen <iNeedCopyLen)//not have a complete header
                {
                    iNeedCopyLen = iLeftLen;
                }

                memcpy(pPacketHeader + iUsedLen, pCurBuf->wsaBuf.buf + iBeginPointInBuffer,
                    iNeedCopyLen);

                iUsedLen += iNeedCopyLen;
                iLeftLen -= iNeedCopyLen;
                iBeginPointInBuffer += iNeedCopyLen;

				//if is enough complete header, find next buffer
                if (iUsedLen<iHeaderLen)
                {
                    pCurBuf = pCurBuf->pNext;
                    uiPacketIndex++;
                    continue;
                }
            }


			//a wrong packet,the magic number of the header is not CX_PACKET_HEADER_FLAG
			if (packetHeader.wFlag != CX_PACKET_HEADER_FLAG )
			{
				sprintf_s(szInfo, 1024, "Received a error packet,the magic number is unmatched, connection id = %lld,packet sequence number:%lld\n",
					m_uiConnectionIndex, pCurBuf->nSequenceNum);
				bFindErrorPacket = true;
				m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
				break;
			}

            //a wrong header data
            //wrong packet , the packet size must be less than 1MB
            if (packetHeader.dwDataLen > CX_MAX_CHACHE_SIZE)
            {
                sprintf_s(szInfo, 1024, "Received a error packet,the size of the packet is larger than 10MB ,connection id = %lld,packet sequence number:%lld\n",
                    m_uiConnectionIndex, pCurBuf->nSequenceNum);
				bFindErrorPacket = true;
				m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
				break;
            }
            int64 iBeginTime = GetCurrentTimeMS();
            int iBufferLen = sizeof(CXMessageData)- 1 + packetHeader.dwDataLen;
			if (pMessageData == NULL)
			{
				pMessageData = (PCXMessageData)GetBuffer(iBufferLen);
				if (pMessageData == NULL)
				{
					sprintf_s(szInfo, 1024, "Failed to get %d-bytes buffer  ,This :%x, Cache address:%x\n",
						iBufferLen,(void*)this, (void*)m_lpCacheObj);
					m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
					printf(szInfo);

					bAllcateMemoryFails = true;
					break;
				}
				pMessageData->pConObj = (void*)this;
				pMessageData->dwType = 1;
				pMessageData->dwDataLen = packetHeader.dwDataLen;
				pMessageData->iBeginTime = iBeginTime;
				pMessageData->iSequenceNum = 0;
				pMessageData->iBeginProcessTime = iBeginTime;
				pMessageData->pNext = NULL;
			}

            byte *pBodyData = (byte*)&pMessageData->bodyData;
            iPacketLen = packetHeader.dwDataLen + sizeof(CXPacketHeader);
			iNeedCopyLen = iPacketLen - iUsedLen;
			
			if (iLeftLen > 0 && iNeedCopyLen>0)
			{
				if (iLeftLen < iNeedCopyLen)
					iNeedCopyLen = iLeftLen;

				int iBeginOffset = iUsedLen - iHeaderLen;
				memcpy(pBodyData + iBeginOffset, pCurBuf->wsaBuf.buf + iBeginPointInBuffer, iNeedCopyLen);
				iUsedLen += iNeedCopyLen;
				iBeginPointInBuffer += iNeedCopyLen;
			}

            if (iUsedLen < iPacketLen) //not have a complete packet
            {
                pCurBuf = pCurBuf->pNext;
                uiPacketIndex++;
                continue;
            }

			m_iLastUnpackedMessageSequenceNum++;
			pMessageData->iSequenceNum = m_iLastUnpackedMessageSequenceNum;

            m_encryptType = (CXDataParserImpl::CXENCRYPT_TYPE)packetHeader.byEncryptFlag;
            m_compressType = (CXDataParserImpl::CXCOMPRESS_TYPE)packetHeader.byCompressFlag;

			pCurBuf->wsaBuf.len -= (iBeginPointInBuffer - pCurBuf->nCurDataPointer);
			pCurBuf->nCurDataPointer = iBeginPointInBuffer;
			if (pCurBuf->wsaBuf.len == 0)// not left any data, free it
			{
				pCurBuf = pCurBuf->pNext;
			}

			PCXBufferObj pNeedDeleteBuf = m_pListReadBuf;
			while (pNeedDeleteBuf && pNeedDeleteBuf != pCurBuf)
			{
				m_pListReadBuf = pNeedDeleteBuf->pNext;
				//sprintf_s(szInfo, 1024, "Free Buffer ,buffer address = %x,buffer index = %I64i, \n\n", pNeedDeleteBuf,pNeedDeleteBuf->nSequenceNum);
				//m_pLogHandle->Log(CXLog::CXLOG_INFO, szInfo);
				//printf_s(szInfo);

				FreeCXBufferObj(pNeedDeleteBuf);

				m_lockRead.Lock();
				m_uiLastProcessBufferIndex++;
				m_uiNumberOfReceivedBufferInList--;
				m_lockRead.Unlock();

				//g_lock.Lock();
				//g_iTotalProcessPacket++;
				//g_lock.Unlock();

				pNeedDeleteBuf = m_pListReadBuf;
			}

			if (pCurBuf == NULL)
			{
				m_pListReadBuf = m_pListReadBufEnd = NULL;
			}

			if (bLockBySelf)
				m_lock.Unlock();

            if (VerifyPacketBodyData(&packetHeader, pBodyData))
            {
				if ((CXDataParserImpl::CXENCRYPT_TYPE)packetHeader.byEncryptFlag != CXDataParserImpl::CXENCRYPT_TYPE_NONE ||
					(CXDataParserImpl::CXCOMPRESS_TYPE)packetHeader.byCompressFlag != CXDataParserImpl::CXCOMPRESS_TYPE_NONE)
				{
					if (m_pDataParserHandle == NULL)// need to parse data
					{
						FreeBuffer(pMessageData);
						pMessageData = NULL;
						return -6;
					}

					//using task pool
					if (m_pTaskPool != NULL && m_bAsyncParseData)
					{
						CXTaskPool* pTaskPool = (CXTaskPool*)m_pTaskPool;
						CXTaskParseMessage *pTask = (CXTaskParseMessage *)pTaskPool->GetFreeTaskObject(
							CXTaskBase::CX_RUNING_TASK_TYPE_PARSE_DATA);
						if (pTask == NULL)
						{
							sprintf_s(szInfo, 1024, "Failed to new a message parser object \n");
							m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
							bAllcateMemoryFails = true;
							break;
						}

						pTask->SetTaskInfo(pMessageData, packetHeader);
						pTaskPool->PushTask(pTask);
					}
					else
					{
						if (!ParseMessage(pMessageData, packetHeader))
						{
							FreeBuffer(pMessageData);
							pMessageData = NULL;
							return -5;
						}
					}
				}
				else
				{
					CXGuidObject guidObj(false);
					string strRequestID = guidObj.ConvertGuid(pMessageData->bodyData.byRequestID);
					if (m_pIOStatHandle != NULL)
					{
						if (!IsProxyConnection())
						{
                            m_pIOStatHandle->BeginIOStat("total", strRequestID, pMessageData->iBeginTime);
                        }
					}
                    PushReceivedMessage(pMessageData);
				}
            }
            else // the packet is wrong packet, break it ,and close this connection
            {
                sprintf_s(szInfo, 1024, "Received a error packet,verify the packet data failed,connection id = %lld,packet sequence number:%lld \n",
                    m_uiConnectionIndex, pCurBuf->nSequenceNum);
                m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);

                FreeBuffer(pMessageData);
                pMessageData = NULL;

                //bFindErrorPacket = true;
                //break;
				return -4;
            }

			if (bLockBySelf)
				m_lock.Lock();
			uiPacketIndex = m_uiLastProcessBufferIndex;
			pMessageData = NULL;
			memset(pPacketHeader, 0, sizeof(CXPacketHeader));
            iUsedLen = 0;
            continue;
        }
        if(bLockBySelf)
            m_lock.Unlock();

        if(pMessageData !=NULL)
            FreeBuffer(pMessageData);

        if (bAllcateMemoryFails)
        {
            return -3;
        }

		if (bFindErrorPacket)
		{
			return -4;
		}

        return RETURN_SUCCEED;
    }

	//push received message to message queue
	void CXConnectionObject::PushReceivedMessage(PCXMessageData pMes, bool bAddRes)
	{
		if (m_pfnProcessOnePacket != NULL)
		{
            if(bAddRes)
                AddReceivedPacketNumber();
			//g_lock.Lock();
			//g_iTotalMessage++;
			//g_lock.Unlock();
			//m_uiNumberOfReceivedPacketInQueue++;
			m_pfnProcessOnePacket(*this, pMes);
		}
		else
		{
			OnProcessOnePacket(*this, pMes);
		}
	}

    bool CXConnectionObject::VerifyPacketBodyData(PCXPacketHeader pHeader, byte *pData)
    {
        DWORD dwCheckSum = 0;
        for (DWORD i = 0; i<pHeader->dwDataLen; i++)
        {
            dwCheckSum += (unsigned int)pData[i];
        }
        if (dwCheckSum != pHeader->dwCheckSum)
        {
            return false;
        }

        return true;
    }

	bool CXConnectionObject::ParseMessage(PCXMessageData pMessageData, CXPacketHeader &packetHeader)
	{
		if (pMessageData == NULL || m_lpServer == NULL)
		{
			if(pMessageData)
				FreeBuffer(pMessageData);
			return false;
		}

		char szInfo[1024] = { 0 };	
		CXCommunicationServer *pComServer = (CXCommunicationServer *)m_lpServer;

		int iNewBufferLen = packetHeader.dwOrignDataLen*2;
		//using the rsa encryption , the buffer size must be larger than 256 bytes
		if (iNewBufferLen < 256)
		{
			iNewBufferLen = 256;
		}
        bool bUseThreadCache = true;
		byte* pbyFirstCache = pComServer->GetFirstThreadCache(iNewBufferLen);
        if (pbyFirstCache == NULL)
        {
            pbyFirstCache = (byte*)GetBuffer(iNewBufferLen);
            if (pbyFirstCache == NULL)
            {
                sprintf_s(szInfo, 1024, "Failed to get %d-bytes thread cache to parse data  ,This :%x, Cache address:%x\n",
                    iNewBufferLen, (void*)this, (void*)m_lpCacheObj);
                m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
                FreeBuffer(pMessageData);
                return false;
            }
            else
            {
                bUseThreadCache = false;
            }
        }
		
		//the rsa encryption use not less than 256 bytes buffer
		DWORD dwOutputBufLen = packetHeader.dwOrignDataLen+sizeof(CXMessageData)-1;
		if (dwOutputBufLen < 512)
		{
			dwOutputBufLen = 512;
		}
		else
		{
			//left not less than 256 bytes buffer
			dwOutputBufLen += 256;
		}
		PCXMessageData pNewMes = (PCXMessageData)GetBuffer(dwOutputBufLen);
		if (pNewMes == NULL)
		{
			sprintf_s(szInfo, 1024, "Failed to get %d-bytes buffer to parse data  ,This :%x, Cache address:%x\n",
				packetHeader.dwOrignDataLen, (void*)this, (void*)m_lpCacheObj);
			m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
			FreeBuffer(pMessageData);
            if(!bUseThreadCache)
                FreeBuffer(pbyFirstCache);
			return false;
		}
		pNewMes->pConObj = (void*)this;
		pNewMes->dwType = 1;
		pNewMes->dwDataLen = packetHeader.dwOrignDataLen;
		pNewMes->iBeginTime = pMessageData->iBeginTime;
		pNewMes->iBeginProcessTime = pMessageData->iBeginTime;
		pNewMes->iSequenceNum = pMessageData->iSequenceNum;
		pNewMes->pNext = NULL;
		DWORD dwRetLen = 0;
		dwOutputBufLen = dwOutputBufLen-(sizeof(CXMessageData) - 1);

		byte *pBodyData = (byte*)&pMessageData->bodyData;

		bool bParseRet = m_pDataParserHandle->ParseData((CXDataParserImpl::CXENCRYPT_TYPE)packetHeader.byEncryptFlag,
			(CXDataParserImpl::CXCOMPRESS_TYPE)packetHeader.byCompressFlag,
			pBodyData, pMessageData->dwDataLen,
			(byte*)&pNewMes->bodyData, dwOutputBufLen,
			pbyFirstCache, iNewBufferLen, dwRetLen);
		if (!bParseRet || dwRetLen != packetHeader.dwOrignDataLen)
		{
			sprintf_s(szInfo, 1024, "Received a error packet,an error occur in parsing the packet,connection id = %lld,packet sequence number:%lld \n",
				m_uiConnectionIndex, pMessageData->iSequenceNum);
			m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);

			//FreeBuffer(pMessageData);
			//pMessageData = NULL;
            if (!bUseThreadCache)
                FreeBuffer(pbyFirstCache);
			FreeBuffer(pMessageData);
			FreeBuffer(pNewMes);

			return false;
		}
		else
		{
            if (!bUseThreadCache)
                FreeBuffer(pbyFirstCache);
			FreeBuffer(pMessageData);
			pMessageData = NULL;
			pMessageData = pNewMes;

			sprintf_s(szInfo, 1024, "Received a packet,SequenceNum:%lld, connection index:%lld,pCurMes:%x\n",
				m_uiConnectionIndex, pMessageData->iSequenceNum, pMessageData);
			//m_pLogHandle->Log(CXLog::CXLOG_DEBUG, szInfo);
		}

		CXGuidObject guidObj(false);
		string strRequestID = guidObj.ConvertGuid(pMessageData->bodyData.byRequestID);

        if (!IsProxyConnection())
        {
            m_pIOStatHandle->BeginIOStat("total", strRequestID, pMessageData->iBeginTime);
        }

        PushReceivedMessage(pMessageData);
		return true;
	}

    int  CXConnectionObject::OnProcessOnePacket(CXConnectionObject &conObj, PCXMessageData pMes)
    {
        return RETURN_SUCCEED;
    }

    int CXConnectionObject::PostSend(byte *pData, int iDataLen)
    {
        if (pData==NULL || iDataLen <= 0 || iDataLen>1024 * 1024)
            return INVALID_PARAMETER;

        if (m_nState == CLOSING || m_nState == CLOSED) //closing or closed
        {
            return -4;
        }

        PCXBufferObj pBufObj = GetCXBufferObj();
        if (pBufObj == NULL)
        {
            return -2;
        }

        pBufObj->nOperate = OP_WRITE;

        m_lock.Lock();
#ifdef WIN32
        DWORD dwSend = 0;
        m_uiPostedSentBuffers++;
        if (WSASend(m_sock, &pBufObj->wsaBuf, 1, &dwSend, 0, &pBufObj->ol, NULL) != 0)
        {
            DWORD dwEr = ::WSAGetLastError();
            if (dwEr != WSA_IO_PENDING)
            {
                m_uiPostedSentBuffers--;
                m_lock.Unlock();
                FreeCXBufferObj(pBufObj);
                return -3;
            }
        }
        //

#endif // WIN32

        m_lock.Unlock();
        return RETURN_SUCCEED;
    }

    int CXConnectionObject::PostSend(PCXBufferObj pBufObj, bool bLockBySelf)
    {
        if (pBufObj == NULL || m_pSocketKernel ==NULL || pBufObj->wsaBuf.len <= 0 || pBufObj->wsaBuf.len>1024 * 1024)
            return INVALID_PARAMETER;

        if (m_nState == CLOSING || m_nState == CLOSED) //closing or closed
        {
            return -4;
        }

		pBufObj->nOperate = OP_WRITE;

		if(bLockBySelf)
		    m_lockSend.Lock();

		if (!PushSendBuffer(pBufObj))
		{
			FreeCXBufferObj(pBufObj);
			if (bLockBySelf)
				m_lockSend.Unlock();
			return -2;
		}

		if (m_nState != ESTABLISHED) //creating or pending
		{
			return RETURN_SUCCEED;
		}

		int iRet = SendDataList();

		if (bLockBySelf)
			m_lockSend.Unlock();

		return iRet;
    }

    int CXConnectionObject::PostSendBlocking(PCXBufferObj pBufObj, DWORD &dwSendLen, bool bLockBySelf)
    {
        if (pBufObj == NULL || pBufObj->wsaBuf.len <= 0 || pBufObj->wsaBuf.len>1024 * 1024)
            return INVALID_PARAMETER;

        if (m_nState == CLOSING || m_nState == CLOSED) //closing or closed
        {
            return -4;
        }

        pBufObj->nOperate = OP_WRITE;

        //if(bLockBySelf)
        //    m_lockSend.Lock();
#ifdef WIN32
        if (WSASend(m_sock, &pBufObj->wsaBuf, 1, &dwSendLen, 0, NULL, NULL) != 0)
        {
            DWORD dwEr = ::WSAGetLastError();
            //if (bLockBySelf)
            //    m_lockSend.Unlock();
            return -3;
        }
#else
        int iRet = SendData(pBufObj->wsaBuf.buf, pBufObj->wsaBuf.len,dwSendLen, 0);
        if (iRet!=0 && iRet!=-1)
        {
            DWORD dwEr = errno;
            //if (bLockBySelf)
            //    m_lockSend.Unlock();
            return -3;
        }

#endif // WIN32
        //if (bLockBySelf)
        //    m_lockSend.Unlock();
        return RETURN_SUCCEED;
    }

    int CXConnectionObject::PostRecv(int iBufSize)
    {
        #ifndef WIN32
        return 0;
        #endif // WIN32

        if (iBufSize<=0 || iBufSize>1024*1024)
            return INVALID_PARAMETER;


        if (m_nState == CLOSING || m_nState == CLOSED) //closing or closed
        {
            return -4;
        }

        PCXBufferObj pBufObj = GetCXBufferObj();
        if (pBufObj == NULL)
        {
            return -2;
        }

        m_lockRead.Lock();
        pBufObj->nSequenceNum = ++m_uiRecvBufferIndex;
        pBufObj->nOperate = OP_READ;
        //pBufObj->wsaBuf.len = 200;

#ifdef WIN32
        DWORD dwRecv = 0;
        DWORD dwFlag = 0;
        m_uiPostedRecvBuffers++;
        if (WSARecv(m_sock, &pBufObj->wsaBuf, 1,
            &dwRecv, &dwFlag, &pBufObj->ol, NULL) != 0)
        {
            DWORD dwEr = ::WSAGetLastError();
            if (dwEr != WSA_IO_PENDING)
            {
                m_uiPostedRecvBuffers--;
                m_uiRecvBufferIndex--;
                m_lockRead.Unlock();
                FreeCXBufferObj(pBufObj);
                return -3;
            }
        }

#endif // WIN32
        m_lockRead.Unlock();
        return RETURN_SUCCEED;
    }

    PCXBufferObj CXConnectionObject::GetCXBufferObj(DWORD dwBufSize)
    {
        PCXBufferObj pBufObj = (PCXBufferObj)m_lpCacheObj->GetBuffer(dwBufSize);
        if (pBufObj == NULL)
        {
            return NULL;
        }

        pBufObj->pConObj = (void*)this;
        pBufObj->wsaBuf.len = CX_BUF_SIZE;
        pBufObj->wsaBuf.buf = pBufObj->buf;
        pBufObj->iBufSize = CX_BUF_SIZE;
        return pBufObj;
    }


    void  CXConnectionObject::AddReceivedPacketNumber()
    {

        m_lockRead.Lock();
        m_uiNumberOfReceivedPacketInQueue++;
        m_lockRead.Unlock();
    }

    void  CXConnectionObject::ReduceReceivedPacketNumber()
    {

        m_lockRead.Lock();
        m_uiNumberOfReceivedPacketInQueue--;
        m_lockRead.Unlock();
    }

    void  CXConnectionObject::ReduceNumberOfPostedRecvBuffers()
    {
        m_lockRead.Lock();
        m_uiPostedRecvBuffers--;
        m_lockRead.Unlock();
    }

    void  CXConnectionObject::ReduceNumberOfPostedSentBuffers()
    {
        m_lockSend.Lock();
        m_uiPostedSentBuffers--;
        m_lockSend.Unlock();
    }

    void CXConnectionObject::Close(bool bLockBySelf)
    {
        if (bLockBySelf)
            Lock();

        bool bNeedProcessLeftBuffers = false;
        LockRead();
        if (m_nState < CLOSING)
        {
            closesocket(m_sock);
            m_nState = CLOSING;
        }
        
        UnlockRead();

        //process the left buffer in the read list
        if (bNeedProcessLeftBuffers)
        {
            //RecvPacket(NULL, 0,false);
        }

        if (m_uiPostedSentBuffers == 0 && m_pListReadBuf != NULL)
        {
            //if miss some fragment of the packets
            //free this read buffer list
            PCXBufferObj pCurBuf = m_pListReadBuf;
            while (pCurBuf)
            {
                m_pListReadBuf = pCurBuf->pNext;
                FreeCXBufferObj(pCurBuf);
                pCurBuf = m_pListReadBuf;
            }
            LockRead();
            m_uiNumberOfReceivedBufferInList = 0;
            UnlockRead();
        }

        if (bLockBySelf)
            UnLock();
    }

    //have lock by CXConnectionObject::Lock();
    int  CXConnectionObject::RecvData(PCXBufferObj *ppBufObj, DWORD &dwReadLen)
    {
        int nFlags = 0;
        int nReadLen = 0;
        dwReadLen = 0;

        if (m_nState == CLOSING || m_nState == CLOSED) //closing or closed
        {
            return -4;
        }

        PCXBufferObj pBufObj = GetCXBufferObj();
        if (pBufObj == NULL)
        {
            return -2;
        }

        m_lockRead.Lock();
        pBufObj->nSequenceNum = ++m_uiRecvBufferIndex;
        pBufObj->nOperate = OP_READ;

        int nRet = 0;

        int nRecv = 0;
        int nTotalRead = 0;
        bool bReadOk = false;
        int  nLeftBufLen = pBufObj->iBufSize;
        while (nLeftBufLen>0)
        {
            // must comfirm the sockCur is nonblocking.
            nRecv = recv(m_sock, pBufObj->wsaBuf.buf + nTotalRead, nLeftBufLen, nFlags);
            if (nRecv < 0)
            {
                if (errno == EAGAIN)//in this case, that is not left any data in the network buffer.
                {
                    bReadOk = true;
                    nRet = -1;
                    break;
                }
                else if (errno == ECONNRESET)//receive the RST packet from the peer
                {

                    //Close();
                    nRet = -2;
                    break;
                }
                else if (errno == EINTR)//interrupt by other event
                {
                    continue;
                }
                else // other error
                {
                    //Close();
                    nRet = -2;
                    break;
                }
            }
            else if (nRecv == 0) //the peer had closed the socket normally and had sent the FIN packet.
            {
                //Close();
                nRet = -3;
                break;
            }
            else// recvNum > 0
            {
                nTotalRead += nRecv;
                nLeftBufLen -= nRecv;
                if (nLeftBufLen == 0)
                {
                    bReadOk = true;
                    break;
                }
            }
        }

        dwReadLen = pBufObj->wsaBuf.len = nTotalRead;
        if (nTotalRead>0)
        {
            *ppBufObj = pBufObj;
            m_uiPostedRecvBuffers++;
        }
        else
        {
            --m_uiRecvBufferIndex;
        }
        m_lockRead.Unlock();

        if(nTotalRead == 0)// not receive any data
        {
            *ppBufObj = NULL;
            FreeCXBufferObj(pBufObj);
        }
        return nRet;
    }


    //have lock by CXConnectionObject::Lock();
    int  CXConnectionObject::SendData(char *pBuf,
        int nBufLen, DWORD &dwSendLen, int nFlags)
    {

        int nRet = 0;

        bool bWritten = false;
        int nWritenLen = 0;
        int nTotalWritenLen = 0;
        int nLeftDataLen = nBufLen;

        while (nLeftDataLen>0)
        {
            // must comfirm the sockCur is nonblocking.
            nWritenLen = send(m_sock, pBuf + nTotalWritenLen, nLeftDataLen, 0);
            if (nWritenLen == -1)
            {
                if (errno == EAGAIN)// all data had been sent
                {
                    bWritten = true;
                    //nRet = -1;
                    //break;
                    continue;
                }
                else if (errno == ECONNRESET)//receive the RST packet from the peer
                {

                    //Close();
                    nRet = -2;
                    break;
                }
                else if (errno == EINTR)//interrupt by other event
                {
                    continue;
                }
                else // other error
                {

                    nRet = -2;
                    break;
                }
            }

            if (nWritenLen == 0)//the peer had closed the socket normally and had sent the FIN packet.
            {
                //Close();
                nRet = -3;
                break;
            }

            // nTotalWritenLen > 0
            nTotalWritenLen += nWritenLen;
            if (nWritenLen == nLeftDataLen)
            {
                break;
            }
            nLeftDataLen -= nWritenLen;
        }

        dwSendLen = nTotalWritenLen;

        return nRet;
    }

    //send a packet to the peer
    bool  CXConnectionObject::SendPacket(const byte* pbData, DWORD dwLen, DWORD dwMesCode, PCXMessageData pMes,bool bLockBySelf)
    {
		if (pbData == NULL|| dwLen==0 || m_lpServer == NULL)
		{
			return false;
		}

		//if (bLockBySelf)
		//	m_lockSend.Lock();

        DWORD dwPacketLen = dwLen + sizeof(CXPacketData) - 1;
        DWORD dwBufferLen = dwPacketLen + sizeof(CXBufferObj) - CX_BUF_SIZE;
        DWORD dwPacketBodyLen = dwPacketLen - sizeof(CXPacketHeader);

        PCXBufferObj pBufObj = GetCXBufferObj(dwBufferLen);
        if (pBufObj == NULL)
        {
            return false;
        }
        PCXPacketBodyData pBodyData = (PCXPacketBodyData)(pBufObj->wsaBuf.buf + sizeof(CXPacketHeader));
        memcpy(pBodyData->buf, pbData, dwLen);
        bool bRet = SendPacket(pBufObj, dwLen, dwMesCode, pMes, bLockBySelf);
		if (!bRet)
		{
			//FreeCXBufferObj(pBufObj);
		}
		return bRet;
    }

	//send a packet to the peer
	//the pBufObj will be freed inside this function, the caller not need to free it
	//pBufObj: input packet buffer, must be allocate from memory cache
	//dwLen:   the data size of the CXPacketBodyData.buf.
	//pMes:    the received message in this event from own connection, if NULL , this function will call by other object internally
    bool CXConnectionObject::SendPacket(PCXBufferObj pBufObj,DWORD dwLen, DWORD dwMesCode, 
        PCXMessageData pMes, bool bLockBySelf)
    {
        if (pBufObj == NULL || dwLen == 0 || m_lpServer == NULL)
        {
			if (pBufObj != NULL)
			{
				FreeCXBufferObj(pBufObj);
			}
            return false;
        }

		//if (bLockBySelf)
		//	m_lockSend.Lock();

        PCXPacketBodyData pBodyData = (PCXPacketBodyData)(pBufObj->wsaBuf.buf + sizeof(CXPacketHeader));
        pBodyData->dwMesCode = dwMesCode;
        pBodyData->dwPacketNum = m_uiSendBufferIndex++;
        if (pMes != NULL)//this message is null, show that this packet is a third-party packet
        {
            memcpy(pBodyData->byObjectGuid, pMes->bodyData.byObjectGuid, CX_GUID_LEN);
            memcpy(pBodyData->byRequestID, pMes->bodyData.byRequestID, CX_GUID_LEN);
        }
		else
		{
			memcpy(pBodyData->byObjectGuid, m_byObjectID, CX_GUID_LEN);
			memcpy(pBodyData->byRequestID, m_byRequestID, CX_GUID_LEN);
		}
        
        pBufObj->nSequenceNum = m_uiCurSendingPacketSequenceNum++;
        pBufObj->wsaBuf.len = dwLen + sizeof(CXPacketData) - 1;

        DWORD dwPacketLen = pBufObj->wsaBuf.len;
        DWORD dwPacketBodyLen = dwPacketLen - sizeof(CXPacketHeader);
        DWORD dwOrignDataLen = dwPacketBodyLen;

		if (m_encryptType != CXDataParserImpl::CXENCRYPT_TYPE_NONE ||
			m_compressType != CXDataParserImpl::CXCOMPRESS_TYPE_NONE)
		{
			if (m_pDataParserHandle == NULL)
			{
				FreeCXBufferObj(pBufObj);
				return false;
			}

			//using task pool
			if (m_pTaskPool != NULL && m_bAsyncPrepareData)
			{
				CXTaskPool* pTaskPool = (CXTaskPool*)m_pTaskPool;
				CXTaskPrepareMessage *pTask = (CXTaskPrepareMessage *)pTaskPool->GetFreeTaskObject(
					CXTaskBase::CX_RUNING_TASK_TYPE_PREPARE_DATA);
				if (pTask == NULL)
				{
					char szInfo[1024] = { 0 };
					sprintf_s(szInfo, 1024, "Failed to new a message parser object \n");
					m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);

					FreeCXBufferObj(pBufObj);
					return false;
				}
				m_bAsynchronouslyMes = true;
				pTask->SetTaskInfo(this, pMes, pBufObj);
				pTaskPool->PushTask(pTask);
				
				return true;
			}
			else
			{
				return PreparedAndSendPacket(pBufObj, pMes);
			}
		}
		else
		{
			return SendAssembledPacket(pBufObj, pBufObj->wsaBuf.len - sizeof(CXPacketHeader));
		}
    }

	//send the entire packet
	//the pInputBuf will be freed inside this function, the caller not need to free it
	bool CXConnectionObject::PreparedAndSendPacket(PCXBufferObj pInputBuf, PCXMessageData pMes)
	{
		DWORD dwPacketLen = pInputBuf->wsaBuf.len;
		DWORD dwPacketBodyLen = dwPacketLen - sizeof(CXPacketHeader);
		DWORD dwOrignDataLen = dwPacketBodyLen;
		
		byte * pInputData = (byte *)(pInputBuf->wsaBuf.buf + sizeof(CXPacketHeader));

		//the buffer size must be not less than 256 bytes
		DWORD dwCacheBufLen = dwPacketBodyLen * 2;
		if (dwCacheBufLen < 256)
			dwCacheBufLen = 256;

		CXCommunicationServer *pComServer = (CXCommunicationServer *)m_lpServer;

        bool bUseThreadCache = true;
        byte* pCacheBuf = pComServer->GetFirstThreadCache(dwCacheBufLen);
        if (pCacheBuf == NULL)
        {
            pCacheBuf = (byte*)GetBuffer(dwCacheBufLen);
            if (pCacheBuf == NULL)
            {
                FreeCXBufferObj(pInputBuf);
                return false;
            }
            else
            {
                bUseThreadCache = false;
            }
        }
		
		//the rsa encryption use not less than 256 bytes buffer
		DWORD dwBufferLen = dwPacketBodyLen * 2 + sizeof(CXPacketHeader) + sizeof(CXBufferObj) - CX_BUF_SIZE;
		if (dwBufferLen < 512)
			dwBufferLen = 512;
		else
		{
			dwBufferLen += 256;
		}
		PCXBufferObj pBufObj = GetCXBufferObj(dwBufferLen);
		if (pBufObj == NULL)
		{
			FreeCXBufferObj(pInputBuf);
            if(!bUseThreadCache)
                FreeBuffer(pCacheBuf);
			return false;
		}

		byte * pbyPacketBodyData = (byte*)(pBufObj->wsaBuf.buf + sizeof(CXPacketHeader));

		DWORD dwOutputBufLen = dwCacheBufLen;

		DWORD  dwPreparedDataLen = 0;
		bool bFunRet = m_pDataParserHandle->PrepareData(m_encryptType, m_compressType,
			pInputData, dwPacketBodyLen, pbyPacketBodyData, dwOutputBufLen,
			pCacheBuf, dwCacheBufLen, dwPreparedDataLen);
		if (!bFunRet)
		{
			FreeCXBufferObj(pInputBuf);
            if (!bUseThreadCache)
                FreeBuffer(pCacheBuf);
			FreeCXBufferObj(pBufObj);
			return false;
		}
        else
        {
            FreeCXBufferObj(pInputBuf);
            if (!bUseThreadCache)
                FreeBuffer(pCacheBuf);
        }

		dwPacketLen = dwPreparedDataLen + sizeof(CXPacketHeader);
		dwPacketBodyLen = dwPreparedDataLen;
		pBufObj->nSequenceNum = pInputBuf->nSequenceNum;
		pBufObj->wsaBuf.len = dwPacketLen;

		return SendAssembledPacket(pBufObj, dwOrignDataLen);
	}

	//send the assembled packet
	//the pBufObj will be freed inside this function, the caller not need to free it
	bool CXConnectionObject::SendAssembledPacket(PCXBufferObj pBufObj, DWORD dwOrignDataLen)
	{
		DWORD dwPacketBodyLen = pBufObj->wsaBuf.len - sizeof(CXPacketHeader);
		byte* pbyPacketBodyData = (byte*)(pBufObj->wsaBuf.buf + sizeof(CXPacketHeader));

		DWORD dwCheckSum = 0;
		for (DWORD i = 0; i < dwPacketBodyLen; i++)
		{
			dwCheckSum += (unsigned int)pbyPacketBodyData[i];
		}

		//send the header
		PCXPacketHeader pTcpHeader = (PCXPacketHeader)pBufObj->wsaBuf.buf;
		pTcpHeader->dwDataLen = dwPacketBodyLen;
		pTcpHeader->dwCheckSum = dwCheckSum;
		pTcpHeader->wFlag = CX_PACKET_HEADER_FLAG;
		pTcpHeader->byCompressFlag = (byte)m_compressType;
		pTcpHeader->byEncryptFlag = (byte)m_encryptType;
		pTcpHeader->dwOrignDataLen = dwOrignDataLen;

		DWORD dwSentLen = 0;
		int iRet = PostSend(pBufObj, false);
		//int iRet = PostSendBlocking(pBufObj, dwSentLen, false);
		//FreeCXBufferObj(pBufObj);
		if (iRet != 0)
		{
			return false;
		}
		return true;
	}

    void CXConnectionObject::LockSend()
    {
        m_lockSend.Lock();
    }
    void CXConnectionObject::UnlockSend()
    {
        m_lockSend.Unlock();
    }

    void CXConnectionObject::LockRead()
    {
        m_lockRead.Lock();
    }
    void CXConnectionObject::UnlockRead()
    {
        m_lockRead.Unlock();
    }

    void CXConnectionObject::RecordReleasedTime()
    {
        m_tmReleasedTime = GetCurrentTimeMS();
    }

    // get the current time
    uint64 CXConnectionObject::GetTickCountValue()
    {
        uint64 tmTime = 0;
#ifdef WIN32
        tmTime = GetTickCount64();
#else
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        gettimeofday(&tv, NULL);
        tmTime = (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
        
        return tmTime;
    }

	//pszTimeString: if not NULL,will save the time string , the format is: 2019-07-31_15:39:29
	int64 CXConnectionObject::GetCurrentTimeMS(char *pszTimeString)
	{
		typedef chrono::time_point<chrono::system_clock, chrono::milliseconds> MsClockType;
		MsClockType tp = chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now());

		if (pszTimeString != NULL)
		{
			time_t timeCur = chrono::system_clock::to_time_t(tp);
			std::strftime(pszTimeString, 60, "%Y-%m-%d_%H:%M:%S", std::localtime(&timeCur));
			char szMSTime[10] = { 0 };
			sprintf_s(szMSTime, 10, ".%03d", (int)(tp.time_since_epoch().count() % 1000));
			strcat(pszTimeString, szMSTime);
		}

		return (int64)tp.time_since_epoch().count();
	}

	//output the operation information to the journal log;
	void CXConnectionObject::OutputJournal(PCXMessageData pMes, int64 iBeginTimsMS)
	{
		if (m_pJournalLogHandle == NULL)
			return;

		int64 iEndTime = GetCurrentTimeMS();
		char szInfo[1024] = {0};
		sprintf_s(szInfo, 1024, "code:%d,length:%d,used_time:%lldms\n",
			pMes->bodyData.dwMesCode, pMes->dwDataLen, iEndTime- iBeginTimsMS);
		m_pJournalLogHandle->Log(CXLog::CXLOG_INFO,szInfo);
	}

	//if the message is not the next need-processed message, will add it in the list,
	//process the unpacked message,use the object to prcess the next message
	//pMes: maybe NULL, if need to the left message 
	int CXConnectionObject::ProcessUnpackedMessage(PCXMessageData pInputMes)
	{
		int iRet = RETURN_SUCCEED;
		char szInfo[1024] = { 0 };
		if (m_pRPCObjectManager==NULL || m_lpServer == NULL)
		{
			return INVALID_PARAMETER;
		}

		CXRPCObjectManager *pRpcManager = (CXRPCObjectManager*)m_pRPCObjectManager;
		m_lockSend.Lock();
		if (pInputMes != NULL)
		{
			if (!PushMessage(pInputMes))
			{
				m_lockSend.Unlock();
				Lock();
				CXCommunicationServer *pServer = (CXCommunicationServer *)m_lpServer;
				pServer->CloseConnection(*this, ERROR_IN_PROCESS, false);
				//ProcessConnectionError(this);
				UnLock();
				return -2;
			}
		}

		bool bNeedClose = false;
		ConnectionClosedType emClosedType = NORMAL;
		CXGuidObject guidObj(false);
		bool bAsyncCall = false;

		PCXMessageData pCurMes = m_plstMessageHead;
		if (pCurMes == NULL)
		{
			m_lockSend.Unlock();
			return iRet;
		}

        if (pInputMes != NULL)
        {
            sprintf_s(szInfo, 1024, "Input mes, iSequenceNum=%lld,last iSequenceNum=%lld,conid=%lld\n",
                pInputMes->iSequenceNum, m_uiLastProcessedMessageSequenceNum, this->GetConnectionIndex());
            //m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
        }
        else
        {
            sprintf_s(szInfo, 1024, "Input mes, iSequenceNum=%lld,last iSequenceNum=%lld,conid=%lld\n",
                pCurMes->iSequenceNum, m_uiLastProcessedMessageSequenceNum, this->GetConnectionIndex());
            //m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
        }

		while (pCurMes->iSequenceNum==(m_uiLastProcessedMessageSequenceNum+1))
		{
			int64 iBeginTimeInProcess = GetCurrentTimeMS(NULL);

            sprintf_s(szInfo, 1024, "process mes ,iSequenceNum=%lld,last iSequenceNum=%lld,conid=%lld\n",
                pCurMes->iSequenceNum, m_uiLastProcessedMessageSequenceNum, this->GetConnectionIndex());
            //m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);

			CXConnectionSession * pSession = (CXConnectionSession*)GetSession();
			string strObjectGuid = guidObj.ConvertGuid(pCurMes->bodyData.byObjectGuid);
            string strRequestID = guidObj.ConvertGuid(pCurMes->bodyData.byRequestID);

            CXRPCObjectServer * pObjectServer = NULL;
			bAsyncCall = false;
			if (strObjectGuid != "")
			{
				if (pSession != NULL)
				{
					pSession->Lock();
					pObjectServer = (CXRPCObjectServer *)pSession->FindObject(strObjectGuid, false);

                    if (pObjectServer==NULL && this->IsProxyConnection())
                    {
                        //the server generate a new object for this proxy request
                        pObjectServer = (CXRPCObjectServer *)PopRPCObj(strRequestID,false);
                        if (pObjectServer != NULL)
                        {
                            this->SetObjectID(pCurMes->bodyData.byObjectGuid);
                            pSession->RemoveObject(pObjectServer->GetObjectID(), false);
                            pSession->AddObject(strObjectGuid, (void*)pObjectServer, false);
                            ((CXRPCObjectClientInServer*)pObjectServer)->SetAlignedIDWithServer(true);
                        }
                    }
				}

				if (pObjectServer != NULL)
				{
					if (pSession != NULL)
						pSession->UnLock();
				}
				else
				{
                    //in here , strObjectGuid is the class id of a class of object
					pObjectServer = pRpcManager->GetFreeObject(strObjectGuid);
					if (pObjectServer != NULL)
					{
						if (!pObjectServer->IsUniqueInstance())
						{
                            if (pSession != NULL)
                            {
                                //change to the real object id
                                strObjectGuid = pObjectServer->GetObjectID();
                                pSession->AddObject(strObjectGuid, (void*)pObjectServer, false);
                                //change the request object guid to the read object id 
                                memcpy(pCurMes->bodyData.byObjectGuid, pObjectServer->GetObjectIDBytes(), CX_GUID_LEN);
                                sprintf_s(szInfo, 1024, "Create the object %s\n", strObjectGuid.c_str());
                                m_pLogHandle->Log(CXLog::CXLOG_INFO, szInfo);
                            }
						}
						pObjectServer->SetLogHandle(GetLogHandle());
						pObjectServer->SetJournalLogHandle(GetJournalLogHandle());
						pObjectServer->SetIOStat(GetIOStat());
                        pObjectServer->SetCommunicationServer(this->GetServer());
					}
					else //unknown object
					{
						sprintf_s(szInfo, 1024, "Not found the object %s, maybe not registered\n", strObjectGuid.c_str());
						m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);

						strObjectGuid = "{0307F567-72FC-4355-8192-9E37DC766D2E}";
						pObjectServer = pRpcManager->GetFreeObject(strObjectGuid);

						if (pObjectServer)
						{
							pObjectServer->SetLogHandle(GetLogHandle());
							pObjectServer->SetJournalLogHandle(GetJournalLogHandle());
							pObjectServer->SetIOStat(GetIOStat());
						}
					}
					if (pSession != NULL)
						pSession->UnLock();
				}


				if (pObjectServer != NULL)
				{
					m_bAsynchronouslyMes = false;
					m_bAsyncPrepareData = false;
					if (m_encryptType!= CXDataParserImpl::CXENCRYPT_TYPE_NONE 
						|| m_compressType != CXDataParserImpl::CXCOMPRESS_TYPE_NONE)
					{
						m_bAsyncPrepareData = true;
					}
                    
					iRet = pObjectServer->ProcessMessage(pCurMes);
					if (iRet != RETURN_SUCCEED)
					{
						if (iRet == CXRET_ASYNC_CALLBACK)
						{
                            bAsyncCall = true;
						}
                        else
                        {
							bNeedClose = true;
							emClosedType = ERROR_IN_PROCESS;
							printf("Process message fails\n");
                        }
					}

					// this process had gived to a asynchronous call, maybe give to third party, maybe give to a task thread.
					// the message can not been free
					if (m_bAsynchronouslyMes)
					{
						bAsyncCall = true;
					}
				}
			}
			else
			{
				sprintf_s(szInfo, 1024, "Receive a incorrect packet, the object guid is empty\n");
				m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);

				bNeedClose = true;
				emClosedType = ERROR_IN_PROCESS;
			}

			m_plstMessageHead = pCurMes->pNext;
			if (m_plstMessageHead == NULL)
			{
				m_plstMessageEnd = NULL;
			}

			// this process had gived to a asynchronous call, maybe give to third party, maybe give to a task thread.
			// the message can not been free
			if (bAsyncCall)
			{
				break;
			}

            
            uint64 uiSequenceNum = pCurMes->iSequenceNum;
            
			ReduceReceivedPacketNumber();
			m_uiLastProcessedMessageSequenceNum++;
			FreeBuffer(pCurMes);

			//char szInfo[1024] = { 0 };
			//sprintf_s(szInfo, 1024, "End to process message,sequenceNum:%lld,last sequenceNum:%lld connection index:%lld,message code:%d\n",
			//	uiSequenceNum, m_uiLastProcessedMessageSequenceNum, GetConnectionIndex(), pCurMes->bodyData.dwMesCode);
			//m_pJournalLogHandle->Log(CXLog::CXLOG_DEBUG, szInfo);

			m_lockSend.Unlock();

            if (pObjectServer != NULL && pObjectServer->IsNeedToDestroy())
            {
                if (pRpcManager != NULL)
                {
                    pObjectServer->Destroy();
                    pRpcManager->FreeObject(pObjectServer);
                }
            }

			if (bNeedClose)
			{
				Lock();
				CXCommunicationServer *pServer = (CXCommunicationServer *)m_lpServer;
				pServer->CloseConnection(*this, emClosedType, false);
				//ProcessConnectionError(this);
				UnLock();
			}
			else
			{
				if (GetState() >= CXConnectionObject::CLOSING)//closing
				{
					Lock();
					if (GetState() == CXConnectionObject::CLOSING)//closed
					{
						//ProcessConnectionError(pCon);
						char szInfo[1024] = { 0 };
						sprintf_s(szInfo, 1024, "Close a closing connection after process a message,connection id=%lld,error code is %d,desc is '%s'\n",
							GetConnectionIndex(), errno, strerror(errno));
						//m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
						CXCommunicationServer *pServer = (CXCommunicationServer *)m_lpServer;
						pServer->CloseConnection(*this, SOCKET_CLOSED, false);
					}
					UnLock();
				}
			}

            //sprintf_s(szInfo, 1024, "End to process message2,SequenceNum:%lld, connection index:%lld,pCurMes:%x\n",
            //    uiSequenceNum, GetConnectionIndex(), pCurMes);
            //m_pLogHandle->Log(CXLog::CXLOG_DEBUG, szInfo);

			m_lockSend.Lock();
			pCurMes = m_plstMessageHead;
			if (pCurMes == NULL)
			{
				break;
			}
		}

		m_lockSend.Unlock();
		return iRet;
	}

	bool CXConnectionObject::PushMessage(PCXMessageData pMes)
	{
		if (pMes != NULL)
		{
			PCXMessageData pCurMes = NULL;
			PCXMessageData pPrevMes = NULL;

			pMes->pNext = NULL;


			//if the read buffer list is empty, add the current received packet to the end;
			if (m_plstMessageHead == NULL && m_plstMessageEnd == NULL)
			{
				m_plstMessageHead = m_plstMessageEnd = pMes;
			}
			//if the sequence number of the current received packet is larger than the sequence number 
			//of the last packet in the read list, add the current received packet to the end of the read list;
			else if (m_plstMessageEnd != NULL && (m_plstMessageEnd->iSequenceNum < pMes->iSequenceNum))
			{
				m_plstMessageEnd->pNext = pMes;
				m_plstMessageEnd = pMes;
				if (m_plstMessageHead == NULL)
				{
					char szInfo[1024] = { 0 };
					sprintf_s(szInfo, 1024, "The message list had an error, the end pointer is not empty, but the header pointer is empty\n");
					m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
					return false;
				}
			}
			//if the sequence number of the current received packet is less than the sequence number 
			//of the first packet in the read list, add the current received packet to the header of the read list;
			else if (m_plstMessageHead != NULL && (m_plstMessageHead->iSequenceNum > pMes->iSequenceNum))
			{
				pMes->pNext = m_plstMessageHead;
				m_plstMessageHead = pMes;
				if (m_plstMessageEnd == NULL)
				{
					char szInfo[1024] = { 0 };
					sprintf_s(szInfo, 1024, "The message list had an error, the header pointer is not empty, but the end pointer is empty\n");
					m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
					return false;
				}
			}
			else //add the current received packet to the middle of the read list
			{
				pCurMes = m_plstMessageHead->pNext;
				pPrevMes = m_plstMessageHead;
				while (pCurMes)
				{
					if (pCurMes->iSequenceNum > pMes->iSequenceNum)
					{
						pPrevMes->pNext = pMes;
						pMes->pNext = pCurMes;
						break;
					}
					pPrevMes = pCurMes;
					pCurMes = pCurMes->pNext;
				}
			}

			return true;
		}
		else
		{
			return false;
		}
	}

	//push this buffer to in the sent list, sorted by the sequence number of the buffer
	bool   CXConnectionObject::PushSendBuffer(PCXBufferObj pBufObj)
	{
		if (pBufObj != NULL)
		{
			PCXBufferObj pCurBuf = NULL;
			PCXBufferObj pPrevBuf = NULL;

			pBufObj->pNext = NULL;

			//if the read buffer list is empty, add the current received packet to the end;
			if (m_pListSendBuf == NULL && m_pListSendBufEnd == NULL)
			{
				m_pListSendBuf = m_pListSendBufEnd = pBufObj;
			}
			//if the sequence number of the current received packet is larger than the sequence number 
			//of the last packet in the read list, add the current received packet to the end of the read list;
			else if (m_pListSendBufEnd != NULL && (m_pListSendBufEnd->nSequenceNum < pBufObj->nSequenceNum))
			{
				m_pListSendBufEnd->pNext = pBufObj;
				m_pListSendBufEnd = pBufObj;
				if (m_pListSendBuf == NULL)
				{
					char szInfo[1024] = { 0 };
					sprintf_s(szInfo, 1024, "The sent list had an error, the end pointer is not empty, but the header pointer is empty\n");
					m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
					return false;
				}
			}
			//if the sequence number of the current received packet is less than the sequence number 
			//of the first packet in the read list, add the current received packet to the header of the read list;
			else if (m_pListSendBuf != NULL && (m_pListSendBuf->nSequenceNum > pBufObj->nSequenceNum))
			{
				pBufObj->pNext = m_pListSendBuf;
				m_pListSendBuf = pBufObj;
				if (m_pListSendBufEnd == NULL)
				{
					char szInfo[1024] = { 0 };
					sprintf_s(szInfo, 1024, "The sent list had an error, the header pointer is not empty, but the end pointer is empty\n");
					m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
					return false;
				}
			}
			else //add the current received packet to the middle of the read list
			{
				pCurBuf = m_pListSendBuf->pNext;
				pPrevBuf = m_pListSendBuf;
				while (pCurBuf)
				{
					if (pCurBuf->nSequenceNum > pBufObj->nSequenceNum)
					{
						pPrevBuf->pNext = pBufObj;
						pBufObj->pNext = pCurBuf;
						break;
					}
					pPrevBuf = pCurBuf;
					pCurBuf = pCurBuf->pNext;
				}
			}

			return true;
		}
		else
		{
			return false;
		}
	}

	// send the data in the sending list
	int CXConnectionObject::SendDataList()
	{
		int nRet = 0;

		bool bNeedClose = false;
		ConnectionClosedType emClosedType = NORMAL;
		/*
		bool bLockingBySelf = false;

		
		if (m_lockSend.GetLockingCount() > 0)
		{
			bLockingBySelf = true;
			m_lockSend.Unlock();
		}
		

		m_lockSend.Lock();
		*/

		PCXBufferObj pCurBuf = m_pListSendBuf;
		if (pCurBuf == NULL)
		{
			//if(!bLockingBySelf)
			//	m_lockSend.Unlock();
			return 0;
		}

		while (pCurBuf->nSequenceNum == (m_uiLastSentPacketSequenceNum + 1))
		{

#ifdef WIN32
			DWORD dwSend = 0;
            m_uiPostedSentBuffers++;
			nRet = -1;
			if (WSASend(m_sock, &pCurBuf->wsaBuf, 1, &dwSend, 0, &pCurBuf->ol, NULL) != 0)
			{
				DWORD dwEr = ::WSAGetLastError();
				if (dwEr != WSA_IO_PENDING)
				{
					bNeedClose = true;
					emClosedType = SOCKET_CLOSED;
					nRet = -3;
                    m_uiPostedSentBuffers--;
					//m_uiLastSentPacketSequenceNum++;
					//break;
				}
			}
#else

			int nWritenLen = 0;
			int nTotalWritenLen = 0;
			int nLeftDataLen = pCurBuf->wsaBuf.len;
			char *pDataBuf = pCurBuf->wsaBuf.buf+ pCurBuf->nCurDataPointer;

			while (nLeftDataLen > 0)
			{
				// must comfirm the sockCur is nonblocking.
				nWritenLen = send(m_sock, pDataBuf + nTotalWritenLen, nLeftDataLen, 0);
				if (nWritenLen == -1)
				{
					if (errno == EAGAIN)// all data had been sent
					{
						pCurBuf->nCurDataPointer += nTotalWritenLen;
						pCurBuf->wsaBuf.len = nLeftDataLen;
						nRet = -1;
						break;
					}
					else if (errno == ECONNRESET)//receive the RST packet from the peer
					{
						//Close();
						nRet = -2;
						bNeedClose = true;
						emClosedType = SOCKET_PEER_CLOSED;
						break;
					}
					else if (errno == EINTR)//interrupt by other event
					{
						continue;
					}
					else // other error
					{
						nRet = -2;
						bNeedClose = true;
						emClosedType = SOCKET_OTHER_ERROR;
						break;
					}
				}

				if (nWritenLen == 0)//the peer had closed the socket normally and had sent the FIN packet.
				{
					//Close();
					nRet = -3;
					bNeedClose = true;
					emClosedType = SOCKET_PEER_CLOSED;
					break;
				}

				nTotalWritenLen += nWritenLen;
				if (nWritenLen == nLeftDataLen)
				{
					break;
				}
				nLeftDataLen -= nWritenLen;
			}
#endif // WIN32
			if (nRet != -1)
			{
				m_uiLastSentPacketSequenceNum++;
				m_pListSendBuf = pCurBuf->pNext;
				if (m_pListSendBuf == NULL)
				{
					m_pListSendBufEnd = NULL;
				}

				FreeCXBufferObj(pCurBuf);
			}
			//add the buffer to the list
			//if appear EAGIN, this function will call by the epollout event again
			else
			{
#ifdef WIN32
				
				nRet = 0;
				m_uiLastSentPacketSequenceNum++;
				m_pListSendBuf = pCurBuf->pNext;
				if (m_pListSendBuf == NULL)
				{
					m_pListSendBufEnd = NULL;
				}
#else
				CXSocketServerKernel *pKernel = (CXSocketServerKernel*)m_pSocketKernel;
				if (!pKernel->SetWaitWritingEvent(*this, true))
				{
					nRet = -4;
					break;
				}
				nRet = 0;
				break;
#endif
				
			}


			//m_lockSend.Unlock();

			if (bNeedClose)
			{
				Lock();
				CXCommunicationServer *pServer = (CXCommunicationServer *)m_lpServer;
				pServer->CloseConnection(*this, emClosedType, false);
				//ProcessConnectionError(this);
				UnLock();
			}
			else
			{
				if (GetState() >= CXConnectionObject::CLOSING)//closing
				{
					Lock();
					if (GetState() == CXConnectionObject::CLOSING)//closed
					{
						//ProcessConnectionError(pCon);
						char szInfo[1024] = { 0 };
						sprintf_s(szInfo, 1024, "Close a closing connection after process a message,connection id=%lld,error code is %d,desc is '%s'\n",
							GetConnectionIndex(), errno, strerror(errno));
						//m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
						CXCommunicationServer *pServer = (CXCommunicationServer *)m_lpServer;
						pServer->CloseConnection(*this, SOCKET_CLOSED, false);
					}
					UnLock();
				}
			}

			//m_lockSend.Lock();

			pCurBuf = m_pListSendBuf;
			if (pCurBuf == NULL)
			{
				break;
			}
		}
		//if (!bLockingBySelf)
		//	m_lockSend.Unlock();
		return nRet;
	}

    //get a sending buffer to write data, use it to send 
    PCXBufferObj CXConnectionObject::GetSendCXBufferObj(DWORD dwSize)
    {
        PCXBufferObj pBufObj = NULL;
		DWORD dwBufSize = dwSize + (sizeof(CXBufferObj) - CX_BUF_SIZE) + sizeof(CXPacketData) - 1;
		/*
        if (m_encryptType != CXDataParserImpl::CXENCRYPT_TYPE_NONE ||
            m_compressType != CXDataParserImpl::CXCOMPRESS_TYPE_NONE)
        {
            CXCommunicationServer *pComServer = (CXCommunicationServer *)m_lpServer;
            pBufObj = (PCXBufferObj)pComServer->GetFirstThreadCache(dwBufSize);
            if (pBufObj == NULL)
            {
                return NULL;
            }
        }
        else
        {
            pBufObj = (PCXBufferObj)m_lpCacheObj->GetBuffer(dwBufSize);
            if (pBufObj == NULL)
            {
                return NULL;
            }
        }
		*/

		pBufObj = (PCXBufferObj)m_lpCacheObj->GetBuffer(dwBufSize);
		if (pBufObj == NULL)
		{
			return NULL;
		}

        pBufObj->pConObj = (void*)this;
        pBufObj->wsaBuf.len = CX_BUF_SIZE;
        pBufObj->wsaBuf.buf = pBufObj->buf;
        pBufObj->iBufSize = CX_BUF_SIZE;
		return pBufObj;
    }

    void CXConnectionObject::FreeSendCXBufferObj(PCXBufferObj pBuf)
    { 
		/*
        CXCommunicationServer *pComServer = (CXCommunicationServer *)m_lpServer;
        PCXBufferObj pBufObj = (PCXBufferObj)pComServer->GetFirstThreadCache(1);
        if (pBufObj == pBuf)
        {
            return ;
        }
		*/
        m_lpCacheObj->FreeBuffer(pBuf); 
    }

    //get the writable data pointer in the CXBufferObj , use to write data
    byte* CXConnectionObject::GetWritableBuffer(PCXBufferObj pBufObj)
    {
        DWORD dwFrontSize = sizeof(CXPacketData) - 1;
        if (pBufObj->iBufSize <= dwFrontSize)
            return NULL;
        return (byte*)(pBufObj->wsaBuf.buf + dwFrontSize);
    }

	//when a asynchronous task had been finished, will call this function to process 
	//iTaskType: ==1 parsed data, uncompress and decrypt
	//           ==2 prepare data, compress and encrypt
	//           ==3 time-consuming calculation
	//           ==4 connect to remote peer and sent data
	int  CXConnectionObject::CallbackTaskProcess(int iProcessRet, int iTaskType, PCXMessageData pMes, string strRequestID)
	{
		int iRet = RETURN_SUCCEED;
		switch ((CXTaskBase::CX_RUNING_TASK_TYPE)iTaskType)
		{
		case CXTaskBase::CX_RUNING_TASK_TYPE_PARSE_DATA://parse data
		{
			if (iProcessRet != 0)
			{
				char szInfo[1024] = { 0 };
				sprintf_s(szInfo, 1024, "Failed to parse the packet data,close this connection,connection id=%lld,error code is %d,desc is '%s'\n",
					GetConnectionIndex(), errno, strerror(errno));
				m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);

				ConnectionClosedType emClosedType = SOCKET_CLOSED;
				bool bNeedClose = false;
				bNeedClose = true;
				emClosedType = ERROR_IN_PROCESS;
				if (!bNeedClose)
				{
					if (GetState() == CXConnectionObject::CLOSING)//closing
					{
						bNeedClose = true;
					}
					else if (GetState() >= CXConnectionObject::CLOSED)//closed
					{
						return RETURN_SUCCEED;
					}
				}

				if (bNeedClose)
				{
					if (GetState() >= CXConnectionObject::CLOSED)//closed
					{
						return RETURN_SUCCEED;
					}
					CXCommunicationServer *pComServer = (CXCommunicationServer *)m_lpServer;
					pComServer->CloseConnection(*this, emClosedType);
				}
			}
			break;
		}
		case CXTaskBase::CX_RUNING_TASK_TYPE_PREPARE_DATA: // prepare data, compress and encrypt
		{
            if (pMes == NULL && strRequestID =="")
                return RETURN_SUCCEED;

            if (pMes != NULL)//this message is myself received message, need to free it
            {
				pMes->dwType = 2;
				PushReceivedMessage(pMes,false);
            }
            else
            {
                PopRPCObj(strRequestID);
            }

			break;
		}
		default:
			break;
		}
		
		return iRet;
	}

	int  CXConnectionObject::ProcessAsyncMessageEnd(PCXMessageData pMessageData)
	{
		if (pMessageData==NULL)
		{
			return INVALID_PARAMETER;
		}

		CXGuidObject guidObj(false);
		
		CXConnectionSession * pSession = (CXConnectionSession*)GetSession();
		string strObjectGuid = guidObj.ConvertGuid(pMessageData->bodyData.byObjectGuid);

		if (strObjectGuid != "")
		{
			CXRPCObjectServer * pObjectServer = NULL;
			CXRPCObjectManager *pRpcManager = (CXRPCObjectManager*)m_pRPCObjectManager;
			pObjectServer = pRpcManager->GetStaticObject(strObjectGuid);
			if (pObjectServer != NULL)
			{
				pObjectServer->SetLogHandle(GetLogHandle());
				pObjectServer->SetJournalLogHandle(GetJournalLogHandle());
				pObjectServer->SetIOStat(GetIOStat());
			}
			else //unknown object
			{
				strObjectGuid = "{0307F567-72FC-4355-8192-9E37DC766D2E}";
				pObjectServer = pRpcManager->GetStaticObject(strObjectGuid);
				if (pObjectServer)
				{
					pObjectServer->SetLogHandle(GetLogHandle());
					pObjectServer->SetJournalLogHandle(GetJournalLogHandle());
					pObjectServer->SetIOStat(GetIOStat());
				}
			}
			if (pObjectServer!=NULL)
			{
				pObjectServer->AsynCallbackResult(pMessageData);
			}
		}

		int iRet = RETURN_SUCCEED;
		m_lockSend.Lock();
		ReduceReceivedPacketNumber();
		m_uiLastProcessedMessageSequenceNum++;
		m_lockSend.Unlock();

		if (pMessageData->iProcessedRet ==-2) //need to closed this connection
		{
			Lock();
			CXCommunicationServer *pServer = (CXCommunicationServer *)m_lpServer;
			pServer->CloseConnection(*this, ERROR_IN_PROCESS, false);
			UnLock();
		}
		else
		{
			Lock();
			if (GetState() >= CXConnectionObject::CLOSING)//closing
			{
				CXCommunicationServer *pServer = (CXCommunicationServer *)m_lpServer;
				pServer->CloseConnection(*this, SOCKET_CLOSED, false);
			}
			UnLock();
		}

		return iRet;
	}

	void CXConnectionObject::PushRPCObj(string strPacketGuid, void* pObj, bool bLockBySelf)
	{
	    if(bLockBySelf)
            m_lockSend.Lock();
		m_mapRPCObj[strPacketGuid] = pObj;
		if(bLockBySelf)
            m_lockSend.Unlock();
	}
	void* CXConnectionObject::PopRPCObj(string strPacketGuid, bool bLockBySelf)
	{
		void *pObj = NULL;
		if(bLockBySelf)
            m_lockSend.Lock();
		map<string, void*>::iterator it = m_mapRPCObj.find(strPacketGuid);
		if (it != m_mapRPCObj.end())
		{
			pObj = it->second;
            m_mapRPCObj.erase(it);
		}
		if(bLockBySelf)
            m_lockSend.Unlock();

		return pObj;
	}

    //get the rpc object from the map
    void* CXConnectionObject::GetRPCObj(string strPacketGuid, bool bLockBySelf)
    {
        void *pObj = NULL;
        if(bLockBySelf)
            m_lockSend.Lock();
        map<string, void*>::iterator it = m_mapRPCObj.find(strPacketGuid);
        if (it != m_mapRPCObj.end())
        {
            pObj = it->second;
        }
        if(bLockBySelf)
            m_lockSend.Unlock();

        return pObj;
    }

	void CXConnectionObject::FreeSendingList()
	{
		m_lockSend.Lock();
		PCXBufferObj pCurBuf = m_pListSendBuf;
		if (pCurBuf == NULL)
		{
			m_lockSend.Unlock();
			return;
		}

		while (pCurBuf!=NULL)
		{
			m_pListSendBuf = pCurBuf->pNext;
			FreeSendCXBufferObj(pCurBuf);

			if (m_pListSendBuf == NULL)
			{
				m_pListSendBufEnd = NULL;
				break;
			}
			pCurBuf = m_pListSendBuf;
		}
		m_lockSend.Unlock();
	}

}