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
* @file cmdOsData.c
*
* @brief This file contains common functions for mainCmd I/O streams
*
* @version   8
********************************************************************************
*/
#include <cmdShell/os/cmdOs.h>

/* a pointer to config file name
 * Default is "config.txt"
 */
GT_CHAR *cmdOsConfigFileName = "config.txt";
/* a pointer to directory name with trailing slash added
 * usually this is a current(startup) directory 
 * Default is NULL
 */
GT_CHAR *cmdOsConfigFilePath = NULL;

/* default command shell
 * Default is CMD_STARTUP_SHELL_CMDSHELL_E
 */
CMD_STARTUP_SHELL_TYPE_ENT cmdDefaultShellType = CMD_STARTUP_SHELL_CMDSHELL_E;


