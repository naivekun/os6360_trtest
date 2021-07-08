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
* @file prvTgfNetIfFromCpuToVidxExcludedPort.h
*
* @brief Send a FROM_CPU packet to VIDX with excluded port and ensure the
* port doesn't receive the packet
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfNetIfFromCpuToVidxExcludedPorth
#define __prvTgfNetIfFromCpuToVidxExcludedPorth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfNetIfFromCpuToVidxExcludedPort function
* @endinternal
*
* @brief   Test to check FROM_CPU eDSA "excluded port" feature.
*         Send a FROM_CPU packet to VIDX with excluded port and ensure the
*         port doesn't receive the packet
*/
GT_VOID prvTgfNetIfFromCpuToVidxExcludedPort
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfNetIfFromCpuToVidxExcludedPorth */

