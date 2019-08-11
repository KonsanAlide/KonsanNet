#ifndef __CXPACKETCODEDEFINE_H__
#define __CXPACKETCODEDEFINE_H__
#include <map>
#include <string>
using namespace std;

#define CX_SESSION_LOGIN_CODE          1
#define CX_SESSION_LOGIN_REPLY_CODE    2

#define CX_SESSION_LOGOUT_CODE         3
#define CX_SESSION_LOGOUT_REPLY_CODE   4

#define CX_SESSION_SETITING_CODE       5
#define CX_SESSION_SETITING_REPLY_CODE 6

#define CX_HEAERT_BEAT_CODE            7
#define CX_HEAERT_BEAT_REPLY_CODE      8

#define CX_SET_RECV_BUF_SIZE_CODE            9
#define CX_SET_RECV_BUF_SIZE_REPLY_CODE      10

#define CX_REPAIR_DPT_CODE             11
#define CX_REPAIR_DPT_REPLY_CODE       12

#define CX_FILE_OPEN_CODE              2001
#define CX_FILE_OPEN_REPLY_CODE        2002

#define CX_FILE_SEEK_CODE              2003
#define CX_FILE_SEEK_REPLY_CODE        2004

#define CX_FILE_READ_CODE              2005
#define CX_FILE_READ_REPLY_CODE        2006

#define CX_FILE_WRITE_CODE             2007
#define CX_FILE_WRITE_REPLY_CODE       2008

#define CX_FILE_CLOSE_CODE             2009
#define CX_FILE_CLOSE_REPLY_CODE       2010

#define CX_FILE_GET_LENGTH_CODE        2011
#define CX_FILE_GET_LENGTH_REPLY_CODE  2012

#define CX_FILE_GET_CUR_POS_CODE       2013
#define CX_FILE_GET_CUR_POS_REPLY_CODE 2014

#define CX_FILE_WRITE_MANY_CODE        2015
#define CX_FILE_WRITE_MANY_REPLY_CODE  2016

// the map for the RPC object name and the its GUID
const map<string, string> g_mapRPCObjectGuid = {
	{"CXHeartBeatV1",        "{FE66615A-E22B-4A3D-936D-61EEDEDA2080}"},
	{"CXConnectionLoginV1",  "{6E34084E-970B-427F-A973-2763A19D7C07}"},
	{"CXRPCObjectV1",        "{2A3E8648-A997-4457-947C-23ABA77D815D}"},
	{"CXFileTcpV1",          "{A888441E-F757-4775-B69A-CCEBD63156FB}"},
	{"CXMoveDiskV1",         "{06C98D20-35A4-4450-B485-E3BC6FF8A41E}"}
};
#endif //__CXPACKETCODEDEFINE_H__