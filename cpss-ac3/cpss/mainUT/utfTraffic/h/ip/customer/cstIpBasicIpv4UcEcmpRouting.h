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
* @file cstIpBasicIpv4UcEcmpRouting.h
*
* @brief Basic IPV4 UC ECMP Routing
*
* @version   1
********************************************************************************
*/
#ifndef __cstIpBasicIpv4UcEcmpRoutingh
#define __cstIpBasicIpv4UcEcmpRoutingh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef CHX_FAMILY
GT_STATUS cstIpBasicIpv4UcEcmpRoutingConfig
(
    GT_VOID
);

GT_STATUS cstIpBasicIpv4UcEcmpRoutingRestore
(
    GT_VOID
);

GT_VOID cstIpBasicIpv4UcEcmpRoutingTrafficGenerate
(
    GT_BOOL randomEnable
);

GT_STATUS cstIpBasicIpv4UcEcmpRoutingTest
(
    GT_VOID
);
#endif /*CHX_FAMILY*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cstIpBasicIpv4UcEcmpRoutingh */


