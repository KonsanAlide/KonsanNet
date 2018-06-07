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
#ifndef CXSOCKETSERVERIMPL_H
#define CXSOCKETSERVERIMPL_H

#include "CXServerStructDefine.h"
#include "CXConnectionObject.h"

namespace CXCommunication
{
    //have lock by CXConnectionObject::lock
    typedef int (*POnReadCallback)(CXConnectionObject& conObj, PCXBufferObj pBufObj,
        DWORD dwTransDataOfBytes);
    //typedef int (*POnWriteCallback)(CXConnectionObject &conObj,PCXBufferObj pBufObj,
    //    DWORD dwTransDataOfBytes);
    //have lock by CXConnectionObject::lock
    typedef int (*POnClose)(CXConnectionObject& conObj, ConnectionClosedType emClosedType);
    typedef int(*POnAccept)(void *pServer, cxsocket sock,sockaddr_in &addrRemote);

    class CXSocketServerImpl
    {
        private:
            bool   m_bStarted;

        protected:
            POnReadCallback m_pfOnRead;
            POnClose        m_pfOnClose;
            POnAccept       m_pfOnAccept;
            void *          m_pServer;

        public:
            CXSocketServerImpl();
            virtual ~CXSocketServerImpl();

            //virtual bool SetNonblocking(int sock)=0;
            virtual int Start(unsigned short iListeningPort,int iWaitThreadNum)=0;
            virtual int Stop()=0;
            bool    IsStarted(){ return m_bStarted;}
            void    SetStarted(bool bStarted){ m_bStarted = bStarted;}
            void    SetOnReadCallbackFun(POnReadCallback pfn){m_pfOnRead= pfn;}
            void    SetOnCloseCallbackFun(POnClose pfn){m_pfOnClose= pfn;}
            void    SetOnAcceptCallbackFun(POnAccept pfn) { m_pfOnAccept = pfn; }
            void    SetServer(void *pServer) { m_pServer =pServer; }
            void *  GetServer() { return m_pServer; }

            virtual int OnAccept(void *pServer,cxsocket sock, sockaddr_in &addrRemote) = 0;
            virtual int OnRead(CXConnectionObject& conObj, PCXBufferObj pBufObj,
                DWORD dwTransDataOfBytes)=0;
            virtual int OnWrite(CXConnectionObject& conObj)=0;
            virtual void OnClose(CXConnectionObject& conObj, ConnectionClosedType emClosedType)=0;

        protected:
    };
}
#endif // CXSOCKETSERVERIMPL_H
