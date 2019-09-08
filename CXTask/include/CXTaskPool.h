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
#ifndef __CXTASK_POOL_H__
#define __CXTASK_POOL_H__

#include "PlatformDataTypeDefine.h"
#include "CXTaskBase.h"
#include "CXThreadPool.h"
#include "CXMemoryCacheManager.h"
#include <queue>
#include <map>
#include <list>
#include "CXSpinLock.h"
#include "CXQueue.h"
#include "CXEvent.h"
using namespace std;
namespace CXCommunication
{
	class CXTaskPool
	{
	public:
		
	public:
		CXTaskPool();
		virtual ~CXTaskPool();
		//register a class of task to the pool
		void          RegisterTaskClass(CXTaskBase *pTask);
        int           Create(int iPoolSize,int iInitedTaskQueueSize,CXMemoryCacheManager *pCacheObj,
                             bool bAllocateThreadCache=false,DWORD dwThreadCacheSize=0);
        bool          ResizePool(int iPoolSize);
        void          Destroy();

        //push a new task to the waitting queue, wait to process
		virtual void  PushTask(CXTaskBase *pTask);

        //stop the task if the task is running.
		virtual void  StopTask(CXTaskBase *pTask);

        CXTaskBase*   GetFreeTaskObject(CXTaskBase::CX_RUNING_TASK_TYPE type);
		void          FreeTaskObject(CXTaskBase *pTask);

		static  DWORD Run(void *pParas);
		CXMemoryCacheManager* GetMemoryCache() { return m_lpCacheObj; }

		long          GetRunningTasksNumber() { return m_lRunningTaskNum; }

    protected:
        virtual DWORD ProcessTasks();

	protected:
		bool                        m_bRunning;
        CXSpinLock                  m_lock;
        DWORD                       m_dwPoolSize;

		map<int, CXTaskBase*>       m_mapRegisteredTaskClass;

		//the pool size after reduce
		DWORD                       m_dwReducedPoolSize;
		bool                        m_bReducingPoolSize;

        DWORD                       m_dwInitedQueueSize;
        CXEvent                     m_event; 
		CXEvent                     m_eventResizeFinished;

		list<CXThread*>             m_lstThreadPool;
        CXThreadPool                m_threadTasks;
		CXMemoryCacheManager *      m_lpCacheObj;

        //the tasks needed to process
        CXQueue                     m_queueOutstandingTask;
        //queue<CXTaskBase*>          m_queueOutstandingTask;

        //key: the type of the task , is a value in the CX_RUNING_TASK_TYPE enumration
        //value: the free list of the task ,the task type is the key.
        map<int, list<CXTaskBase*>> m_mapFreeTaskObj;

		//the number of the tasks that is running
		long                        m_lRunningTaskNum;
		
	};
}
#endif // __CXTASK_POOL_H__

