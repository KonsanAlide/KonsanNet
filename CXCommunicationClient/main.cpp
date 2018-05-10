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


using namespace std;
using namespace CXCommunication;
CXNetworkInitEnv g_networkInit;
CXLog g_cxLog;


int MakePacketBuf(PCXPacketData &ppPacket, int iDataLen)
{
    int iPacketLen = iDataLen + sizeof(CXPacketHeader);
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


int ThreadConnectAndSend(PVOID pVoid)
{
    CXFileTcpClientTest test;
    test.Test((int)pVoid);
    return -1;
    CXSocketAddress addr("127.0.0.1", 4355);
    int nThreadID = (int)pVoid;

    DWORD dwThreadID = GetCurrentThreadId();
    for (int i = 0; i<1; i++)
    {
        CXTcpClient tcpClient;

        if (RETURN_SUCCEED != tcpClient.Create())
        {
            printf_s("Failed to create socket ,thread id = %d, id=%d\n", nThreadID, i);
            return 1;
        }
        //TRACE("Try to connect to peer ,thread id = %d, id=%d\n", nThreadID, i);

        LARGE_INTEGER liLast;
        liLast.QuadPart = 0;
        char szDesc[2048] = { 0 };
        sprintf(szDesc, "Try to connect to peer ,thread id = %d, id=%d\n", nThreadID, i);
        //TraceWasteTime(liLast,szDesc);
        if (RETURN_SUCCEED != tcpClient.Connect(addr))
        {
            printf_s("Failed to connect to peer ,thread id = %d, id=%d\n", nThreadID, i);
            return 1;
        }
        printf_s("Had connected to peer ,thread id = %d, id=%d\n", nThreadID, i);
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
    printf_s("######Thread id = %d exit\n", nThreadID);
    return 0;
}
void *ThreadWork(void *pVoid)
{
    ThreadConnectAndSend(pVoid);
    return 0;
}


int main()
{
    g_networkInit.InitEnv(); 

    if (!g_cxLog.Initialize("D:\\C++Project\\CXNetworkCommunication\\trunk\\log\\clientlog.log"))
    {
        printf_s("Failed to initialize the log recorder.\n");
        return -1;
    }

    CXThread workThread[50];
    int i = 0;
    for (i = 0;i<50;i++)
    {
        workThread[i].Start(ThreadWork,(void*)i);
    }
    

    //server.WaitThreadsExit();
    cin.get();
    return 0;
}
