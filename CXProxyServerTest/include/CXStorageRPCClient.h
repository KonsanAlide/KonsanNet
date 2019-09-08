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
#ifndef __CXSTORAGERPCCLIENT_H__
#define __CXSTORAGERPCCLIENT_H__

#include "CXRPCObjectClientInServer.h"
#include "CXFilePacketStructure.h"
#include "CXFile64.h"
namespace CXCommunication
{
    class CXStorageRPCClient : public CXRPCObjectClientInServer
    {
	public:
		enum SEEKTYPE
		{
			begin=0,
			current,
			end
		};
		enum OPENTYPE
		{
			modeRead = 0,
			modeWrite,
			modeReadWrite,
			modeNoTruncateWrite,
			modeNoTruncateReadWrite
		};
    public:
        CXStorageRPCClient();
        ~CXStorageRPCClient();

		//<derive from the father>
        virtual string GetObjectClassName() { return "CXStorageTcpV1"; }
        virtual int  DispatchMes(PCXMessageData pMes);
        virtual int  SendData(CXConnectionObject * pCon, const byte *pbyData, DWORD dwDataLen);
        virtual CXRPCObjectServer* CreateObject();
        virtual void Destroy();
        virtual void MessageToString(PCXMessageData pMes, string &strMes);
		//virtual int  Close();
		//</derive from the father>

		//<all function  >
		int    Open(CXRPCRequest request,string &strRemoteFilePath, OPENTYPE type, bool bOpenExisted = false);
		int    Read(CXRPCRequest request, int iWantReadLen, uint64 uiOffset = 0, SEEKTYPE type = current);
		int    Seek(CXRPCRequest request, int64 pos, SEEKTYPE type);
		int    Write(CXRPCRequest request, const byte* pBuf, int iBufLen, uint64 uiOffset = 0, SEEKTYPE type = current);
		int    GetFileLength(CXRPCRequest request);
		int    GetCurrentPosition(CXRPCRequest request);
		int    Close(CXRPCRequest request);

		bool   IsOpen() { return m_bIsOpened; }

		int    OnOpen(PCXMessageData pMes,CXConnectionObject *pCon);
		int    OnRead(PCXMessageData pMes, CXConnectionObject *pCon);
		int    OnSeek(PCXMessageData pMes, CXConnectionObject *pCon);
		int    OnWrite(PCXMessageData pMes, CXConnectionObject *pCon);
		int    OnGetFileLength(PCXMessageData pMes, CXConnectionObject *pCon);
		int    OnGetCurrentPosition(PCXMessageData pMes, CXConnectionObject *pCon);
		int    OnClose(PCXMessageData pMes, CXConnectionObject *pCon);

		//</derive from the father>

		string GetFilePath() { return m_strCurrentFilePath; }


    private:
		string   m_strCurrentFilePath;
		bool     m_bIsOpened;
    };
}
#endif //__CXSTORAGERPCCLIENT_H__

