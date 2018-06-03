#include <iostream>
#include "include/CXQueue.h"
#include <vector>
#include <thread>
#include "CXSpinLock.h"
#include <queue>

using namespace std;

using namespace CXCommunication;
CXSpinLock g_lock;
void TestStdQueue(int iTheadNumber, queue<void*> *pQueue)
{
    printf("Thread %d running\n", iTheadNumber);

    int i = 0;
    for (i = 0; i<100000; i++)
    {
        g_lock.Lock();
        pQueue->push((void*)i);
        g_lock.Unlock();
    }

    for (i = 0; i<100000; i++)
    {
        g_lock.Lock();
        int iValue = (int)pQueue->front();
        pQueue->pop();
        g_lock.Unlock();
    }
    printf("Thread %d end\n", iTheadNumber);
}

void TestQueue(int iTheadNumber, CXQueue *pQueue)
{
    printf("Thread %d running\n", iTheadNumber);

    int i = 0;
    for (i = 0; i<100000; i++)
    {
        g_lock.Lock();
        pQueue->Push((void*)i);
        g_lock.Unlock();
    }

    for (i = 0; i<100000; i++)
    {
        g_lock.Lock();
        int iValue = (int)pQueue->Front();
        pQueue->Pop();
        g_lock.Unlock();
    }
    printf("Thread %d end\n", iTheadNumber);
}

void NoMoreMemory()
{
    cout << "Unable to statisty request for memory" << endl;
    abort();
}
int main()
{
    //set_new_handler(NoMoreMemory);
    cout << "Hello world!" << endl;
    vector<thread*> threadVec;
    
    CXQueue *pQueue = new CXQueue();
    int nRet = pQueue->Init();
    DWORD dwTickCoutBegin = GetTickCount();
    if(nRet==0)
    {  
        for (int i = 0;i<100;i++)
        {
            thread *testThread = new thread(TestQueue,i, pQueue);
            threadVec.push_back(testThread);
        }
    }
    int iVectorSize = threadVec.size();
    for (int i = 0; i<iVectorSize; i++)
    {
        if(threadVec[i]->joinable())
            threadVec[i]->join();
    }
    DWORD dwTickCoutEnd1 = GetTickCount();
    printf("use time1 = %d\n", dwTickCoutEnd1- dwTickCoutBegin);
    
    queue<void*> queueTest;
    if (nRet == 0)
    {
        for (int i = 100; i<200; i++)
        {
            thread *testThread = new thread(TestStdQueue, i, &queueTest);
            threadVec.push_back(testThread);
        }
    }
    iVectorSize = threadVec.size();
    for (int i = 100; i<iVectorSize; i++)
    {
        if (threadVec[i]->joinable())
            threadVec[i]->join();
    }

    DWORD dwTickCoutEnd2 = GetTickCount();
    printf("use time1 = %d\n", dwTickCoutEnd2-dwTickCoutEnd1);
    
    cin.get();
    return 0;
}
