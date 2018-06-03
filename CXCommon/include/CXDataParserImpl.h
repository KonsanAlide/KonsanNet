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
            CXDataParserImpl();
            virtual ~CXDataParserImpl();

            virtual bool ParseData(bool bEncrypted,bool bCompressed,
                const byte *pbSrcBuf,DWORD dwSrcDataLen,
                byte *pbDestBuf, DWORD dwDestBufLen, 
                DWORD &dwReadDataLen)=0;

            virtual bool PrepareData(bool bEncrypted, bool bCompressed,
                const byte *pbSrcBuf, DWORD dwSrcDataLen,
                byte *pbDestBuf, DWORD dwDestBufLen,
                DWORD &dwReadDataLen) = 0;
        protected:
        private:
    };
}

#endif // CXDATAPARSERIMPL_H
