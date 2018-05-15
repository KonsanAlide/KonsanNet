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

#include <memory.h>
#include <time.h>
#include "CXConnectionObject.h"
#include "CXLog.h"
#include "PlatformFunctionDefine.h"
extern CXLog g_cxLog;
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
        m_sock = 0;
        m_iConnectionType = 1;

        m_tmAcceptTime = 0;

        m_uiConnectionIndex = 0;

        m_uiRecvBufferIndex = 0;
        m_uiLastProcessBufferIndex = 0;

        //==0 initialize
        //==1 pending connection
        //==2 recv data packets
        //==3 closing
        //==5 closed
        m_nState = 0;

        //last packet time
        m_tmLastPacketTime = 0;
        memset(&m_addrRemote, 0, sizeof(m_addrRemote));

        m_lpServer = NULL;
        m_lpCacheObj = NULL;
        m_lpLargeBlockCacheObj = NULL;
        m_nUsedNumber = 0;

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

        m_tmAcceptTime = time(NULL);

        m_uiConnectionIndex = uiConnIndex;

        //==0 initialize
        //==1 pending connection
        //==2 recv data packets
        //==3 closing
        //==5 closed
        m_nState = 1;
        UnLock();
    }

    void CXConnectionObject::RecordCurrentPacketTime()
    {
        m_tmLastPacketTime = time(NULL);
    }

    int CXConnectionObject::RecvPacket(PCXBufferObj pBufObj, DWORD dwTransDataOfBytes)
    {
        PCXBufferObj pCurBuf = NULL;
        PCXBufferObj pPrevBuf = NULL;
        int iLeftLen = 0;
        int iUsedLen = 0;
        int iPacketLen = 0;
        int iNeedCopyLen = 0;
        int iBeginPointInBuffer = 0;
        bool bAllcateMemoryFails = false;
        char szInfo[1024] = { 0 };


        m_lock.Lock();

        //sprintf_s(szInfo, 1024, "Receive buffer,buffer index = %I64i,datalen = %d,pBufObj=%x\n", pBufObj->nSequenceNum,
        //    dwTransDataOfBytes, pBufObj);
        //g_cxLog.Log(CXLog::CXLOG_INFO, szInfo);
        //printf_s(szInfo);

        m_tmLastPacketTime = time(NULL);

        if (m_pListReadBuf==NULL ||
            (m_pListReadBuf !=NULL && m_pListReadBuf->nSequenceNum > pBufObj->nSequenceNum))
        {
            pBufObj->pNext = m_pListReadBuf;
            m_pListReadBuf = pBufObj;
            if (m_pListReadBufEnd == NULL)
            {
                m_pListReadBufEnd = m_pListReadBuf;
            }
        }
        else if (m_pListReadBufEnd != NULL && m_pListReadBufEnd->nSequenceNum < pBufObj->nSequenceNum)
        {
            m_pListReadBufEnd->pNext = pBufObj;
            m_pListReadBufEnd = pBufObj;
        }
        else
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

        m_uiNumberOfReceivedBufferInList++;
        m_uiNumberOfPostBuffers--;


        //debug
        /*
        pCurBuf = m_pListReadBuf;
        while (pCurBuf)
        {
            if (pCurBuf->nSequenceNum != (m_uiLastProcessBufferIndex + 1))//is not the next data buffer
            {
                break;
            }

            m_pListReadBuf = m_pListReadBuf->pNext;

            sprintf_s(szInfo, 1024, "Free Buffer ,buffer address = %x,buffer index = %I64i, \n\n", pCurBuf, pCurBuf->nSequenceNum);
            g_cxLog.Log(CXLog::CXLOG_INFO, szInfo);
            printf_s(szInfo);
            //printf("####Free Buffer ,buffer index = %I64i\n", pCurBuf->nSequenceNum);

            FreeBuffer(pCurBuf);

            m_uiNumberOfReceivedBufferInList--;
            m_uiLastProcessBufferIndex++;
            if (m_pListReadBuf == NULL)
            {
                m_pListReadBufEnd = NULL;
                break;
            }


            pCurBuf = m_pListReadBuf->pNext;
        }
        m_lock.Unlock();
        return 0;
        */

        PCXBufferObj pBuf = GetBuffer();
        PCXPacketHeader pHeader = NULL;

        if (pBuf == NULL)
        {
            sprintf_s(szInfo, 1024, "Getbuffer null ,This :%x, Cache address:%x\n", this, m_lpCacheObj);
            g_cxLog.Log(CXLog::CXLOG_ERROR, szInfo);
            printf(szInfo);

            m_lock.Unlock();
            return -3;
        }

        uint64 uiPacketIndex = m_uiLastProcessBufferIndex;
        pCurBuf = m_pListReadBuf;

        while (pCurBuf)
        {
            if (pCurBuf->nSequenceNum != (uiPacketIndex+1))//is not the next data buffer
            {
                break;
            }

            iLeftLen = pCurBuf->wsaBuf.len;
            iNeedCopyLen = 0;
            iBeginPointInBuffer = pCurBuf->nCurDataPointer;


            if (iLeftLen<0)
            {
                //printf("#######iLeftLen<0 ,connection id = %I64i,pCurBuf :%x, Cache address:%x\n",
                //    m_uiConnectionIndex,pCurBuf, m_lpCacheObj);

                sprintf_s(szInfo, 1024, "iLeftLen<0 ,connection id = %I64i,pCurBuf :%x\n",
                        m_uiConnectionIndex,pCurBuf);
                g_cxLog.Log(CXLog::CXLOG_ERROR, szInfo);
                printf(szInfo);
                break;
            }

            if(iUsedLen<sizeof(CXPacketHeader))//not have a complete header, copy from this buffer
            {
                iNeedCopyLen = sizeof(CXPacketHeader)- iUsedLen;
                if (iLeftLen <iNeedCopyLen)//not have a complete header
                {
                    iNeedCopyLen = iLeftLen;
                }

                memcpy(pBuf->wsaBuf.buf + iUsedLen, pCurBuf->wsaBuf.buf + iBeginPointInBuffer,
                    iNeedCopyLen);

                iUsedLen += iNeedCopyLen;
                iLeftLen -= iNeedCopyLen;
                iBeginPointInBuffer += iNeedCopyLen;

                if (iUsedLen<sizeof(CXPacketHeader))//not have a complete header
                {
                    pCurBuf = pCurBuf->pNext;
                    uiPacketIndex++;
                    continue;
                }
            }

            pHeader = (PCXPacketHeader)(pBuf->wsaBuf.buf);
            iPacketLen = (pHeader->wDataLen + sizeof(CXPacketHeader));

            if (pBuf->wsaBuf.buf[0] == '$')
            {
                sprintf_s(szInfo, 1024, "Error packet,pBuf->wsaBuf.buf[0] == \'$\' ,connection id = %I64i,pBuf :%x\n",
                    m_uiConnectionIndex, pBuf);
                g_cxLog.Log(CXLog::CXLOG_ERROR, szInfo);
                printf(szInfo);
            }
            if (iLeftLen + iUsedLen < iPacketLen) //not have a complete packet
            {
                if (iLeftLen > 0)
                {
                    memcpy(pBuf->wsaBuf.buf + iUsedLen, pCurBuf->wsaBuf.buf + iBeginPointInBuffer,
                        iLeftLen);
                    iUsedLen += iLeftLen;
                }

                pCurBuf = pCurBuf->pNext;
                uiPacketIndex++;
                continue;
            }

            //debug code
            if (pHeader->dwFlag != 0x25f7)
            {
                sprintf_s(szInfo, 1024, "Error packet,pHeader->dwFlag != 0x25f7 ,connection id = %I64i,pBuf :%x\n",
                    m_uiConnectionIndex, pBuf);
                g_cxLog.Log(CXLog::CXLOG_ERROR, szInfo);
                printf(szInfo);
            }

            iNeedCopyLen = (iPacketLen - iUsedLen);
            if (iPacketLen <= pBuf->wsaBuf.len)
            {
                memcpy(pBuf->wsaBuf.buf + iUsedLen, pCurBuf->wsaBuf.buf + iBeginPointInBuffer,
                    iNeedCopyLen);
                pBuf->wsaBuf.len = iPacketLen;
            }

            //debug
            //sprintf_s(szInfo, 1024, "####Dispach packet ,connection id = %I64i,datalen = %d,pHeader->dwPacketNum=%d,buffer address = %x\n", GetConnectionIndex(), pBuf->wsaBuf.len, pHeader->dwPacketNum, pBuf);
            //g_cxLog.Log(CXLog::CXLOG_INFO, szInfo);
            //printf_s(szInfo);

            if (m_pfnProcessOnePacket != NULL)
            {
                m_uiNumberOfReceivedPacketInQueue++;
                m_pfnProcessOnePacket(*this, pBuf);
            }
            else
            {
                OnProcessOnePacket(*this, pBuf);
            }

            iBeginPointInBuffer += iNeedCopyLen;


            int iTotalUsedLen = 0;
            PCXBufferObj pNeedDeleteBuf = m_pListReadBuf;
            while (pNeedDeleteBuf && pNeedDeleteBuf!= pCurBuf)
            {
                m_pListReadBuf = pNeedDeleteBuf->pNext;
                iTotalUsedLen += pNeedDeleteBuf->wsaBuf.len;

                //sprintf_s(szInfo, 1024, "Free Buffer ,buffer address = %x,buffer index = %I64i, \n\n", pNeedDeleteBuf,pNeedDeleteBuf->nSequenceNum);
                //g_cxLog.Log(CXLog::CXLOG_INFO, szInfo);
                //printf_s(szInfo);

                //printf("\n\n####Free Buffer ,buffer index = %I64i\n\n", pNeedDeleteBuf->nSequenceNum);
                FreeBuffer(pNeedDeleteBuf);
                m_uiLastProcessBufferIndex++;
                m_uiNumberOfReceivedBufferInList--;
                pNeedDeleteBuf = m_pListReadBuf;
            }

            //debug code
            //if ((iTotalUsedLen + (iBeginPointInBuffer - pCurBuf->nCurDataPointer)) != iPacketLen)
            //{
            //    int sadfl = 1;
            //}

            pCurBuf->wsaBuf.len -= (iBeginPointInBuffer - pCurBuf->nCurDataPointer);
            pCurBuf->nCurDataPointer = iBeginPointInBuffer;

            if (pCurBuf->wsaBuf.len == 0) //this buffer have left not any data
            {
                uiPacketIndex++;
                m_uiLastProcessBufferIndex++;
                if (pCurBuf == m_pListReadBuf)
                {
                    m_pListReadBuf = pCurBuf->pNext;
                    //printf("\n\n####Free Buffer ,buffer index = %I64i\n\n", pCurBuf->nSequenceNum);
                    //sprintf_s(szInfo, 1024, "Free Buffer ,buffer address = %x,buffer index = %I64i, \n\n", pCurBuf, pCurBuf->nSequenceNum);
                    //g_cxLog.Log(CXLog::CXLOG_INFO, szInfo);
                    //printf_s(szInfo);

                    FreeBuffer(pCurBuf);
                    m_uiNumberOfReceivedBufferInList--;
                    if (m_pListReadBuf == NULL)
                    {
                        m_pListReadBufEnd = NULL;
                        m_lock.Unlock();
                        return RETURN_SUCCEED;
                    }
                }
                else
                {
                    //printf("\n\n####Free Buffer ,buffer index = %I64i\n\n", pCurBuf->nSequenceNum);
                    //sprintf_s(szInfo, 1024, "Free Buffer ,buffer address = %x,buffer index = %I64i, \n\n", pCurBuf, pCurBuf->nSequenceNum);
                    //g_cxLog.Log(CXLog::CXLOG_INFO, szInfo);
                    //printf_s(szInfo);

                    FreeBuffer(pCurBuf);
                }
                pCurBuf = m_pListReadBuf;
            }
            else //this buffer have left some data
            {

            }

            iPacketLen = 0;
            iUsedLen = 0;

            pBuf = GetBuffer();
            if (pBuf == NULL)
            {
                //printf("#######Getbuffer null ,This :%x, Cache address:%x\n",
                //    this, m_lpCacheObj);
                sprintf_s(szInfo, 1024, "Getbuffer null ,This :%x, Cache address:%x\n", this, m_lpCacheObj);
                g_cxLog.Log(CXLog::CXLOG_ERROR, szInfo);
                printf(szInfo);

                bAllcateMemoryFails = true;
                break;
            }

            continue;
        }

        m_lock.Unlock();

        FreeBuffer(pBuf);

        if (bAllcateMemoryFails)
        {
            return -3;
        }

        return RETURN_SUCCEED;
    }

    int  CXConnectionObject::OnProcessOnePacket(CXConnectionObject &conObj, PCXBufferObj pBufObj)
    {
        return RETURN_SUCCEED;
    }

    int CXConnectionObject::PostSend(byte *pData, int iDataLen)
    {

        if (pData==NULL || iDataLen <= 0 || iDataLen>1024 * 1024)
            return INVALID_PARAMETER;

        if (m_nState == 3 || m_nState == 4) //closing or closed
        {
            return -4;
        }

        PCXBufferObj pBufObj = GetBuffer();
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
                m_lpCacheObj->FreeObject(pBufObj);
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

        if (m_nState == 3 || m_nState == 4) //closing or closed
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
                m_lpCacheObj->FreeObject(pBufObj);
                return -3;
            }
        }
        //m_uiNumberOfPostBuffers++;
