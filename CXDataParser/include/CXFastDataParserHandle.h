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
#ifndef CXFASTDATAPARSERHANDLE_H
#define CXFASTDATAPARSERHANDLE_H
#include "PlatformDataTypeDefine.h"
#ifdef WIN32
#include<windows.h>
#endif
#include "CXDataParserImpl.h"
#include <string>

using namespace std;
namespace CXCommunication
{
    class CXFastDataParserHandle : public CXDataParserImpl
    {
        public:
            CXFastDataParserHandle();
            virtual ~CXFastDataParserHandle();

			//decrypt and uncompress the data
            bool ParseData(CXENCRYPT_TYPE byEncryptedFlag, CXCOMPRESS_TYPE byCompressedFlag,
                const byte *pbSrcBuf, DWORD dwSrcDataLen,
                byte *pbDestBuf, DWORD dwDestBufLen,
				byte *pCacheBuf, DWORD dwCacheBufLen,
                DWORD &dwReturnDataLen);

			//encrypt and compress the data
            bool PrepareData(CXENCRYPT_TYPE byEncryptedFlag, CXCOMPRESS_TYPE byCompressedFlag,
                const byte *pbSrcBuf, DWORD dwSrcDataLen,
                byte *pbDestBuf, DWORD dwDestBufLen,
				byte *pCacheBuf, DWORD dwCacheBufLen,
                DWORD &dwReturnDataLen);

			void SetPubKey(string strKey) { m_strPubKey = strKey; }
			void SetPrivKey(string strKey) { m_strPrivKey= strKey; }
			bool SetBlowfishInfo(byte *pbyKey,int iKeyLen,byte *pbyIv,int iIvLen);

			bool LoadRSAKeyFiles(string strPrivateFile,string strPublicFile);
			bool LoadBlowfishKeyFiles(string strKeyFile, string strIvFile);

        protected:
			int RSAEncryptData(const string &strPubKey, byte *pbyInputBuf, size_t iInputBufLen,
				byte *pbyOutputBuf, DWORD & dwOutputBufLen, bool bOAEP = true);
			int RSADecryptData(const string &strPrivKey, byte *pbyInputBuf, size_t iInputBufLen,
				byte *pbyOutputBuf, DWORD & dwOutputBufLen, bool bOAEP = true);

			int BlowfishEncryptData(const byte *pbyKey, int iKeyLen,
				byte *pbyIvData, int iIvDataLen,
				byte *pbyInputBuf, size_t iInputBufLen,
				byte *pbyOutputBuf, DWORD & dwOutputBufLen);

			int BlowfishDecryptData(const byte *pbyKey, int iKeyLen,
				byte *pbyIvData, int iIvDataLen,
				byte *pbyInputBuf, size_t iInputBufLen,
				byte *pbyOutputBuf, DWORD & dwOutputBufLen);

			bool ZlibCompressData(int iDeflateLevel,
				byte *pbyInputBuf, size_t iInputBufLen,
				byte *pbyOutputBuf, DWORD & dwOutputBufLen);

			bool ZlibUncompressData(int iDeflateLevel,
				byte *pbyInputBuf, size_t iInputBufLen,
				byte *pbyOutputBuf, DWORD & dwOutputBufLen);

			bool GzipCompressData(int iDeflateLevel,
				byte *pbyInputBuf, size_t iInputBufLen,
				byte *pbyOutputBuf, DWORD & dwOutputBufLen);

			bool GzipUncompressData(int iDeflateLevel,
				byte *pbyInputBuf, size_t iInputBufLen,
				byte *pbyOutputBuf, DWORD & dwOutputBufLen);


			bool ReadKeyFile(string strFilePathName, string &strOutputContent);

        private:
			string m_strPubKey;
			string m_strPrivKey;
			byte   m_byKey[128];
			byte   m_byIv[128];
			int    m_iKeyLen;
			int    m_iIvLen;
    };
}

#endif // CXFASTDATAPARSERHANDLE_H
