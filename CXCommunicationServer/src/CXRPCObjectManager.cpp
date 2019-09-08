/****************************************************************************
Copyright (c) 2018-2019 Chance Yang

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Description:
*****************************************************************************/
#include "CXRPCObjectManager.h"
using  namespace CXCommunication;

CXRPCObjectManager::CXRPCObjectManager()
{
	m_uiUsingObjects =0;
	m_uiFreeObjects = 0;
}

CXRPCObjectManager::~CXRPCObjectManager()
{
    m_lock.Lock();
    unordered_map<string, CXRPCObjectServer *>::iterator it= m_mapRegisterObject.begin();
    for (; it != m_mapRegisterObject.end();)
    {
        it = m_mapRegisterObject.erase(it);
    }

	unordered_map<string, CXRPCObjectServer *>::iterator itUsing = m_mapUsingObjs.begin();
	for (; itUsing != m_mapUsingObjs.end();)
	{
		if (itUsing->second != NULL)
		{
			itUsing->second->Destroy();
			delete itUsing->second;
		}
		itUsing = m_mapUsingObjs.erase(itUsing);
	}

	unordered_map<string, list<CXRPCObjectServer *>>::iterator itFree = m_mapFreeObjs.begin();
	for (; itFree != m_mapFreeObjs.end();)
	{
		list<CXRPCObjectServer *>::iterator itList = itFree->second.begin();
		for (; itList != itFree->second.end();)
		{
			CXRPCObjectServer * pObj = *itList;
			if (pObj != NULL)
			{
				pObj->Destroy();
				delete pObj;
			}
			itList = itFree->second.erase(itList);
		}
		itFree = m_mapFreeObjs.erase(itFree);
	}

    m_lock.Unlock();
}

void CXRPCObjectManager::Register(const string &strObjectClassGuid, CXRPCObjectServer *pObj)
{
    m_lock.Lock();
    m_mapRegisterObject[strObjectClassGuid] = pObj;
    m_lock.Unlock();
    
}

CXRPCObjectServer * CXRPCObjectManager::GetFreeObject(const string &strObjectClassGuid)
{
    CXRPCObjectServer * pObj = NULL;

    m_lock.Lock();
	unordered_map<string, list<CXRPCObjectServer *>>::iterator itFree = m_mapFreeObjs.find(strObjectClassGuid);
	if (itFree != m_mapFreeObjs.end())
	{
		if (itFree->second.size() > 0)
		{
			pObj = itFree->second.front();
			itFree->second.pop_front();
			m_uiFreeObjects--;
			if (pObj != NULL)
			{
				pObj->Init();
				m_uiUsingObjects++;
				m_mapUsingObjs[pObj->GetObjectID()] = pObj;
			}
		}
	}

	if (pObj == NULL)
	{
		unordered_map<string, CXRPCObjectServer *>::iterator it;
		it = m_mapRegisterObject.find(strObjectClassGuid);
		if (it != m_mapRegisterObject.end())
		{
			if (it->second->IsUniqueInstance())
			{
				pObj = it->second;
			}
			else
			{
				pObj = it->second->CreateObject();
				if (pObj != NULL)
				{
					pObj->Init();
					m_uiUsingObjects++;
					m_mapUsingObjs[pObj->GetObjectID()] = pObj;
				}
			}
		}
	}

	m_lock.Unlock();

    return pObj;
}

CXRPCObjectServer * CXRPCObjectManager::GetStaticObject(const string &strObjectClassGuid)
{
	CXRPCObjectServer * pObj = NULL;

	m_lock.Lock();
	unordered_map<string, CXRPCObjectServer *>::iterator it;
	it = m_mapRegisterObject.find(strObjectClassGuid);
	if (it != m_mapRegisterObject.end())
	{
		pObj = it->second;
		m_lock.Unlock();
	}
	else
	{
		m_lock.Unlock();
	}

	return pObj;
}

// free a object to the free pool
void CXRPCObjectManager::FreeObject(CXRPCObjectServer * pObj)
{
	if (pObj->IsUniqueInstance())
		return;
	m_lock.Lock();

	unordered_map<string, CXRPCObjectServer *>::iterator itUsing = m_mapUsingObjs.find(pObj->GetObjectID());
	if (itUsing != m_mapUsingObjs.end())
	{
		m_mapUsingObjs.erase(itUsing);
		unordered_map<string, list<CXRPCObjectServer *>>::iterator itFree = m_mapFreeObjs.find(pObj->GetClassID());
		if (itFree != m_mapFreeObjs.end())
		{
			itFree->second.push_back(pObj);
		}
		else
		{
			list<CXRPCObjectServer *> lstObjs;
			lstObjs.push_back(pObj);
			m_mapFreeObjs[pObj->GetClassID()] = lstObjs;
		}
	}

	m_lock.Unlock();
}

CXRPCObjectServer * CXRPCObjectManager::FindUsingObject(const string &strObjectID)
{
	CXRPCObjectServer * pObj = NULL;

	m_lock.Lock();
	unordered_map<string, CXRPCObjectServer *>::iterator itUsing = m_mapUsingObjs.find(strObjectID);
	if (itUsing != m_mapUsingObjs.end())
	{
		pObj = itUsing->second;
	}
	m_lock.Unlock();

	return pObj;
}

void CXRPCObjectManager::Lock()
{
    m_lock.Lock();
}
void CXRPCObjectManager::UnLock()
{
    m_lock.Unlock();
}

void CXRPCObjectManager::TravelTimeoutRequest()
{
	m_lock.Lock();
	unordered_map<string, CXRPCObjectServer *>::iterator itUsing = m_mapUsingObjs.begin();
	for (; itUsing != m_mapUsingObjs.end();++itUsing)
	{
		if (itUsing->second != NULL)
		{
			itUsing->second->DetectTimeoutRequests();
		}
	}
	m_lock.Unlock();
}
