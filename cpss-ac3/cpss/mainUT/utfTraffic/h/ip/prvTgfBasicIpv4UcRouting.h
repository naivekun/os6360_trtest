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
* @file prvTgfBasicIpv4UcRouting.h
*
* @brief Basic IPV4 UC Routing
*
* @version   17
********************************************************************************
*/
#ifndef __prvTgfBasicIpv4UcRoutingh
#define __prvTgfBasicIpv4UcRoutingh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <common/tgfLpmGen.h>
#include <common/tgfIpGen.h>

/**
* @enum PRV_TGF_IPV4_STACKING_SYSTEM_ENT
 *
 * @brief Enumeration of stacking systems.
*/
typedef enum{

    /** not stacking system. */
    PRV_TGF_IPV4_STACKING_SYSTEM_NONE_E,

    /** @brief stacking system where the
     *  tested device is the routing device.
     */
    PRV_TGF_IPV4_STACKING_SYSTEM_ROUTING_DEVICE_E,

    /** @brief stacking system
     *  where the tested device is the device that get routing decisions
     *  from previous device.
     */
    PRV_TGF_IPV4_STACKING_SYSTEM_AFTER_ROUTING_DEVICE_E

} PRV_TGF_IPV4_STACKING_SYSTEM_ENT;

/**
* @enum PRV_TGF_URPF_MODE_ENT
 *
 * @brief Enumeration of uRPF check.
*/
typedef enum{

    /** @brief In this mode, the packet assigned eVLAN is compared to the SIP
     *  nexthop eVLAN assignment.
     */
    PRV_TGF_EVLAN_BASED_URPF_MODE_E,

    /** @brief In this mode, the packet assigned source (Device, ePort) is
     *  compared to the SIP nexthop (Device, ePort) assignment.
     */
    PRV_TGF_EPORT_BASED_URPF_MODE_E,

    /** @brief In this mode, the SIP next-hop packet command assignment is checked to
     *  be different than a DROP command
     */
    PRV_TGF_LOOSE_URPF_MODE_E

} PRV_TGF_URPF_MODE_ENT;

/**
* @internal prvTgfBasicIpv4UcRoutingBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] fdbPortNum               - the FDB port num to set in the macEntry destination Interface
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcRoutingBaseConfigurationSet
(
    GT_U32   fdbPortNum
);

/**
* @internal prvTgfBasicIpv4UcRoutingUnicastRpfEnableSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] prvUntUrpfMode           - unicast RPF mode
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcRoutingUnicastRpfEnableSet
(
    GT_U32                            prvUtfVrfId,
    PRV_TGF_URPF_MODE_ENT             prvUntUrpfMode
);

/**
* @internal prvTgfBasicIpv4UcRoutingUnicastRpfLooseModeTypeSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] prvUntUrpfLooseModeType  - unicast RPF Loose mode type
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcRoutingUnicastRpfLooseModeTypeSet
(
    GT_U32                            prvUtfVrfId,
    PRV_TGF_URPF_LOOSE_MODE_TYPE_ENT  prvUntUrpfLooseModeType
);

/**
* @internal prvTgfBasicIpv4UcRoutingUnicastSipSaEnableSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcRoutingUnicastSipSaEnableSet
(
    GT_U32                            prvUtfVrfId
);

/**
* @internal prvTgfBasicIpv4UcRoutingRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] sendPortIndex            - index of port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] useBulk                  - whether to use bulk operation to add prefixes
* @param[in] enableIpv4McRouting      - whether to enable IPv4 MC routing in the virtual router,
*                                      not relevant for policy based routing.
* @param[in] enableIpv6McRouting      - whether to enable IPv6 MC routing in the virtual router,
*                                      not relevant for policy based routing.
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcRoutingRouteConfigurationSet
(
    GT_U32   prvUtfVrfId,
    GT_U8    sendPortIndex,
    GT_U32    nextHopPortNum,
    GT_BOOL  useBulk,
    GT_BOOL  enableIpv4McRouting,
    GT_BOOL  enableIpv6McRouting
);

/**
* @internal prvTgfBasicIpv4UcRoutingRouteSipLookupConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration - SIP Lookup
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcRoutingRouteSipLookupConfigurationSet
(
    GT_U32   prvUtfVrfId
);

/**
* @internal prvTgfBasicIpv4LpmConfigurationSet function
* @endinternal
*
* @brief   Set LPM basic configuration
*
* @param[in] portNum                  - port number to disable the SIP lookup on
* @param[in] bypassEnabled            - the bypass enabling status:
*                                      GT_TRUE  - enable bypassing of lookup stages 8-31
*                                      GT_FALSE - disable bypassing of lookup stages 8-31
* @param[in] sipLookupEnable          - GT_TRUE:  enable SIP Lookup on the port
*                                      GT_FALSE: disable SIP Lookup on the port
*                                       None.
*/
GT_VOID prvTgfBasicIpv4LpmConfigurationSet
(
    GT_PORT_NUM portNum,
    GT_BOOL     bypassEnabled,
    GT_BOOL     sipLookupEnable
);

