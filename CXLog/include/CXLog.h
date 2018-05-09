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
        CXLOG_DEBUG,
        CXLOG_INFO,
        CXLOG_WARNNING,
        CXLOG_ERROR
    };

public:
    CXLog();
    virtual ~CXLog();

    bool Initialize(string strFilePath);

    //start the thread
    void Log(CXLOG_LEVEL levelType,const char *pszLogContent);

    int  Run();
private:
    CXThread m_threadLog;
    list<string> m_lstLogContent;
    CXEvent m_event;
    string  m_strFilePath;
    bool    m_bRunning;
    CXSpinLock m_lock;
};

#endif //CXLOG_H 