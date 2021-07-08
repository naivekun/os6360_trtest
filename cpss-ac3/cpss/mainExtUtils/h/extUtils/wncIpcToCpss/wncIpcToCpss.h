#ifndef __WNC_IPC_TO_CPSS_H
#define __WNC_IPC_TO_CPSS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
    
/* For task */
/*
#include <gtOs/gtOsSharedUtil.h>
*/
#include <gtOs/gtOsSharedIPC.h>
#include <gtOs/gtOsSharedData.h>
#include <gtOs/gtOsSharedMalloc.h>
#include <gtOs/gtOsInit.h>
    
/* JSON parser */
#include <extUtils/wncIpcToCpss/cJSON.h>
#include <extUtils/wncIpcToCpss/debug.h>

#define LSOCKET_TO_CPSS "/tmp/lsocket_to_cpss"
#define LISTEN_BACKLOG  5
#define STR_BUF         1024
#define N_MAX_PARAM 30
/*#define MAX_STRUCT_BUFSZ 4096*/ /* one page of Linux OS */
#define MAX_STRUCT_BUFSZ 5120

#define handle_error(msg) do { \
    perror(msg); \
    if (0 != wncIpcToCpssSingSem) { \
        cpssOsSigSemDelete(wncIpcToCpssSingSem); wncIpcToCpssSingSem = 0; \
    } \
    return GT_ERROR; \
} while (0)

typedef GT_STATUS (*CPSS_API_FUNC_PTR)(GT_UINTPTR, ...);

extern GT_VOIDFUNCPTR osShellGetFunctionByName(const char* funcName);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __WNC_IPC_TO_CPSS_H */
