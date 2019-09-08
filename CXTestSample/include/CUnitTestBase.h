#pragma once
#include<string>
using namespace std;
#ifdef WIN32
#include<windows.h>
#endif
#include "PlatformDataTypeDefine.h"
class CUnitTestBase
{
public:
	CUnitTestBase();
	virtual ~CUnitTestBase();
	virtual void Test()=0;

	int64_t GetCurrentTimeMS(char *pszTimeString = NULL);
};

