#include "CXSessionMessageProcess.h"
#include "CXPacketCodeDefine.h"
#include "CXSessionPacketStructure.h"
#include "CXPacketCodeDefine.h"
#include <string>
using namespace CXCommunication;
using namespace std;

CXSessionMessageProcess::CXSessionMessageProcess()
{
}


CXSessionMessageProcess::~CXSessionMessageProcess()
{
}

int CXSessionMessageProcess::SessionLogin(PCXBufferObj pBuf, CXConnectionSession ** ppSession)
{
    if (pBuf == NULL)
    {
        return INVALID_PARAMETER;
    }

    CXConnectionObject *pCon = (CXConnectionObject*)pBuf->pConObj;
    PCXBufferObj pSendBuf = pCon->GetBuffer();
    PCXCommonMessageReply pReply = (PCXCommonMessageReply)(pSendBuf->wsaBuf.buf + sizeof(CXPacketData) - 1);
    int i = 1;
    int iRet = RETURN_SUCCEED;
    while (i-->0)
    {
        PCXSessionLogin pPacket = (PCXSessionLogin)(pBuf->wsaBuf.buf + sizeof(CXPacketData) - 1);
        if (pPacket->dwUserDataLen <= 0 || pPacket->dwUserDataLen >= (pBuf->wsaBuf.len))
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

                pSession = pManager->GetFreeSession();
                pSession->AddMainConnection(*pCon);
                string strGuid = pManager->BuildSessionGuid();
                pSession->SetSesssionGuid(strGuid);
                *ppSession = pSession;
                pCon->SetSession(pSession);
                strcpy(pReply->szData, strGuid.c_str());
                pReply->dwDataLen  =strGuid.length() + 1;
            }
            else
            {
                pPacket->szUserData[pPacket->dwUserDataLen] = '\0';
                string strSessionGuid = pPacket->szUserData;
                pSession = pManager->FindUsingSession(strSessionGuid);
                if (pSession == NULL)
                {
                    iRet = -2;
                    pReply->dwReplyCode = 204;
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
    
    int iTransRet = SendCommonMessageReply(pCon, CX_SESSION_LOGIN_REPLY_CODE, pSendBuf);
    pCon->FreeBuffer(pSendBuf);
    if (iTransRet == -2 || iRet==-2) //need to close connection
    {
        return -2;
    }
    return iRet;
}

int CXSessionMessageProcess::SessionLogout(PCXBufferObj pBuf, CXConnectionSession &session)
{
    return RETURN_SUCCEED;
}

int CXSessionMessageProcess::SessionSetting(PCXBufferObj pBuf, CXConnectionSession &session)
{
    return RETURN_SUCCEED;
}

int CXSessionMessageProcess::SendCommonMessageReply(CXConnectionObject * pCon,
    DWORD deMessageCode, PCXBufferObj pBuf)
{
    PCXCommonMessageReply pReply = (PCXCommonMessageReply)(pBuf->wsaBuf.buf + sizeof(CXPacketData) - 1);
    int iPacketBodyLen = sizeof(DWORD)+sizeof(CXCommonMessageReply) - 1 + pReply->dwDataLen;
    byte *pBodyData = (byte*)(pBuf->wsaBuf.buf + sizeof(CXPacketHeader));


    DWORD dwCheckSum = 0;
    for (int i = 0; i<iPacketBodyLen; i++)
    {
        dwCheckSum += (unsigned int)pBodyData[i];
    }

    //send the header
    PCXPacketHeader pTcpHeader = (PCXPacketHeader)pBuf->wsaBuf.buf;

    pTcpHeader->wDataLen = iPacketBodyLen;
    pTcpHeader->byReserve = 0x15;
    pTcpHeader->byType = 2;
    pTcpHeader->dwFlag = 0x25f7;
    pTcpHeader->dwCheckSum = dwCheckSum;
    pTcpHeader->dwPacketNum = 0;

    PCXPacketData pPacket = (PCXPacketData)(pBuf->wsaBuf.buf);
    pPacket->dwMesCode = deMessageCode;
    int iPacketLen = iPacketBodyLen + sizeof(CXPacketHeader);
    pBuf->wsaBuf.len = iPacketLen;

    DWORD dwSendLen = 0;
    int iRet = pCon->PostSendBlocking(pBuf, dwSendLen);
    if (iRet != 0)
    {
        return -2;
    }
    return RETURN_SUCCEED;
}

