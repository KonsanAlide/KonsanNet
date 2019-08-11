#include "CXLog.h"
#include <time.h>
#include <chrono>
#include <ctime>
using namespace std;
#include <sstream>
#include "PlatformFunctionDefine.h"

void* ThreadLog(void* lpvoid);
CXLog::CXLog()
{
    m_strFilePath = "";
    m_bRunning = false;
    m_bOutputLevel = CXLOG_DEBUG;
    m_bDirectWrite = false;
	m_iFlushIntervalMs = 1000;
}

CXLog::~CXLog()
{
    m_bRunning = false;
    m_event.SetEvent();
    m_threadLog.Wait();
}


bool CXLog::Initialize(string strFilePath, CXLOG_LEVEL bOutputLevel, bool bDirectWrite)
{
    if (strFilePath.length()<=0)
        return false;

    m_bDirectWrite = bDirectWrite;
    if (!m_bDirectWrite)
    {
        int iRet = m_threadLog.Start(ThreadLog, this);
        if (iRet != 0)
        {
            return false;
        }
    }

    
    m_bRunning = true;
    m_strFilePath = strFilePath;
    m_bOutputLevel = bOutputLevel;

    return true;
}

//pszTimeString: if not NULL,will save the time string , the format is: 2019-07-31_15:39:29
int64 CXLog::GetCurrentTimeMS(char *pszTimeString)
{
	typedef chrono::time_point<chrono::system_clock, chrono::milliseconds> MsClockType;
	MsClockType tp = chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now());

	if (pszTimeString != NULL)
	{
		time_t timeCur = chrono::system_clock::to_time_t(tp);
		DWORD dwMillsSecond = (DWORD)(timeCur % 1000);
		std::strftime(pszTimeString, 60, "%Y-%m-%d_%H:%M:%S", std::localtime(&timeCur));
		char szMSTime[10] = { 0 };
		sprintf_s(szMSTime, 10, ".%d", dwMillsSecond);
		strcat(pszTimeString, szMSTime);
	}

	return (int64)tp.time_since_epoch().count();
}


//start the thread
void CXLog::Log(CXLOG_LEVEL levelType, const char *pszLogContent)
{
    if ( levelType< m_bOutputLevel)
    {
        return;
    }

	char szCurTime[255] = { 0 };
	GetCurrentTimeMS(szCurTime);
	string strTime= szCurTime;
	/*
    std::string strTime;
    std::stringstream ssTime;
    std::time_t currenttime = std::time(0);
    char tmCur[255] = {0};
    std::strftime(tmCur, 255, "%Y-%m-%d %H:%M:%S", std::localtime(&currenttime));
    ssTime << tmCur;
    strTime = ssTime.str();
	*/
    string strLog = "Time:"+strTime + " ";

    string strType = "Info";
    switch (levelType)
    {
    case CXLog::CXLOG_DEBUG:
        strType = "Debug   ";
        break;
    case CXLog::CXLOG_INFO:
		strType = "Info    ";
        break;
    case CXLog::CXLOG_WARNNING:
        strType = "Warnning";
        break;
    case CXLog::CXLOG_ERROR:
        strType = "Error   ";
        break;
    default:
		strType = "Unknown ";
        break;
    }
	strLog += "Level:" + strType + " ";
	strLog += "Message:";
	strLog += pszLogContent;

    if (!m_bDirectWrite)
    {
        m_lock.Lock();
        m_lstLogContent.push_back(strLog);
        m_lock.Unlock();
        m_event.SetEvent();
    }
    else
    {
        m_lock.Lock();
        FILE * fileLog = fopen(m_strFilePath.c_str(), "a");
        if (fileLog == NULL)
        {
            printf("Failed to open log file\n");
            m_lock.Unlock();
            return ;
        }

        if (strLog.rfind('\n') != (strLog.size() - 1))
        {
            strLog += "\n";
        }
        fprintf(fileLog, strLog.c_str());
        fclose(fileLog);
        m_lock.Unlock();
    }
}


int  CXLog::Run()
{
    FILE * fileLog = fopen(m_strFilePath.c_str(), "a");
    if (fileLog == NULL)
    {
        printf("Failed to open log file\n");
        return -1;
    }
	int64 iLastTimeMs = GetCurrentTimeMS();
	int64 iCurTimeMs = iLastTimeMs;
	DWORD dwInterval = 100;
	if (m_iFlushIntervalMs < dwInterval)
		dwInterval = m_iFlushIntervalMs;

    while (m_bRunning)
    {
        m_event.WaitForSingleObject(dwInterval);

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
		iCurTimeMs = GetCurrentTimeMS();
		if (iCurTimeMs - iLastTimeMs > (int64)m_iFlushIntervalMs)
		{
			fflush(fileLog);
			iLastTimeMs = iCurTimeMs;
		}
    }
   
    fclose(fileLog);

    return 0;
}

void* ThreadLog(void* lpvoid)
{
    CXLog * pServer = (CXLog*)lpvoid;
    return (void*)pServer->Run();
}