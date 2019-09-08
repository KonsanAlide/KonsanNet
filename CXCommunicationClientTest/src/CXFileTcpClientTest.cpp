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
#include "CXLog.h"
#include "CXFile64.h"
#include "PlatformFunctionDefine.h"
#include "CXFastDataParserHandle.h"

#ifdef WIN32
#include <fcntl.h>
#include <io.h>
#else
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#endif

const string g_strDestIP = "127.0.0.1";
//const string g_strDestIP = "192.168.0.118";
const int iRemotePort = 4355;
//const int iRemotePort = 4354;

const string g_strRemoteFilePath = "/home/cheng/test/TeamViewer_Setup_zhcn.exe";
//const string g_strRemoteFilePath = "D:/Tool/TeamViewer_Setup_zhcn.exe";

const string g_strLocalReadFilePath = "D:/Tool/TeamViewer_Setup_zhcn.exe";

extern CXLog g_logHandle;
using namespace CXCommunication;
CXFileTcpClientTest::CXFileTcpClientTest()
{
}


CXFileTcpClientTest::~CXFileTcpClientTest()
{
}

int CXFileTcpClientTest::Download(int iNumber)
{
    CXFileTcpClient client;
#ifdef WIN32
    long lBegin = GetTickCount();
#else
    long lBegin = time(NULL);

#endif
    client.SetRemoteServerInfo(g_strDestIP, iRemotePort,"test","123");
    string strRemoteFilePath = g_strRemoteFilePath;

#ifdef WIN32
    string strLocalFilePath = "D:/DataReceive/TeamViewer_Setup_zhcn.exe";
#else
    string strLocalFilePath = "/home/cheng/DataReceive/TeamViewer_Setup_zhcn.exe";
#endif


    char szNumber[20] = {0};
    sprintf(szNumber,"%d", iNumber);
    strLocalFilePath += szNumber;
    FILE * fileLocal = fopen(strLocalFilePath.c_str(),"wb");
    if (fileLocal == NULL)
    {
        return -1;
    }

	CXFastDataParserHandle dataParserHandle;

	string strLocalPath = "";
#ifdef WIN32
	char szFilePath[MAX_PATH] = { 0 }, szDrive[MAX_PATH] = { 0 };
	char szDir[MAX_PATH] = { 0 }, szFileName[MAX_PATH] = { 0 }, szExt[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));
	_splitpath(szFilePath, szDrive, szDir, szFileName, szExt);
	strLocalPath = szDrive;
	strLocalPath.append(szDir);
#else
	strLocalPath = "./";
