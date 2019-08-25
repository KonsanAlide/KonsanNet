/****************************************************************************
Copyright (c) 2018-2019 Chance Yang

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
#ifndef __CXRPCOBJECTCLIENT_H__
#define __CXRPCOBJECTCLIENT_H__

#include "CXClientSocketChannel.h"
#include "CXClientConnectionSession.h"
#include "PlatformDataTypeDefine.h"
#include <string>
using namespace std;

namespace CXCommunication
{
    class CXRPCObjectClient
    {
    public:
        enum CX_RPC_OBJECT_OPENTYPE
        {
            modeRead=0,
            modeWrite,
            modeReadWrite
        };

    public:
		CXRPCObjectClient();
        virtual ~CXRPCObjectClient();
		void        SetRemoteServerInfo(string strRemoteIP, unsigned short unRemotePort,
			                         string strRemoteUser, string strRemotePassword);

        virtual int Open(CX_RPC_OBJECT_OPENTYPE type)=0;
		virtual int Close()=0;
        bool        IsOpen() { return m_bIsOpened; }

		virtual string GetObjectName() { return "CXRPCObjectV1"; }
        string  GetGuid() { return m_strObjectGuid; }

		void    SetDataPaserHandle(CXDataParserImpl *pHandle);
		void    SetEncryptParas(CXDataParserImpl::CXENCRYPT_TYPE encryptType);
		void    SetCompressParas(CXDataParserImpl::CXCOMPRESS_TYPE compressType);
        //pszTimeString: if not NULL,will save the time string , the format is: 2019-07-31_15:39:29
        //return value : the current time 
        int64   GetCurrentTimeMS(char *pszTimeString = NULL);

	protected:
		void     GetObjectGuid();

    protected:
        string   m_strRemoteIP;
        WORD     m_unRemotePort;
        string   m_strRemoteFilePath;
		byte     m_byPacketData[CLIENT_BUF_SIZE];
		string   m_strRemoteUser;
		string   m_strRemotePassword;
		bool     m_bIsOpened;
		string   m_strObjectGuid;
		byte     m_byObjectGuid[CX_GUID_LEN];

        CXClientSocketChannel     m_cmmClient;
		CXClientSocketChannel     m_dataClient;
        CXClientConnectionSession m_cxSession;
    };
}

#endif
