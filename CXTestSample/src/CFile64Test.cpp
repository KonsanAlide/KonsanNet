#include "CFile64Test.h"
#include <iostream>
#include "CXFile64.h"
using namespace std;


CFile64Test::CFile64Test()
{
}


CFile64Test::~CFile64Test()
{
}


void CFile64Test::Test()
{

    CXFile64 file;
    string strFilePath = "D:/迅雷下载/mysql-installer-community-5.7.21.0.msi";
    if (!file.Open(strFilePath,CXFile64::modeRead))
    {
        printf("Failed to open the file %s\n", strFilePath.c_str());
        return ;
    }
    uint64 uiPos = 12390901;
    if (!file.Seek(uiPos, CXFile64::begin))
    {
        printf("Failed to seek the file to the pos %I64i\n", uiPos);
        file.Close();
        return ;
    }
    uint64 uiFileLen = 0;
    if (!file.GetFileLength(uiFileLen))
    {
        printf("Failed to get the file length\n");
        file.Close();
        return ;
    }

    uint64 uiTotalReadLen = 0;
    DWORD dwNeedRead = 4096;
    DWORD dwReadLen = 0;
    byte byBuf[4096] = {0};
    while (true)
    {
        if (file.Read(byBuf, dwNeedRead, dwReadLen))
        {
            uiTotalReadLen += dwReadLen;
            if (dwReadLen != dwNeedRead)
            {
                
            }
            if (dwReadLen == 0)
            {
                printf("Read to file end\n");
                break;
            }
        }
        else
        {
            string strError = file.GetErrorDescription();
            printf("Failed to read file,error=%s\n", strError.c_str());

            break;
        }
    }

    
    file.Close();

	return ;
}

