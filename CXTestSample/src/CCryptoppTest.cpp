#include "CCryptoppTest.h"
#include <iostream>
#include "PlatformDataTypeDefine.h"
#include "cryptlib.h"
#include "files.h"
#include "rsa.h"
#include "randpool.h"
#include "hex.h"
#include "osrng.h"
#include "blowfish.h"
#include "modes.h"
#include "eax.h"

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

using namespace std;
using namespace CryptoPP;

CCryptoppTest::CCryptoppTest()
{
}


CCryptoppTest::~CCryptoppTest()
{
}



void CCryptoppTest::Test()
{
	char szSeed[600] = "";

	AutoSeededRandomPool randPool;
	//RandomPool randPool;
	//randPool.IncorporateEntropy((byte *)szSeed, strlen(szSeed));
	string strLocalPath = "";
#ifdef WIN32
	char szFilePath[MAX_PATH] = { 0 }, szDrive[MAX_PATH] = { 0 };
	char szDir[MAX_PATH] = { 0 }, szFileName[MAX_PATH] = { 0 }, szExt[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));
	_splitpath(szFilePath, szDrive, szDir, szFileName, szExt);
	strLocalPath = szDrive;
	strLocalPath.append(szDir);
#else
	strLocalPath = "./";
#endif

	string strPrivFileName = strLocalPath+"privateKey.conf";
	RSAES_PKCS1v15_Decryptor priv(randPool, 1024);
	string strPrivKey = "";
	HexEncoder privFile(new StringSink(strPrivKey));
	//HexEncoder privFile(new FileSink(strPrivFileName.c_str()));
	priv.AccessMaterial().Save(privFile);
	privFile.MessageEnd();
	//privFile.(ssDestBuf);

	string strPubFileName = strLocalPath + "pubKey.conf";
	RSAES_PKCS1v15_Encryptor pub(priv);
	string strPubKey = "";
	HexEncoder pubFile(new StringSink(strPubKey));
	//HexEncoder pubFile(new FileSink(strPubFileName.c_str()));
	pub.AccessMaterial().Save(pubFile);
	pubFile.MessageEnd();

	printf("privKey:%s \n pubKey:%s\n", strPrivKey.c_str(), strPubKey.c_str());


	//TestRSA(strPubKey, strPrivKey);
	TestBlowfish();

    //D:\MyProject\CXNetworkCommunicationNew\trunk\bin\Debug
	/*

	ArraySource pubKey(strPubKey.c_str(), true, new HexDecoder);

	RSAES_PKCS1v15_Encryptor pubEncryptor(pubKey);
	size_t fixedLen = pubEncryptor.FixedMaxPlaintextLength();


	//randPool.Put(seed, sizeof(seed));

	//char szSeed[128] = "seed#$&*";
	//RandomPool randPool;
	char pszInputBuf[256] = "sdfjl1jl231jl2j房价拉升江东父老就SJFLSLDF234902";
	DWORD dwInputBufLen = strlen(pszInputBuf);
	char pszOutputBuf[256] = { 0 };
	DWORD dwOutputBufLen = 256;
	//ArraySink asDestBuf((byte*)pszOutputBuf, dwOutputBufLen);
	//HexEncoder outputHexEncoder(&asDestBuf);
	//PK_EncryptorFilter pkEncryptorFilter(randPool, pubEncryptor, &outputHexEncoder);

	std::string result = "";
	//StringSource(pszInputBuf, true, new PK_EncryptorFilter(randPool, pub, new HexEncoder(new StringSink(result))));
	ArraySink *dstArr = new ArraySink((byte*)pszOutputBuf, dwOutputBufLen);
	ArraySource source((byte *)pszInputBuf, dwInputBufLen, true, new PK_EncryptorFilter(randPool, pub, dstArr));
	size_t putLen = dstArr->TotalPutLength();
	//StringSource((byte *)pszInputBuf, dwInputBufLen, &pkEncryptorFilter);
	size_t iOutputLen = pubEncryptor.CiphertextLength(dwInputBufLen);

	char pszTempBuf[256] = { 0 };
	RSAES_PKCS1v15_Encryptor encryptor(pub);
	encryptor.Encrypt(randPool, (byte *)pszInputBuf, dwInputBufLen, (byte *)pszTempBuf);

	string chilper;
	StringSource((byte *)pszInputBuf, dwInputBufLen, true, new PK_EncryptorFilter(randPool, pub, new StringSink(chilper)));
	printf("chilper = %s, length = %d\n", chilper.c_str(), strlen(chilper.c_str()));

	//printf("Encrypt:%s\n", pszOutputBuf);

	char pszDeOutputBuf[256] = "";
	DWORD dwDeOutputBufLen = 256;
	//ArraySink asDecryptDestBuf((byte*)pszDeOutputBuf, dwDeOutputBufLen);
	//HexEncoder deHexEncoder(&asDecryptDestBuf);
	StringSource privDeKey(strPrivKey.c_str(), true, new HexDecoder);
	RSAES_PKCS1v15_Decryptor privDe(privDeKey);


	size_t MaxfixedLen = privDe.FixedCiphertextLength();

	AutoSeededRandomPool randPool2;
	//std::string result;
	ArraySink *dstArrDe = new ArraySink((byte*)pszDeOutputBuf, dwDeOutputBufLen);
	ArraySource sourceDe((byte*)pszOutputBuf, putLen, true, new PK_DecryptorFilter(randPool2, privDe, dstArrDe));
	size_t iDeLen = dstArrDe->TotalPutLength();

	//StringSource(pszOutputBuf, iOutputLen,true, 
	//	new HexDecoder(new PK_DecryptorFilter(randPool, privDe, new StringSink(result))));

	printf("Decrypted:%s\n", pszDeOutputBuf);

	//PK_EncryptorFilter pkEncryptorFilter(randPool, pubEncryptor, &outputHexEncoder);
	//StringSource((byte *)pszInputBuf, dwInputBufLen, &pkEncryptorFilter);
	//size_t iOutputLen = pubEncryptor.CiphertextLength(dwInputBufLen);



	//pubEncryptor.Encrypt(randPool, (byte *)pszInputBuf, dwInputDataLen, (byte*)pszOutputBuf);
	//size_t iOutputLen = pubEncryptor.CiphertextLength(dwInputDataLen); //Sign message
	*/
	return ;
}


