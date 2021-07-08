/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

#ifndef __cmdOsh
#define __cmdOsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef CMDOS_NO_CMDCOMMON
#include <cmdShell/common/cmdCommon.h>
#endif

/* a pointer to config file name
 * Default is "config.txt"
 */
extern GT_CHAR *cmdOsConfigFileName;
/* a pointer to directory name with trailing slash added
 * usually this is a current(startup) directory 
 * Default is NULL
 */
extern GT_CHAR *cmdOsConfigFilePath;

/**
* @enum CMD_STARTUP_SHELL_TYPE_ENT
 *
 * @brief enum that describes default mainCmd shell
*/
typedef enum{

    /** @brief The default shell is cmdShell
     *  (GalTis shell)
     */
    CMD_STARTUP_SHELL_CMDSHELL_E,

    /** The default shell is luaCLI */
    CMD_STARTUP_SHELL_LUACLI_E

} CMD_STARTUP_SHELL_TYPE_ENT;
/* default command shell
 * Default is CMD_STARTUP_SHELL_CMDSHELL_E
 */
extern CMD_STARTUP_SHELL_TYPE_ENT cmdDefaultShellType;

/**
* @internal cmdOsGetServerAndLocalIP function
* @endinternal
*
* @brief   Returns TFTP server IP address and local IP
*
* @param[out] local_ip
* @param[out] server_ip
*/
GT_STATUS cmdOsGetServerAndLocalIP(
  OUT GT_CHAR *local_ip,
  OUT GT_CHAR *server_ip
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __cmdOsh */
/* Do Not Add Anything Below This Line */



