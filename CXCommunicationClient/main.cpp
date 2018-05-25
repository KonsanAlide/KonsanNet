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
CXLog g_cxLog;


int MakePacketBuf(PCXPacketData &ppPacket, int iDataLen)
{
    int iPacketLen = iDataLen + sizeof(PCXPacketData)-1;
    byte *pData = new byte[iPacketLen];
    if (pData == NULL)
    {
        return -2;
    }

    memset(pData, 0, iPacketLen);

    ppPacket = (PCXPacketData)pData;
    ppPacket->header.wDataLen = iPacketLen - sizeof(ppPacket->header);
    return 0;

}


int ThreadConnectAndSend(void* pVoid)
{
    CXFileTcpClientTest test;
    test.Test((long)pVoid);
    return -1;
    CXSocketAddress addr("192.168.0.103", 4355);
    long nThreadID = (long)pVoid;
#ifdef WIN32
    DWORD dwThreadID = GetCurrentThreadId();
#else
    DWORD dwThreadID = pthread_self();
#endif
    for (int i = 0; i<1; i++)
    {
        CXTcpClient tcpClient;

        if (RETURN_SUCCEED != tcpClient.Create())
        {
            printf("Failed to create socket ,thread id = %d, id=%d\n", nThreadID, i);
            return 1;
        }
        //TRACE("Try to connect to peer ,thread id = %d, id=%d\n", nThreadID, i);

        char szDesc[2048] = { 0 };
        sprintf(szDesc, "Try to connect to peer ,thread id = %d, id=%d\n", nThreadID, i);
        //TraceWasteTime(liLast,szDesc);
        if (RETURN_SUCCEED != tcpClient.Connect(addr))
        {
            printf("Failed to connect to peer ,thread id = %d, id=%d\n", nThreadID, i);
            return 1;
        }
        printf("Had connected to peer ,thread id = %d, id=%d\n", nThreadID, i);
        //sprintf(szDesc, "Had connected to peer ,thread id = %d\n", nThreadID);

        //TraceWasteTime(liLast, szDesc);

        int iDataLen = 2048;
        PCXPacketData pPacket = NULL;
        MakePacketBuf(pPacket, iDataLen);
        memset(pPacket->buf, '$', pPacket->header.wDataLen);

        for (int i = 0; i<10000; i++)
        {
            int nSent = 0;
            int nRet = tcpClient.SendPacket(pPacket, iDataLen, nSent);
            if (nRet <= 0)
                break;
            //printf_s("######Packet id = %d exit\n", i);
            //printf_s("Send packet id = %d,\n",i);
        }

    }
    printf("######Thread id = %d exit\n", nThreadID);
    return 0;
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
        int iDataLen = 1024;
        PCXPacketData pPacket = NULL;
        MakePacketBuf(pPacket, iDataLen);
        memset(pPacket->buf, '$', pPacket->header.wDataLen);
        for (int i = 0; i<100000; i++)
        {
            int nSent = 0;
            int nRet = pMainChanel->SendPacket(pPacket, iDataLen, nSent);
            if (nRet <= 0)
                break;
            //Sleep(1);
        }
    }
    session.Close();
    delete pMainChanel;
    delete pDataChanel;
    
}

void TestConection()
{
    
    CXSocketAddress addr("192.168.0.105", 4355);

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
            int iDataLen = 1024;
            PCXPacketData pPacket = NULL;
            MakePacketBuf(pPacket, iDataLen);
            memset(pPacket->buf, '$', pPacket->header.wDataLen);
            for (int i = 0; i<100000; i++)
            {
                int nSent = 0;
                int nRet = pConnect->SendPacket(pPacket, iDataLen, nSent);
                if (nRet <= 0)
                    break;
                //Sleep(1);
            }
        }
        
        pConnect->Close();
        delete pConnect;

    }
}

void *ThreadWork(void *pVoid)
{
    ThreadConnectAndSend(pVoid);
    //TestConection();
    //TestSession();
    return 0;
}
int main()
{
    g_networkInit.InitEnv();

    if (!g_cxLog.Initialize("D:\\C++Project\\CXNetworkCommunication\\trunk\\log\\clientlog.log"))
    {
        printf("Failed to initialize the log recorder.\n");
        return -1;
    }

    //TestSession();
    //TestConection();
    //return -2;

    CXThread workThread[1000];
    int i = 0;
    for (i = 0;i<100;i++)
    {
        workThread[i].Start(ThreadWork,(void*)i);
    }


    //server.WaitThreadsExit();
    cin.get();
    return 0;
}
