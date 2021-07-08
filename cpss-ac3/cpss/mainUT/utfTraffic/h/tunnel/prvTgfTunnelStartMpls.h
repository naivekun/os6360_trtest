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
* @file prvTgfTunnelStartMpls.h
*
* @brief Tunnel Start: Mpls functionality
*
* @version   4
********************************************************************************
*/
#ifndef __prvTgfTunnelStartMplsh
#define __prvTgfTunnelStartMplsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @enum PRV_TGF_TUNNEL_MPLS_LSR_PUSH_TYPE_ENT
 *
 * @brief Type of MPLS LSR PUSH Tunnel Start Test
*/
typedef enum
{
    PRV_TGF_TUNNEL_MPLS_LSR_PUSH_1_LABEL_E,
    PRV_TGF_TUNNEL_MPLS_LSR_PUSH_2_LABEL_E,
    PRV_TGF_TUNNEL_MPLS_LSR_PUSH_3_LABEL_E,
    PRV_TGF_TUNNEL_MPLS_LSR_SWAP_E,
    PRV_TGF_TUNNEL_MPLS_LSR_POP_SWAP_E,

    PRV_TGF_TUNNEL_MPLS_LSR_PUSH_TYPE_MAX_E
} PRV_TGF_TUNNEL_MPLS_LSR_PUSH_TYPE_ENT;

/**
* @enum PRV_TGF_TUNNEL_MPLS_LSR_POP_TYPE_ENT
 *
 * @brief Type of MPLS LSR POP Tunnel Start Test
*/
typedef enum{

    PRV_TGF_TUNNEL_MPLS_LSR_POP_1_LABEL_E,

    PRV_TGF_TUNNEL_MPLS_LSR_POP_2_LABEL_E,

    PRV_TGF_TUNNEL_MPLS_LSR_POP_TYPE_MAX_E

} PRV_TGF_TUNNEL_MPLS_LSR_POP_TYPE_ENT;


/**
* @internal prvTgfTunnelMplsPush function
* @endinternal
*
* @brief   MPLS LSR PUSH Functionality
*
* @param[in] pushType                 - Type of MPLS LSR PUSH Tunnel Start Test
*                                       None
*/
GT_VOID prvTgfTunnelMplsPush
(
    IN PRV_TGF_TUNNEL_MPLS_LSR_PUSH_TYPE_ENT pushType
);

/**
* @internal prvTgfTunnelMplsPop function
* @endinternal
*
* @brief   MPLS LSR POP Functionality
*
* @param[in] popType                  - Type of MPLS LSR POP Tunnel Start Test
*                                       None
*/
GT_VOID prvTgfTunnelMplsPop
(
    IN PRV_TGF_TUNNEL_MPLS_LSR_POP_TYPE_ENT popType
);

/**
* @internal prvTgfTunnelMplsPushAndEgrQosCfgAdd function
* @endinternal
*
* @brief   MPLS LSR PUSH and adding egress qos configuration
*/
GT_VOID prvTgfTunnelMplsPushAndEgrQosCfgAdd
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartMplsEgressQosMappingPassengerEthernet function
* @endinternal
*
* @brief   MPLS TS for Ethernet passenger + egress EXP-to-EXP remap by egress EPort
*         configuration
*         config + send traffic + verify results + restore
*/
GT_VOID prvTgfTunnelStartMplsEgressQosMappingPassengerEthernet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartMplsModifyOuterVlanTagPassengerEthernet function
* @endinternal
*
* @brief   Test that checking 'outer vlan tag' modifications on TS packets.
*         config + send traffic + verify results + restore
*/
GT_VOID prvTgfTunnelStartMplsModifyOuterVlanTagPassengerEthernet
(
    GT_VOID
);



/**
* @internal prvTgfTunnelMplsQosSet function
* @endinternal
*
* @brief   MPLS Egress QoS mapping set
*
* @param[in] egressPort               - the port to set configuration on.
* @param[in] doRemap                  - do we set/restore the exp-to-exp remap
*                                       None
*/
GT_VOID prvTgfTunnelMplsQosSet
(
    IN GT_U32  egressPort,
    IN GT_BOOL doRemap
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelStartMplsh */


