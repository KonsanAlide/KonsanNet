/****************************************************************************
Copyright (c) 2018 Charles Yang

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Description£º
*****************************************************************************/
#include "CXFastDataParserHandle.h"
#include "snappy.h"
#include "cryptlib.h"
#include "files.h"
#include "rsa.h"
#include "randpool.h"
#include "hex.h"
#include "osrng.h"
#include "blowfish.h"
#include "modes.h"
#include "eax.h"
#include "gzip.h"
#include "mqueue.h"
#include "channels.h"
#include "zlib.h"

//file defines and implement
#ifdef WIN32
#include <fcntl.h>
#include <io.h>
#else
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#endif


using namespace snappy;
using namespace CryptoPP;
namespace CXCommunication
{
    CXFastDataParserHandle::CXFastDataParserHandle()
    {
		m_strPubKey = "";
		m_strPrivKey = "";
		memset(m_byKey,0, 128);
		memset(m_byIv, 0, 128);
		m_iKeyLen = 0;
		m_iIvLen = 0;
    }

    CXFastDataParserHandle::~CXFastDataParserHandle()
    {
        //dtor
    }

	bool CXFastDataParserHandle::SetBlowfishInfo(byte *pbyKey, int iKeyLen, byte *pbyIv, int iIvLen)
	{
		if (pbyKey == NULL || iKeyLen == 0 || pbyIv == NULL || iIvLen == 0)
			return false;
		int iCopyLen = iKeyLen;
		if (iCopyLen > 128)
			return false;
		memcpy(m_byKey, pbyKey, iCopyLen);
		m_iKeyLen = iCopyLen;
		iCopyLen = iIvLen;
		if (iCopyLen > 128)
			return false;
		memcpy(m_byIv, pbyIv, iCopyLen);
		m_iIvLen = iCopyLen;
		return true;
	}

