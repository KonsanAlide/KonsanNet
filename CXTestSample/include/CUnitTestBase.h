#pragma once
#include<string>
using namespace std;
#include<windows.h>
class CUnitTestBase
{
public:
	CUnitTestBase();
	virtual ~CUnitTestBase();
	virtual void Test()=0;

	int64_t GetCurrentTimeMS(char *pszTimeString = NULL);
};

