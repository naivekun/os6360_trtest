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
* @file initDbUserExit.c
*
* @brief The "cmdUserInitDatabase()" function .
* Overwrites the same named empty function from
* from [mainCmd] cmdShell/shell/ directory at link build step.
*
* @version   7
********************************************************************************
*/

/***** Include files ***************************************************/

#if (defined CPSS_PRODUCT) || (defined PSS_PRODUCT)
#include <prestera/tapi/sysConfig/gtSysConfig.h>
#include <prestera/tapi/logicalIf/gtLif.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#endif 

#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>
#include <cmdShell/shell/userInitCmdDb.h>


/*******************************************************************************
 * External usage variables
 ******************************************************************************/
#if (defined CPSS_PRODUCT) || (defined PSS_PRODUCT)
extern GT_INTFUNCPTR gtInitSystemFuncPtr;
extern GT_DEVICE internalBase;
#endif

#ifdef IMAGE_HELP_SHELL
extern void imageHelpShellInit(void);
#endif
/**
* @internal cmdUserInitDatabase function
* @endinternal
*
* @brief   The "cmdUserInitDatabase()" function placeholder.
*         Called by "cmdInit" and "cmdInitDebug" functions.
*         Enables some command shell features .
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cmdUserInitDatabase
(
    GT_VOID
)
{
#if (defined CPSS_PRODUCT) || (defined PSS_PRODUCT)
    /* Make it TWIST - D */
    internalBase = GT_98EX120D;/* was GT_98MX610BS --- 0x006011AB */
    gtInitSystemFuncPtr = (GT_INTFUNCPTR)gtInitSystem;
#endif 

#ifdef IMAGE_HELP_SHELL
    imageHelpShellInit();
#endif
    return GT_OK ;
}