void CCryptoppTest::TestRSA(const string &strPubKey, const string &strPrivKey)
{
	DWORD dwReadLen = 1024 * 1024;
	DWORD dwInputBufLen = dwReadLen * 2;
	byte *pbyReadData = new byte[dwInputBufLen];
	memset(pbyReadData, 0, dwInputBufLen);

	DWORD dwCacheLen = dwInputBufLen;
	byte *pbyCache = new byte[dwCacheLen];
	memset(pbyCache, 0, dwCacheLen);

	DWORD dwOutputLen = dwInputBufLen;
	byte *pbyOutputBuf = new byte[dwOutputLen];
	memset(pbyOutputBuf, 0, dwOutputLen);

	string strLocalFilePath = "D:/server.log";
	FILE * fileLocal = fopen(strLocalFilePath.c_str(), "rb");
	if (fileLocal == NULL)
	{
		return ;
	}
	size_t szReadSize = fread(pbyReadData, 1, dwReadLen, fileLocal);
	fclose(fileLocal);
	int64_t iBeginEncrypt = GetCurrentTimeMS();
	RSAEncryptData(strPubKey, pbyReadData, szReadSize, pbyOutputBuf, dwOutputLen,false);
	int64_t iMidlle = GetCurrentTimeMS();
	RSADecryptData(strPrivKey,  pbyOutputBuf, dwOutputLen, pbyCache, dwCacheLen, false);
	int64_t iEnd = GetCurrentTimeMS();

	printf("encryted time:%lldms,decryted time:%lldms,", iMidlle- iBeginEncrypt, iEnd- iMidlle);

}


