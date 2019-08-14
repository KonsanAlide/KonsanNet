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
#ifndef __CXIOSTAT_H__
#define __CXIOSTAT_H__

#include "CXServerStructDefine.h"
#include "CXLog.h"
#include "CXThread.h"
#include "CXSpinLock.h"
#include "CXEvent.h"
#include <map>
using namespace std;
namespace CXCommunication
{
    class CXIOStat
    {
		struct IOStatTimeNode {
			int64 iUsedTime;
			bool  bEnd;
		};
    public:
		CXIOStat();
        virtual ~CXIOStat();
		bool Start();
		void Stop();

		void BeginIOStat(string strFlag,string strIOGuid,int64 iTimeMS);
		void EndIOStat(string strFlag,string strIOGuid, int64 iTimeMS);
		//push a io stat info 
		void PushIOStat(string strFlag, string strIOGuid, int64 iUsedTimeMS);

		void SetLogHandle(CXLog * handle) { m_pLogHandle = handle; }

		void Output();

		void SetOutputIntervalMS(int iIntervalMS) { m_iOutputIntervalMs= iIntervalMS; }

	private:
		map<string, map<string, IOStatTimeNode>>m_mapStat;
		CXLog *    m_pLogHandle;
		CXThread   m_thread;
		bool       m_bRunning;
		string     m_strStatContent;
		CXSpinLock m_spinLock;
		int        m_iOutputIntervalMs;
		CXEvent    m_eveWait;
    };
}
#endif // __CXIOSTAT_H__
