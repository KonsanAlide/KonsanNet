/****************************************************************************
Copyright (c) 2018-2019 Charles Yang

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
#ifndef __CXSERVERCONFIG_H__
#define __CXGUIDGENERATE_H__

#include <string>
using namespace std;
#ifdef WIN32
#include <windows.h>
#endif
#include "PlatformDataTypeDefine.h"

typedef struct CX_CONFING_NET{
	//the timeout milliseconds that try to connect the other computer
	DWORD dwConnectedTimeoutMS;

	//when a client's connection is idle dwIdleTimeMS milliseconds, close it
	DWORD dwClientIdleTimeMS;

	//the maximum milliseconds of the used time that process a message
	DWORD dwProcessMesTimeoutMS;

	//record slow operation
	bool  bRecodSlowOps;

	//the milliseconds of a slow operation
	DWORD dwSlowOpMS;

	//when a connection had been close, after dwRetrieveConnetionMS milliseconds, retrieve it to the free list
	DWORD dwRetrieveConnetionMS;

	//if this a connection that start by myself,that is a proxy connection, keep this persistent connection, 
	//when this connection had been corrupted by accidence, reconnect it.
	bool  bPersistProxyConnection;

	//when a proxy's connection is idle dwIdleTimeMS milliseconds, close it
	DWORD dwProxyIdleTimeMS;

}CXConfigNet,*PCXConfigNet;

class CXServerConfig
{
public:
	CXServerConfig();
    ~CXServerConfig();

    bool Load();

	void SetConfigFilePath(string strPath) { m_strFilePath=strPath; }

private:
	string m_strFilePath;

};
#endif //__CXGUIDGENERATE_H__

