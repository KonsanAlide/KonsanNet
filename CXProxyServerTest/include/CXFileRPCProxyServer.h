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
#ifndef __CXFILERPCPROXYSERVER_H__
#define __CXFILERPCPROXYSERVER_H__

#include "CXRPCObjectServer.h"
#include "CXStorageRPCClient.h"
#include "CXFilePacketStructure.h"
#include "CXConnectionObject.h"
#include "CXFile64.h"
namespace CXCommunication
{
    class CXFileRPCProxyServer : public CXRPCObjectServer
    {
    public:
        enum FileRPCProxyServerState {
            CX_INTER_FILE_OPEN_REPLY = 1,
            CX_INTER_FILE_READ_REPLY,
            CX_INTER_FILE_WRITE_REPLY,
            CX_INTER_FILE_SEEK_REPLY,
            CX_INTER_FILE_CLOSE_REPLY,
            CX_INTER_FILE_GET_FILE_LEN_REPLY,
            CX_INTER_FILE_GET_FILE_POS_REPLY,
        };
    public:
		CXFileRPCProxyServer();
        ~CXFileRPCProxyServer();

        virtual string GetObjectClassName() { return "CXFileTcpV1"; }

        virtual int DispatchMes(PCXMessageData pMes);

        virtual int SendData(CXConnectionObject * pCon, const byte *pbyData, DWORD dwDataLen);

        virtual CXRPCObjectServer* CreateObject();

        virtual void Destroy();

        virtual void MessageToString(PCXMessageData pMes, string &strMes);

		int    OpenFile(PCXMessageData pMes, CXConnectionObject *pCon);
		int    OpenFileReply(PCXMessageData pMes, CXConnectionObject *pCon, const CXRPCReply & rpcReply);

		int    CloseFile(PCXMessageData pMes, CXConnectionObject *pCon);
		int    CloseFileReply(PCXMessageData pMes, CXConnectionObject *pCon, const CXRPCReply & rpcReply);

		int    Write(PCXMessageData pMes, CXConnectionObject *pCon);
		int    WriteReply(PCXMessageData pMes, CXConnectionObject *pCon, const CXRPCReply & rpcReply);


		int    Read(PCXMessageData pMes, CXConnectionObject *pCon);
		int    ReadReply(PCXMessageData pMes, CXConnectionObject *pCon, const CXRPCReply & rpcReply);


		int    Seek(PCXMessageData pMes, CXConnectionObject *pCon);
		int    SeekReply(PCXMessageData pMes, CXConnectionObject *pCon, const CXRPCReply & rpcReply);

		int    GetFileLength(PCXMessageData pMes,CXConnectionObject *pCon);
		int    GetFileLengthReply(PCXMessageData pMes, CXConnectionObject *pCon, const CXRPCReply & rpcReply);

		int    GetCurrentFilePosition(PCXMessageData pMes,CXConnectionObject *pCon);
		int    GetCurrentFilePositionReply(PCXMessageData pMes, CXConnectionObject *pCon, const CXRPCReply & rpcReply);

        int    ReplyClient(PCXMessageData pMes, DWORD dwReplyCode, CXConnectionObject *pCon, const CXRPCReply & rpcReply);

    private:
		string   m_strCurrentFilePath;
		bool     m_bFileOpened;
		CXStorageRPCClient *m_pStorageObj;
    };
}
#endif //__CXFILERPCPROXYSERVER_H__

