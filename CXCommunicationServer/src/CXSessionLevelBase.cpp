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
#include "CXSessionLevelBase.h"
#include "CXPacketCodeDefine.h"
#include "CXSessionPacketStructure.h"
#include "PlatformFunctionDefine.h"
using namespace CXCommunication;
CXSessionLevelBase::CXSessionLevelBase()
{
    m_bIsUniqueInstance = true;
    GetObjectGuid();
}


CXSessionLevelBase::~CXSessionLevelBase()
{
}

//return value:==-2 need to close the connection
int CXSessionLevelBase::ProcessMessage(PCXMessageData pMes)
{
    DWORD dwMessageCode = pMes->bodyData.dwMesCode;
    int iReceiveDataLen = pMes->dwDataLen;
    CXConnectionObject *pCon = (CXConnectionObject*)pMes->pConObj;
    if (pCon == NULL)
    {
        return INVALID_PARAMETER;
    }

    CXConnectionSession *pSession = (CXConnectionSession*)pCon->GetSession();

    int iRet = RETURN_SUCCEED;
    switch (dwMessageCode)
    {
    case CX_SESSION_LOGIN_CODE:
        iRet = SessionLogin(pMes);
        break;
    case CX_SESSION_LOGOUT_CODE:
        iRet = SessionLogout(pMes, *pSession);
        if (iRet != RETURN_SUCCEED)
        {
            printf("SessionLogout fails\n");
        }
        break;
    case CX_SESSION_SETITING_CODE:

        iRet = SessionSetting(pMes, *pSession);
        if (iRet != RETURN_SUCCEED)
        {
            printf("SessionSetting fails\n");
        }
        break;
    default:
        {
            break;
        }
    }

    return iRet;
}


CXRPCObjectServer* CXSessionLevelBase::CreateObject()
{
    return (CXRPCObjectServer*)new CXSessionLevelBase;
}

int CXSessionLevelBase::SendData(CXConnectionObject * pCon, const byte *pbyData, DWORD dwDataLen)
{
    return RETURN_SUCCEED;
}

void CXSessionLevelBase::Destroy()
{
}