void CCryptoppTest::TestBlowfish()
{

	string strLocalPath = "";
#ifdef WIN32
	char szFilePath[MAX_PATH] = { 0 }, szDrive[MAX_PATH] = { 0 };
	char szDir[MAX_PATH] = { 0 }, szFileName[MAX_PATH] = { 0 }, szExt[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));
	_splitpath(szFilePath, szDrive, szDir, szFileName, szExt);
	strLocalPath = szDrive;
	strLocalPath.append(szDir);
#else
	strLocalPath = "./";
#endif


	int  iIvDataLen = Blowfish::BLOCKSIZE;
	int  iKeyLen = Blowfish::DEFAULT_KEYLENGTH;
	//for (int i = 0; i < 3; i++)
	{
		AutoSeededRandomPool prng;
		iKeyLen += 8;
		iIvDataLen += 8;
		SecByteBlock key(iKeyLen);
		prng.GenerateBlock(key, key.size());

		byte *pbyIV = new byte[iIvDataLen];
		prng.GenerateBlock(pbyIV, iIvDataLen);

		cout << "key length: " << iKeyLen << endl;
		cout << "key length (min): " << Blowfish::MIN_KEYLENGTH << endl;
		cout << "key length (max): " << Blowfish::MAX_KEYLENGTH << endl;
		cout << "block size: " << Blowfish::BLOCKSIZE << endl;
		cout << "iv length: " << iIvDataLen << endl;

		/*********************************\
		\*********************************/
		string encoded = "";
		// Pretty print key
		StringSource(key, key.size(), true,
			new HexEncoder(
				new StringSink(encoded)
			) // HexEncoder
		); // StringSource
		cout << "key: " << encoded << endl;

		string strKeyFile = strLocalPath + "key.conf";
		FILE * fileLocal = fopen(strKeyFile.c_str(), "wb");
		if (fileLocal == NULL)
		{
			return;
		}
		size_t szWrittenSize = fwrite(key.BytePtr(), 1, key.size(), fileLocal);
		fclose(fileLocal);

		string strIvFile = strLocalPath + "iv.conf";
		fileLocal = fopen(strIvFile.c_str(), "wb");
		if (fileLocal == NULL)
		{
			return;
		}
		szWrittenSize = fwrite(pbyIV, 1, iIvDataLen, fileLocal);
		fclose(fileLocal);


		DWORD dwReadLen = 1024 * 1024;
		DWORD dwInputBufLen = dwReadLen * 2;
		byte *pbyReadData = new byte[dwInputBufLen];
		memset(pbyReadData, 0, dwInputBufLen);

		DWORD dwCacheLen = dwInputBufLen;
		byte *pbyCache = new byte[dwCacheLen];
		memset(pbyCache, 0, dwCacheLen);

		DWORD dwOutputLen = dwInputBufLen;
		byte *pbyOutputBuf = new byte[dwOutputLen];
		memset(pbyOutputBuf, 0, dwOutputLen);

		string strLocalFilePath = "D:/server.log";
		fileLocal = fopen(strLocalFilePath.c_str(), "rb");
		if (fileLocal == NULL)
		{
			return;
		}
		size_t szReadSize = fread(pbyReadData, 1, dwReadLen, fileLocal);
		fclose(fileLocal);
		int64_t iBeginEncrypt = GetCurrentTimeMS();
		BlowfishEncryptData(key.BytePtr(), key.size(), pbyIV, iIvDataLen, pbyReadData, szReadSize, pbyOutputBuf, dwOutputLen);
		int64_t iMidlle = GetCurrentTimeMS();
		BlowfishDecryptData(key.BytePtr(), key.size(), pbyIV, iIvDataLen, pbyOutputBuf, dwOutputLen, pbyCache, dwCacheLen);
		int64_t iEnd = GetCurrentTimeMS();

		if (dwCacheLen != szReadSize || memcmp(pbyReadData, pbyCache, szReadSize) != 0)
		{
			printf("Failed to encrypt data used Blowfish\n");
		}
		delete[]pbyReadData;
		delete[]pbyCache;
		delete[]pbyOutputBuf;

		printf("Blowfish encrypt:\n encryted time:%lldms,decryted time:%lldms\n", iMidlle - iBeginEncrypt, iEnd - iMidlle);
		delete[]pbyIV;
	}
}


