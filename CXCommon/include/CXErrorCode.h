#ifndef __CXERROR_CODE_H__
#define __CXERROR_CODE_H__

// the result of the process
typedef enum CX_RESULT
{
    CXRET_SUCCEED = 0,
    CXRET_INVALID_PARAMETER           = -1,
    CXRET_CREATE_SOCKET_FAILED        = -2,
    CXRET_BIND_ADDR_FAILED            = -3,
    CXRET_CONNECT_REMOTE_FAILED       = -4,
    CXRET_SEND_DATA_FAILED            = -5,
    CXRET_PARSE_DATA_FAILED           = -6,
    CXRET_PREPARE_DATA_FAILED         = -7,
    CXRET_ALLOCATE_MEMORY_FAILED      = -8,
    CXRET_CONNECTION_LOGIN_FAILED     = -9,
    CXRET_BUILD_CONNECTION_FAILED     = -10,
    CXRET_ASYNC_CALLBACK              = -11,
    CXRET_ALLOCATE_TASK_FAILED        = -12,
    CXRET_NOT_FOUND_REQUESTED_CON     = -13, //not found the requested connection
    CXRET_NOT_FOUND_REQUESTED_OBJ     = -14, //not found the requested object
	CXRET_NOTIFY_REMOTE_FAILED        = -15, //failed to notify the remote object
    CXRET_MESSAGE_INCOMPLETE          = -16, //the received message is incomplete
}CXResultCode;

#endif //__CXERROR_CODE_H__
