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
* @file prvTgfBasicIpv6UcRouting.h
*
* @brief Basic IPV6 UC Routing
*
* @version   7
********************************************************************************
*/
#ifndef __prvTgfBasicIpv6UcRoutingh
#define __prvTgfBasicIpv6UcRoutingh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <trafficEngine/tgfTrafficEngine.h>
#include <common/tgfLpmGen.h>


/**
* @internal prvTgfBasicIpv6UcRoutingTestAsDsa function
* @endinternal
*
* @brief   set the tests to be with egress DSA tag (extended DSA/eDSA) on the egress ports.
*
* @param[in] egressDsaType            - type of DSA tag
*                                       None
*/
GT_STATUS prvTgfBasicIpv6UcRoutingTestAsDsa
(
    TGF_DSA_TYPE_ENT    egressDsaType
);

/**
* @internal prvTgfBasicIpv6UcRoutingBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfBasicIpv6UcRoutingBaseConfigurationSet
(
    GT_VOID
);


/**
* @internal prvTgfBasicIpv6LpmConfigurationSet function
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
GT_VOID prvTgfBasicIpv6LpmConfigurationSet
(
    GT_PORT_NUM portNum,
    GT_BOOL     bypassEnabled,
    GT_BOOL     sipLookupEnable
);

/**
* @internal prvTgfBasicIpv6UcRoutingRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] useBulk                  - whether to use bulk operation to add prefixes
*                                       None
*/
GT_VOID prvTgfBasicIpv6UcRoutingRouteConfigurationSet
(
    GT_U32   prvUtfVrfId,
    GT_BOOL  useBulk
);

/**
* @internal prvTgfBasicIpv6UcRoutingRouteSipLookupConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration - SIP Lookup
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
GT_VOID prvTgfBasicIpv6UcRoutingRouteSipLookupConfigurationSet
(
    GT_U32   prvUtfVrfId
);

/**
* @internal prvTgfBasicIpv6UcRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] useSecondPkt             - whether to use the second packet (prvTgfPacket2PartArray)
* @param[in] expectTraffic            - whether to expect traffic or not
*                                       None
*/
GT_VOID prvTgfBasicIpv6UcRoutingTrafficGenerate
(
    GT_BOOL  useSecondPkt,
    GT_BOOL  expectTraffic,
    GT_PHYSICAL_PORT_NUM           sendPortNum
);

/**
* @internal prvTgfBasicIpv6UcRoutingTrafficGenerate4BytesMode function
* @endinternal
*
* @brief   Generate traffic expect no traffic
*/
GT_VOID prvTgfBasicIpv6UcRoutingTrafficGenerate4BytesMode
(
    GT_VOID
);

/**
* @internal prvTgfBasicIpv6UcRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] useBulk                  - whether to use bulk operation to delete the prefixes
*                                       None
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfBasicIpv6UcRoutingConfigurationRestore
(
    GT_U32   prvUtfVrfId,
    GT_BOOL  useBulk
);

/**
* @internal prvTgfIpv6UcRollBackCheck function
* @endinternal
*
* @brief   Set LTT Route Configuration
*/
GT_VOID   prvTgfIpv6UcRollBackCheck(GT_VOID);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBasicIpv6UcRoutingh */