#else

#endif // WIN32

        m_lock.Unlock();
        return RETURN_SUCCEED;
    }

    int CXConnectionObject::PostSendBlocking(PCXBufferObj pBufObj, DWORD &dwSendLen)
    {
        if (pBufObj == NULL || pBufObj->wsaBuf.len <= 0 || pBufObj->wsaBuf.len>1024 * 1024)
            return INVALID_PARAMETER;

        if (m_nState == 3 || m_nState == 4) //closing or closed
        {
            return -4;
        }

        pBufObj->nOperate = OP_WRITE;

        m_lock.Lock();
#ifdef WIN32

        if (WSASend(m_sock, &pBufObj->wsaBuf, 1, &dwSendLen, 0, NULL, NULL) != 0)
        {
            DWORD dwEr = ::WSAGetLastError();

            m_lock.Unlock();
            return -3;
        }
#else
        int iRet = SendData(pBufObj->wsaBuf.buf, pBufObj->wsaBuf.len,dwSendLen, 0);
        if (iRet!=0)
        {
            DWORD dwEr = errno;

            m_lock.Unlock();
            return -3;
        }

#endif // WIN32

        m_lock.Unlock();
        return RETURN_SUCCEED;
    }

    int CXConnectionObject::PostRecv(int iBufSize)
    {
        #ifndef WIN32
        return 0;
        #endif // WIN32

        if (iBufSize<=0 || iBufSize>1024*1024)
            return INVALID_PARAMETER;

        m_lock.Lock();
        if (m_nState == 3 || m_nState == 4) //closing or closed
        {
            m_lock.Unlock();
            return -4;
        }

        PCXBufferObj pBufObj = GetBuffer();
        if (pBufObj == NULL)
        {
            m_lock.Unlock();
            return -2;
        }

        pBufObj->nSequenceNum = ++m_uiRecvBufferIndex;
        pBufObj->nOperate = OP_READ;
        //printf("*****Post buffer begin,buffer index = %I64i,pBufObj=%x,pBufObj->nOperate=%d\n", pBufObj->nSequenceNum,
        //    pBufObj, pBufObj->nOperate);

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
                m_lock.Unlock();
                m_lpCacheObj->FreeObject(pBufObj);
                return -3;
            }
            else
            {
                m_uiNumberOfPostBuffers++;
            }
        }
        else
        {
            if (dwRecv > 0)
            {
                int l = 0;
            }
            m_uiNumberOfPostBuffers++;
        }
        //printf("*****Post buffer,buffer index = %I64i,pBufObj=%x,pBufObj->nOperate=%d\n", pBufObj->nSequenceNum,
        //    pBufObj, pBufObj->nOperate);
