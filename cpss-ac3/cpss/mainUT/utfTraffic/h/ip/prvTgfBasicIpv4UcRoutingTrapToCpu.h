/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfBasicIpv4UcRoutingTrapToCpu.c
*
* DESCRIPTION:
*       Basic IPV4 UC Routing Trap To CPU
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef __prvTgfBasicIpv4UcRoutingTrapToCpuh
#define __prvTgfBasicIpv4UcRoutingTrapToCpuh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/**
* @internal prvTgfBasicIpv4UcRoutingTrapToCpuConfigurationSet function
* @endinternal
*
* @brief   Trap To CPU Configuration Set
*
* @param[in] sendPortNum              - port sending traffic
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcRoutingTrapToCpuConfigurationSet
(
    GT_U32               sendPortNum
);

/**
* @internal prvTgfBasicIpv4UcRoutingTrapToCpuTrafficGenerate function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcRoutingTrapToCpuTrafficGenerate
(
    GT_U32    sendPortNum,
    GT_U32    nextHopPortNum
);

/**
* @internal prvTgfBasicIpv4UcRoutingTrapToCpuConfigurationRestore function
* @endinternal
*
* @brief   Trap To CPU Configuration Restore
*
* @param[in] sendPortNum              - port sending traffic
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcRoutingTrapToCpuConfigurationRestore
(
    GT_U32               sendPortNum
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBasicIpv4UcRoutingTrapToCpuh */