    bool CXFastDataParserHandle::ParseData(CXENCRYPT_TYPE byEncryptedFlag,
		CXCOMPRESS_TYPE byCompressedFlag,
        const byte *pbSrcBuf, DWORD dwSrcDataLen,
        byte *pbDestBuf, DWORD dwDestBufLen,
		byte *pCacheBuf, DWORD dwCacheBufLen,
        DWORD &dwReturnDataLen)
    {
		bool bRet = false;
		dwReturnDataLen = 0;
		char *pszInputBuf = (char*)pbSrcBuf;
		DWORD dwInputDataLen = dwSrcDataLen;
		char *pszOutputBuf = (char*)pbDestBuf;
		DWORD dwOutputBufLen = dwDestBufLen;

		bool bCompressed = false;
		bool bEncrypted = false;
		if (byEncryptedFlag != CXDataParserImpl::CXENCRYPT_TYPE_NONE)
		{
			bEncrypted = true;
		}
		if (byCompressedFlag != CXDataParserImpl::CXCOMPRESS_TYPE_NONE)
		{
			bCompressed = true;
		}

		if (bEncrypted)
		{
			if (bCompressed)
			{
				pszOutputBuf = (char*)pCacheBuf;
				dwOutputBufLen = dwCacheBufLen;
			}

			switch ((int)byEncryptedFlag)
			{
			case CXENCRYPT_TYPE_RSA_PKCS1v15://RSA_PKCS1v15
			{
				if (0 != RSADecryptData(m_strPrivKey, (byte*)pszInputBuf, dwInputDataLen,
					(byte*)pszOutputBuf, dwOutputBufLen,false))
				{
					return false;
				}
				break;
			}
			case CXENCRYPT_TYPE_RSA_OAEP://RSA_OAEP
			{
				if (0 != RSADecryptData(m_strPubKey, (byte*)pszInputBuf, dwInputDataLen,
					(byte*)pszOutputBuf, dwOutputBufLen))
				{
					return false;
				}
				break;
			}
			case CXENCRYPT_TYPE_BLOWFISH://Blowfish 
			{
                uint64 uiBegin = GetCurrentTimeMS();
				if (0 != BlowfishDecryptData((byte*)&m_byKey,m_iKeyLen, (byte*)&m_byIv,m_iIvLen,
					(byte*)pszInputBuf, dwInputDataLen,
					(byte*)pszOutputBuf, dwOutputBufLen))
				{
					return false;
				}
                uint64 uiEnd = GetCurrentTimeMS();
                uint64 iUsedTime = uiEnd - uiBegin;
				break;
			}
			
			default:
				dwOutputBufLen = 0;
				break;
			}
		}

		if (bCompressed)
		{
			if (bEncrypted)
			{
				pszInputBuf = (char*)pCacheBuf;
				dwInputDataLen = dwOutputBufLen;
				pszOutputBuf = (char*)pbDestBuf;
				dwOutputBufLen = dwDestBufLen;
			}

			switch ((int)byCompressedFlag)
			{
			case CXCOMPRESS_TYPE_GZIP://gzib
            {
                uint64 uiBegin = GetCurrentTimeMS();
				int iDeflateLevel = 0;
				if (!GzipUncompressData(iDeflateLevel,(byte*)pszInputBuf, dwInputDataLen, 
					(byte*)pszOutputBuf, dwOutputBufLen))
				{
					return false;
				}
               
                uint64 uiEnd = GetCurrentTimeMS();
                uint64 iUsedTime = uiEnd - uiBegin;

                break;
            }
			case CXCOMPRESS_TYPE_SNAPPY://snappy
			{
				size_t ulength = 0;
				if (!GetUncompressedLength(pszInputBuf, dwInputDataLen, &ulength)) {
					return false;
				}
				// On 32-bit builds: max_size() < kuint32max.  Check for that instead
				// of crashing (e.g., consider externally specified compressed data).
				if (ulength > dwOutputBufLen) {
					return false;
				}

				bRet = RawUncompress(pszInputBuf, dwInputDataLen, pszOutputBuf);
				if (!bRet)
				{
					return false;
				}
				dwOutputBufLen = ulength;
				break;
			}
            case CXCOMPRESS_TYPE_ZLIB://zlib
            {
                uint64 uiBegin = GetCurrentTimeMS();
				int iDeflateLevel = 0;
				if (!ZlibUncompressData(iDeflateLevel, (byte*)pszInputBuf, dwInputDataLen,
					(byte*)pszOutputBuf, dwOutputBufLen))
				{
					return false;
				}

                uint64 uiEnd = GetCurrentTimeMS();
                uint64 iUsedTime = uiEnd - uiBegin;
                break;
            }
			default:
				dwOutputBufLen = 0;
				break;
			}
		}

		

		dwReturnDataLen = dwOutputBufLen;
		
        return true;
    }

