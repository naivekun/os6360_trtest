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
* @file cmdShell.h
*
* @brief This file contains OS shell function
*
* @version   1
********************************************************************************
*/

#ifndef __cmdShellh
#define __cmdShellh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cmdShell/common/cmdCommon.h>
#include <extUtils/IOStream/IOStream.h>


/**
* @internal cmdOsShell function
* @endinternal
*
* @brief   Start OS shell
*         It is assumed that stdout is redirected to current stream
*/
GT_STATUS cmdOsShell(IOStreamPTR IOStream);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ifndef __cmdShellh */


