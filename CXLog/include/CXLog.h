#ifndef CXLOG_H
#define CXLOG_H

#ifdef WIN32  
#include <Windows.h>  
#include <process.h>  
#else  //WIN32  
#include <pthread.h>  
#endif  //WIN32  
#include <string>
#include <list>

#include "PlatformDataTypeDefine.h" 
#include "CXThread.h"
#include "CXSpinLock.h"
#include "CXEvent.h"

using namespace std;
class CXLog
{
public:
    enum CXLOG_LEVEL
    {
        CXLOG_DEBUG=0,
        CXLOG_INFO,
        CXLOG_WARNNING,
        CXLOG_ERROR
    };

public:
    CXLog();
    virtual ~CXLog();

    bool Initialize(string strFilePath, CXLOG_LEVEL bOutputLevel = CXLOG_DEBUG,bool bDirectWrite = false);

    //start the thread
    void Log(CXLOG_LEVEL levelType,const char *pszLogContent);

    int  Run();

	//set the interval time that flush the logs data to disk
	void SetFlushInterval(int iMillisecond) { m_iFlushIntervalMs = iMillisecond; }

private:
	//pszTimeString: if not NULL,will save the time string , the format is: 2019-07-31_15:39:29
	//return value : the current time 
	int64      GetCurrentTimeMS(char *pszTimeString = NULL);
private:
    CXThread m_threadLog;
    list<string> m_lstLogContent;
    CXEvent m_event;
    string  m_strFilePath;
    bool    m_bRunning;
    CXSpinLock m_lock;
    CXLOG_LEVEL m_bOutputLevel;
    bool    m_bDirectWrite;
	int     m_iFlushIntervalMs;
};

#endif //CXLOG_H 