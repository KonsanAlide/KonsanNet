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
#include "CXConnectionsManager.h"
#include <time.h>
#ifndef WIN32
#include<sys/time.h>
#endif
#include "PlatformFunctionDefine.h"

void* ThreadDetect(void* lpvoid);
namespace CXCommunication
{
    CXConnectionsManager::CXConnectionsManager()
    {
        m_uiCurrentConnectionIndex = 0;
        m_pfOnClose = NULL;
        m_bStarted = false;
        m_pLogHandle = NULL;
    }

    CXConnectionsManager::~CXConnectionsManager()
    {
        Destroy();
    }

    CXConnectionObject * CXConnectionsManager::FindConnectionInPendingMap(uint64 uiConIndex)
    {
        m_lockPendingConnections.Lock();
        if(m_mapPendingConnections.find(uiConIndex) != m_mapPendingConnections.end())
        {
           CXConnectionObject * pObj = m_mapPendingConnections[uiConIndex];
           m_lockPendingConnections.Unlock();
           return pObj;
        }
        else
        {
            m_lockPendingConnections.Unlock();
            return NULL;
        }
    }

    CXConnectionObject * CXConnectionsManager::GetFreeConnectionObj()
    {
        CXConnectionObject * pObj = NULL;
        m_lockFreeConnections.Lock();
        int iSize = m_queueFreeConnections.size();
        if(iSize<=0)
        {
            for(int i=0;i<1000;i++)
            {
                CXConnectionObject * pTempObj = new CXConnectionObject;
                if(pTempObj!=NULL)
                {
                    m_queueFreeConnections.push(pTempObj);
                }
            }
        }

        iSize = m_queueFreeConnections.size();
        if(iSize>0)
        {
            pObj = m_queueFreeConnections.front();
            m_queueFreeConnections.pop();
        }
        m_lockFreeConnections.Unlock();
        pObj->SetState(CXConnectionObject::INITIALIZED);
        return pObj;
    }

    //return value:
    //    ==0 succeed
    //    ==-1 find a same index connection
    int CXConnectionsManager::AddPendingConnection(CXConnectionObject * pObj)
    {
        m_lockPendingConnections.Lock();
        if(m_mapPendingConnections.find(pObj->GetConnectionIndex()) != m_mapPendingConnections.end())
        {
            m_lockPendingConnections.Unlock();
           return -1;
        }
        else
        {
            m_mapPendingConnections.insert(make_pair(pObj->GetConnectionIndex(),pObj));
            m_lockPendingConnections.Unlock();
            return 0;
        }
    }

    void CXConnectionsManager::RemovePendingConnection(CXConnectionObject * pObj)
    {
        m_lockPendingConnections.Lock();
        unordered_map<uint64, CXConnectionObject *>::iterator it;
        it =  m_mapPendingConnections.find(pObj->GetConnectionIndex());
        if( it!= m_mapPendingConnections.end())
        {
           m_mapPendingConnections.erase(it);
           m_lockPendingConnections.Unlock();
        }
        else
        {
            m_lockPendingConnections.Unlock();
        }
    }

    void CXConnectionsManager::RemovePendingConnection(uint64 uiConIndex)
    {
        m_lockPendingConnections.Lock();
        unordered_map<uint64, CXConnectionObject *>::iterator it;
        it =  m_mapPendingConnections.find(uiConIndex);
        if( it!= m_mapPendingConnections.end())
        {
           m_mapPendingConnections.erase(it);
           m_lockPendingConnections.Unlock();
        }
        else
        {
            m_lockPendingConnections.Unlock();
        }
    }

    void CXConnectionsManager::AddFreeConnectionObj(CXConnectionObject * pObj)
    {
        m_lockFreeConnections.Lock();
        m_queueFreeConnections.push(pObj);
        m_lockFreeConnections.Unlock();
    }


    //return value:
    //    ==0 succeed
    //    ==-1 find a same index connection
    int CXConnectionsManager::AddUsingConnection(CXConnectionObject * pObj)
    {
        m_lockUsingConnections.Lock();
        if (m_mapUsingConnections.find(pObj->GetConnectionIndex()) != m_mapUsingConnections.end())
        {
            m_lockUsingConnections.Unlock();
            return -1;
        }
        else
        {
            m_mapUsingConnections[pObj->GetConnectionIndex()] = pObj;
            //m_mapUsingConnections.insert(make_pair(pObj->GetConnectionIndex(), pObj));
            m_lockUsingConnections.Unlock();
            return 0;
        }
    }

    uint64 CXConnectionsManager::GetCurrentConnectionIndex()
    {
        m_lockUsingConnections.Lock();
        uint64 uiIndex = ++m_uiCurrentConnectionIndex;
        m_lockUsingConnections.Unlock();
        return uiIndex;
    }

    void CXConnectionsManager::Destroy()
    {
        Stop();
        unordered_map<uint64, CXConnectionObject *>::iterator it;
        it = m_mapUsingConnections.begin();
        for (; it != m_mapUsingConnections.end(); )
        {
            it = m_mapUsingConnections.erase(it);
        }
    }

    uint64 CXConnectionsManager::GetTotalConnectionsNumber()
    {
        return m_mapUsingConnections.size();
    }

