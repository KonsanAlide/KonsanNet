#include "CXSessionsManager.h"

using  namespace CXCommunication;

CXSessionsManager::CXSessionsManager()
{
}

CXSessionsManager::~CXSessionsManager()
{
    Destroy();
}

void CXSessionsManager::AddFreeSession(CXConnectionSession * pObj)
{
    m_lockFreeSessions.Lock();
    m_queueFreeSessions.push(pObj);
    m_lockFreeSessions.Unlock();
}

CXConnectionSession * CXSessionsManager::GetFreeSession()
{
    CXConnectionSession * pObj = NULL;
    m_lockFreeSessions.Lock();
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
    m_lockFreeSessions.Unlock();
    return pObj;
}

int  CXSessionsManager::AddUsingSession(CXConnectionSession * pObj)
{
    m_lockUsingSessions.Lock();
    if (m_mapUsingSessions.find(pObj->GetSessionGuid()) != m_mapUsingSessions.end())
    {
        m_lockUsingSessions.Unlock();
        return -1;
    }
    else
    {
        m_mapUsingSessions[pObj->GetSessionGuid()] = pObj;
        m_lockUsingSessions.Unlock();
        return 0;
    }
}
void CXSessionsManager::RemoveUsingSession(CXConnectionSession * pObj)
{
    RemoveUsingSession(pObj->GetSessionGuid());
}

void CXSessionsManager::RemoveUsingSession(string strSessionGuid)
{
    m_lockUsingSessions.Lock();
    unordered_map<string, CXConnectionSession *>::iterator it;
    it = m_mapUsingSessions.find(strSessionGuid);
    if (it != m_mapUsingSessions.end())
    {
        m_mapUsingSessions.erase(it);
        m_lockUsingSessions.Unlock();
    }
    else
    {
        m_lockUsingSessions.Unlock();
    }
}

string CXSessionsManager::BuildSessionGuid()
{
    return m_cxGuidGenerater.GenerateGuid();
}

CXConnectionSession * CXSessionsManager::FindUsingSession(string strSessionGuid)
{
    m_lockUsingSessions.Lock();
    unordered_map<string, CXConnectionSession *>::iterator it;
    it = m_mapUsingSessions.find(strSessionGuid);
    if (it != m_mapUsingSessions.end())
    {
        m_lockUsingSessions.Unlock();
        return (CXConnectionSession*)(it->second);
    }
    else
    {
        m_lockUsingSessions.Unlock();
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
