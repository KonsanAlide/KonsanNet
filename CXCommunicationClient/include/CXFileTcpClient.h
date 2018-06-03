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

#include "CXClientSocketChannel.h"
#include "CXClientConnectionSession.h"
#include "PlatformDataTypeDefine.h"
#include <string>
using namespace std;

namespace CXCommunication
{
    class CXFileTcpClient
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
            modeRead,
            modeWrite,
            modeReadWrite,
            modeNoTruncateWrite,
            modeNoTruncateReadWrite
        };

    public:
        CXFileTcpClient();
        virtual ~CXFileTcpClient();
        int  Open(string strRemoteFilePath,OPENTYPE type);
        int  Read(byte* pBuf,int iWantReadLen,int *piReadLen);
        int  Seek(uint64 pos, SEEKTYPE type);
        int  Write(const byte* pBuf, int iBufLen, int *piWrittenLen);
        int  Close();
        bool IsOpen() { return m_bIsOpened; }
        void SetRemoteServerInfo(string strRemoteIP,unsigned short unRemotePort,
            string strRemoteUser, string strRemotePassword);
        string GetFilePathName() { return m_strRemoteFilePath; }
        int GetFileLength(uint64 & uiFileLength);
        byte * GetSendBuffer(DWORD &dwBufSize);
        byte * GetRecvBuffer(DWORD &dwBufSize);

    private:
        string m_strRemoteIP;
        unsigned short m_unRemotePort;
        string m_strRemoteFilePath;
        CXClientSocketChannel m_cmmClient;
        CXClientSocketChannel m_dataClient;
        CXClientConnectionSession m_cxSession;

        byte  m_byPacketData[CLIENT_BUF_SIZE];

        string m_strRemoteUser;
        string m_strRemotePassword;

        bool   m_bIsOpened;
    };
}

#endif
