#include <iostream>
#include "include/CXMemoryCache.h"
#include <vector>
#include <thread>

using namespace std;

typedef struct _BUFFER_OBJ
{
	void *pSockObj;
    char buf[4096];
	int  nUsedLen;

	// 这里是添加来处理粘包的操作的，如果这个buffer里面有两个包的内容，
	// 那么先把第一个包的内容处理了，剩余的数据在这个buffer里面的开始位置就是nCurDataPointer
	// nCurDataPointer也意味着下一个包的开始位置
	int nCurDataPointer;

    #define OP_ACCEPT  1
    #define OP_READ    2
    #define OP_WRITE   3
	int nOperate;
}BufferObj, *PBufferObj;

void TestCache(int iTheadNumber,CXMemoryCache *highCache)
{
    printf("Thread %d running\n", iTheadNumber);
    vector<PBufferObj> objVec;
    int i = 0;
    for (i = 0; i<10000; i++)
    {
        //printf("Thread %d ,GetObject loop %d\n", iTheadNumber,i);
        PBufferObj pObj = (PBufferObj)highCache->GetObject();
        if (pObj)
        {
            pObj->pSockObj = NULL;
            memset(pObj->buf, i, 4095);
            pObj->nUsedLen = 4095;
        }
        objVec.push_back(pObj);
    }

    for (i = 0; i<10000; i++)
    {
        PBufferObj pObj = objVec[i];
        if (pObj)
        {
            //printf("Thread %d ,FreeObject loop %d\n", iTheadNumber, i);
            highCache->FreeObject((void*)pObj);
        }
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

    CXMemoryCache *highCache = new CXMemoryCache();
    int nRet = highCache->Initialize(sizeof(BufferObj),1000);
    if(nRet==0)
    {
        
        for (int i = 0;i<100;i++)
        {
            thread *testThread = new thread(TestCache,i, highCache);
            threadVec.push_back(testThread);
        }
    }
    int iVectorSize = threadVec.size();
    for (int i = 0; i<iVectorSize; i++)
    {
        if(threadVec[i]->joinable())
            threadVec[i]->join();
    }
    highCache->Destroy();

    return 0;
}
