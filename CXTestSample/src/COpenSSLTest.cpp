#include "COpenSSLTest.h"
//#include <openssl/rsa.h>  
//#include <openssl/err.h>  
//#include <openssl/pem.h>  
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


//#pragma comment(lib,"Crypt32.lib")
//#pragma comment(lib,"D:/MyProject/CXNetworkCommunicationNew/trunk/third_libs/i386/windows/debug/libssl.lib")
//#pragma comment(lib,"D:/MyProject/CXNetworkCommunicationNew/trunk/third_libs_source/openssl-OpenSSL_1_1_1c/libcrypto.lib")

COpenSSLTest::COpenSSLTest()
{
}


COpenSSLTest::~COpenSSLTest()
{
}

void COpenSSLTest::Test()
{

}
/*
bool generateRSAKey(std::string strKey[2]);
void COpenSSLTest::Test()
{
	string strKey[2] ;
	strKey[0] = "";
	strKey[1] = "";
	generateRSAKey(strKey);

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
		return;
	}
	size_t szReadSize = fread(pbyReadData, 1, dwReadLen, fileLocal);
	fclose(fileLocal);
	int64_t iBeginEncrypt = GetCurrentTimeMS();
	RSAEncryptData(strKey[0], pbyReadData, szReadSize, pbyOutputBuf, dwOutputLen);
	int64_t iMidlle = GetCurrentTimeMS();
	RSADecryptData(strKey[1], pbyOutputBuf, dwOutputLen, pbyCache, dwCacheLen);
	int64_t iEnd = GetCurrentTimeMS();

	printf("encryted time:%lldms,decryted time:%lldms,", iMidlle - iBeginEncrypt, iEnd - iMidlle);
	int l = 0;
}

  
#define KEY_LENGTH  1024               // 密钥长度  
#define PUB_KEY_FILE "pubkey.pem"    // 公钥路径  
#define PRI_KEY_FILE "prikey.pem"    // 私钥路径  

// 函数方法生成密钥对   
bool generateRSAKey(std::string strKey[2])
{

	// 公私密钥对    
	size_t pri_len=0;
	size_t pub_len = 0;
	char *pri_key = NULL;
	char *pub_key = NULL;

	// 生成密钥对    
	RSA *keypair = RSA_generate_key(KEY_LENGTH, RSA_F4, NULL, NULL);

	BIO *pri = BIO_new(BIO_s_mem());
	BIO *pub = BIO_new(BIO_s_mem());

	PEM_write_bio_RSAPrivateKey(pri, keypair, NULL, NULL, 0, NULL, NULL);
	PEM_write_bio_RSAPublicKey(pub, keypair);

	// 获取长度    
	pri_len = BIO_pending(pri);
	pub_len = BIO_pending(pub);

	// 密钥对读取到字符串
	pri_key = (char *)malloc(pri_len + 1);
	pub_key = (char *)malloc(pub_len + 1);

	BIO_read(pri, pri_key, pri_len);
	BIO_read(pub, pub_key, pub_len);

	pri_key[pri_len] = '\0';
	pub_key[pub_len] = '\0';

	// 存储密钥对    
	strKey[0] = pub_key;
	strKey[1] = pri_key;

	// 存储到磁盘（这种方式存储的是begin rsa public key/ begin rsa private key开头的）  
	FILE *pubFile = fopen(PUB_KEY_FILE, "w");
	if (pubFile == NULL)
	{
		return false;
	}
	fputs(pub_key, pubFile);
	fclose(pubFile);

	FILE *priFile = fopen(PRI_KEY_FILE, "w");
	if (priFile == NULL)
	{
		return false;
	}
	fputs(pri_key, priFile);
	fclose(priFile);

	// 内存释放  
	RSA_free(keypair);
	BIO_free_all(pub);
	BIO_free_all(pri);

	free(pri_key);
	free(pub_key);
	return true;
}

int COpenSSLTest::RSAEncryptData(const string &strPubKey, byte *pbyInputBuf, size_t iInputBufLen,
	byte *pbyOutputBuf, DWORD & dwOutputBufLen)
{
	if (strPubKey.empty() || iInputBufLen==0)
	{
		return -1;
	}
	BIO *bio = BIO_new_mem_buf(strPubKey.c_str(), strPubKey.length());
	if (bio == NULL) {
		return -2;
	}
	///从BIO中获取公钥
	RSA* pRSAPublicKey = PEM_read_bio_RSAPublicKey(bio, NULL, NULL, NULL);
	if (pRSAPublicKey == NULL)
	{
		BIO_free(bio);
		return -3;
	}
	BIO_free(bio);

	int nLen = RSA_size(pRSAPublicKey);

	int iRet = 0;
	size_t iTotalOutputDataLen = 0;
	size_t iFixedLen = nLen - 11;
	for (size_t i = 0; i < iInputBufLen; i += iFixedLen)
	{
		size_t iEncryptDataLen = iFixedLen < (iInputBufLen - i) ? iFixedLen : (iInputBufLen - i);
		int ret = RSA_public_encrypt(iEncryptDataLen, pbyInputBuf + i, 
			pbyOutputBuf + iTotalOutputDataLen, pRSAPublicKey, RSA_PKCS1_PADDING);
		if (ret < 0)
		{
			iRet =-4;
			break;
		}

		iTotalOutputDataLen += ret;
	}
	dwOutputBufLen = iTotalOutputDataLen;

	RSA_free(pRSAPublicKey);
	CRYPTO_cleanup_all_ex_data();
	return 0;
}
int COpenSSLTest::RSADecryptData(const string &strPrivKey, byte *pbyInputBuf, size_t iInputBufLen,
	byte *pbyOutputBuf, DWORD & dwOutputBufLen)
{
	if (strPrivKey.empty() || iInputBufLen == 0)
	{
		return -1;
	}
	BIO *bio = BIO_new_mem_buf(strPrivKey.c_str(), strPrivKey.length());
	if (bio == NULL) {
		return -2;
	}
	///从BIO中获取公钥
	RSA* pRSAPriKey = PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, NULL);
	if (pRSAPriKey == NULL)
	{
		BIO_free(bio);
		return -3;
	}
	BIO_free(bio);
	
	int nLen = RSA_size(pRSAPriKey);

	size_t iTotalOutputDataLen = 0;
	size_t iFixedLen = nLen;
	int iRet = 0;
	for (size_t i = 0; i < iInputBufLen; i += iFixedLen)
	{
		size_t iDecryptDataLen = iFixedLen < (iInputBufLen - i) ? iFixedLen : (iInputBufLen - i);
		size_t iOutputLen = 0;
		if (dwOutputBufLen - iTotalOutputDataLen < iFixedLen)
		{
			iRet  = -2;
			break;
		}

		int ret = RSA_private_decrypt(iDecryptDataLen, pbyInputBuf + i, 
			pbyOutputBuf + iTotalOutputDataLen, pRSAPriKey, RSA_PKCS1_PADDING);
		if (ret < 0)
		{
			iRet = -4;
			break;
		}

		iTotalOutputDataLen += ret;
	}
	dwOutputBufLen = iTotalOutputDataLen;

	
	RSA_free(pRSAPriKey);
	CRYPTO_cleanup_all_ex_data();
	return iRet;
}
*/