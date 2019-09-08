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

Description£ºThe structure of the data packet used in the session comnunication.
             eg: user log in and log out,chanel setting.
*****************************************************************************/
#ifndef __CXSESSIONPACKETSTRUCTURE_H__
#define __CXSESSIONPACKETSTRUCTURE_H__

#include "PlatformDataTypeDefine.h"
#pragma pack(1)

#ifndef _CX_SESSION_LOGIN
typedef struct _CX_SESSION_LOGIN
{
    //==1 message session
    //==2 file transmission session
    //==3 data transmission session
    //==4 object RPC session
    DWORD dwSessionType;
	//==0 user name and password authentication
	//==1 key authentication
    byte  byUserType;
    //==1 major message connection
    //==2 minor messsage connection
    //==3 data connection
    //==4 object connection
    byte  byConnectionType;
    byte  byEncryptType;
    byte  byReserve;
    DWORD dwUserDataLen;
    //if byConnectionType==1: the szUserData is username + '\r'+userpassword
    //else the szUserData is sessionId + '\r'+verifyCode
    char  szUserData[1];
}CXSessionLogin, *PCXSessionLogin;
#endif

#ifndef _CX_SESSION_LOGOUT
typedef struct _CX_SESSION_LOGOUT
{
    DWORD dwSessionType;
    byte  byUserType;
    byte  byEncryptType;
    byte  byReserve[2];
    DWORD dwUserDataLen;
    //username
    char  szUserData[1];
}CXSessionLogout, *PCXSessionLogout;
#endif


#pragma pack()

#endif //__CXSESSIONPACKETSTRUCTURE_H__
