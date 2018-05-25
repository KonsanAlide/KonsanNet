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
#ifndef __CXCLIENTSOCKETCHANNEL_H__
#define __CXCLIENTSOCKETCHANNEL_H__
#include "CXTcpClient.h"
#include <string>
using namespace std;
namespace CXCommunication
{
    class CXClientSocketChannel : public CXTcpClient
    {
    public:
        enum ChannelType
        {
            //==1 major message connection
            //==2 minor messsage connection
            //==3 data connection
            //==4 object connection
            MAJOR_MES_CONNECTION=1,
            MINOR_MES_CONNECTION,
            DATA_CONNECTION,
            OBJECT_CONNECTION
        };
    public:
        CXClientSocketChannel();
        virtual ~CXClientSocketChannel();

        void SetChannelType(ChannelType cType) { m_channelType = cType; }
        ChannelType GetChannelType() { return m_channelType; }
        void SetSessionID(string strSessionID) { m_strSessionID= strSessionID; }
        void SetVerifyCode(string strCode) { m_strVerifyCode = strCode; }
        string GetSessionID(){ return m_strSessionID; }
        string GetVerifyCode() { return m_strVerifyCode; }
        bool IsOpen() { return m_bIsOpened; }

        void SetUserInfo(string strRemoteUser, string strRemotePassword);

        //Return value:==0 open the channel successfully
        //             ==-1 invaild parameter,the session id is empty or the verify code is empty.
        //             ==-2 failed to connect server
        //             ==-3 failed to login the server.
        int OpenChannel(const CXSocketAddress& addressRemote);

        //login to the server by the user name ,password
        //Received value: ==RETURN_SUCCEED the socket was created successfully 
        //                ==INVALID_PARAMETER invalid inputed parameters
        //                ==-2 this socket object had not created
        //                ==-3 failed to connect the peer 
        virtual int Login(const string &strUserName, const string &strPassword,
            int iUserType = 1, int iSessionType = 1,
            string strSessionGuid = "", string strVerifyCode = "");


    private:
        ChannelType m_channelType;
        string m_strSessionID;
        string m_strVerifyCode;
        bool   m_bIsOpened;
        string m_strRemoteUser;
        string m_strRemotePassword;
    };
}
#endif //__CXCLIENTSOCKETCHANNEL_H__