int CCryptoppTest::RSAEncryptData(const string &strPubKey, byte *pbyInputBuf, size_t iInputBufLen,
	byte *pbyOutputBuf, DWORD & dwOutputBufLen, bool bOAEP)
{
	try
	{
		StringSource pubKey(strPubKey.c_str(), true, new HexDecoder);
		//OAEP encrypt
		//RSAES_OAEP_SHA_Encryptor pubEncryptor(pubKey);

		//PKCS1v15 encrypt
		RSAES_PKCS1v15_Encryptor pubEncryptor(pubKey);

		AutoSeededRandomPool randPool;

		
		size_t iTotalOutputDataLen = 0;
		size_t iFixedLen = pubEncryptor.FixedMaxPlaintextLength();

		for (size_t i = 0; i < iInputBufLen; i += iFixedLen)
		{
			size_t iEncryptDataLen = iFixedLen < (iInputBufLen - i) ? iFixedLen : (iInputBufLen - i);
			size_t iOutputLen = pubEncryptor.CiphertextLength(iEncryptDataLen);
			if (dwOutputBufLen - iTotalOutputDataLen < iOutputLen)
			{
				return -2;
			}
			pubEncryptor.Encrypt(randPool, pbyInputBuf + i, iEncryptDataLen, pbyOutputBuf + iTotalOutputDataLen);
			//ArraySource source(pbyInputBuf+i, iEncryptDataLen, true,
			//	new PK_EncryptorFilter(randPool, pubEncryptor, 
			//		new ArraySink(pbyOutputBuf+iTotalOutputDataLen, iOutputLen)));
			iTotalOutputDataLen += iOutputLen;
		}
		dwOutputBufLen = iTotalOutputDataLen;
		
		/*
		ArraySink *dstArr = new ArraySink(pbyOutputBuf, dwOutputBufLen);
		ArraySource source(pbyInputBuf, 110, true,
			new PK_EncryptorFilter(randPool, pubEncryptor, dstArr));
		dwOutputBufLen = dstArr->TotalPutLength();
		*/
	}
	catch (...)
	{
		return -3;
	}

	return 0;
}

int CCryptoppTest::RSADecryptData(const string &strPrivKey, byte *pbyInputBuf, size_t iInputBufLen,
	byte *pbyOutputBuf, DWORD & dwOutputBufLen, bool bOAEP)
{
	try
	{
		StringSource privKey(strPrivKey.c_str(), true, new HexDecoder);
		//OAEP decrypt
		//RSAES_OAEP_SHA_Decryptor privDecryptor(privKey);

		//PKCS1v15 encrypt
		RSAES_PKCS1v15_Decryptor privDecryptor(privKey);

		AutoSeededRandomPool randPool;

		size_t iTotalOutputDataLen = 0;
		size_t iFixedLen = privDecryptor.FixedCiphertextLength();

		for (size_t i = 0; i < iInputBufLen; i += iFixedLen)
		{
			size_t iDecryptDataLen = iFixedLen < (iInputBufLen - i) ? iFixedLen : (iInputBufLen - i);
			size_t iOutputLen = privDecryptor.MaxPlaintextLength(iDecryptDataLen);
			if (dwOutputBufLen - iTotalOutputDataLen < iOutputLen)
			{
				return -2;
			}
			DecodingResult retDecode = privDecryptor.Decrypt(randPool, pbyInputBuf + i, iDecryptDataLen, pbyOutputBuf + iTotalOutputDataLen);
			if (!retDecode.isValidCoding)
			{
				return -4;
			}

			if (retDecode.messageLength > iOutputLen)
			{
				return -5;
			}

			//ArraySource source(pbyInputBuf+i, iDecryptDataLen, true,
			//	new PK_DecryptorFilter(randPool, privDecryptor, 
			//		new ArraySink(pbyOutputBuf+iTotalOutputDataLen, iOutputLen)));

			iTotalOutputDataLen += retDecode.messageLength;
		}
		dwOutputBufLen = iTotalOutputDataLen;
		
		/*
		ArraySink *dstArr = new ArraySink(pbyOutputBuf, dwOutputBufLen);
		ArraySource source(pbyInputBuf, iInputBufLen, true,
			new PK_DecryptorFilter(randPool, privDecryptor, dstArr));
		dwOutputBufLen = dstArr->TotalPutLength();
		*/
	}
	catch (...)
	{
		return -3;
	}

	return 0;
}

