#include "CXRPCObjectManager.h"
using  namespace CXCommunication;

CXRPCObjectManager::CXRPCObjectManager()
{
}

CXRPCObjectManager::~CXRPCObjectManager()
{
    m_lock.Lock();
    unordered_map<string, CXRPCObjectServer *>::iterator it= m_mapRegisterObject.begin();
    for (; it != m_mapRegisterObject.end();)
    {
        it = m_mapRegisterObject.erase(it);
    }

    m_lock.Unlock();
}

void CXRPCObjectManager::Register(const string &strObjectGuid, CXRPCObjectServer *pObj)
{
    m_lock.Lock();
    m_mapRegisterObject[strObjectGuid] = pObj;
    m_lock.Unlock();
    
}

CXRPCObjectServer * CXRPCObjectManager::GetRPCObject(const string &strObjectGuid)
{
    CXRPCObjectServer * pObj = NULL;

    m_lock.Lock();
    unordered_map<string, CXRPCObjectServer *>::iterator it;
    it = m_mapRegisterObject.find(strObjectGuid);
    if (it != m_mapRegisterObject.end())
    {
        if (it->second->IsUniqueInstance())
        {
            pObj = it->second;
        }
        else
        {
            pObj = it->second->CreateObject();
        }
        
        m_lock.Unlock();
    }
    else
    {
        m_lock.Unlock();
    }

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
