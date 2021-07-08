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
* @file prvTgfTunnelTermSourceEportAssignment.h
*
* @brief Verify the functionality of TTI action Source Eport Assignment field
* for parallel lookup
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfTunnelTermSourceEportAssignmenth
#define __prvTgfTunnelTermSourceEportAssignmenth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal tgfTunnelTermSourceEportAssignment function
* @endinternal
*
* @brief   Test of TTI Source ePort Assignment (main func)
*         INPUTS:
*/
GT_VOID tgfTunnelTermSourceEportAssignment
(
    GT_BOOL sourceEPortAssignmentEnableArray[],
    GT_U32  lookupNumSourceEportExpected,
    GT_BOOL restoreConfig
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelTermSourceEportAssignmenth */

