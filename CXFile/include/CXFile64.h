#ifndef CXFILE64_H
#define CXFILE64_H

#ifdef WIN32  
#include <Windows.h>  
#else  //WIN32  
#endif  //WIN32  
#include <string>

#include "PlatformDataTypeDefine.h" 

using namespace std;
class CXFile64
{
public:
    enum SEEKTYPE
    {
        begin,
        current,
        end
    };
    enum OPENTYPE
    {
        modeRead,
        modeWrite,
        modeReadWrite,
        modeNoTruncateWrite,
        modeNoTruncateReadWrite
    };

public:
    CXFile64();
    virtual ~CXFile64();

    bool Open(string strFilePath,OPENTYPE type, DWORD dwFlagsAndAttributes=0);

    //return value : ==true, if read to end of the file, the dwReadLen will be set to zero.
    bool Read(byte* pBuf, DWORD dwWantReadLen, DWORD &dwReadLen);

    bool Seek(int64 pos, SEEKTYPE type);
    bool GetCurrentPosition(uint64 & uiPos);
    bool Write(const byte* pBuf, DWORD dwBufLen, DWORD &dwWrittenLen);
    bool Flush();
    bool TruncateFile(uint64 uiFileLen);
    bool Close();
    bool IsOpen() { return m_bIsOpened; }
    string GetFilePathName() { return m_strFilePath; }
    bool GetFileLength(uint64 & uiFileLength);
    bool GetLastModifiedTime(time_t & tmFileModified);
    DWORD GetErrorCode();
    string GetErrorDescription();


private:
    string  m_strFilePath;
    bool    m_bRunning;
    bool    m_bIsOpened;
#ifdef WIN32
    HANDLE m_file;
#else
    int    m_file;
#endif
};

#endif //CXFILE64_H 