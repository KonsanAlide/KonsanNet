#include <iostream>
#include "CCryptoppTest.h"
#include "COpenSSLTest.h"

int main()
{

	CCryptoppTest cryptTest;
	cryptTest.Test();

    COpenSSLTest opensslTest;
    //opensslTest.Test();
    return 0;
}
