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
* @file cmdVersion.c
*
* @brief Includes software version information for the command shell part of
* the Prestera software suite.
*
* @version   3
********************************************************************************
*/

#include <cmdShell/common/cmdVersion.h>
#include <cmdShell/common/cmdCommon.h>

#define CMD_SHELL_VERSION "v1.2a3"

#ifdef CMD_SHELL_DEBUG
#define DBG_INFO(x) cmdOsPrintf x
#else
#define DBG_INFO(x)
#endif


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
)
{
    if(version == NULL)
        return GT_BAD_PARAM;

    if(cmdOsStrlen(CMD_SHELL_VERSION) > CMD_SHELL_VERSION_MAX_LEN)
    {
        DBG_INFO(("CMD_SHELL_VERSION exceeded max len\n"));
        return GT_FAIL;
    }

    cmdOsMemCpy(version->version,CMD_SHELL_VERSION,cmdOsStrlen(CMD_SHELL_VERSION)+ 1);
    return GT_OK;
}