    bool CXFastDataParserHandle::PrepareData(CXENCRYPT_TYPE byEncryptedFlag, 
		CXCOMPRESS_TYPE byCompressedFlag,
		const byte *pbSrcBuf, DWORD dwSrcDataLen,
		byte *pbDestBuf, DWORD dwDestBufLen,
		byte *pCacheBuf, DWORD dwCacheBufLen,
		DWORD &dwReturnDataLen)
    {
		bool bRet = false;
		dwReturnDataLen = 0;
		char *pszInputBuf = (char*)pbSrcBuf;
		DWORD dwInputDataLen = dwSrcDataLen;
		char *pszOutputBuf = (char*)pbDestBuf;
		DWORD dwOutputBufLen = dwDestBufLen;

		bool bCompressed = false;
		bool bEncrypted = false;

		if (byEncryptedFlag != CXDataParserImpl::CXENCRYPT_TYPE_NONE)
		{
			bEncrypted = true;
		}
		if (byCompressedFlag != CXDataParserImpl::CXCOMPRESS_TYPE_NONE)
		{
			bCompressed = true;
		}

		if (bCompressed)
		{
			if (bEncrypted)
			{
				pszOutputBuf = (char*)pCacheBuf;
				dwOutputBufLen = dwCacheBufLen;
			}
			if (dwCacheBufLen < dwSrcDataLen)
			{
				return false;
			}
			switch ((int)byCompressedFlag)
			{
			case CXCOMPRESS_TYPE_GZIP://gzip
			{
                uint64 uiBegin = GetCurrentTimeMS();

				int iDeflateLevel = 0;
				if (!GzipCompressData(iDeflateLevel, (byte*)pszInputBuf, dwInputDataLen,
					(byte*)pszOutputBuf, dwOutputBufLen))
				{
					return false;
				}
                
                uint64 uiEnd = GetCurrentTimeMS();
                uint64 iUsedTime = uiEnd - uiBegin;
				break;
			}
			case CXCOMPRESS_TYPE_SNAPPY://snappy
			{
				size_t uiDestLen = 0;
				RawCompress((char*)pszInputBuf, dwInputDataLen, pszOutputBuf, &uiDestLen);
				dwOutputBufLen = uiDestLen;
				break;
			}
            case CXCOMPRESS_TYPE_ZLIB://zlib
            {
                uint64 uiBegin = GetCurrentTimeMS();
				int iDeflateLevel = 0;
				if (!ZlibCompressData(iDeflateLevel, (byte*)pszInputBuf, dwInputDataLen,
					(byte*)pszOutputBuf, dwOutputBufLen))
				{
					return false;
				}               
                uint64 uiEnd = GetCurrentTimeMS();
                uint64 iUsedTime = uiEnd - uiBegin;
                break;
            }
            
			default:
				dwOutputBufLen = 0;
				break;
			}
		}

		if (bEncrypted)
		{
			if (bCompressed)
			{
				pszInputBuf = (char*)pCacheBuf;
				dwInputDataLen = dwOutputBufLen;
				pszOutputBuf = (char*)pbDestBuf;
				dwOutputBufLen = dwDestBufLen;
			}

			if (dwOutputBufLen < dwInputDataLen)
			{
				return false;
			}
			switch ((int)byEncryptedFlag)
			{
			case CXENCRYPT_TYPE_RSA_PKCS1v15://RSA_PKCS1v15
			{
				if (0 != RSAEncryptData(m_strPubKey, (byte*)pszInputBuf, dwInputDataLen,
					(byte*)pszOutputBuf, dwOutputBufLen,false))
				{
					return false;
				}
				break;
			}
			case CXENCRYPT_TYPE_RSA_OAEP://RSA_OAEP
			{
				if (0 != RSAEncryptData(m_strPubKey, (byte*)pszInputBuf, dwInputDataLen,
					(byte*)pszOutputBuf, dwOutputBufLen))
				{
					return false;
				}
				break;
			}
			case CXENCRYPT_TYPE_BLOWFISH://Blowfish 
			{
                uint64 uiBegin = GetCurrentTimeMS();
				if (0 != BlowfishEncryptData((byte*)&m_byKey, m_iKeyLen, (byte*)&m_byIv, m_iIvLen,
					(byte*)pszInputBuf, dwInputDataLen,
					(byte*)pszOutputBuf, dwOutputBufLen))
				{
					return false;
				}
                uint64 uiEnd = GetCurrentTimeMS();
                uint64 iUsedTime = uiEnd - uiBegin;
				break;
			}

			default:
				dwOutputBufLen = 0;
				break;
			}
		}

		
		dwReturnDataLen = dwOutputBufLen;
		return true;
    }


	int CXFastDataParserHandle::RSAEncryptData(const string &strPubKey, byte *pbyInputBuf, size_t iInputBufLen,
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
		}
		catch (...)
		{
			return -3;
		}

