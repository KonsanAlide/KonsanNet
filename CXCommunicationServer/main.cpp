#include <iostream>
#include "CXCommunicationServer.h"
#include "CXNetworkInitEnv.h"
#include "CXLog.h"
#include "CXGuidGenerate.h"
using namespace std;
using namespace CXCommunication;
CXNetworkInitEnv g_networkInit;
CXLog g_cxLog;
CXGuidGenerate g_cxGuidGenerater;
int main()
{
    g_networkInit.InitEnv(); 

    if (!g_cxLog.Initialize("D:\\C++Project\\CXNetworkCommunication\\trunk\\log\\log.log"))
    {
        printf_s("Failed to initialize the log recorder.\n");
        return -1;
    }

    CXCommunicationServer server;
    if(server.Start(4355,4)!=0)
    {
        cout<<"Failed to start communication server!\n"<<endl;
        return -1;
    }
    else
    {
        printf_s("Start communication server successfully.\n");
    }

    //server.WaitThreadsExit();
    cin.get();
    server.Stop();
    cout << "CXServer exit" << endl;
    return 0;
}
