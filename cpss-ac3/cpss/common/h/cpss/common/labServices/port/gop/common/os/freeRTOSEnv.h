#ifndef __freeRTOSEnv_h__
#define __freeRTOSEnv_h__

/* implementations are in inFreeRTOS */

void tfp_sprintf
(
    char * s,
    char *fmt,
    ...
);

GT_U32 osStrlen
(
    IN const void * source
);

GT_VOID * osMemCpy
(
    IN GT_VOID *       destination,
    IN const GT_VOID * source,
    IN GT_U32       size
);

#define hwsStrLen  osStrlen
#define hwsSprintf tfp_sprintf
#define hwsMemCpy  osMemCpy

#endif /* __freeRTOSEnv_h__ */
