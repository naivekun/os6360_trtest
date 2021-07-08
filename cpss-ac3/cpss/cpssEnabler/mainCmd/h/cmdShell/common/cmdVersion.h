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
* @file cmdVersion.h
*
* @brief Includes software version information for the command shell part of
* the Prestera software suite.
*
* @version   6
********************************************************************************
*/

#ifndef __cmdVersionh
#define __cmdVersionh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cmdShell/os/cmdOs.h>

#define CMD_SHELL_VERSION_MAX_LEN 30
/**
* @struct GT_CMD_SHELL_VERSION
 *
 * @brief This struct holds the package version.
*/
typedef struct{

    GT_U8 version[CMD_SHELL_VERSION_MAX_LEN];

} GT_CMD_SHELL_VERSION;


/**
* @internal gtCmdShellVersion function
* @endinternal
*
* @brief   This function returns the version of the Command shell part of the
*         Prestera SW suite.
*
* @param[out] version                  - Command shell software version.
*                                       GT_OK on success,
*                                       GT_BAD_PARAM on bad parameters,
*                                       GT_FAIL otherwise.
*/
GT_STATUS gtCmdShellVersion
(
    OUT GT_CMD_SHELL_VERSION   *version
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cmdVersionh */




