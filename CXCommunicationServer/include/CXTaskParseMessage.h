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
#ifndef __CXTASK_PARSE_MESSAGE_H__
#define __CXTASK_PARSE_MESSAGE_H__
#include "CXConnectionObject.h"
#include "CXTaskBase.h"

namespace CXCommunication
{
	class CXTaskParseMessage :public CXTaskBase
	{
	public:
		CXTaskParseMessage();
		virtual ~CXTaskParseMessage();
		virtual CXTaskBase* CreateObject();
		virtual DWORD ProcessTask();

		void          SetTaskInfo(PCXMessageData  pMessageData, CXPacketHeader &packetHeader);
		virtual void  Reset();
	private:
		PCXMessageData      m_pMessageData;
		CXPacketHeader      m_packetHeader;
	};
}
#endif // __CXTASK_PARSE_MESSAGE_H__

