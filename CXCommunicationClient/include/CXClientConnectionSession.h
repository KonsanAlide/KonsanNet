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

Description£ºthe connection object of the client socket
*****************************************************************************/
#ifndef __CXCLIENTCONNECTIONSESSION_H__
#define __CXCLIENTCONNECTIONSESSION_H__
//#include "PlatformDataTypeDefine.h"

#include <string>
#include <list>
using namespace std;
#include "CXClientSocketChannel.h"
#include "CXSpinLock.h"

#ifdef WIN32
#include <unordered_map>
#else
#define GCC_VERSION (__GNUC__ * 10000 \
    + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

#if GCC_VERSION >= 40300
#include <tr1/unordered_map>
using namespace std::tr1;
#define hash_map unordered_map

#else
#include <ext/hash_map>
using namespace __gnu_cxx;
#endif

using namespace std;
#endif

namespace CXCommunication
{
    class CXClientConnectionSession
    {
    public:
        CXClientConnectionSession();
        virtual ~CXClientConnectionSession();
        //void SetSessionID(string strSessionID) { m_strSessionID = strSessionID; }
        //void SetVerifyCode(string strCode) { m_strVerifyCode = strCode; }
        //string GetSessionID() { return m_strSessionID; }
        //string GetVerifyCode() { return m_strVerifyCode; }

        void Lock();
        void UnLock();
        int AddMainConnection(CXClientSocketChannel &conObj);
        int AddMessageConnection(CXClientSocketChannel &conObj);
        int AddDataConnection(CXClientSocketChannel &conObj);

        int RemoveConnection(CXClientSocketChannel &conObj);

        void Close();

        void SetData(string strKey, void *pData);
        void *GetData(string strKey);
        void RemoveData(string strKey);

        int  GetConnectionNumber();

        //open a socket channel
        //Return value:==0 open the channel successfully
        //             ==-1 invaild parameter,the session id is empty or the verify code is empty.
        //             ==-2 failed to connect server
        //             ==-3 failed to login the server.
        //             ==-4 the major channel exists.
        //             ==-5 Unknown error.
        int OpenChannel(CXClientSocketChannel &conObj,
            CXClientSocketChannel::ChannelType chanelType);

        void SetUserInfo(string strRemoteUser, string strRemotePassword);
        void SetRemoteAddress(const CXSocketAddress& addressRemote) { m_addressRemote.SetAddress(addressRemote.GetAddress()); }

    private:
        CXClientSocketChannel * m_pMmainConnetion;
        list<CXClientSocketChannel*> m_lstMessageConnections;
        list<CXClientSocketChannel*> m_lstDataConnections;
        list<CXClientSocketChannel*> m_lstObjectConnections;
        CXSpinLock m_lock;
        string m_strSessionID;
        string m_strVerifyCode;
        unordered_map<string, void*> m_mapData;
        CXSocketAddress  m_addressRemote;
        string m_strRemoteUser;
        string m_strRemotePassword;
    };
}
#endif //__CXCLIENTCONNECTIONSESSION_H__

