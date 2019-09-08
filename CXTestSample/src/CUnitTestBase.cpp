#include "CUnitTestBase.h"
#include <string>
#include <cassert>
using namespace std;
#ifdef WIN32
#include <fcntl.h>
#include <io.h>
#else
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#endif

#include <memory.h>
#include <time.h>
#ifndef WIN32
#include<sys/time.h>
#endif

#include <chrono>
#include <ctime>
#include "PlatformFunctionDefine.h"
CUnitTestBase::CUnitTestBase()
{
}


CUnitTestBase::~CUnitTestBase()
{
}

//pszTimeString: if not NULL,will save the time string , the format is: 2019-07-31_15:39:29
int64_t CUnitTestBase::GetCurrentTimeMS(char *pszTimeString)
{
	typedef chrono::time_point<chrono::system_clock, chrono::milliseconds> MsClockType;
	MsClockType tp = chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now());

	if (pszTimeString != NULL)
	{
		time_t timeCur = chrono::system_clock::to_time_t(tp);
		std::strftime(pszTimeString, 60, "%Y-%m-%d_%H:%M:%S", std::localtime(&timeCur));
		char szMSTime[10] = { 0 };
		sprintf_s(szMSTime, 10, ".%03d", (int)(tp.time_since_epoch().count() % 1000));
		strcat(pszTimeString, szMSTime);
	}

	return (int64_t)tp.time_since_epoch().count();
}