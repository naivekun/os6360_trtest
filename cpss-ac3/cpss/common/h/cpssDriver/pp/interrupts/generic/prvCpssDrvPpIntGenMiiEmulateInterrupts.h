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
* @file prvCpssDrvPpIntGenMiiEmulateInterrupts.h
*
* @brief Includes general definitions for the mii interrupts emulation handling
*
* @version   3
********************************************************************************
*/
#ifndef __prvCpssDrvPpIntGenMiiEmulateInterruptsh
#define __prvCpssDrvPpIntGenMiiEmulateInterruptsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>

/**
* @internal interruptMiiRxSR function
* @endinternal
*
*/
GT_STATUS interruptMiiRxSR
(
    IN GT_U8_PTR        segmentList[],
    IN GT_U32           segmentLen[],
    IN GT_U32           numOfSegments,
    IN GT_U32           rxQueue
);

/**
* @internal rawSocketRx function
* @endinternal
*
*/

GT_STATUS rawSocketRx
(
    IN GT_U8_PTR        packet,
    IN GT_U32           packetLen,
    IN GT_U32           rxQueue
);

/**
* @internal interruptMiiTxEndSR function
* @endinternal
*
*/
GT_STATUS interruptMiiTxEndSR
(
    IN GT_U8              * segmentList[],
    IN GT_U32               numOfSegment
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDrvPpIntGenMiiEmulateInterruptsh */



