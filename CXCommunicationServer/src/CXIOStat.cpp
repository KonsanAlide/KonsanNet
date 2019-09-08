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
#include "CXIOStat.h"
#include <sstream>
using namespace CXCommunication;

DWORD ThreadOutput(void* lpvoid);
CXIOStat::CXIOStat()
{
	m_pLogHandle = NULL;
	m_bRunning = false;
	m_strStatContent = "";
	m_iOutputIntervalMs = 60000;
}


CXIOStat::~CXIOStat()
{
	Stop();
}

bool CXIOStat::Start()
{
	m_bRunning = true;
	int iRet = m_thread.Start(ThreadOutput, this);
	if (iRet != 0)
	{
		m_bRunning = false;
		return false;
	}
	return true;
}
void CXIOStat::Stop()
{
	if (m_bRunning)
	{
		m_bRunning = false;
		m_eveWait.SetEvent();
		m_thread.Wait();
	}
}

void CXIOStat::BeginIOStat(string strFlag, string strIOGuid, int64 iTimeMS)
{
	if (!m_bRunning)
		return;
	m_spinLock.Lock();
	map<string, map<string, IOStatTimeNode>>::iterator it = m_mapStat.find(strFlag);
	if(it!= m_mapStat.end())
	{
		map<string, IOStatTimeNode >::iterator itMark = it->second.find(strIOGuid);
		if (itMark != it->second.end())
		{
			itMark->second.iUsedTime = iTimeMS;
			itMark->second.bEnd = false;
		}
		else
		{
			IOStatTimeNode node;
			node.iUsedTime = iTimeMS;
			node.bEnd = false;
			it->second[strIOGuid] = node;
		}
	}
	else
	{
		map<string, IOStatTimeNode> mapFlag;
		IOStatTimeNode node;
		node.iUsedTime = iTimeMS;
		node.bEnd = false;
		mapFlag[strIOGuid] = node;
		m_mapStat[strFlag] = mapFlag;
	}
	m_spinLock.Unlock();
}

void CXIOStat::EndIOStat(string strFlag, string strIOGuid, int64 iTimeMS)
{
	if (!m_bRunning)
		return;
	m_spinLock.Lock();
	map<string, map<string, IOStatTimeNode>>::iterator it = m_mapStat.find(strFlag);
	if (it != m_mapStat.end())
	{
		map<string, IOStatTimeNode >::iterator itMark = it->second.find(strIOGuid);
		if (itMark != it->second.end())
		{
			itMark->second.iUsedTime = iTimeMS- itMark->second.iUsedTime;
			itMark->second.bEnd = true;
		}
	}

	m_spinLock.Unlock();
}

//push a io stat info 
void CXIOStat::PushIOStat(string strFlag, string strIOGuid, int64 iUsedTimeMS)
{
	if (!m_bRunning)
		return;
	m_spinLock.Lock();
	map<string, map<string, IOStatTimeNode>>::iterator it = m_mapStat.find(strFlag);
	if (it != m_mapStat.end())
	{
		map<string, IOStatTimeNode >::iterator itMark = it->second.find(strIOGuid);
		if (itMark != it->second.end())
		{
			itMark->second.iUsedTime = iUsedTimeMS;
			itMark->second.bEnd = true;
		}
		else
		{
			IOStatTimeNode node;
			node.iUsedTime = iUsedTimeMS;
			node.bEnd = true;
			it->second[strIOGuid] = node;
		}
	}
	else
	{
		map<string, IOStatTimeNode> mapFlag;
		IOStatTimeNode node;
		node.iUsedTime = true;
		node.bEnd = true;
		mapFlag[strIOGuid] = node;
		m_mapStat[strFlag] = mapFlag;
	}
	m_spinLock.Unlock();
}

void CXIOStat::Output()
{
	uint64 iIndex = 0;
	while (m_bRunning)
	{
		m_eveWait.WaitForSingleObject(m_iOutputIntervalMs);
		if (!m_bRunning)
		{
			break;
		}

		
		m_spinLock.Lock();
		map<string, map<string, IOStatTimeNode>>::iterator it = m_mapStat.begin();
		for (;it != m_mapStat.end();++it)
		{
			string strFlagName = it->first;
			stringstream strContent;
			strContent<<"used_time_stat,mark:"<< strFlagName;
			int64 iNumber = 0;
			int64 iAvgTime = 0;
			int64 iTotalTime = 0;
			int64 iMinTime = 0;
			int64 iMaxTime = 0;
			
			map<string, IOStatTimeNode>oldMap;
			map<string, IOStatTimeNode >::iterator itMark = it->second.begin();
			for (; itMark != it->second.end(); ++itMark)
			{
				if (itMark->second.bEnd)
				{
					if (iMinTime > itMark->second.iUsedTime)
						iMinTime = itMark->second.iUsedTime;
					if (iMaxTime < itMark->second.iUsedTime)
						iMaxTime = itMark->second.iUsedTime;
					iNumber++;
					iTotalTime += itMark->second.iUsedTime;
				}
				else
				{
					oldMap[itMark->first] = itMark->second;
				}
			}
			if(iNumber>0)
				iAvgTime = iTotalTime / iNumber;
			strContent << ",count:" << iNumber;
			strContent << ",avg:" << iAvgTime;
			strContent << ",min:" << iMinTime;
			strContent << ",max:" << iMaxTime;
			it->second.clear();
			it->second = oldMap;

			m_pLogHandle->Log(CXLog::CXLOG_ERROR, strContent.str().c_str());
		}
		m_spinLock.Unlock();
	}
}


DWORD ThreadOutput(void* lpvoid)
{
	CXIOStat * pServer = (CXIOStat*)lpvoid;
	pServer->Output();
	return 0;
}


