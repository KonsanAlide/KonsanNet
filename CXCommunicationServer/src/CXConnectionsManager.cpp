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
#include <chrono>
#include <ctime>
#include "CXRPCObjectManager.h"
using namespace std;


void* ThreadDetect(void* lpvoid);
namespace CXCommunication
{
    CXConnectionsManager::CXConnectionsManager()
    {
        m_uiCurrentConnectionIndex = 0;
        m_pfOnClose = NULL;
        m_bStarted = false;
        m_pLogHandle = NULL;
		m_pObjectPool = NULL;
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
    void CXConnectionsManager::AddUsingConnection(CXConnectionObject * pObj)
    {
        m_lockUsingConnections.Lock();
        if (!pObj->IsProxyConnection())
        {
            unordered_map<uint64, CXConnectionObject *>::iterator itFind = m_mapUsingConnections.find(pObj->GetConnectionIndex());
            if (itFind == m_mapUsingConnections.end())
            {
                m_mapUsingConnections[pObj->GetConnectionIndex()] = pObj;
            } 
        }
        m_lockUsingConnections.Unlock();
    }

    CXConnectionObject *  CXConnectionsManager::FindUsingConnection(uint64 uiConIndex)
    {
        CXConnectionObject * pObj = NULL;
        m_lockUsingConnections.Lock();
        unordered_map<uint64, CXConnectionObject *>::iterator it = m_mapUsingConnections.find(uiConIndex);
        if (it != m_mapUsingConnections.end())
        {
            pObj = it->second;
        }
        m_lockUsingConnections.Unlock();
        return pObj;
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
        while (m_bStarted)
        {
            m_eveWaitDetect.WaitForSingleObject(50);
            if (!m_bStarted)
            {
                break;
            }

			DetectTimeoutConnections();
			TravelReleaseList();
			DetectRequestTimeout();
            
        }
        return true;
    }

    DWORD ThreadDetect(void* lpvoid)
    {
        CXConnectionsManager * pServer = (CXConnectionsManager*)lpvoid;
        bool bRet = pServer->DetectConnections();
        if (bRet)
            return 0;
        else
            return (DWORD)(-1);
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

        if (pObj->IsProxyConnection())
        {
            char szBuf[128] = { 0 };
            sprintf_s(szBuf, 128, "%s:%d", pObj->GetRemoteIP().c_str(), pObj->GetRemotePort());
            unordered_map<string, unordered_map<uint64, CXConnectionObject *>>::iterator it;
            it = m_mapAttachProxyConnectionsPool.find(szBuf);
            if (it != m_mapAttachProxyConnectionsPool.end())
            {
                unordered_map<uint64, CXConnectionObject *>::iterator itCon;
                itCon = it->second.find(pObj->GetConnectionIndex());
                if (itCon != it->second.end())
                {
                    it->second.erase(itCon);
                    m_lockUsingConnections.Unlock();

                    m_lockReleasedConnections.Lock();
                    m_listReleasedConnections.push_back(pObj);
                    m_lockReleasedConnections.Unlock();
                    return;
                }
            }
            m_lockUsingConnections.Unlock();
        }
        else
        {
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

	CXConnectionObject * CXConnectionsManager::GetProxyClient(string strIP, WORD wPort)
	{
		CXConnectionObject * pCon = NULL;
		char szBuf[128] = { 0 };
		sprintf_s(szBuf, 128, "%s:%d", strIP.c_str(), wPort);
		unordered_map<string, unordered_map<uint64, CXConnectionObject *>>::iterator it;
		it = m_mapAttachProxyConnectionsPool.find(szBuf);
		if (it != m_mapAttachProxyConnectionsPool.end())
		{
			
		}
		return pCon;
	}


	void CXConnectionsManager::DetectTimeoutConnections()
	{
		int64 uiCurTime = GetCurrentTimeMS();
		uint64 uiBeginTime = 0;
		queue<CXConnectionObject *> queueTimeOutConnections;
		m_lockUsingConnections.Lock();
		unordered_map<uint64, CXConnectionObject *>::iterator it;
		it = m_mapUsingConnections.begin();
		for (; it != m_mapUsingConnections.end(); ++it)
		{
			CXConnectionObject * pCon = it->second;
			if (pCon->GetTimeOutMSeconds() != 0xffffffff)
			{
				uiBeginTime = pCon->GetLastPacketTime();
				if (uiBeginTime == 0)
				{
					uiBeginTime = pCon->GetAcceptedTime();
				}
				if (uiCurTime > uiBeginTime && (uiCurTime- uiBeginTime)> (pCon->GetTimeOutMSeconds() * 2))
				{
					queueTimeOutConnections.push(pCon);
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
				if (m_pLogHandle != NULL)
					m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
				m_pfOnClose(*pCon, TIME_OUT);
			}
		}
	}
	void CXConnectionsManager::TravelReleaseList()
	{
		int64 uiCurTime = GetCurrentTimeMS();
		m_lockReleasedConnections.Lock();
		list<CXConnectionObject*>::iterator itReleased = m_listReleasedConnections.begin();
		for (; itReleased != m_listReleasedConnections.end();)
		{
			CXConnectionObject * pCon = *itReleased;
			if (pCon != NULL)
			{
				if (uiCurTime > pCon->GetReleasedTime())
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
	void CXConnectionsManager::DetectRequestTimeout()
	{
		if (m_pObjectPool != NULL)
		{
			((CXRPCObjectManager*)m_pObjectPool)->TravelTimeoutRequest();
		}
	}

	//pszTimeString: if not NULL,will save the time string , the format is: 2019-07-31_15:39:29
	int64 CXConnectionsManager::GetCurrentTimeMS()
	{
		typedef chrono::time_point<chrono::system_clock, chrono::milliseconds> MsClockType;
		MsClockType tp = chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now());
		return (int64)tp.time_since_epoch().count();
	}

	CXConnectionObject * CXConnectionsManager::GetFreeProxyClient(string strIP, WORD wPort)
	{
		CXConnectionObject* pCon = NULL;
		m_lockFreeConnections.Lock();
		
		char szBuf[128] = { 0 };
		sprintf_s(szBuf, 128, "%s:%d", strIP.c_str(), wPort);
		unordered_map<string, list<CXConnectionObject *>>::iterator it;
		it = m_mapDetachedProxyConnectionsPool.find(szBuf);
		if (it != m_mapDetachedProxyConnectionsPool.end())
		{
            bool  bFind = false;
            do
            {
                if (it->second.size() > 0)
                {
                    pCon = it->second.front();
                    it->second.pop_front();
                    if (pCon->GetState() != CXConnectionObject::ESTABLISHED)
                    {
                        if (m_pfOnClose != NULL)
                        {
                            char  szInfo[1024] = { 0 };
                            sprintf_s(szInfo, 1024, "Found a proxy connection is closed,close it, connection index is %lld\n",
                                pCon->GetConnectionIndex());
                            if (m_pLogHandle != NULL)
                                m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
                            m_pfOnClose(*pCon, TIME_OUT);
                        }
                    }
                    else
                    {
                        bFind = true;
                        break;
                    }
                }
                else
                {
                    break;
                }
            } while (!bFind);
		}

		if (pCon == NULL)
		{
			try
			{
				pCon = new CXConnectionObject();
				if (pCon == NULL)
				{
					return NULL;
				}
                pCon->SetState(CXConnectionObject::CREATING);
			}
			catch (const bad_alloc& e)
			{
			}
		}
		m_lockFreeConnections.Unlock();
		return pCon;
	}

    void CXConnectionsManager::AttachProxyConnection(CXConnectionObject * pObj)
    {
        m_lockUsingConnections.Lock();
        char szBuf[128] = { 0 };
        sprintf_s(szBuf, 128, "%s:%d", pObj->GetRemoteIP().c_str(), pObj->GetRemotePort());
        unordered_map<string, unordered_map<uint64, CXConnectionObject *>>::iterator it;
        it = m_mapAttachProxyConnectionsPool.find(szBuf);
        if (it != m_mapAttachProxyConnectionsPool.end())
        {
            it->second.insert(make_pair(pObj->GetConnectionIndex(), pObj));
        }
        else
        {
            unordered_map<uint64, CXConnectionObject *>mapProxyCons;
            mapProxyCons.insert(make_pair(pObj->GetConnectionIndex(), pObj));
            m_mapAttachProxyConnectionsPool.insert(make_pair(szBuf, mapProxyCons));
        }
        m_lockUsingConnections.Unlock();
    }

    void CXConnectionsManager::DetachProxyConnection(CXConnectionObject * pObj)
    {
        m_lockUsingConnections.Lock();
        char szBuf[128] = { 0 };
        sprintf_s(szBuf, 128, "%s:%d", pObj->GetRemoteIP().c_str(), pObj->GetRemotePort());
        unordered_map<string, unordered_map<uint64, CXConnectionObject *>>::iterator it;
        it = m_mapAttachProxyConnectionsPool.find(szBuf);
        if (it != m_mapAttachProxyConnectionsPool.end())
        {
            m_mapAttachProxyConnectionsPool.erase(it);

            unordered_map<string, list<CXConnectionObject *>>::iterator itDetach;
            itDetach = m_mapDetachedProxyConnectionsPool.find(szBuf);
            if (itDetach != m_mapDetachedProxyConnectionsPool.end())
            {
                itDetach->second.push_back(pObj);
            }
            else
            {
                list<CXConnectionObject *> lstDetach;
                lstDetach.push_back(pObj);
                m_mapDetachedProxyConnectionsPool[szBuf] = lstDetach;
            }
        }
        m_lockUsingConnections.Unlock();
    }
}