    //the thread used to detect the timeout event of the connections
    //contain the process of the pending connections
    bool CXConnectionsManager::DetectConnections()
    {
        uint64 iIndex = 0;
        while (m_bStarted)
        {
            m_eveWaitDetect.WaitForSingleObject(50);
            if (!m_bStarted)
            {
                break;
            }
            iIndex++;
#ifdef WIN32
            uint64 uiCurTime = GetTickCount64();
#else
            struct timeval tv;
            gettimeofday(&tv, NULL);
            uint64 uiCurTime = (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
            queue<CXConnectionObject *> queueTimeOutConnections;
            m_lockUsingConnections.Lock();
            unordered_map<uint64, CXConnectionObject *>::iterator it;
            it = m_mapUsingConnections.begin();
            for (; it != m_mapUsingConnections.end(); ++it)
            {
                CXConnectionObject * pCon = it->second;
                if (pCon->GetTimeOutMSeconds() != 0xffffffff)
                {
                    if (pCon->GetState() < CXConnectionObject::CLOSING)
                    {
                        if (pCon->GetLastPacketTime() == 0)
                        {
                            if (uiCurTime>pCon->GetAcceptedTime())
                            {
                                if (uiCurTime - pCon->GetAcceptedTime() > (pCon->GetTimeOutMSeconds() * 2))
                                {
                                    queueTimeOutConnections.push(pCon);
                                }
                            }
                        }
                        else
                        {
                            if (uiCurTime>pCon->GetLastPacketTime())
                            {
                                if (uiCurTime - pCon->GetLastPacketTime() > (pCon->GetTimeOutMSeconds() * 2))
                                {
                                    queueTimeOutConnections.push(pCon);
                                }
                            }
                        }
                    }
                    else if (iIndex % 10 == 0) //clear the time out connections
                    {
                        uint64 nTimeSpec = 0;
                        if (pCon->GetLastPacketTime() == 0)
                        {
                            if (uiCurTime > pCon->GetAcceptedTime())
                            {
                                nTimeSpec = uiCurTime - pCon->GetAcceptedTime();
                            }
                        }
                        else if (uiCurTime > pCon->GetLastPacketTime())
                        {
                            nTimeSpec = uiCurTime - pCon->GetLastPacketTime();
                        }

                        if (nTimeSpec > 0 && nTimeSpec > (pCon->GetTimeOutMSeconds() * 2))
                        {
                            queueTimeOutConnections.push(pCon);
                        }
                    }
                }
            }

            m_lockUsingConnections.Unlock();

            while (queueTimeOutConnections.size() > 0)
            {
                CXConnectionObject * pCon = queueTimeOutConnections.front();
                queueTimeOutConnections.pop();
                if (m_pfOnClose != NULL)
                {

                    char  szInfo[1024] = { 0 };
                    sprintf_s(szInfo, 1024, "A connection is timeout,close it, connection index is %lld\n",
                        pCon->GetConnectionIndex());
                    if(m_pLogHandle!= NULL)
                        m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
                    m_pfOnClose(*pCon, TIME_OUT);
                }
            }

#ifdef WIN32
            uiCurTime = GetTickCount64();
#else
            gettimeofday(&tv, NULL);
            uiCurTime = (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
            
            m_lockReleasedConnections.Lock();
            list<CXConnectionObject*>::iterator itReleased = m_listReleasedConnections.begin();
            for (; itReleased!= m_listReleasedConnections.end();)
            {
                CXConnectionObject * pCon = *itReleased;
                if (pCon != NULL)
                {
                    if (uiCurTime>pCon->GetReleasedTime())
                    {
                        if (uiCurTime - pCon->GetReleasedTime() > 2000)
                        {
                            AddFreeConnectionObj(pCon);
                            itReleased = m_listReleasedConnections.erase(itReleased);
                            continue;
                        }
                    }
                }
                ++itReleased;
            }
            m_lockReleasedConnections.Unlock();
            
        }
        return true;
    }

    void* ThreadDetect(void* lpvoid)
    {
        CXConnectionsManager * pServer = (CXConnectionsManager*)lpvoid;
        bool bRet = pServer->DetectConnections();
        if (bRet)
            return 0;
        else
            return (void*)(-1);
    }

    // start the detected thread
    bool CXConnectionsManager::StartDetectThread()
    {
        m_bStarted = true;
        int iRet = m_threadTimeOutDetect.Start(ThreadDetect,this);
        if (iRet != 0)
        {
            m_bStarted = false;
            return false;
        }
        return true;
    }
    // start the detected thread
    void CXConnectionsManager::Stop()
    {
        if (m_bStarted)
        {
            m_bStarted = false;
            m_eveWaitDetect.SetEvent();
            m_threadTimeOutDetect.Wait();
        }
    }

    void CXConnectionsManager::ReleaseConnection(CXConnectionObject * pObj)
    {
        m_lockUsingConnections.Lock();
        unordered_map<uint64, CXConnectionObject *>::iterator it;
        it = m_mapUsingConnections.find(pObj->GetConnectionIndex());
        if (it != m_mapUsingConnections.end())
        {
            m_mapUsingConnections.erase(it);
            m_lockUsingConnections.Unlock();

            m_lockReleasedConnections.Lock();
            m_listReleasedConnections.push_back(pObj);
            m_lockReleasedConnections.Unlock();
        }
        else
        {
            m_lockUsingConnections.Unlock();
        }
    }

}

