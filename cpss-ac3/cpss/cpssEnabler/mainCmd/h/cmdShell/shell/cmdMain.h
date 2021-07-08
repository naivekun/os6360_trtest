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
* @file cmdMain.h
*
* @brief command shell entry point implementation
*
* @version   2
********************************************************************************
*/
#ifndef __cmdMainh
#define __cmdMainh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***** Include files ***************************************************/

#include <cmdShell/common/cmdCommon.h>
#if (defined PSS_PRODUCT) || (defined CPSS_PRODUCT)
    #include <cmdShell/shell/cmdMultiTapi.h>
#else
    #include <cpss/common/cpssTypes.h>
#endif

/**
* @internal cmdInit function
* @endinternal
*
* @brief   commander initialization (entry point); this function spawns
*         configured streaming threads and immediately returns to caller.
* @param[in] devNum                   - The device number of the serial port.
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - failed to allocate memory for socket
* @retval GT_FAIL                  - on error
*/
GT_STATUS cmdInit
(
    IN  GT_U32  devNum
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cmdMainh */


