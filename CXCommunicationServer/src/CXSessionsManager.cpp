#include "CXSessionsManager.h"
using  namespace CXCommunication;

CXSessionsManager::CXSessionsManager()
{
}

CXSessionsManager::~CXSessionsManager()
{
    Destroy();
}

CXConnectionSession * CXSessionsManager::GetFreeSession()
{
    CXConnectionSession * pObj = NULL;
    m_lock.Lock();
    int iSize = m_queueFreeSessions.size();
    if (iSize <= 0)
    {
        for (int i = 0; i<1000; i++)
        {
            CXConnectionSession * pTempObj = new CXConnectionSession;
            if (pTempObj != NULL)
            {
                m_queueFreeSessions.push(pTempObj);
            }
        }
    }

    iSize = m_queueFreeSessions.size();
    if (iSize>0)
    {
        pObj = m_queueFreeSessions.front();
        m_queueFreeSessions.pop();
    }
    m_lock.Unlock();
    return pObj;
}

int  CXSessionsManager::AddUsingSession(CXConnectionSession * pObj)
{
    m_lock.Lock();
    if (m_mapUsingSessions.find(pObj->GetSessionGuid()) != m_mapUsingSessions.end())
    {
        m_lock.Unlock();
        return -1;
    }
    else
    {
        m_mapUsingSessions[pObj->GetSessionGuid()] = pObj;
        m_lock.Unlock();
        return 0;
    }
}

void CXSessionsManager::CloseSession(CXConnectionSession * pObj, bool bNeedLockBySelf)
{
    if(bNeedLockBySelf)
        m_lock.Lock();
    unordered_map<string, CXConnectionSession *>::iterator it;
    it = m_mapUsingSessions.find(pObj->GetSessionGuid());
    if (it != m_mapUsingSessions.end())
    {
        m_mapUsingSessions.erase(it);
        pObj->Destroy();
        m_queueFreeSessions.push(pObj);
    }

    if (bNeedLockBySelf)
        m_lock.Unlock();
}

string CXSessionsManager::BuildSessionGuid()
{
    return m_cxGuidGenerater.GenerateGuid();
}

CXConnectionSession * CXSessionsManager::FindUsingSession(string strSessionGuid)
{
    m_lock.Lock();
    unordered_map<string, CXConnectionSession *>::iterator it;
    it = m_mapUsingSessions.find(strSessionGuid);
    if (it != m_mapUsingSessions.end())
    {
        m_lock.Unlock();
        return (CXConnectionSession*)(it->second);
    }
    else
    {
        m_lock.Unlock();
    }
    return NULL;
}

void CXSessionsManager::Destroy()
{
    unordered_map<string, CXConnectionSession *>::iterator it;
    it = m_mapUsingSessions.begin();
    for (;it!= m_mapUsingSessions.end();)
    {
        it=m_mapUsingSessions.erase(it);
    }

    for (int i=0;i<m_queueFreeSessions.size();)
    {
        m_queueFreeSessions.pop();
    }
}

void CXSessionsManager::Lock()
{
    m_lock.Lock();
}
void CXSessionsManager::UnLock()
{
    m_lock.Unlock();
}