#endif // WIN32

        m_lock.Unlock();
        return RETURN_SUCCEED;
    }

    PCXBufferObj CXConnectionObject::GetBuffer()
    {
        PCXBufferObj pBufObj = (PCXBufferObj)m_lpCacheObj->GetObject();
        if (pBufObj == NULL)
        {
            return NULL;
        }
        //printf("*****Get buffer,pBufObj=%x,Connection ID = %I64i\n",pBufObj, m_uiConnectionIndex);

        pBufObj->pConObj = (void*)this;
        pBufObj->wsaBuf.len = BUF_SIZE;
        pBufObj->wsaBuf.buf = pBufObj->buf;
        return pBufObj;
    }

    void CXConnectionObject::AddProcessPacketNumber()
    {
        m_lock.Lock();
        m_iProcessPacketNumber++;
        m_lock.Unlock();
    }


    void  CXConnectionObject::AddReceivedBufferNumber()
    {
        m_uiNumberOfReceivedBufferInList++;
    }
    void  CXConnectionObject::ReduceReceivedBufferNumber()
    {
        m_uiNumberOfReceivedBufferInList--;
    }

    void  CXConnectionObject::AddReceivedPacketNumber()
    {
        m_uiNumberOfReceivedPacketInQueue++;
    }
    void  CXConnectionObject::ReduceReceivedPacketNumber()
    {
        m_uiNumberOfReceivedPacketInQueue--;
    }

    void  CXConnectionObject::ReduceNumberOfPostBuffers()
    {
        m_uiNumberOfPostBuffers--;
    }

    void CXConnectionObject::Close()
    {
        closesocket(m_sock);
        m_nState = 3;
    }

    //have lock by CXConnectionObject::Lock();
    int  CXConnectionObject::RecvData(PCXBufferObj *ppBufObj,DWORD &dwReadLen)
    {
        int nFlags=0;
        int nReadLen=0;
        dwReadLen = 0;

        m_lock.Lock();
        if (m_nState == 3 || m_nState == 4) //closing or closed
        {
            m_lock.Unlock();
            return -4;
        }

        PCXBufferObj pBufObj = GetBuffer();
        if (pBufObj == NULL)
        {
            m_lock.Unlock();
            return -2;
        }
        *ppBufObj = pBufObj;

        pBufObj->nSequenceNum = ++m_uiRecvBufferIndex;
        pBufObj->nOperate = OP_READ;

        int nRet = 0;

        int nRecv = 0;
        int nTotalRead = 0;
        bool bReadOk = false;
        int  nLeftBufLen = pBufObj->wsaBuf.len;
        while(nLeftBufLen>0)
        {
            // must comfirm the sockCur is nonblocking.
            nRecv = recv(m_sock, pBufObj->wsaBuf.buf + nTotalRead, nLeftBufLen, nFlags);
            if(nRecv < 0)
            {
                if(errno == EAGAIN)//in this case, that is not left any data in the network buffer.
                {
                    bReadOk = true;
                    nRet = -1;
                    break;
                }
                else if (errno == ECONNRESET)//receive the RST packet from the peer
                {

                    Close();
                    //CloseAndDisable(sockCur, events[i]);
                    cout << "counterpart send out RST\n";
                    nRet = -2;
                    break;
                }
                else if (errno == EINTR)//interrupt by other event
                {
                    continue;
                }
                else // other error
                {
                    Close();
                    cout << "unrecovable error\n";
                    nRet = -2;
                    break;
                }
            }
            else if( nRecv == 0) //the peer had closed the socket normally and had sent the FIN packet.
            {
                //CloseAndDisable(sockCur, events[i]);
                Close();
                cout << "counterpart has shut off\n";
                nRet = -2;
                break;
            }
            else// recvNum > 0
            {
                nTotalRead += nRecv;
                nLeftBufLen-=nRecv;
                if ( nLeftBufLen == 0)
                {
                    bReadOk = true;
                    break;
                }
            }
        }

        dwReadLen = pBufObj->wsaBuf.len = nTotalRead;
        m_lock.Unlock();
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

        while(nLeftDataLen>0)
        {
            // must comfirm the sockCur is nonblocking.
            nWritenLen = send(m_sock, pBuf + nTotalWritenLen, nLeftDataLen, 0);
            if (nWritenLen == -1)
            {
                if (errno == EAGAIN)// all data had been sent
                {
                    bWritten = true;
                    nRet = -1;
                    break;
                }
                else if(errno == ECONNRESET)//receive the RST packet from the peer
                {

                    //CloseAndDisable(sockCur, events[i]);
                    Close();
                    cout << "counterpart send out RST\n";
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
                //CloseAndDisable(sockCur, events[i]);
                Close();
                cout << "counterpart has shut off\n";
                nRet = -3;
                break;
            }

            // nTotalWritenLen > 0
            nTotalWritenLen += nWritenLen;
            if (nWritenLen == nLeftDataLen)
            {
                break;
            }
            nLeftDataLen-=nWritenLen;
        }

        dwSendLen = nTotalWritenLen;

        return nRet;
    }


}