#endif

	dataParserHandle.LoadRSAKeyFiles(strLocalPath + "privateKey.conf", strLocalPath + "pubKey.conf");
	dataParserHandle.LoadBlowfishKeyFiles(strLocalPath + "key.conf", strLocalPath + "iv.conf");

	client.SetDataPaserHandle((CXDataParserImpl*)&dataParserHandle);
	//client.SetEncryptParas(CXDataParserImpl::CXENCRYPT_TYPE_BLOWFISH);
	//client.SetEncryptParas(CXDataParserImpl::CXENCRYPT_TYPE_RSA_PKCS1v15);
	//client.SetEncryptParas(CXDataParserImpl::CXENCRYPT_TYPE_BLOWFISH);
    //client.SetEncryptParas(CXDataParserImpl::CXENCRYPT_TYPE_NONE);
	//client.SetCompressParas(CXDataParserImpl::CXCOMPRESS_TYPE_SNAPPY);
    //client.SetCompressParas(CXDataParserImpl::CXCOMPRESS_TYPE_GZIP);
    //client.SetCompressParas(CXDataParserImpl::CXCOMPRESS_TYPE_ZLIB);
    //client.SetCompressParas(CXDataParserImpl::CXCOMPRESS_TYPE_NONE);

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
        client.Close();
        return -3;
    }

    iRet = client.Seek(0,CXFileTcpClient::begin);
    if (iRet != 0)
    {
        fclose(fileLocal);
        client.Close();
        return -4;
    }
    //byte byFileData[CLIENT_BUF_SIZE] = {0};
    //int iReadLenInOnce = CLIENT_BUF_SIZE;
    //int iLeftBufLen = CLIENT_BUF_SIZE - (sizeof(CXPacketHeader) + sizeof(DWORD)) - sizeof(CXFileReadReply) + 1;
    
    int iOneDataLen = 409600;
    byte *byFileData = new byte[iOneDataLen];
    uint64 uiReadLen = 0;
    int iReadLenInOnce = iOneDataLen;
    int iHaveReadLen = 0;
    size_t sWriteLen = 0;
    
    //printf("Open file %s to write\n", strLocalFilePath.c_str());

    byte *pFileData = new byte[uiFileLen];

    while (uiReadLen<uiFileLen)
    {
        iReadLenInOnce = iOneDataLen;
        if (iReadLenInOnce > (uiFileLen - uiReadLen))
        {
            iReadLenInOnce = uiFileLen - uiReadLen;
        }
        uint64 uiBegin = client.GetCurrentTimeMS();
        iRet = client.Read(byFileData, iReadLenInOnce, &iHaveReadLen);
        uint64 uiEnd = client.GetCurrentTimeMS();
        uint64 iUsedTime = uiEnd - uiBegin;
        if (iReadLenInOnce != iHaveReadLen)
        {
            printf("Need read %d,only read %d,total read %lld,file is %s\n", iReadLenInOnce, 
                iHaveReadLen, (uiReadLen+iHaveReadLen), strLocalFilePath.c_str());
            //printf("%s\n",strLocalFilePath.c_str());
        }
        if (iRet != 0 && iRet != -10)
        {
            break;
        }
        
        memcpy(pFileData + uiReadLen, byFileData, iHaveReadLen);
        sWriteLen = iHaveReadLen;
        //sWriteLen=fwrite(byFileData, 1, iHaveReadLen, fileLocal);
        uiReadLen += iHaveReadLen;
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

    if (uiReadLen != uiFileLen)
    {
        delete[]byFileData;
        delete[]pFileData;

        char  szInfo[1024] = { 0 };
        sprintf_s(szInfo, 1024, "Failed to download file %s\n",
            strRemoteFilePath.c_str());
        g_logHandle.Log(CXLog::CXLOG_INFO, szInfo);

        printf(szInfo);

        return 0;
    }

#ifdef WIN32
    long lEnd = GetTickCount();
    long use = lEnd - lBegin;
    printf_s("Download file %s ,used time %d ms\n", strRemoteFilePath.c_str(), use);
#else
    long lEnd = time(NULL);
    long use = lEnd - lBegin;
    printf("Download file %s ,used time %d s\n", strRemoteFilePath.c_str(), use);
#endif

#ifdef WIN32
    strRemoteFilePath = "D:/Tool/TeamViewer_Setup_zhcn.exe";
#else
    strRemoteFilePath = "/home/cheng/test/TeamViewer_Setup_zhcn.exe";
#endif




    //if (!CompareFile(strRemoteFilePath, strLocalFilePath))
    if (!CompareFile(strRemoteFilePath, pFileData, uiReadLen))
    {
        char  szInfo[1024] = { 0 };
        sprintf_s(szInfo, 1024, "Not same files,fiel1=%s,file2=%s\n",
            strRemoteFilePath.c_str(), strLocalFilePath.c_str());
        g_logHandle.Log(CXLog::CXLOG_INFO, szInfo);

        printf(szInfo);
        int l = 0;
    }
	else
	{
        char  szInfo[1024] = { 0 };
        sprintf_s(szInfo, 1024, "Same files,fiel1=%s,file2=%s\n",
            strRemoteFilePath.c_str(), strLocalFilePath.c_str());
        g_logHandle.Log(CXLog::CXLOG_INFO, szInfo);

        printf(szInfo);
	}
    delete[]byFileData;
    delete[]pFileData;
    return 0;
}

