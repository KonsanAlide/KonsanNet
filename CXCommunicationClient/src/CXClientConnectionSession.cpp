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
#include "CXClientConnectionSession.h"

using namespace CXCommunication;
CXClientConnectionSession::CXClientConnectionSession()
{
    m_pMmainConnetion = NULL;
    m_strSessionID = "";
    m_strVerifyCode = "";
    m_strRemoteUser = "";
    m_strRemotePassword = "";
}


CXClientConnectionSession::~CXClientConnectionSession()
{
    Close();
}

void CXClientConnectionSession::Lock()
{
    m_lock.Lock();
}
void CXClientConnectionSession::UnLock()
{
    m_lock.Unlock();
}

int CXClientConnectionSession::AddMainConnection(CXClientSocketChannel &conObj)
{
    Lock();
    m_pMmainConnetion = &conObj;
    UnLock();
    return RETURN_SUCCEED;
}
int CXClientConnectionSession::AddMessageConnection(CXClientSocketChannel &conObj)
{
    Lock();
    m_lstMessageConnections.push_back(&conObj);
    UnLock();
    return RETURN_SUCCEED;
}
int CXClientConnectionSession::AddDataConnection(CXClientSocketChannel &conObj)
{
    Lock();
    m_lstDataConnections.push_back(&conObj);
    UnLock();
    return RETURN_SUCCEED;
}

int CXClientConnectionSession::RemoveConnection(CXClientSocketChannel &conObj)
{
    Lock();
    if (m_pMmainConnetion == &conObj)
    {
        m_pMmainConnetion = NULL;
        UnLock();
        return RETURN_SUCCEED;
    }
    else
    {

        list<CXClientSocketChannel*>::iterator it = m_lstMessageConnections.begin();
        for (; it != m_lstMessageConnections.end(); ++it)
        {
            if (*it == &conObj)
            {
                m_lstMessageConnections.erase(it);
                UnLock();
                return RETURN_SUCCEED;
            }
        }

        it = m_lstDataConnections.begin();
        for (; it != m_lstDataConnections.end(); ++it)
        {
            if (*it == &conObj)
            {
                m_lstDataConnections.erase(it);
                UnLock();
                return RETURN_SUCCEED;
            }
        }
    }


    UnLock();
    return -2;
}

void CXClientConnectionSession::Close()
{
    Lock();
    if (m_pMmainConnetion != NULL)
    {
        m_pMmainConnetion->Close();
        m_pMmainConnetion = NULL;
    }

    list<CXClientSocketChannel*>::iterator it = m_lstMessageConnections.begin();
    for (; it != m_lstMessageConnections.end(); )
    {
        CXClientSocketChannel *pChannel = *it;
        pChannel->Close();
        it = m_lstMessageConnections.erase(it);
    }

    it = m_lstDataConnections.begin();
    for (; it != m_lstDataConnections.end();)
    {
        CXClientSocketChannel *pChannel = *it;
        pChannel->Close();
        it = m_lstDataConnections.erase(it);
    }

    UnLock();
    m_strSessionID = "";
    m_strVerifyCode = "";
}

void  CXClientConnectionSession::SetData(string strKey, void *pData)
{
    Lock();
    m_mapData[strKey] = pData;
    UnLock();
}
void *CXClientConnectionSession::GetData(string strKey)
{
    void *pData = NULL;
    Lock();
    pData = m_mapData[strKey];
    UnLock();
    return pData;
}

void CXClientConnectionSession::RemoveData(string strKey)
{
    Lock();
    unordered_map<string, void*>::iterator it = m_mapData.find(strKey);
    if (it != m_mapData.end())
    {
        m_mapData.erase(it);
    }
    UnLock();
}

int  CXClientConnectionSession::GetConnectionNumber()
{
    int iNumber = 0;
    Lock();
    if (m_pMmainConnetion != NULL)
        iNumber++;

    iNumber += m_lstMessageConnections.size();
    iNumber += m_lstDataConnections.size();

    UnLock();
    return iNumber;
}


void CXClientConnectionSession::SetUserInfo(string strRemoteUser, string strRemotePassword)
{
    m_strRemoteUser = strRemoteUser;
    m_strRemotePassword = strRemotePassword;
}

//Open a socket channel
//Return value:==0 open the channel successfully
//             ==-1 invaild parameter,the session id is empty or the verify code is empty.
//             ==-2 failed to connect server
//             ==-3 failed to login the server.
//             ==-4 the major channel exists.
//             ==-5 Unknown error.
int CXClientConnectionSession::OpenChannel(CXClientSocketChannel &conObj,
    CXClientSocketChannel::ChannelType chanelType)
{
    if (!m_addressRemote.IsHaveAddress())
    {
        return INVALID_PARAMETER;
    }

    m_lock.Lock();
    int iRet = RETURN_SUCCEED;
    if (chanelType == CXCommunication::CXClientSocketChannel::MAJOR_MES_CONNECTION)
    {
        if (m_pMmainConnetion != NULL && m_pMmainConnetion->IsOpen())
        {
            m_lock.Unlock();
            return -4;
        }
    }
    CXClientSocketChannel *pChannel = &conObj;

    pChannel->SetChannelType(chanelType);
    pChannel->SetUserInfo(m_strRemoteUser, m_strRemotePassword);
    pChannel->SetSessionID(m_strSessionID);
    pChannel->SetVerifyCode(m_strVerifyCode);
    int iOperationRet = pChannel->OpenChannel(m_addressRemote);
    if (iOperationRet == RETURN_SUCCEED)
    {
        switch (chanelType)
        {
        case CXCommunication::CXClientSocketChannel::MAJOR_MES_CONNECTION:
            m_pMmainConnetion = pChannel;
            m_strSessionID= pChannel->GetSessionID();
            m_strVerifyCode = pChannel->GetVerifyCode();
            break;
        case CXCommunication::CXClientSocketChannel::MINOR_MES_CONNECTION:
            m_lstMessageConnections.push_back(pChannel);
            break;
        case CXCommunication::CXClientSocketChannel::DATA_CONNECTION:
            m_lstDataConnections.push_back(pChannel);
            break;
        case CXCommunication::CXClientSocketChannel::OBJECT_CONNECTION:
        {
            m_lstObjectConnections.push_back(pChannel);
            break;
        }
        default:
            return -5;
        }
    }
    else
    {
        pChannel->Close();
    }
    m_lock.Unlock();
    return iOperationRet;
}