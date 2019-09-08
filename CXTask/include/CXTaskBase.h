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
#ifndef __CXTASK_BASE_H__
#define __CXTASK_BASE_H__

#include "PlatformDataTypeDefine.h"
#include "CXThreadPool.h"
#include "CXMemoryCacheManager.h"
#include "CXEvent.h"
namespace CXCommunication
{
	class CXTaskBase
	{
	public:
		//iTaskType==1 parsed data, uncompress and decrypt
		//iTaskType==2 prepare data, compress and encrypt
		//iTaskType==3 time-consuming calculation
		//iTaskType==4 connect to remote peer and sent data
		enum CX_RUNING_TASK_TYPE
		{
			CX_RUNING_TASK_TYPE_PARSE_DATA                      =1,
			CX_RUNING_TASK_TYPE_PREPARE_DATA                    =2,
			CX_RUNING_TASK_TYPE_CALCUTION                       =3,
			CX_RUNING_TASK_TYPE_CREATE_CONNECITON               =4,
		};
		
	public:
		CXTaskBase();
		virtual ~CXTaskBase();
		virtual CXTaskBase* CreateObject() = 0;
		virtual void  StopTask();
		virtual DWORD ProcessTask()=0;
		virtual DWORD Run();
		

		//return value:
		//==0 the event has been set,
		//==ETIMEDOUT
		//==otherwise, error value
		DWORD         WaitFinished(DWORD dwMilliseconds=INFINITE);
		bool          IsSetParas() { return m_bSetParas; }
		DWORD         GetTaskResult() { return m_dwResult; }
		virtual void  Reset()=0;
		void          SetMemoryCache(CXMemoryCacheManager* pCache) { m_lpCacheObj = pCache; }
		CXMemoryCacheManager* GetMemoryCache() { return m_lpCacheObj; }
		bool          IsRunning() { return m_bRunning; }
		CX_RUNING_TASK_TYPE GetTaskType() { return m_taskType; }

	protected:
		bool          m_bRunning;
		bool          m_bSetParas;
		DWORD         m_dwResult;
		CXEvent       m_event;
		CX_RUNING_TASK_TYPE   m_taskType;
		CXMemoryCacheManager *m_lpCacheObj;
	};
}
#endif // __CXTASK_BASE_H__

