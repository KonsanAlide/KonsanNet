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
#ifndef CXDATAPARSERIMPL_H
#define CXDATAPARSERIMPL_H
#include "PlatformDataTypeDefine.h"
#ifdef WIN32
#include<windows.h>
#endif
namespace CXCommunication
{
    class CXDataParserImpl
    {
	public:
		enum CXENCRYPT_TYPE {
			CXENCRYPT_TYPE_NONE         = 0x0,
			CXENCRYPT_TYPE_RSA_PKCS1v15 = 0x81,
			CXENCRYPT_TYPE_RSA_OAEP     = 0x82,
			CXENCRYPT_TYPE_BLOWFISH     = 0x83,
		};

		enum CXCOMPRESS_TYPE {
			CXCOMPRESS_TYPE_NONE   = 0x0,
			CXCOMPRESS_TYPE_GZIP   = 0x81,
			CXCOMPRESS_TYPE_SNAPPY = 0x82,
            CXCOMPRESS_TYPE_ZLIB   = 0x83,
		};

    public:
        CXDataParserImpl();
        virtual ~CXDataParserImpl();

        virtual bool ParseData(CXENCRYPT_TYPE byEncryptedFlag, CXCOMPRESS_TYPE byCompressedFlag,
            const byte *pbSrcBuf,DWORD dwSrcDataLen,
            byte *pbDestBuf, DWORD dwDestBufLen, 
			byte *pCacheBuf,DWORD dwCacheBufLen,
            DWORD &dwReturnDataLen)=0;

        virtual bool PrepareData(CXENCRYPT_TYPE byEncryptedFlag, CXCOMPRESS_TYPE byCompressedFlag,
            const byte *pbSrcBuf, DWORD dwSrcDataLen,
            byte *pbDestBuf, DWORD dwDestBufLen,
			byte *pCacheBuf, DWORD dwCacheBufLen,
            DWORD &dwReturnDataLen) = 0;

        //pszTimeString: if not NULL,will save the time string , the format is: 2019-07-31_15:39:29
        //return value : the current time 
        int64   GetCurrentTimeMS(char *pszTimeString = NULL);

    protected:
    private:
    };
}

#endif // CXDATAPARSERIMPL_H
