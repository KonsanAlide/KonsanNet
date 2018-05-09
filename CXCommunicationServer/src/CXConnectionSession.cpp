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
#include "CXConnectionObject.h"
#include "CXConnectionSession.h"
#include <memory.h>
#include <time.h>
namespace CXCommunication
{
    CXConnectionSession::CXConnectionSession()
    {
        m_pMmainConnetion = NULL;
        m_strSessionGuid="";
        m_strTemporaryVerificationCode = "";
    }

    CXConnectionSession::~CXConnectionSession()
    {
        //dtor
    }

    void CXConnectionSession::Lock()
    {
        m_lock.Lock();
    }
    void CXConnectionSession::UnLock()
    {
        m_lock.Unlock();
    }

   
    int CXConnectionSession::AddMainConnection(CXConnectionObject &conObj)
    {
        Lock();
        m_pMmainConnetion = &conObj;
        UnLock();
        return RETURN_SUCCEED;
    }
    int CXConnectionSession::AddMessageConnection(CXConnectionObject &conObj)
    {
        Lock();
        m_lstMessageConnections.emplace_back(&conObj);
        UnLock();
        return RETURN_SUCCEED;
    }
    int CXConnectionSession::AddDataConnection(CXConnectionObject &conObj)
    {
        Lock();
        m_lstDataConnections.emplace_back(&conObj);
        UnLock();
        return RETURN_SUCCEED;
    }

    int CXConnectionSession::RemoveConnection(CXConnectionObject &conObj)
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

            list<CXConnectionObject*>::iterator it = m_lstMessageConnections.begin();
            for (; it != m_lstMessageConnections.end(); it++)
            {
                if (*it == &conObj)
                {
                    m_lstMessageConnections.erase(it);
                    UnLock();
                    return RETURN_SUCCEED;
                }
            }

            it = m_lstDataConnections.begin();
            for (; it != m_lstDataConnections.end(); it++)
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

    void CXConnectionSession::Destroy()
    {
        m_pMmainConnetion = NULL;
        m_strSessionGuid = "";
        m_strTemporaryVerificationCode = "";
    }

    void  CXConnectionSession::SetData(string strKey, void *pData)
    {
        Lock();
        m_mapData[strKey] = pData;
        UnLock();
    }
    void *CXConnectionSession::GetData(string strKey)
    {
        void *pData = NULL;
        Lock();
        pData = m_mapData[strKey];
        UnLock();
        return pData;
    }

    void CXConnectionSession::RemoveData(string strKey)
    {
        Lock();
        unordered_map<string, void*>::iterator it = m_mapData.find(strKey);
        if (it != m_mapData.end())
        {
            m_mapData.erase(it);
        }
        UnLock();
    }

    int  CXConnectionSession::GetConnectionNumber()
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
}
