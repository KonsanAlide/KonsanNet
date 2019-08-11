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
#include "CXClientSocketChannel.h"
#include "CXCommonPacketStructure.h"
#include "CXFilePacketStructure.h"
#include "CXPacketCodeDefine.h"
#include "CXSessionPacketStructure.h"
#ifdef WIN32
#else
#include <string.h>
#endif


using namespace CXCommunication;

CXClientSocketChannel::CXClientSocketChannel()
{
    m_channelType = MAJOR_MES_CONNECTION;
    m_strSessionID = "";
    m_strVerifyCode = "";
    m_bIsOpened = false;

    memset(m_byLoginObjGuid, 0, CX_GUID_LEN);
    string strName = "CXConnectionLoginV1";
    map<string, string>::const_iterator  it = g_mapRPCObjectGuid.find(strName);
    if (it != g_mapRPCObjectGuid.end())
    {
        string strObjectGuid = it->second;
        CXGuidObject guidObject(false);
        guidObject.ConvertGuid(strObjectGuid, m_byLoginObjGuid);
    }
}


CXClientSocketChannel::~CXClientSocketChannel()
{
}

void CXClientSocketChannel::SetUserInfo(string strRemoteUser, string strRemotePassword)
{
    m_strRemoteUser = strRemoteUser;
    m_strRemotePassword = strRemotePassword;
}


//Return value:==0 open the channel successfully
//             ==-1 invaild parameter,the session id is empty or the verify code is empty.
//             ==-2 failed to connect server
//             ==-3 failed to login the server.
int CXClientSocketChannel::OpenChannel(const CXSocketAddress& addressRemote)
{
    int iRet = RETURN_SUCCEED;
    switch (m_channelType)
    {
    case CXCommunication::CXClientSocketChannel::MAJOR_MES_CONNECTION:
        break;
    case CXCommunication::CXClientSocketChannel::MINOR_MES_CONNECTION:
    case CXCommunication::CXClientSocketChannel::DATA_CONNECTION:
    case CXCommunication::CXClientSocketChannel::OBJECT_CONNECTION:
    default:
        if (m_strSessionID == "" || m_strVerifyCode == "")
        {
            iRet = -1;
            return iRet;
        }
        break;
    }

    int iOperationRet = 0;
    if (m_bIsOpened)
    {
        return RETURN_SUCCEED;
    }

    if (Create(false) != RETURN_SUCCEED)
    {
        return -2;
    }

    if (Connect(addressRemote) != RETURN_SUCCEED)
    {
        return -2;
    }

    if (m_channelType == CXCommunication::CXClientSocketChannel::MAJOR_MES_CONNECTION)
    {
        if (Login(m_strRemoteUser, m_strRemotePassword) != RETURN_SUCCEED)
        {
            return -3;
        }

    }
    else
    {
        if (Login(m_strSessionID, m_strVerifyCode) != RETURN_SUCCEED)
        {
            return -3;
        }
    }

    if (m_strSessionID == "" || m_strVerifyCode == "")
    {
        iRet = -1;
        return iRet;
    }

    /*
    int iBufSize = 0;
    byte *pData = GetBuffer(iBufSize);
    if (pData == NULL)
    {
        return -2;
    }

    memset(pData, 0, CLIENT_BUF_SIZE);
    */

    m_bIsOpened = true;
    return RETURN_SUCCEED;
}

//login to the server by the user name ,password
//Received value: ==RETURN_SUCCEED the socket was created successfully
//                ==INVALID_PARAMETER invalid inputed parameters
//                ==-2 this socket object had not created
//                ==-3 failed to connect the peer
int CXClientSocketChannel::Login(const string &strUserName, const string &strPassword,
    int iUserType, int iSessionType,
    string strSessionGuid, string strVerifyCode)
{
    int iRet = RETURN_SUCCEED;
    if (strUserName == "" || strUserName.length()>128 || strPassword == "" || strPassword.length()>256)
    {
        return INVALID_PARAMETER;
    }

    DWORD dwBufSize = 0;
    byte byMessageData[CLIENT_BUF_SIZE] = {0};
    byte *pData = byMessageData;
    if (pData == NULL)
    {
        return -2;
    }

    memset(pData, 0, dwBufSize);

    PCXSessionLogin pMes = (PCXSessionLogin)(pData);
    pMes->dwSessionType = iSessionType;
    pMes->byUserType = iUserType;
    pMes->byConnectionType = (byte)m_channelType;

    string strUserData = strUserName + "\r" + strPassword;
    pMes->dwUserDataLen = strUserData.length() + 1;
    memcpy(pMes->szUserData, strUserData.c_str(), pMes->dwUserDataLen);

    int iMesLen = sizeof(CXSessionLogin) - 1 + pMes->dwUserDataLen;

    byte byOldGuid[CX_GUID_LEN] = {0};
    GetRPCObjectGuid(byOldGuid);
    SetRPCObjectGuid(m_byLoginObjGuid);
    int iTransDataLen = 0;
    int iTransRet = SendPacket(pData, iMesLen, CX_SESSION_LOGIN_CODE);
    if (iTransRet != 0)
    {
        Close();
        SetRPCObjectGuid(byOldGuid);
        return -3;
    }
    SetRPCObjectGuid(byOldGuid);

    memset(pData, 0, CLIENT_BUF_SIZE);
    DWORD dwMesCode = 0;
    iTransRet = RecvPacket(pData, CLIENT_BUF_SIZE, iTransDataLen, dwMesCode);
    if (iTransRet != 0)
    {
        Close();
        return -4;
    }

    if (dwMesCode != CX_SESSION_LOGIN_REPLY_CODE)
    {
        Close();
        return -5;
    }

    PCXCommonMessageReply pReply = (PCXCommonMessageReply)pData;
    if (pReply->dwReplyCode != 200)
    {
        Close();
        return -6;
    }
    if (m_channelType == CXCommunication::CXClientSocketChannel::MAJOR_MES_CONNECTION)
    {
        char* pPos = strchr(pReply->szData, '\r');
        if (pPos == NULL || (pPos - pReply->szData)>(pReply->dwDataLen - 1))
        {
            m_strSessionID = "";
            m_strVerifyCode = "";
        }
        else
        {
            *pPos = '\0';
            m_strSessionID = pReply->szData;
            m_strVerifyCode = pPos + 1;
        }
    }


    return RETURN_SUCCEED;
}

int CXClientSocketChannel::SendHeartPacket()
{
    int iRet = RETURN_SUCCEED;


    DWORD dwBufSize = 0;
    byte byMessageData[CLIENT_BUF_SIZE] = { 0 };
    byte *pData = byMessageData;

    memset(pData, 0, dwBufSize);

    int iMesLen = 0;

    int iTransDataLen = 0;
    int iTransRet = SendPacket(pData, iMesLen, CX_HEAERT_BEAT_CODE);
    if (iTransRet != 0)
    {
        Close();
        return -3;
    }
    memset(pData, 0, CLIENT_BUF_SIZE);
    DWORD dwMesCode = 0;
    iTransRet = RecvPacket(pData, CLIENT_BUF_SIZE, iTransDataLen, dwMesCode);
    if (iTransRet != 0)
    {
        Close();
        return -4;
    }

    if (dwMesCode != CX_HEAERT_BEAT_REPLY_CODE)
    {
        Close();
        return -5;
    }

    return RETURN_SUCCEED;
}
