/****************************************************************************
Copyright (c) 2018 Chance Yang

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
#ifndef __CXFILETCPCLIENT_H__
#define __CXFILETCPCLIENT_H__

#include "CXRPCObjectClient.h"
#include <string>
using namespace std;

namespace CXCommunication
{
    class CXFileTcpClient:public CXRPCObjectClient
    {
    public:
        enum SEEKTYPE
        {
            begin,
            current,
            end
        };
        enum OPENTYPE
        {
            modeRead=0,
            modeWrite,
            modeReadWrite,
            modeNoTruncateWrite,
            modeNoTruncateReadWrite
        };

    public:
        CXFileTcpClient();
        virtual ~CXFileTcpClient();
		int  Open(CX_RPC_OBJECT_OPENTYPE type);
        int  Open(string strRemoteFilePath,OPENTYPE type,bool bOpenExisted = false);
        int  Read(byte* pBuf,int iWantReadLen,int *piReadLen);
        int  Seek(int64 pos, SEEKTYPE type);
        int  Write(const byte* pBuf, int iBufLen, int *piWrittenLen,uint64 uiOffset=0, SEEKTYPE type = current);
        int  Close();

        string GetFilePathName() { return m_strRemoteFilePath; }
        int GetFileLength(uint64 & uiFileLength);

        int  GetCurrentPosition(uint64 & uiPos);

        //notify the peer to modify the size of the received buffer 
        int  SetPeerRecvBufSize(DWORD dwSize);

		virtual string  GetObjectClassName() { return "CXFileTcpV1"; }

    private:
    };
}

#endif
