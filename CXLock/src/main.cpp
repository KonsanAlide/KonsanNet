#include <iostream>

using namespace std;
#include "CXSpinLock.h"
int main()
{
    cout << "Hello world!" << endl;
	CXSpinLock lock;
    lock.Lock();
    //count<<"Lock this thread."<<endl;
    lock.Unlock();
    return 0;
}