		return 0;
	}

	int CXFastDataParserHandle::RSADecryptData(const string &strPrivKey, byte *pbyInputBuf, size_t iInputBufLen,
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
				//	new PK_DecryptorFilter(randPool, pubDecryptor, 
				//		new ArraySink(pbyOutputBuf+iTotalOutputDataLen, iOutputLen)));

				iTotalOutputDataLen += retDecode.messageLength;
			}
			dwOutputBufLen = iTotalOutputDataLen;
		}
		catch (...)
		{
			return -3;
		}

		return 0;
	}


	int CXFastDataParserHandle::BlowfishEncryptData(const byte *pbyKey, int iKeyLen,
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
	int CXFastDataParserHandle::BlowfishDecryptData(const byte *pbyKey, int iKeyLen,
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
			ArraySource s1(pbyInputBuf, iInputBufLen, true, pFilter);
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

	bool CXFastDataParserHandle::ZlibCompressData(int iDeflateLevel,
		byte *pbyInputBuf, size_t iInputBufLen,
		byte *pbyOutputBuf, DWORD & dwOutputBufLen)
	{
		try
		{
			ZlibCompressor zlib;
			size_t dwRemainLen = zlib.Put(pbyInputBuf, iInputBufLen);
			zlib.MessageEnd();

			if (dwRemainLen > 0)
			{
				return false;
			}

			word64 avail = zlib.MaxRetrievable();
			if (avail > 0 && avail < dwOutputBufLen)
			{
				dwOutputBufLen = zlib.Get(pbyOutputBuf, dwOutputBufLen);
				if (avail > dwOutputBufLen)
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			return true;
		}
		catch (const CryptoPP::Exception& e)
		{
			return false;
		}
	}

	bool CXFastDataParserHandle::ZlibUncompressData(int iDeflateLevel,
		byte *pbyInputBuf, size_t iInputBufLen,
		byte *pbyOutputBuf, DWORD & dwOutputBufLen)
	{
		try
		{
			ArraySink *pArrDst = new ArraySink(pbyOutputBuf, dwOutputBufLen);
			ZlibDecompressor unzlib(pArrDst);
			size_t dwRemainLen = unzlib.Put(pbyInputBuf, iInputBufLen);
			unzlib.MessageEnd();


			dwOutputBufLen = pArrDst->TotalPutLength();
			if (dwOutputBufLen == 0 || dwRemainLen > 0)
			{
				return false;
			}
			return true;
		}
		catch (const CryptoPP::Exception& e)
		{
			return false;
		}
	}

	bool CXFastDataParserHandle::GzipCompressData(int iDeflateLevel,
		byte *pbyInputBuf, size_t iInputBufLen,
		byte *pbyOutputBuf, DWORD & dwOutputBufLen)
	{
		try
		{
			Gzip zipper;
			size_t dwRemainLen = zipper.Put(pbyInputBuf, iInputBufLen);
			zipper.MessageEnd();

			if (dwRemainLen > 0)
			{
				return false;
			}

			word64 avail = zipper.MaxRetrievable();
			if (avail > 0 && avail < dwOutputBufLen)
			{
				dwOutputBufLen = zipper.Get(pbyOutputBuf, dwOutputBufLen);
				if (avail > dwOutputBufLen)
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			return true;
		}
		catch (const CryptoPP::Exception& e)
		{
			return false;
		}
	}

	bool CXFastDataParserHandle::GzipUncompressData(int iDeflateLevel,
		byte *pbyInputBuf, size_t iInputBufLen,
		byte *pbyOutputBuf, DWORD & dwOutputBufLen)
	{
		try
		{
			ArraySink *pArrDst = new ArraySink(pbyOutputBuf, dwOutputBufLen);
			Gunzip unzipper(pArrDst);
			size_t dwRemainLen = unzipper.Put(pbyInputBuf, iInputBufLen);
			unzipper.MessageEnd();


			dwOutputBufLen = pArrDst->TotalPutLength();
			if (dwOutputBufLen == 0 || dwRemainLen > 0)
			{
				return false;
			}
			return true;
		}
		catch (const CryptoPP::Exception& e)
		{
			return false;
		}
	}


    unsigned long get_file_size(const char *path)
    {
        unsigned long filesize = -1;
        struct stat statbuff;
        if(stat(path, &statbuff) < 0){
            return filesize;
        }else{
            filesize = statbuff.st_size;
        }
        return filesize;
    }

	bool CXFastDataParserHandle::ReadKeyFile(string strFilePathName, string &strOutputContent)
	{
		strOutputContent = "";
		FILE * fileKey = fopen(strFilePathName.c_str(), "r");
		if (fileKey == NULL)
		{
			return false;
		}

		int iPrivFileLen = 0;
#ifdef WIN32
		iPrivFileLen = filelength(fileno(fileKey));
#else
		iPrivFileLen = get_file_size(strFilePathName.c_str());
#endif

		char szFileData[CX_BUF_SIZE] = { 0 };
		int  iTotalReadLen = 0;
		int  iReadLenInOnce = CX_BUF_SIZE;

		while (iTotalReadLen < iPrivFileLen)
		{
			iReadLenInOnce = CX_BUF_SIZE;
			if (iReadLenInOnce > (iPrivFileLen - iTotalReadLen))
			{
				iReadLenInOnce = (iPrivFileLen - iTotalReadLen);
			}

			int iRead = fread(szFileData, 1, iReadLenInOnce, fileKey);

			if (iRead <= 0)
			{
				if (feof(fileKey) != 0)
				{
					break;
				}
				else
				{
					fclose(fileKey);
					return false;
				}
			}

			strOutputContent += szFileData;
			memset(szFileData, 0, CX_BUF_SIZE);
		}
		fclose(fileKey);
		return true;
	}

	bool CXFastDataParserHandle::LoadRSAKeyFiles(string strPrivateFile, string strPublicFile)
	{
		if (strPrivateFile.length() > 0)
		{
			if (!ReadKeyFile(strPrivateFile, m_strPrivKey))
			{
				return false;
			}
		}

		if (strPublicFile.length() > 0)
		{
			if (!ReadKeyFile(strPublicFile, m_strPubKey))
			{
				return false;
			}
		}
	
		return true;
	}
	bool CXFastDataParserHandle::LoadBlowfishKeyFiles(string strKeyFile, string strIvFile)
	{
		FILE * fileKey = NULL;
		if (strKeyFile.length() > 0)
		{
			fileKey = fopen(strKeyFile.c_str(), "rb");
			if (fileKey == NULL)
			{
				return false;
			}

			int iFileLen = 0;
#ifdef WIN32
			iFileLen = filelength(fileno(fileKey));
#else
			iFileLen = get_file_size(strKeyFile.c_str());
#endif
			if (iFileLen > 128)
			{
				fclose(fileKey);
				return false;
			}

			int iRead = fread(m_byKey, 1, iFileLen, fileKey);
			if (iRead != iFileLen)
			{
				fclose(fileKey);
				return false;
			}
			m_iKeyLen = iFileLen;
			fclose(fileKey);
		}

		if (strIvFile.length() > 0)
		{
			fileKey = fopen(strIvFile.c_str(), "rb");
			if (fileKey == NULL)
			{
				return false;
			}

			int iFileLen = 0;
#ifdef WIN32
			iFileLen = filelength(fileno(fileKey));
#else
			iFileLen = get_file_size(strIvFile.c_str());
#endif
			if (iFileLen > 128)
			{
				fclose(fileKey);
				return false;
			}

			int iRead = fread(m_byIv, 1, iFileLen, fileKey);
			if (iRead != iFileLen)
			{
				fclose(fileKey);
				return false;
			}
			m_iIvLen = iFileLen;
			fclose(fileKey);
		}

		return true;
	}
}
