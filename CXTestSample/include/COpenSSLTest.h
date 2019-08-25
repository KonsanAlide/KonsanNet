#pragma once
#include "CUnitTestBase.h"
class COpenSSLTest :public CUnitTestBase
{
public:
    COpenSSLTest();
    virtual ~COpenSSLTest();
	virtual void Test();

	int RSAEncryptData(const string &strPubKey, byte *pbyInputBuf, size_t iInputBufLen,
		byte *pbyOutputBuf, DWORD & dwOutputBufLen);
	int RSADecryptData(const string &strPrivKey, byte *pbyInputBuf, size_t iInputBufLen,
		byte *pbyOutputBuf, DWORD & dwOutputBufLen);
};

