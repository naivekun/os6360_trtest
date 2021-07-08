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
* @file gtHsuDrv.h
*
* @brief Includes HSU function for HSU functionality
*
* @version   3
********************************************************************************
*/
#ifndef __gtHsuDrvh
#define __gtHsuDrvh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal extDrvHsuMemBaseAddrGet function
* @endinternal
*
* @brief   Get start address of HSU area
*
* @param[out] hsuPtrAddr               - Pointer address to beginning of HSU area
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvHsuMemBaseAddrGet
(
    OUT  GT_U32 **hsuPtrAddr
);

/**
* @internal extDrvHsuWarmRestart function
* @endinternal
*
* @brief   Performs warm restart of the 8548 cpu
*/

GT_VOID extDrvHsuWarmRestart
(
    GT_VOID
);

/**
* @internal extDrvHsuInboundSdmaEnable function
* @endinternal
*
* @brief   This routine enables cpu inbound sdma access .
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvHsuInboundSdmaEnable
(
    GT_VOID
);

/**
* @internal extDrvHsuInboundSdmaDisable function
* @endinternal
*
* @brief   This routine disables cpu inbound sdma access .
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvHsuInboundSdmaDisable
(
    GT_VOID
);

/**
* @internal extDrvHsuInboundSdmaStateGet function
* @endinternal
*
* @brief   This routine gets the state of cpu inbound sdma access.
*
* @param[out] enablePtr                - (pointer to) inbound sdma status .
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvHsuInboundSdmaStateGet
(
    OUT GT_BOOL *enablePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __gtHsuDRv */



