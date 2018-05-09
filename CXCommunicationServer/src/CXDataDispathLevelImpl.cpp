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
#include "CXDataDispathLevelImpl.h"
#include "CXCommunicationServer.h"
namespace CXCommunication
{
    CXDataDispathLevelImpl::CXDataDispathLevelImpl()
    {
        m_pServer = NULL;
        m_ppReadMessageQueue = NULL;
        m_ppWriteMessageQueue = NULL;
        m_iQueueNumber = 4;
    }

    CXDataDispathLevelImpl::~CXDataDispathLevelImpl()
    {
        //dtor
    }

    int CXDataDispathLevelImpl::Start(CXCommunicationServer *pServer,int iQueueNumber)
    {
        if(pServer==NULL || iQueueNumber>100 || iQueueNumber<1)
            return INVALID_PARAMETER;

        m_pServer = pServer;
        //m_pSocketServerImpl->SetOnReadCallbackFun(CXDataDispathLevelImpl::OnRecv);
        //m_pSocketServerImpl->SetOnCloseCallbackFun(CXDataDispathLevelImpl::OnClose);
        try
        {
            m_ppReadMessageQueue = new CXMessageQueue[iQueueNumber];
            m_ppWriteMessageQueue = new CXMessageQueue[iQueueNumber];
            m_iQueueNumber = iQueueNumber;
        }
        catch (const bad_alloc& e)
        {
            return -2;
        }
        
        return RETURN_SUCCEED;
    }

    int CXDataDispathLevelImpl::Stop()
    {
        
        return RETURN_SUCCEED;
    }


    int CXDataDispathLevelImpl::OnRecv(CXConnectionObject &conObj,PCXBufferObj pBufObj,
        DWORD dwTransDataOfBytes)
    {
        return 0;
    }

    int CXDataDispathLevelImpl::OnClose(CXConnectionObject &conObj)
    {
        return 0;
    }

    int  CXDataDispathLevelImpl::PushReadPacket(int iIndex, PCXBufferObj pBufObj)
    {
        if (iIndex <0 || iIndex>m_iQueueNumber || pBufObj==NULL)
            return INVALID_PARAMETER;

        m_ppReadMessageQueue[iIndex].PushMessage(pBufObj);

        return RETURN_SUCCEED;
    }
    int  CXDataDispathLevelImpl::PushWritePacket(int iIndex, PCXBufferObj pBufObj)
    {
        if (iIndex <0 || iIndex>m_iQueueNumber || pBufObj == NULL)
            return INVALID_PARAMETER;

        m_ppWriteMessageQueue[iIndex].PushMessage(pBufObj);

        return RETURN_SUCCEED;
    }

    CXMessageQueue *CXDataDispathLevelImpl::GetReadMessageQueue(int iIndex)
    {
        if (m_ppReadMessageQueue == NULL)
        {
            return NULL;
        }
        return  &m_ppReadMessageQueue[iIndex];
    }

}
