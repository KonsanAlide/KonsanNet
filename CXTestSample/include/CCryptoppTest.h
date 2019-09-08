#pragma once
#include "CUnitTestBase.h"
#ifdef WIN32
#include<windows.h>
#endif

class CCryptoppTest :public CUnitTestBase
{
public:
	CCryptoppTest();
	virtual ~CCryptoppTest();
	virtual void Test();

	void TestRSA(const string &strPubKey, const string &strPrivKey);
	void TestBlowfish();

	int RSAEncryptData(const string &strPubKey, byte *pbyInputBuf, size_t iInputBufLen,
		byte *pbyOutputBuf, DWORD & dwOutputBufLen, bool bOAEP = true);
	int RSADecryptData(const string &strPrivKey, byte *pbyInputBuf, size_t iInputBufLen,
		byte *pbyOutputBuf, DWORD & dwOutputBufLen, bool bOAEP = true);

	int BlowfishEncryptData(const byte *pbyKey, int iKeyLen, 
		byte *pbyIvData,int iIvDataLen, 
		byte *pbyInputBuf, size_t iInputBufLen,
		byte *pbyOutputBuf, DWORD & dwOutputBufLen);
	int BlowfishDecryptData(const byte *pbyKey, int iKeyLen,
		byte *pbyIvData, int iIvDataLen,
		byte *pbyInputBuf, size_t iInputBufLen,
		byte *pbyOutputBuf, DWORD & dwOutputBufLen);
};

