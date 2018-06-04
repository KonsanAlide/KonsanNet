#include "CXSessionMessageProcess.h"
#include "CXPacketCodeDefine.h"
#include "CXSessionPacketStructure.h"
#include "CXPacketCodeDefine.h"
#include <string>
#ifndef WIN32
#include<string.h>
#endif
using namespace CXCommunication;
using namespace std;

CXSessionMessageProcess::CXSessionMessageProcess()
{
}


CXSessionMessageProcess::~CXSessionMessageProcess()
{
}

int CXSessionMessageProcess::SessionLogin(PCXMessageData pMes, CXConnectionSession ** ppSession)
{
    if (pMes == NULL)
    {
        return INVALID_PARAMETER;
    }

    CXConnectionObject *pCon = (CXConnectionObject*)pMes->pConObj;

    DWORD dwSendMesLen = sizeof(CXCommonMessageReply);
    byte  bySendBuf[1024] = {0};
    PCXCommonMessageReply pReply = (PCXCommonMessageReply)bySendBuf;
    int i = 1;
    int iRet = RETURN_SUCCEED;
    while (i-->0)
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

                if (pPos == NULL || (pPos - pPacket->szUserData)>(pPacket->dwUserDataLen - 1))
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

                *ppSession = pSession;

                pCon->SetSession(pSession);

                string strCode = pSession->GetVerificationCode();
                strcpy(pReply->szData, strGuid.c_str());
                strcat(pReply->szData, "\r");
                strcat(pReply->szData, strCode.c_str());
                pReply->dwDataLen  =strlen(pReply->szData) + 1;
                dwSendMesLen += pReply->dwDataLen;
            }
            else
            {
                pPacket->szUserData[pPacket->dwUserDataLen] = '\0';

                string strSessionGuid = "";
                string strVerifyCode = "";
                pPacket->szUserData[pPacket->dwUserDataLen] = '\0';
                char* pPos = strchr(pPacket->szUserData, '\r');

                if (pPos == NULL || (pPos - pPacket->szUserData)>(pPacket->dwUserDataLen - 1))
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
                if (iVerifyRet !=0)
                {
                    iRet = -6;
                    if(iVerifyRet==-1)
                        pReply->dwReplyCode = 205;
                    else
                        pReply->dwReplyCode = 206;
                    break;
                }
                pCon->SetSession(pSession);

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
    if (!bRet || iRet==-2) //need to close connection
    {
        return -2;
    }
    return iRet;
}

int CXSessionMessageProcess::SessionLogout(PCXMessageData pMes, CXConnectionSession &session)
{
    return RETURN_SUCCEED;
}

int CXSessionMessageProcess::SessionSetting(PCXMessageData pMes, CXConnectionSession &session)
{
    return RETURN_SUCCEED;
}