int CXSessionLevelBase::SessionLogin(PCXMessageData pMes)
{
    if (pMes == NULL)
    {
        return INVALID_PARAMETER;
    }

    CXConnectionObject *pCon = (CXConnectionObject*)pMes->pConObj;

    DWORD dwSendMesLen = sizeof(CXCommonMessageReply);
    byte  bySendBuf[1024] = { 0 };
    PCXCommonMessageReply pReply = (PCXCommonMessageReply)bySendBuf;
    int i = 1;
    int iRet = RETURN_SUCCEED;
    while (i-- > 0)
    {
        PCXSessionLogin pPacket = (PCXSessionLogin)pMes->bodyData.buf;
        if (pPacket->dwUserDataLen <= 0 || pPacket->dwUserDataLen >= (pMes->dwDataLen))
        {
            iRet = -3;
            pReply->dwReplyCode = 201;
            break;
        }

        CXConnectionSession * pSession = (CXConnectionSession *)pCon->GetSession();
        CXSessionsManager * pManager = (CXSessionsManager *)pCon->GetSessionsManager();
        if (pManager == NULL)
        {
            iRet = -4;
            pReply->dwReplyCode = 202;
            break;
        }
        if (pSession == NULL)
        {
            if (pPacket->byConnectionType == 1) //main message connections
            {
                string strUserName = "";
                string strPassword = "";
                pPacket->szUserData[pPacket->dwUserDataLen] = '\0';
                char* pPos = strchr(pPacket->szUserData, '\r');

                if (pPos == NULL || (pPos - pPacket->szUserData) > (pPacket->dwUserDataLen - 1))
                {
                    iRet = -5;
                    pReply->dwReplyCode = 203;
                    break;
                }

                *pPos = '\0';
                strUserName = pPacket->szUserData;
                strPassword = pPos + 1;

                string strGuid = pManager->BuildSessionGuid();

                //failed to allocate the session from the session manager
                pSession = pManager->GetFreeSession();
                if (pSession == NULL)
                {
                    iRet = -7;
                    pReply->dwReplyCode = 207;
                    break;
                }

                pSession->AddMainConnection(*pCon);
                pSession->SetSesssionGuid(strGuid);
                pSession->ResetVerificationInfo();
                pManager->AddUsingSession(pSession);

                //*ppSession = pSession;

                pCon->SetSession(pSession);

                string strCode = pSession->GetVerificationCode();
                strcpy(pReply->szData, strGuid.c_str());
                strcat(pReply->szData, "\r");
                strcat(pReply->szData, strCode.c_str());
                pReply->dwDataLen = strlen(pReply->szData) + 1;
                dwSendMesLen += pReply->dwDataLen;

                char  szInfo[1024] = { 0 };
                sprintf_s(szInfo, 1024, "Receive a main connection, guid is %s,connection index is %lld,session is %x\n",
                    strGuid.c_str(), pCon->GetConnectionIndex(), pSession);
                pCon->GetLogHandle()->Log(CXLog::CXLOG_INFO, szInfo);
            }
            else
            {
                pPacket->szUserData[pPacket->dwUserDataLen] = '\0';

                string strSessionGuid = "";
                string strVerifyCode = "";
                pPacket->szUserData[pPacket->dwUserDataLen] = '\0';
                char* pPos = strchr(pPacket->szUserData, '\r');

                if (pPos == NULL || (pPos - pPacket->szUserData) > (pPacket->dwUserDataLen - 1))
                {
                    iRet = -5;
                    pReply->dwReplyCode = 203;
                    break;
                }

                *pPos = '\0';
                strSessionGuid = pPacket->szUserData;
                strVerifyCode = pPos + 1;

                pSession = pManager->FindUsingSession(strSessionGuid);
                if (pSession == NULL)
                {
                    iRet = -2;
                    pReply->dwReplyCode = 204;
                    break;
                }

                int iVerifyRet = pSession->VerifyCode(strVerifyCode);
                if (iVerifyRet != 0)
                {
                    iRet = -6;
                    if (iVerifyRet == -1)
                        pReply->dwReplyCode = 205;
                    else
                        pReply->dwReplyCode = 206;
                    break;
                }
                pCon->SetSession(pSession);

                char  szInfo[1024] = { 0 };
                sprintf_s(szInfo, 1024, "Receive a data connection, guid is %s,connection index is %lld,session is %x\n",
                    strSessionGuid.c_str(), pCon->GetConnectionIndex(),pSession);
                pCon->GetLogHandle()->Log(CXLog::CXLOG_INFO, szInfo);

                //==1 major message connection
                //==2 minor messsage connection
                //==3 data connection
                //==4 object connection
                switch (pPacket->byConnectionType)
                {
                case 2:
                    pSession->AddMessageConnection(*pCon);
                    break;
                case 3:
                    pSession->AddDataConnection(*pCon);
                    break;
                case 4:
                default:
                    break;
                }
            }
        }
        else
        {
            iRet = -4;
            pReply->dwReplyCode = 205;
            break;
        }
        pReply->dwReplyCode = 200;

        break;
    }

    bool bRet = pCon->SendPacket(bySendBuf, dwSendMesLen, CX_SESSION_LOGIN_REPLY_CODE);
    if (!bRet || iRet == -2) //need to close connection
    {
        return -2;
    }
    return iRet;
}

int CXSessionLevelBase::SessionLogout(PCXMessageData pMes, CXConnectionSession &session)
{
    return RETURN_SUCCEED;
}

int CXSessionLevelBase::SessionSetting(PCXMessageData pMes, CXConnectionSession &session)
{
    return RETURN_SUCCEED;
}

void CXSessionLevelBase::RecordSlowOps(PCXMessageData pMes)
{

}



