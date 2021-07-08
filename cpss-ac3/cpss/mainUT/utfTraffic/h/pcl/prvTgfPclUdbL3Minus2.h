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
* @file prvTgfPclUdbL3Minus2.h
*
* @brief UDB Anchor L3 Mimus 2 Ingress and Eegress PCL testing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPclUdbL3Minus2h
#define __prvTgfPclUdbL3Minus2h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverIpv4WithL2Test function
* @endinternal
*
* @brief   Test on IPCL L3Minus2 anchor for Tunnel Terminated Ipv4 Over Ipv4 With Passenger L2
*         UDBs for anchor L3Minus2 offset 0-3.
*/
GT_VOID prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverIpv4WithL2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverIpv4NoL2Test function
* @endinternal
*
* @brief   Test on IPCL L3Minus2 anchor for Tunnel Terminated Ipv4 Over Ipv4 Without Passenger L2
*         UDBs for anchor L3Minus2 offset 0-3.
*/
GT_VOID prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverIpv4NoL2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverMplsWithL2Test function
* @endinternal
*
* @brief   Test on IPCL L3Minus2 anchor for Tunnel Terminated Ipv4 Over Mpls With Passenger L2
*         UDBs for anchor L3Minus2 offset 0-3.
*/
GT_VOID prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverMplsWithL2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverMplsNoL2Test function
* @endinternal
*
* @brief   Test on IPCL L3Minus2 anchor for Tunnel Terminated Ipv4 Over Mpls Without Passenger L2
*         UDBs for anchor L3Minus2 offset 0-3.
*/
GT_VOID prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverMplsNoL2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverIpv4WithL2Test function
* @endinternal
*
* @brief   Test on EPCL L3Minus2 anchor for Tunnel Start Ipv4 Over Ipv4 With Passenger L2
*         UDBs for anchor L3Minus2 offset 0-3.
*/
GT_VOID prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverIpv4WithL2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverIpv4NoL2Test function
* @endinternal
*
* @brief   Test on EPCL L3Minus2 anchor for Tunnel Start Ipv4 Over Ipv4 Without Passenger L2
*         UDBs for anchor L3Minus2 offset 0-3.
*/
GT_VOID prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverIpv4NoL2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverMplsWithL2Test function
* @endinternal
*
* @brief   Test on EPCL L3Minus2 anchor for Tunnel Start Ipv4 Over Mpls With Passenger L2
*         UDBs for anchor L3Minus2 offset 0-3.
*/
GT_VOID prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverMplsWithL2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverMplsNoL2Test function
* @endinternal
*
* @brief   Test on EPCL L3Minus2 anchor for Tunnel Start Ipv4 Over Mpls Without Passenger L2
*         UDBs for anchor L3Minus2 offset 0-3.
*/
GT_VOID prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverMplsNoL2Test
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPclUdbL3Minus2h */



