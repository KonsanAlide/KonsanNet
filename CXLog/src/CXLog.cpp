#include "CXLog.h"
#include <time.h>
#include <ctime> 
#include <sstream>  

void* ThreadLog(void* lpvoid);
CXLog::CXLog()
{
    m_strFilePath = "";
    m_bRunning = false;
}

CXLog::~CXLog()
{
    m_bRunning = false;
    m_event.SetEvent();
    m_threadLog.Wait();
}


bool CXLog::Initialize(string strFilePath)
{
    if (strFilePath.length()<=0)
        return false;

    FILE * fileLog = fopen(strFilePath.c_str(), "w");
    if (fileLog == NULL)
    {
        printf("Failed to open log file\n");
        return false;
    }
    fclose(fileLog);

    int iRet = m_threadLog.Start(ThreadLog,this);
    if (iRet != 0)
    {
        return false;
    }
    m_bRunning = true;
    m_strFilePath = strFilePath;

    return true;
}

//start the thread
void CXLog::Log(CXLOG_LEVEL levelType, const char *pszLogContent)
{
    return ;
    std::string strTime;
    std::stringstream ssTime;
    std::time_t currenttime = std::time(0);
    char tmCur[255] = {0};
    std::strftime(tmCur, 255, "%Y-%m-%d %H:%M:%S", std::localtime(&currenttime));
    ssTime << tmCur;
    strTime = ssTime.str();

    string strLog = "Time: "+strTime + " ";

    string strType = "Info: ";
    switch (levelType)
    {
    case CXLog::CXLOG_DEBUG:
        strType = "Debug: ";
        break;
    case CXLog::CXLOG_INFO:
        break;
    case CXLog::CXLOG_WARNNING:
        strType = "Warnning: ";
        break;
    case CXLog::CXLOG_ERROR:
        strType = "Error: ";
        break;
    default:
        break;
    }
    strLog += strType+" "+ pszLogContent;

    m_lock.Lock();
    m_lstLogContent.push_back(strLog);
    m_lock.Unlock();
    m_event.SetEvent();
}


int  CXLog::Run()
{
    FILE * fileLog = fopen(m_strFilePath.c_str(), "w");
    if (fileLog == NULL)
    {
        printf("Failed to open log file\n");
        return -1;
    }

    while (m_bRunning)
    {
        m_event.WaitForSingleObject(100);

        while (m_lstLogContent.size()>0)
        {
            m_lock.Lock();
            string strLog = m_lstLogContent.front();
            m_lstLogContent.pop_front();
            m_lock.Unlock();
            if (strLog.rfind('\n') != (strLog.size() - 1))
            {
                strLog += "\n";
            }
            fprintf(fileLog, strLog.c_str());
        }
        fflush(fileLog);
    }
   
    fclose(fileLog);

    return 0;
}

void* ThreadLog(void* lpvoid)
{
    CXLog * pServer = (CXLog*)lpvoid;
    return (void*)pServer->Run();
}