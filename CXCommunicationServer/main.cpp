#include <iostream>
#include "CXCommunicationServer.h"
#include "CXNetworkInitEnv.h"
#include "CXLog.h"
#include "CXGuidGenerate.h"
#include "CXThread.h"
using namespace std;
using namespace CXCommunication;
CXNetworkInitEnv g_networkInit;
CXLog g_cxLog;
CXGuidGenerate g_cxGuidGenerater;
CXCommunicationServer server;
bool g_bRunning = false;
void* ThreadCount(void* lpvoid)
{
    //server.WaitThreadsExit();
    uint64 uiLastConnectionNumber = 0;
    uint64 uiLastIONumber = 0;
    while (g_bRunning)
    {
#ifdef WIN32
        Sleep(1000);
#else
        sleep(1);
#endif

        //uint64 uiConnectionNumber = server.GetConnectionManager().GetTotalConnectionsNumber();
        uint64 uiConnectionNumber = server.GetTotalConnectionsNumber();
        uint64 uiIONumber = server.GetTotalReceiveBuffers();
        printf("The number of the connections received in one second is : %I64i .\n",
            uiConnectionNumber - uiLastConnectionNumber);
        printf("The number of the io received in one second is : %I64i .\n",
            uiIONumber - uiLastIONumber);
        uiLastConnectionNumber = uiConnectionNumber;
        uiLastIONumber = uiIONumber;
    }
    return 0;
}

int main()
{
    g_networkInit.InitEnv();

    if (!g_cxLog.Initialize("..\\log.log"))
    {
        printf("Failed to initialize the log recorder.\n");
        return -1;
    }
    
    if(server.Start(4355,8)!=0)
    {
        cout<<"Failed to start communication server!\n"<<endl;
        return -1;
    }
    else
    {
        printf("Start communication server successfully.\n");
    }
    g_bRunning = true;
    CXThread threadCount;
    threadCount.Start(ThreadCount,NULL);

    cin.get();
    g_bRunning = false;
    server.Stop();
    threadCount.Wait();
    cout << "CXServer exit" << endl;
    return 0;
}