/**
* @internal prvTgfBasicIpv4UcRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] useSecondPkt             - whether to use the second packet (prvTgfPacket2PartArray)
* @param[in] useSecondArp             - whether to use the second ARP MAC address (prvTgfArpMac1)
* @param[in] isUrpfTest               - whether this is unicast RPF test
* @param[in] expectNoTraffic          - whether to expect traffic or not
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcRoutingTrafficGenerate
(
    GT_U32    sendPortNum,
    GT_U32    nextHopPortNum,
    GT_BOOL  useSecondPkt,
    GT_BOOL  useSecondArp,
    GT_BOOL  isUrpfTest,
    GT_BOOL  expectNoTraffic
);

/**
* @internal prvTgfBasicIpv4UcRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] sendPortIndex            - index of port sending traffic
* @param[in] nextHopPortIndex         - index of port nexthop traffic
* @param[in] useBulk                  - whether to use bulk operation to delete the prefixes
*                                       None
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfBasicIpv4UcRoutingConfigurationRestore
(
    GT_U32   prvUtfVrfId,
    GT_U8    sendPortIndex,
    GT_U8    nextHopPortIndex,
    GT_BOOL  useBulk
);

/**
* @internal prvTgfBasicIpv4UcRoutingInStackingSystemEnable function
* @endinternal
*
* @brief   state the type of stacking system.
*         to allow current device Y get DSA packet from srcDev X , and device Y need to do routing.
*         the target port of th erout is on device X.
*         the test checks the DSA tag that device Y send towards device X.
*         need to see that the 'srcDev' field in the DSA changed from 'X' to 'Y' so
*         device X will not do filter of packet 'from me from cascade port'
* @param[in] stackingSystemType       - indication that type of stacking system.
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcRoutingInStackingSystemEnable
(
    IN PRV_TGF_IPV4_STACKING_SYSTEM_ENT  stackingSystemType
);

/**
* @internal prvTgfBasicIpv4UcRoutingAgeBitGet function
* @endinternal
*
* @brief   Read router next hop table age bits entry.
*/
GT_VOID prvTgfBasicIpv4UcRoutingAgeBitGet
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv4UcRoutingUseEPortEnableSet function
* @endinternal
*
* @brief   Mark that ePort should be used in the test.
*
* @param[in] enable                   - GT_TRUE:  use ePort
*                                      GT_FALSE: don't use ePort
*                                       None
*
* @note Call this function before configuration set with GT_TRUE and after
*       configuration restore with GT_FALSE.
*
*/
GT_VOID prvTgfBasicIpv4UcRoutingUseEPortEnableSet
(
    GT_BOOL     enable
);
/**
* @internal prvTgfBasicIpv4UcMacSaRoutingConfigurationSet function
* @endinternal
*
* @brief   configure MAC SA assignment mode.
*/
GT_VOID prvTgfBasicIpv4UcMacSaRoutingConfigurationSet
(
    IN CPSS_MAC_SA_LSB_MODE_ENT    macSaMode
);

/**
* @internal prvTgfBasicIpv4UcMacSaRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] useSecondPkt             - whether to use the second packet (prvTgfPacket2PartArray)
* @param[in] macSaMode                - CPSS_SA_LSB_PER_PORT_E,
*                                      CPSS_SA_LSB_PER_PKT_VID_E,
*                                      CPSS_SA_LSB_PER_VLAN_E,
*                                      CPSS_SA_LSB_FULL_48_BIT_GLOBAL
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcMacSaRoutingTrafficGenerate
(
    GT_U32    sendPortNum,
    GT_U32    nextHopPortNum,
    GT_BOOL  useSecondPkt,
    CPSS_MAC_SA_LSB_MODE_ENT macSaMode
);

/**
* @internal prvTgfIpv4UcRoutingDefaultOverrideRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                      sendPortNum     - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] beforeDefaultOverride    - GT_TRUE: before default override
*                                      GT_FALSE: after default override
*                                       None
*/
GT_VOID prvTgfIpv4UcRoutingDefaultOverrideRouteConfigurationSet
(
    GT_U32   prvUtfVrfId,
    GT_U8    sendPortIndex,
    GT_U32    nextHopPortNum,
    GT_BOOL  beforeDefaultOverride
);

/**
* @internal prvTgfIpv4UcRoutingDefaultOverrideConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] sendPortIndex            - index of port sending traffic
*                                       None
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfIpv4UcRoutingDefaultOverrideConfigurationRestore
(
    GT_U32   prvUtfVrfId,
    GT_U8    sendPortIndex
);



/**
* @internal prvTgfIpv4UcRollBackCheck function
* @endinternal
*
* @brief   Set LTT Route Configuration
*/
GT_VOID   prvTgfIpv4UcRollBackCheck(GT_VOID);



/**
* @internal prvTgfBasicIpv4EPortToVidxRouteConfigurationSet function
* @endinternal
*
* @brief   Set basic ipv4 routing configurations for EPort that is :
*         eport : 1000 -->E2PHY : VIDX 100 : ports :according to portIdxBmp
* @param[in] portIdxBmp               - bmp of port indexes
*                                       None
*/
GT_VOID   prvTgfBasicIpv4EPortToVidxRouteConfigurationSet(
    IN GT_U32    portIdxBmp
);
/**
* @internal prvTgfBasicIpAddToDefaultSendVlanAndEnableRouting function
* @endinternal
*
* @brief   Add port to default send vlan and enable routing on this port
*
* @param[in] portIdx                  - port index
*                                       None
*/

GT_VOID   prvTgfBasicIpAddToDefaultSendVlanAndEnableRouting
(
    IN GT_U8    portIdx,
    IN CPSS_IP_PROTOCOL_STACK_ENT     protocol
);
/**
* @internal prvTgfBasicIpPickPortFromPipe function
* @endinternal
*
* @brief   Picks a port according to required pipe number
*
* @param[in] requiredPipeNum          - pipe number
* @param[in] skipIndex                - we do not want to pick destination port ,so provide index that should not be chosed
*
* @param[out] portIdxPtr               - port index
*
* @retval GT_OK                    - Port picked ok
* @retval GT_FAIL                  - Failure to pick a port
*/

GT_STATUS prvTgfBasicIpPickPortFromPipe
(
   IN GT_U8    requiredPipeNum,
   IN GT_U8    skipIndex,
   OUT GT_U8  * portIdxPtr 
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBasicIpv4UcRoutingh */


