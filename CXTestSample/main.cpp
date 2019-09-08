#include <iostream>
#include "CCryptoppTest.h"
#include "COpenSSLTest.h"
#include "CThreadPoolTest.h"

int main()
{

	CCryptoppTest cryptTest;
	//cryptTest.Test();

    COpenSSLTest opensslTest;
    //opensslTest.Test();

	CThreadPoolTest threadTest;
	threadTest.Test();
    return 0;
}