//byte iv[Blowfish::BLOCKSIZE];

int CCryptoppTest::BlowfishEncryptData(const byte *pbyKey, int iKeyLen,
	byte *pbyIvData, int iIvDataLen,
	byte *pbyInputBuf, size_t iInputBufLen,
	byte *pbyOutputBuf, DWORD & dwOutputBufLen)
{
	int iRet = 0;
	if (pbyKey==NULL || iKeyLen==0 
		|| pbyIvData == NULL || iIvDataLen == 0
		|| pbyInputBuf == NULL || iInputBufLen == 0
		|| pbyOutputBuf == NULL || dwOutputBufLen == 0)
	{
		return -1;
	}

	AutoSeededRandomPool prng;
	try
	{
		EAX< Blowfish >::Encryption encryptor;
		encryptor.SetKeyWithIV(pbyKey, iKeyLen, pbyIvData, iIvDataLen);

		ArraySink *dstArr = new ArraySink(pbyOutputBuf, dwOutputBufLen);
		ArraySource s1(pbyInputBuf, iInputBufLen, true,
			new AuthenticatedEncryptionFilter(encryptor, dstArr) // StreamTransformationFilter
		); 
		size_t putLen = dstArr->TotalPutLength();
		dwOutputBufLen = putLen;
	}
	catch (const CryptoPP::Exception& e)
	{
		return -2;
	}

	return iRet;
}
int CCryptoppTest::BlowfishDecryptData(const byte *pbyKey, int iKeyLen,
	byte *pbyIvData, int iIvDataLen,
	byte *pbyInputBuf, size_t iInputBufLen,
	byte *pbyOutputBuf, DWORD & dwOutputBufLen)
{
	int iRet = 0;
	if (pbyKey == NULL || iKeyLen == 0
		|| pbyIvData == NULL || iIvDataLen == 0
		|| pbyInputBuf == NULL || iInputBufLen == 0
		|| pbyOutputBuf == NULL || dwOutputBufLen == 0)
	{
		return -1;
	}

	AutoSeededRandomPool prng;
	try
	{
		EAX< Blowfish >::Decryption decryptor;
		decryptor.SetKeyWithIV(pbyKey, iKeyLen, pbyIvData, iIvDataLen);

		ArraySink *dstArr = new ArraySink(pbyOutputBuf, dwOutputBufLen);
		AuthenticatedDecryptionFilter *pFilter = new AuthenticatedDecryptionFilter(decryptor, dstArr);
		ArraySource s1(pbyInputBuf, iInputBufLen, true, pFilter );
		size_t putLen = dstArr->TotalPutLength();
		//delete pFilter;
		//delete dstArr;
		dwOutputBufLen = putLen;
	}
	catch (const CryptoPP::Exception& e)
	{
		//cerr << e.what() << endl;
		return -2;
	}
	return iRet;
}
