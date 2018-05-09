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
namespace CXCommunication
{
    CXConnectionsManager::CXConnectionsManager()
    {
        //m_queueFreeConnections
        m_uiCurrentConnectionIndex = 0;
    }

    CXConnectionsManager::~CXConnectionsManager()
    {
        //dtor
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

    void CXConnectionsManager::RemoveUsingConnection(CXConnectionObject * pObj)
    {
        m_lockUsingConnections.Lock();
        unordered_map<uint64, CXConnectionObject *>::iterator it;
        it = m_mapUsingConnections.find(pObj->GetConnectionIndex());
        if (it != m_mapUsingConnections.end())
        {
            m_mapUsingConnections.erase(it);
            m_lockUsingConnections.Unlock();
        }
        else
        {
            m_lockUsingConnections.Unlock();
        }
    }

    void CXConnectionsManager::RemoveUsingConnection(uint64 uiConIndex)
    {
        m_lockUsingConnections.Lock();
        unordered_map<uint64, CXConnectionObject *>::iterator it;
        it = m_mapUsingConnections.find(uiConIndex);
        if (it != m_mapUsingConnections.end())
        {
            m_mapUsingConnections.erase(it);
            m_lockUsingConnections.Unlock();
        }
        else
        {
            m_lockUsingConnections.Unlock();
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
        unordered_map<uint64, CXConnectionObject *>::iterator it;
        it = m_mapUsingConnections.begin();
        for (; it != m_mapUsingConnections.end(); it++)
        {
            it = m_mapUsingConnections.erase(it);
        }
    }

}
