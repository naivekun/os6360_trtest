/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file tgfCscdEPortMode.h
*
* @brief CPSS Cascade remote port remapping
*
* @version   1
********************************************************************************
*/
#ifndef __tgfCscdEPortMode
#define __tgfCscdEPortMode

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal tgfCscdEPortModeConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         configure Port 0 as DSA port,
*         set <Number Of DSA Source Port Bits>  = 5;
*         <Number Of DSA Source Device Bits> = 0;
*         <Physical Port Base> = 10;
*/
GT_VOID tgfCscdEPortModeConfigurationSet
(
    GT_VOID
);

/**
* @internal tgfCscdEPortModeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = 00:00:00:00:00:56,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         fdb learning return match on port 13
*/
GT_VOID tgfCscdEPortModeTrafficGenerate
(
    GT_VOID
);

/**
* @internal tgfCscdEPortModeConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID tgfCscdEPortModeConfigurationRestore
(
    GT_VOID
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfCscdEPortMode */


