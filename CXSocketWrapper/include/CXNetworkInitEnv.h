#ifndef __CXNETWORKINITENV_H__
#define __CXNETWORKINITENV_H__

#include "SocketDefine.h"
namespace CXCommunication
{
    class CXNetworkInitEnv
    {
    public:
        CXNetworkInitEnv();
        ~CXNetworkInitEnv();
        int InitEnv();
        int ClearEnv();
    };
}
#endif