int CXFileTcpClientTest::Upload(int iNumber)
{
    CXFileTcpClient client;
#ifdef WIN32
    long lBegin = GetTickCount();
#else
    long lBegin = time(NULL);

#endif
    client.SetRemoteServerInfo(g_strDestIP, 4355, "test", "123");

    string strLocalFilePath = g_strLocalReadFilePath;


#ifdef WIN32
    string strRemoteFilePath = "D:/DataReceive/TeamViewer_Setup_zhcn.exe";
#else
    string strRemoteFilePath = "/home/cheng/DataReceive/TeamViewer_Setup_zhcn.exe";
#endif

    CXFile64 fileLocal;
    if (!fileLocal.Open(strLocalFilePath.c_str(),CXFile64::modeRead))
    {
        return -1;
    }

    char szNumber[20] = { 0 };
    sprintf(szNumber, "_%d", iNumber);
    strRemoteFilePath += szNumber;
    int iRet = client.Open(strRemoteFilePath, CXFileTcpClient::modeWrite);
    if (iRet != 0)
    {
        fileLocal.Close();
        return -2;
    }
    uint64 uiFileLen = 0;

    if (!fileLocal.GetFileLength(uiFileLen))
    {
        fileLocal.Close();
        client.Close();
        return -3;
    }

    //byte byFileData[CLIENT_BUF_SIZE] = {0};
    //int iReadLenInOnce = CLIENT_BUF_SIZE;
    //int iLeftBufLen = CLIENT_BUF_SIZE - (sizeof(CXPacketHeader) + sizeof(DWORD)) - sizeof(CXFileReadReply) + 1;

    int iOneDataLen = 409600;
    byte *byFileData = new byte[iOneDataLen];
    uint64 uiReadLen = 0;
    DWORD iReadLenInOnce = iOneDataLen;
    DWORD iHaveReadLen = 0;

    //printf("Open file %s to write\n", strLocalFilePath.c_str());

    byte *pFileData = new byte[uiFileLen];

    while (uiReadLen < uiFileLen)
    {
        iReadLenInOnce = iOneDataLen;
        if (iReadLenInOnce > (uiFileLen - uiReadLen))
        {
            iReadLenInOnce = uiFileLen - uiReadLen;
        }
        if (!fileLocal.Read(byFileData, iReadLenInOnce, iHaveReadLen) || iHaveReadLen != iReadLenInOnce)
        {
            printf("Need read %d,only read %d,total read %lld,file is %s\n", iReadLenInOnce,
                iHaveReadLen, (uiReadLen + iHaveReadLen), strLocalFilePath.c_str());
            break;
        }

        int iHadWrittenLen = 0;
        iRet = client.Write(byFileData, iReadLenInOnce,&iHadWrittenLen);
        if (iRet != 0 && iRet != -10)
        {
            break;
        }

        if (iHadWrittenLen != iReadLenInOnce)
        {
            break;
        }

        memcpy(pFileData + uiReadLen, byFileData, iHadWrittenLen);
        //sWriteLen=fwrite(byFileData, 1, iHaveReadLen, fileLocal);
        uiReadLen += iHadWrittenLen;

        if (iRet == -9)
        {
            break;
        }
        else if (iRet == -10)
        {
            break;
        }
    }
    fileLocal.Close();

    iRet = client.Close();

#ifdef WIN32
    long lEnd = GetTickCount();
    long use = lEnd - lBegin;
    printf_s("Upload file %s ,used time %d ms\n", strRemoteFilePath.c_str(), use);
#else
    long lEnd = time(NULL);
    long use = lEnd - lBegin;
    printf("Upload file %s ,used time %d s\n", strRemoteFilePath.c_str(), use);
#endif

    //if (!CompareFile(strRemoteFilePath, strLocalFilePath))
    if (!CompareFile(strRemoteFilePath, pFileData, uiReadLen))
    {
        char  szInfo[1024] = { 0 };
        sprintf_s(szInfo, 1024, "Not same files,fiel1=%s,file2=%s\n",
            strRemoteFilePath.c_str(), strLocalFilePath.c_str());
        g_logHandle.Log(CXLog::CXLOG_INFO, szInfo);

        printf(szInfo);
        int l = 0;
    }
    else
    {
        char  szInfo[1024] = { 0 };
        sprintf_s(szInfo, 1024, "Same files,fiel1=%s,file2=%s\n",
            strRemoteFilePath.c_str(), strLocalFilePath.c_str());
        g_logHandle.Log(CXLog::CXLOG_INFO, szInfo);

        printf(szInfo);
    }
    delete[]byFileData;
    delete[]pFileData;
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

bool CXFileTcpClientTest::CompareFile(string strFile1, byte *pFileData,int iDataLen)
{

    FILE * fileLocal1 = fopen(strFile1.c_str(), "rb");
    if (fileLocal1 == NULL)
    {
        return false;
    }

#ifdef WIN32
    int iFileLen1 = filelength(fileno(fileLocal1));
    int iFileLen2 = iDataLen;
#else
    int iFileLen1 = get_file_size(strFile1.c_str());
    int iFileLen2 = iDataLen;
#endif
    if (iFileLen1 != iFileLen2)
    {
        return false;
    }

    byte byFileData[CLIENT_BUF_SIZE] = { 0 };
    uint64 uiReadLen = 0;
    int iReadLenInOnce = CLIENT_BUF_SIZE;
    int iHaveReadLen = 0;
    size_t sWriteLen = 0;
    int iLeftBufLen = CLIENT_BUF_SIZE;

    bool bRet = true;

    while (uiReadLen < iFileLen1)
    {
        iReadLenInOnce = iLeftBufLen;
        if (iReadLenInOnce > (iFileLen1 - uiReadLen))
        {
            iReadLenInOnce = iFileLen1 - uiReadLen;
        }

        int iRead = fread(byFileData, 1, iReadLenInOnce, fileLocal1);

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
                int iCompareRet = memcmp(byFileData, pFileData+ uiReadLen, iRead);
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
            int iCompareRet = memcmp(byFileData, pFileData + uiReadLen, iRead);
            uiReadLen += iRead;
            if (iCompareRet != 0)
            {
                bRet = false;
                break;
            }
        }
    }

    fclose(fileLocal1);
    return bRet;
}
