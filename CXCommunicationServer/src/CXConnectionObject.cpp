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
using namespace std;

//extern CXSpinLock g_lock;
//extern int g_iTotalMessage ;

namespace CXCommunication
{
    CXConnectionObject::CXConnectionObject()
    {
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
        m_uiLastProcessBufferIndex = 0;

        m_nState = INITIALIZED;

        //last packet time
        m_tmLastPacketTime = 0;
        memset(&m_addrRemote, 0, sizeof(m_addrRemote));

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

        // the number of the buffer by posting to iocp model
        m_uiNumberOfPostBuffers = 0;

        m_iProcessPacketNumber = 0;

        m_pSessionsManager = NULL;

        m_pSession = NULL;

        m_pDataParserHandle = NULL;

        //10 seconds
        m_dwTimeOutMSeconds = 10000;

        m_tmReleasedTime = 0;

		m_pJouralLogHandle = NULL;

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

    void CXConnectionObject::Build(cxsocket sock,uint64 uiConnIndex,sockaddr_in addr)
    {
        Lock();
        Reset();
        m_sock = sock;

        m_tmAcceptTime = GetTickCountValue();

        m_uiConnectionIndex = uiConnIndex;

        m_nState = PENDING;
        UnLock();
    }

    void CXConnectionObject::RecordCurrentPacketTime()
    {
        m_tmLastPacketTime = GetTickCountValue();
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
            m_uiNumberOfPostBuffers--;
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
    int CXConnectionObject::RecvPacket(PCXBufferObj pBufObj, DWORD dwTransDataOfBytes,
		byte* pbyThreadCache, DWORD dwCacheLen,bool bLockBySelf)
    {
		if (pBufObj == NULL || pbyThreadCache==NULL || dwCacheLen==0)
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

			/*
			//when read a new packet or found a byte left in the variable:byLastByte
            if (iUsedLen == 0 || iUsedLen == 1)//the beginning of the packet
            {
				//if had received a incorrect packet, we need to find the begin point of the next correct packet.
                int iReturn = JudgeHeaderFlag(pCurBuf,byLastByte,pPacketHeader,iUsedLen,
                    iLeftLen, iBeginPointInBuffer);
                if (iReturn == -1) //not find any thing
                {
                    uiPacketIndex = m_uiLastProcessBufferIndex;
                    pCurBuf = m_pListReadBufEnd;
                    continue;
                }
                else if (iReturn == 1)//find a byte of the header flag
                {
                    pCurBuf= pCurBuf->pNext;
                    uiPacketIndex++;
                    continue;
                }
            }
			*/

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
            //wrong packet , the packet size must be less than 20MB
            if (packetHeader.dwDataLen > CX_MAX_CHACHE_SIZE*2)
            {
                sprintf_s(szInfo, 1024, "Received a error packet,the size of the packet is larger than 10MB ,connection id = %lld,packet sequence number:%lld\n",
                    m_uiConnectionIndex, pCurBuf->nSequenceNum);
				bFindErrorPacket = true;
				m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
				break;
            }

            int iBufferLen = sizeof(CXMessageData)- sizeof(CXPacketBodyData)+ packetHeader.dwDataLen;
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

            if (VerifyPacketBodyData(&packetHeader, pBodyData))
            {
                bool bCompressed = false;
                bool bEncrypted = false;
                if (packetHeader.byCompressFlag & 0x80)
                {
                    bCompressed = true;
                }
                if (packetHeader.byEncryptFlag & 0x80)
                {
                    bEncrypted = true;
                }
                if (m_pDataParserHandle)// need to parse data
                {
                    //m_pDataParserHandle->ParseData();
                }
                if (m_pfnProcessOnePacket != NULL)
                {
                    AddReceivedPacketNumber();
                    //g_lock.Lock();
                    //g_iTotalMessage++;
                    //g_lock.Unlock();
                    //m_uiNumberOfReceivedPacketInQueue++;
                    m_pfnProcessOnePacket(*this, pMessageData);
                }
                else
                {
                    OnProcessOnePacket(*this, pMessageData);
                }
            }
            else // the packet is wrong packet, break it ,and close this connection
            {
                sprintf_s(szInfo, 1024, "Received a error packet,verify the packet data failed,connection id = %lld,packet sequence number:%lld \n",
                    m_uiConnectionIndex, pCurBuf->nSequenceNum);
                m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);

                FreeBuffer(pMessageData);
                pMessageData = NULL;

				bFindErrorPacket = true;
				break;
            }

			pCurBuf->wsaBuf.len -= (iBeginPointInBuffer - pCurBuf->nCurDataPointer);
			pCurBuf->nCurDataPointer = iBeginPointInBuffer;
			if (pCurBuf->wsaBuf.len == 0)// not left any data, free it
			{
				pCurBuf = pCurBuf->pNext;
			}

            PCXBufferObj pNeedDeleteBuf = m_pListReadBuf;
            while (pNeedDeleteBuf && pNeedDeleteBuf!= pCurBuf)
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

			if (pCurBuf== NULL)
			{
				m_pListReadBuf = m_pListReadBufEnd = NULL;
			}

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

        if (WSASend(m_sock, &pBufObj->wsaBuf, 1, &dwSend, 0, &pBufObj->ol, NULL) != 0)
        {
            DWORD dwEr = ::WSAGetLastError();
            if (dwEr != WSA_IO_PENDING)
            {
                m_lock.Unlock();
                FreeCXBufferObj(pBufObj);
                return -3;
            }
        }
        //m_uiNumberOfPostBuffers++;

#endif // WIN32

        m_lock.Unlock();
        return RETURN_SUCCEED;
    }

    int CXConnectionObject::PostSend(PCXBufferObj pBufObj)
    {
        if (pBufObj == NULL || pBufObj->wsaBuf.len <= 0 || pBufObj->wsaBuf.len>1024 * 1024)
            return INVALID_PARAMETER;

        if (m_nState == CLOSING || m_nState == CLOSED) //closing or closed
        {
            return -4;
        }

        pBufObj->nOperate = OP_WRITE;

        m_lock.Lock();
#ifdef WIN32
        DWORD dwSend = 0;

        if (WSASend(m_sock, &pBufObj->wsaBuf, 1, &dwSend, 0, &pBufObj->ol, NULL) != 0)
        {
            DWORD dwEr = ::WSAGetLastError();
            if (dwEr != WSA_IO_PENDING)
            {
                m_lock.Unlock();
                FreeCXBufferObj(pBufObj);
                return -3;
            }
        }
        //m_uiNumberOfPostBuffers++;
#else

#endif // WIN32

        m_lock.Unlock();
        return RETURN_SUCCEED;
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

        if(bLockBySelf)
            m_lockSend.Lock();
#ifdef WIN32
        if (WSASend(m_sock, &pBufObj->wsaBuf, 1, &dwSendLen, 0, NULL, NULL) != 0)
        {
            DWORD dwEr = ::WSAGetLastError();
            if (bLockBySelf)
                m_lockSend.Unlock();
            return -3;
        }
#else
        int iRet = SendData(pBufObj->wsaBuf.buf, pBufObj->wsaBuf.len,dwSendLen, 0);
        if (iRet!=0 && iRet!=-1)
        {
            DWORD dwEr = errno;
            if (bLockBySelf)
                m_lockSend.Unlock();
            return -3;
        }

#endif // WIN32
        if (bLockBySelf)
            m_lockSend.Unlock();
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

        if (WSARecv(m_sock, &pBufObj->wsaBuf, 1,
            &dwRecv, &dwFlag, &pBufObj->ol, NULL) != 0)
        {
            DWORD dwEr = ::WSAGetLastError();
            if (dwEr != WSA_IO_PENDING)
            {
                m_uiRecvBufferIndex--;
                m_lockRead.Unlock();
                FreeCXBufferObj(pBufObj);
                return -3;
            }
            else
            {
                //m_uiNumberOfPostBuffers++;
            }
        }
        else
        {
            //m_uiNumberOfPostBuffers++;
        }

        m_uiNumberOfPostBuffers++;

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
        return pBufObj;
    }

    void CXConnectionObject::AddProcessPacketNumber()
    {

        m_lockRead.Lock();
        m_iProcessPacketNumber++;
        m_lockRead.Unlock();
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

    void  CXConnectionObject::ReduceNumberOfPostBuffers()
    {
        m_lockRead.Lock();
        m_uiNumberOfPostBuffers--;
        m_lockRead.Unlock();
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
        if (m_uiNumberOfPostBuffers == 0)
        {
            //process the left buffer in the read list
            if (m_pListReadBuf!=NULL && (m_pListReadBuf->nSequenceNum == (m_uiLastProcessBufferIndex + 1)))
            {
                bNeedProcessLeftBuffers = true;
            }
        }
        UnlockRead();

        //process the left buffer in the read list
        if (bNeedProcessLeftBuffers)
        {
            //RecvPacket(NULL, 0,false);
        }

        if (m_uiNumberOfPostBuffers == 0 && m_pListReadBuf != NULL)
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
        int  nLeftBufLen = pBufObj->wsaBuf.len;
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
            m_uiNumberOfPostBuffers++;
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
    bool  CXConnectionObject::SendPacket(const byte* pbData, DWORD dwLen, DWORD dwMesCode,
         bool bLockBySelf)
    {
        if (bLockBySelf)
            m_lockSend.Lock();
        DWORD dwPacketLen = dwLen + sizeof(CXPacketData)-1;
        DWORD dwBufferLen = dwPacketLen + sizeof(CXBufferObj) - CX_BUF_SIZE;
        int iPacketBodyLen = dwPacketLen-sizeof(CXPacketHeader);
        PCXBufferObj pBufObj = GetCXBufferObj(dwBufferLen);
        byte *pPacketData =(byte *)pBufObj->wsaBuf.buf;

        byte *pbyPacketBodyData = pPacketData + sizeof(CXPacketHeader);
        PCXPacketBodyData pBodyData = (PCXPacketBodyData)pbyPacketBodyData;
        pBodyData->dwMesCode = dwMesCode;
        pBodyData->dwPacketNum = ++m_uiSendBufferIndex;
        memcpy(pBodyData->buf, pbData, dwLen);

        if (m_pDataParserHandle != NULL)
        {
            //m_pDataParserHandle->PrepareData();
        }

        DWORD dwCheckSum = 0;
        for (DWORD i = 0; i<dwLen; i++)
        {
            dwCheckSum += (unsigned int)pbyPacketBodyData[i];
        }
        dwCheckSum += dwMesCode;

        //send the header
        PCXPacketHeader pTcpHeader = (PCXPacketHeader)pPacketData;
        pTcpHeader->dwDataLen = iPacketBodyLen;
        pTcpHeader->dwCheckSum = dwCheckSum;
        pTcpHeader->wFlag = CX_PACKET_HEADER_FLAG;
        pTcpHeader->byCompressFlag = 0;
        pTcpHeader->byEncryptFlag = 0;
        pTcpHeader->dwOrignDataLen = iPacketBodyLen;

        pBufObj->wsaBuf.len = dwPacketLen;

        DWORD dwSentLen = 0;
        int iRet = PostSendBlocking(pBufObj, dwSentLen, false);
        FreeCXBufferObj(pBufObj);
        if (iRet != 0)
        {
            if (bLockBySelf)
                m_lockSend.Unlock();
            return false;
        }
        if (bLockBySelf)
            m_lockSend.Unlock();
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
        m_tmReleasedTime = GetTickCountValue();
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
			DWORD dwMillsSecond = (DWORD)(timeCur % 1000);
			std::strftime(pszTimeString, 60, "%Y-%m-%d_%H:%M:%S", std::localtime(&timeCur));
			char szMSTime[10] = { 0 };
			sprintf_s(szMSTime, 10, ".%d", dwMillsSecond);
			strcat(pszTimeString, szMSTime);
		}

		return (int64)tp.time_since_epoch().count();
	}

	//output the operation information to the journal log;
	void CXConnectionObject::OutputJournal(PCXMessageData pMes, int64 iBeginTimsMS)
	{
		if (m_pJouralLogHandle == NULL)
			return;

		int64 iEndTime = GetCurrentTimeMS();
		char szInfo[1024] = {0};
		sprintf_s(szInfo, 1024, "code:%d length:%d used_time:%lldms\n",
			pMes->bodyData.dwMesCode, pMes->dwDataLen, iEndTime- iBeginTimsMS);
		m_pJouralLogHandle->Log(CXLog::CXLOG_INFO,szInfo);
	}

}
