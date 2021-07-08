/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file cmdEngine.h
*
* @brief commander database engine api header
*
*
* @version   9
********************************************************************************
*/

#ifndef __cmdEngine_h__
#define __cmdEngine_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***** Include files ***************************************************/

#include <cmdShell/shell/cmdSystem.h>

/**
* @internal cmdEventFields function
* @endinternal
*
* @brief   parses input buffer, and tells whether command has fields as input
*
* @param[in] inBuffer                 - null terminated string holding command buffer
*
* @retval GT_TRUE                  - fields need to be read
* @retval GT_FALSE                 - command has no fields as input
*/
GT_BOOL cmdEventFields
(
    IN GT_CHAR *inBuffer
);

/**
* @internal cmdEventRun function
* @endinternal
*
* @brief   command interpreter; parses and executes single command stored
*         in null terminated string.
* @param[in] inBuffer                 - null terminated string holding command buffer
* @param[in] inFieldBuffer            - null terminated string holding field values
*
* @param[out] outBuffer                - pointer to null terminated string holding output
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
CMD_STATUS cmdEventRun
(
    IN  GT_CHAR *inBuffer,
    IN  GT_CHAR *inFieldBuffer,
    OUT GT_CHAR **outBuffer
);
/**
* @internal cmdEngineInit function
* @endinternal
*
* @brief   initializes engine. Spawn new task for command execution.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fialure
*/
GT_STATUS cmdEngineInit(GT_VOID);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cmdEngine_H__ */



