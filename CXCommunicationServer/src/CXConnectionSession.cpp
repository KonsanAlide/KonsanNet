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
#include "CXRPCObjectServer.h"
namespace CXCommunication
{
    CXConnectionSession::CXConnectionSession()
    {
        m_pMmainConnetion = NULL;
        m_strSessionGuid="";
        m_strVerificationCode = "";
        m_tmBeginOfVerification=0;
        m_iTimeOutSecondsOfVerification=0;
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

            it = m_lstObjectConnections.begin();
            for (; it != m_lstObjectConnections.end(); ++it)
            {
                if (*it == &conObj)
                {
                    m_lstObjectConnections.erase(it);
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
        m_strVerificationCode = "";

        Lock();
        CXRPCObjectServer *pObj = NULL;
        unordered_map<string, void*>::iterator it = m_mapData.begin();
        for (; it != m_mapData.end();)
        {
            pObj = (CXRPCObjectServer *)it->second;
            pObj->Destroy();
            delete pObj;
            it=m_mapData.erase(it);
        }
        
        UnLock();

    }

    void  CXConnectionSession::SetData(string strKey, void *pData,bool bLockBySelf)
    {
        if(bLockBySelf)
            Lock();
        m_mapData[strKey] = pData;
        if(bLockBySelf)
            UnLock();
    }
    void *CXConnectionSession::GetData(string strKey,bool bLockBySelf)
    {
        void *pData = NULL;
        if(bLockBySelf)
            Lock();
        pData = m_mapData[strKey];
        if(bLockBySelf)
            UnLock();
        return pData;
    }

    void CXConnectionSession::RemoveData(string strKey,bool bLockBySelf)
    {
        if(bLockBySelf)
            Lock();
        unordered_map<string, void*>::iterator it = m_mapData.find(strKey);
        if (it != m_mapData.end())
        {
            m_mapData.erase(it);
        }

        if(bLockBySelf)
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

    void CXConnectionSession::ResetVerificationInfo()
    {
        srand(time(NULL));
        int iCode = rand() % 10000000;
        char szTemp[20] = {0};
        sprintf(szTemp,"%08d", iCode);
        m_strVerificationCode = szTemp;

        m_tmBeginOfVerification = time(NULL);

        // the default time out is 10 seconds
        m_iTimeOutSecondsOfVerification =60;
    }

    //verify the verification code
    //==0 succeed
    //==-1 the verification code not match
    //==-2 the verification code is time out
    int CXConnectionSession::VerifyCode(string strCode)
    {
        int iRet = RETURN_SUCCEED;
        if (strCode.compare(m_strVerificationCode) != 0)
        {
            return -1;
        }

        time_t tmCur = time(NULL);
        if (tmCur - m_tmBeginOfVerification > m_iTimeOutSecondsOfVerification)
        {
            return -2;
        }

        return iRet;
    }
}
