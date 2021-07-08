/**
********************************************************************************
* @file osObjIdLib.h
*
* @brief object by id support micro lib
*
* @version   1
********************************************************************************
*/
/*******************************************************************************
* osObjIdLib.h
*
* DESCRIPTION:
*       object by id support micro lib
*
* DEPENDENCIES:
*       None
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef _osObjIdLib_h_
#define _osObjIdLib_h_

#include <limits.h>


#if defined  CPSS_USE_MUTEX_PROFILER 
typedef enum
{
    GRANULAR_LOCKING_TYPE_NONE,/*Mutex is not related to granular locking feature*/
    GRANULAR_LOCKING_TYPE_ZERO_LEVEL,/*Mutex is zero level*/
    GRANULAR_LOCKING_TYPE_DEVICELESS_CONFIGURATION,/*Mutex is non device related ,functionality is configuration*/
    GRANULAR_LOCKING_TYPE_DEVICELESS_RX_TX,/*Mutex is non device related ,functionality is network interface*/
    GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_CONFIGURATION,/*Mutex is  device related ,functionality is configuration*/
    GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_RX_TX/*Mutex is  device related ,functionality is network interface*/
}GT_GRANULAR_LOCKING_TYPE;
#endif


#define OS_OBJ_NAME_LEN_CNS 16
typedef struct osObjectHeaderSTC {
    int     type; /* 0 means unused */
 #if defined  CPSS_USE_MUTEX_PROFILER    
    GT_GRANULAR_LOCKING_TYPE     glType;/*mutex type for granular locking*/
    int     devNum;/*used to indicate device number if this is per device mutex*/
#endif    
    char    name[OS_OBJ_NAME_LEN_CNS];
} OS_OBJECT_HEADER_STC;

typedef void* (*OS_OBJ_ALLOC_FUNC_TYPE)(size_t);
typedef void (*OS_OBJ_FREE_FUNC_TYPE)(void*);
typedef struct {
    OS_OBJECT_HEADER_STC    **list;
    int                     allocated;
    int                     allocChunk;
    size_t                  objSize;
    OS_OBJ_ALLOC_FUNC_TYPE  allocFuncPtr;
    OS_OBJ_FREE_FUNC_TYPE   freeFuncPtr;
} OS_OBJECT_LIST_STC;



/**
* @internal osObjLibGetNewObject function
* @endinternal
*
* @brief   allocate new object
*
* @param[in] objListPtr               - pointer to object list structure
* @param[in] type                     -  assign to object
* @param[in] name                     - object name
*                                       obj Id
*                                       <= 0 on error
*
* @note list must be already protected
*
*/
int osObjLibGetNewObject
(
    OS_OBJECT_LIST_STC      *objListPtr,
    int                     type,
    const char              *name,
    OS_OBJECT_HEADER_STC*   *objPtr
);


#endif /* _osObjIdLib_h_ */

