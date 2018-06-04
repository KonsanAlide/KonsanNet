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

using namespace std;
using namespace CXCommunication;
CXNetworkInitEnv g_networkInit;
CXLog g_logHandle;



int ThreadReadFileTest(void* pVoid)
{
    CXFileTcpClientTest test;
    test.Test((long)pVoid);
    return -1;
}


void TestSession()
{
    CXClientConnectionSession session;

    CXSocketAddress addr("192.168.0.104", 4355);
    session.SetRemoteAddress(addr);
    session.SetUserInfo("test", "123");

    CXClientSocketChannel *pMainChanel = new CXClientSocketChannel();
    CXClientSocketChannel *pDataChanel = new CXClientSocketChannel();
    int iRet = session.OpenChannel(*pMainChanel, CXClientSocketChannel::MAJOR_MES_CONNECTION);
    if (iRet == 0)
    { 
        //iRet = session.OpenChannel(*pDataChanel, CXClientSocketChannel::DATA_CONNECTION); 

        //iRet = session.OpenChannel(*pDataChanel, CXClientSocketChannel::DATA_CONNECTION);

        int iPacketLen = 100;
        byte *pData = new byte[iPacketLen];
        if (pData == NULL)
        {
            return;
        }
        memset(pData, '$', iPacketLen);

        for (int i = 0; i<1000; i++)
        {
            if (i = 999)
            {
                int k = 0;
            }
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
    
    CXSocketAddress addr("192.168.0.104", 4355);

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

            for (int i = 0; i<100000; i++)
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

    if (!g_logHandle.Initialize("../clientlog.log"))
    {
        printf("Failed to initialize the log recorder.\n");
        return -1;
    }

    //TestSession();
    //TestConection();
    //return -2;

    CXThread workThread[1000];
    int i = 0;
    for (i = 0;i<1000;i++)
    {
        workThread[i].Start(ThreadWork,(void*)i);
    }

    for (i = 0; i<1000; i++)
    {
        workThread[i].Wait();
    }


    //server.WaitThreadsExit();
    cin.get();
    return 0;
}
