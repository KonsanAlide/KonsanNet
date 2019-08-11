#include <iostream>
#include "CXFileTcpClient.h"
#include "CXFileTcpClientTest.h"
#include "CXTcpClient.h"
#include "CXNetworkInitEnv.h"
#include "CXThread.h"
#include "CXLog.h"
#include <iostream>
#include "CXCommonPacketStructure.h"
#include "CXSessionPacketStructure.h"
#include "CXPacketCodeDefine.h"
#ifdef WIN32
#else
#include <pthread.h>
#endif

#include "CXClientConnectionSession.h"

//const string g_strDestIP = "127.0.0.1";
//const string g_strDestIP = "192.168.0.104";
const string g_strDestIP = "192.168.0.118";

using namespace std;
using namespace CXCommunication;
CXNetworkInitEnv g_networkInit;
CXLog g_logHandle;
string g_strLocalPath ="";



int ThreadReadFileTest(void* pVoid)
{
    CXFileTcpClientTest test;
    test.Test((long)pVoid);
    return -1;
}


void TestSession()
{
    CXClientConnectionSession session;

    CXSocketAddress addr(g_strDestIP.c_str(), 4355);
    session.SetRemoteAddress(addr);
    session.SetUserInfo("test", "123");

    CXClientSocketChannel *pMainChanel = new CXClientSocketChannel();
    CXClientSocketChannel *pDataChanel = new CXClientSocketChannel();
    int iRet = session.OpenChannel(*pMainChanel, CXClientSocketChannel::MAJOR_MES_CONNECTION);
    if (iRet == 0)
    { 
        //iRet = session.OpenChannel(*pDataChanel, CXClientSocketChannel::DATA_CONNECTION); 

        //iRet = session.OpenChannel(*pDataChanel, CXClientSocketChannel::DATA_CONNECTION);

        int iPacketLen = 200;
        byte *pData = new byte[iPacketLen];
        if (pData == NULL)
        {
            return;
        }
        memset(pData, '$', iPacketLen);

        for (int i = 0; i<10000; i++)
        {
            int nSent = 0;
            int nRet = pMainChanel->SendPacket(pData, iPacketLen, 4001);
            if (nRet != 0)
                break;
        }
    }
    session.Close();
    delete pMainChanel;
    delete pDataChanel;
    
}

void TestConection()
{
    
    CXSocketAddress addr(g_strDestIP.c_str(), 4355);

    for (int i=0;i<1;i++)
    {
        /*
        CXClientConnectionSession *pSession = new CXClientConnectionSession();
        pSession->SetRemoteAddress(addr);
        pSession->SetUserInfo("test", "123");
        CXClientSocketChannel *pMainChanel = new CXClientSocketChannel();
        int iRet = pSession->OpenChannel(*pMainChanel, CXClientSocketChannel::MAJOR_MES_CONNECTION);
        if (iRet != 0)
        {
            printf("Failed to connect the server, index is  %d.\n",i);
        }
        */
        
        CXTcpClient *pConnect = new CXTcpClient();
        pConnect->Create();
        int iRet = pConnect->Connect(addr);
        if (iRet != 0)
        {
            printf("Failed to connect the server, index is  %d.\n", 0);
           
        }
        else
        {
            int iPacketLen = 4096;
            byte *pData = new byte[iPacketLen];
            if (pData == NULL)
            {
                return;
            }
            memset(pData, '$', iPacketLen);

            for (int i = 0; i<10000; i++)
            {
                int nSent = 0;
                int nRet = pConnect->SendPacket(pData, iPacketLen, 4001);
                if (nRet != 0)
                    break;
            }
        }
        
        pConnect->Close();
        delete pConnect;

    }
}

void *ThreadWork(void *pVoid)
{
    ThreadReadFileTest(pVoid);
    //TestConection();
    //TestSession();
    return 0;
}
int main()
{


    g_networkInit.InitEnv();

#ifdef WIN32
    char szFilePath[MAX_PATH] = { 0 }, szDrive[MAX_PATH] = { 0 };
    char szDir[MAX_PATH] = { 0 }, szFileName[MAX_PATH] = { 0 }, szExt[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));
    _splitpath(szFilePath, szDrive, szDir, szFileName, szExt);
    g_strLocalPath = szDrive;
    g_strLocalPath.append(szDir);
#else
    g_strLocalPath = "./";
#endif

    if (g_strLocalPath == "")
    {
        printf("Not get the local path.\n");

        return -3;
    }

    //if (!g_logHandle.Initialize(strLocalPath + "clientlog.log",CXLog::CXLOG_DEBUG,true))
    if (!g_logHandle.Initialize(g_strLocalPath + "clientlog.log", CXLog::CXLOG_INFO, true))
    {
        printf("Failed to initialize the log recorder.\n");
        return -1;
    }


    //TestSession();
    //TestConection();
    //return -2;

#ifdef WIN32
	CoInitialize(NULL);
#endif

    CXThread workThread[1000];
    int k = 0;
    while (k++<1000)
    {
        int i = 0;
        for (i = 0; i < 51; i++)
        {
            workThread[i].Start(ThreadWork, (void*)i);
        }

        for (i = 0; i < 51; i++)
        {
            workThread[i].Wait();
        }

        int l = 0;
    }
    
    printf("Test had finished.\n");


    //server.WaitThreadsExit();
    cin.get();
#ifdef WIN32
	CoUninitialize();
#endif
    return 0;
}
