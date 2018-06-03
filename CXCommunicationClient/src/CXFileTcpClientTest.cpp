/****************************************************************************
Copyright (c) 2018 Charles Yang

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Description£º
*****************************************************************************/
#include "CXFileTcpClientTest.h"
#include "CXFileTcpClient.h"
#include "CXFilePacketStructure.h"

#ifdef WIN32
#include <fcntl.h>
#include <io.h>
#else
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#endif

using namespace CXCommunication;
CXFileTcpClientTest::CXFileTcpClientTest()
{
}


CXFileTcpClientTest::~CXFileTcpClientTest()
{
}

int CXFileTcpClientTest::Test(int iNumber)
{
    CXFileTcpClient client;
#ifdef WIN32
    long lBegin = GetTickCount();
#else
    struct timeval start, end;
    gettimeofday( &start, NULL );
    printf("start : %d.%d\n", start.tv_sec, start.tv_usec);
    sleep(1);
    gettimeofday( &end, NULL );
    printf("end   : %d.%d\n", end.tv_sec, end.tv_usec);
#endif
    client.SetRemoteServerInfo("192.168.0.108",4355,"test","123");
    string strRemoteFilePath = "/home/cheng/test/TeamViewer_Setup_zhcn.exe";

    //client.SetRemoteServerInfo("192.168.0.104",4355,"test","123");
    //string strRemoteFilePath = "D:/Tool/TeamViewer_Setup_zhcn.exe";
    string strLocalFilePath = "D:/DataReceive/TeamViewer_Setup_zhcn.exe";

    char szNumber[20] = {0};
    sprintf(szNumber,"%d", iNumber);
    strLocalFilePath += szNumber;
    FILE * fileLocal = fopen(strLocalFilePath.c_str(),"wb");
    if (fileLocal == NULL)
    {
        return -1;
    }

    int iRet = client.Open(strRemoteFilePath,CXFileTcpClient::modeRead);
    if(iRet!=0)
    {
        fclose(fileLocal);
        return -2;
    }
    uint64 uiFileLen = 0;
    iRet = client.GetFileLength(uiFileLen);
    if (iRet != 0)
    {
        fclose(fileLocal);
        return -3;
    }
    //byte byFileData[CLIENT_BUF_SIZE] = {0};
    //int iReadLenInOnce = CLIENT_BUF_SIZE;
    //int iLeftBufLen = CLIENT_BUF_SIZE - (sizeof(CXPacketHeader) + sizeof(DWORD)) - sizeof(CXFileReadReply) + 1;
    
    int iOneDataLen = 4096;
    byte *byFileData = new byte[iOneDataLen];
    uint64 uiReadLen = 0;
    int iReadLenInOnce = iOneDataLen;
    int iHaveReadLen = 0;
    size_t sWriteLen = 0;
    

    while (uiReadLen<uiFileLen)
    {
        iReadLenInOnce = iOneDataLen;
        if (iReadLenInOnce > (uiFileLen - uiReadLen))
        {
            iReadLenInOnce = uiFileLen - uiReadLen;
        }

        iRet = client.Read(byFileData, iReadLenInOnce, &iHaveReadLen);
        if (iRet != 0 && iRet != -9 && iRet != -10)
        {
            break;
        }
        uiReadLen += iHaveReadLen;
        sWriteLen=fwrite(byFileData, 1, iHaveReadLen, fileLocal);
        if (sWriteLen != iHaveReadLen)
        {
            break;
        }
        if (iRet == -9 )
        {
            break;
        }
        else if(iRet == -10)
        {
            break;
        }
    }

    fflush(fileLocal);
    fclose(fileLocal);



    iRet = client.Close();
    if (iRet != 0)
    {
        return -3;
    }
#ifdef WIN32
    long lEnd = GetTickCount();
    long use = lEnd - lBegin;
    printf_s("Download file %s ,used time %d ms\n", strRemoteFilePath.c_str(), use);
#else
#endif


    strRemoteFilePath = "D:/Tool/TeamViewer_Setup_zhcn.exe";

    if (!CompareFile(strRemoteFilePath, strLocalFilePath))
    {
        printf("The data in two file is not same,fiel1=%s,file2=%s\n", strRemoteFilePath.c_str(), strLocalFilePath.c_str());
        int l = 0;
    }

    return 0;
}

#include <sys/stat.h>

unsigned long get_file_size(const char *path)
{
    unsigned long filesize = -1;
    struct stat statbuff;
    if(stat(path, &statbuff) < 0){
        return filesize;
    }else{
        filesize = statbuff.st_size;
    }
    return filesize;
}

bool CXFileTcpClientTest::CompareFile(string strFile1, string strFile2)
{

    FILE * fileLocal1 = fopen(strFile1.c_str(), "rb");
    if (fileLocal1 == NULL)
    {
        return false;
    }

    FILE * fileLocal2 = fopen(strFile2.c_str(), "rb");
    if (fileLocal2 == NULL)
    {
        return false;
    }
#ifdef WIN32
    int iFileLen1 = filelength(fileno(fileLocal1));
    int iFileLen2 = filelength(fileno(fileLocal2));
#else
    int iFileLen1 = get_file_size(strFile1.c_str());
    int iFileLen2 = get_file_size(strFile2.c_str());

#endif
    if (iFileLen1 != iFileLen2)
    {
        return false;
    }

    byte byFileData[CLIENT_BUF_SIZE] = { 0 };
    byte byFileData2[CLIENT_BUF_SIZE] = { 0 };
    uint64 uiReadLen = 0;
    int iReadLenInOnce = CLIENT_BUF_SIZE;
    int iHaveReadLen = 0;
    size_t sWriteLen = 0;
    int iLeftBufLen = CLIENT_BUF_SIZE;

    bool bRet = true;

    while (uiReadLen<iFileLen1)
    {
        iReadLenInOnce = iLeftBufLen;
        if (iReadLenInOnce >(iFileLen1 - uiReadLen))
        {
            iReadLenInOnce = iFileLen1 - uiReadLen;
        }

        int iRead = fread(byFileData, 1, iReadLenInOnce, fileLocal1);
        int iRead2 = fread(byFileData2, 1, iReadLenInOnce, fileLocal2);
        if (iRead != iRead2)
        {
            bRet = false;
            break;
        }

        if (iRead <= 0)
        {
            break;
        }

        if (iRead != iReadLenInOnce)
        {
            if (iRead <= 0)
            {
                if (feof(fileLocal1) != 0)
                {
                    break;
                }
            }
            else
            {
                int iCompareRet = memcmp(byFileData, byFileData2, iRead);
                if (iCompareRet != 0)
                {
                    bRet = false;
                    break;
                }
                if (feof(fileLocal1) != 0)
                {
                    break;
                }
            }
        }
        else
        {
            uiReadLen += iRead;
            int iCompareRet = memcmp(byFileData, byFileData2, iRead);
            if (iCompareRet != 0)
            {
                bRet = false;
                break;
            }
        }
    }

    fclose(fileLocal2);
    fclose(fileLocal1);
    return bRet;
}
