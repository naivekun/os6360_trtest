/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* tgfPortFwdToLb.h
*
* DESCRIPTION:
*       Check "forwarding to loopback/service port" (abbreviated as "FTL" below)
*       feature.
*       General idea of all tests is:
*       Send a packet - check the packet(s) egressed target port(s)
*       Enable forwarding to loopback for the target port - check the
*       packet egressed via loopback port instead of target port.
*       There are test cases:
*       - single-target FORWARD packet type
*       - multi-target FORWARD packet type
*       - FROM_CPU packet type
*       - TO_ANALYZER packet type
*       - TO_CPU packet type
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef __tgfPortFwdToLb
#define __tgfPortFwdToLb

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



/*******************************************************************************
* tgfPortFwdToLbForwardSingleTargetTest
*
* DESCRIPTION:
*       Check FTL for single-target FORWARD and FROM_CPU packet types.
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID tgfPortFwdToLbForwardSingleTargetTest
(
    GT_VOID
);

/*******************************************************************************
* tgfPortFwdToLbForwardMultiTargetTest
*
* DESCRIPTION:
*       Check FTL feature for multi-target FORWARD packet type.
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID tgfPortFwdToLbForwardMultiTargetTest
(
    GT_VOID
);

/*******************************************************************************
* tgfPortFwdToLbToAnalyzerTest
*
* DESCRIPTION:
*       Check FTL feature for TO_ANALYER packet type.
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID tgfPortFwdToLbToAnalyzerTest
(
    GT_VOID
);

/*******************************************************************************
* tgfPortFwdToLbToCpuTest
*
* DESCRIPTION:
*       Check FTL feature for multi-target FORWARD packet type.
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID tgfPortFwdToLbToCpuTest
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfPortFwdToLb */
