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
* @file prvTgfFdbBasicIpv4UcRouting.h
*
* @brief Fdb ipv4 uc routing
*
* @version   5
********************************************************************************
*/
#ifndef __prvTgfFdbBasicIpv4UcRoutingh
#define __prvTgfFdbBasicIpv4UcRoutingh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfBridgeGen.h>

/**
* @internal prvTgfFdbIpv4UcFdbEntryIndexGet function
* @endinternal
*
* @brief   return static parameter prvTgfIpv4UcFdbEntryIndex
*/
GT_U32 prvTgfFdbIpv4UcFdbEntryIndexGet
(
    GT_VOID
);

/**
* @internal prvTgfFdbBasicIpv4UcRoutingBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfFdbBasicIpv4UcRoutingBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfFdbIpv4UcRoutingRouteConfigurationSet function
* @endinternal
*
* @brief   Set FDB Route Configuration
*
* @param[in] routingByIndex           - GT_TRUE:  set routing entry by index
*                                      GT_FALSE: set routing entry by message
* @param[in] macEntryPtr              - (pointer to) mac entry
*                                       None
*/
GT_VOID prvTgfFdbIpv4UcRoutingRouteConfigurationSet
(
    GT_BOOL                                 routingByIndex,
    PRV_TGF_BRG_MAC_ENTRY_STC               *macEntryPtr
);

/**
* @internal prvTgfFdbIpv4UcRoutingRouteConfigurationGetByIndex function
* @endinternal
*
* @brief   Get FDB Route Entry Configuration By Index
*
* @param[out] validPtr                 - (pointer to) is entry valid
* @param[out] skipPtr                  - (pointer to) is entry skip control
* @param[out] agedPtr                  - (pointer to) is entry aged
* @param[out] hwDevNumPtr              - (pointer to) is HW device number associated with the entry
* @param[out] macEntryPtr              - (pointer to) extended Mac table entry
*                                       None
*/
GT_VOID prvTgfFdbIpv4UcRoutingRouteConfigurationGetByIndex
(
    GT_BOOL                      *validPtr,
    GT_BOOL                      *skipPtr,
    GT_BOOL                      *agedPtr,
    GT_HW_DEV_NUM                *hwDevNumPtr,
    PRV_TGF_BRG_MAC_ENTRY_STC    *macEntryPtr
);

/**
* @internal prvTgfFdbBasicIpv4UcGenericRoutingConfigurationSet function
* @endinternal
*
* @brief   Set FDB Generic Route Configuration
*/
GT_VOID prvTgfFdbBasicIpv4UcGenericRoutingConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfFdbBasicIpv4UcRoutingRouteConfigurationSet function
* @endinternal
*
* @brief   Set FDB Route Configuration
*
* @param[in] routingByIndex           - GT_TRUE:  set routing entry by index
*                                      GT_FALSE: set routing entry by message
*                                       None
*/
GT_VOID prvTgfFdbBasicIpv4UcRoutingRouteConfigurationSet
(
    GT_BOOL routingByIndex
);


/**
* @internal prvTgfFdbBasicIpv4UcRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] expectTraffic            - whether to expect traffic or not
*                                       None
*/
GT_VOID prvTgfFdbBasicIpv4UcRoutingTrafficGenerate
(
    GT_U32    sendPortNum,
    GT_U32    nextHopPortNum,
    GT_BOOL   expectTraffic,
    GT_BOOL   vlanCounterMode
);

/**
* @internal prvTgfFdbBasicIpv4UcGenericRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore generic routing test configuration
*/
GT_VOID prvTgfFdbBasicIpv4UcGenericRoutingConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfFdbBasicIpv4UcRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbBasicIpv4UcRoutingConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfFdbBasicIpv4UcRoutingFillFdb function
* @endinternal
*
* @brief   Test prvTgfFdbBasicIpv4UcRoutingFillFdb main function
*
* @param[in] routingByIndex           - GT_TRUE:  set routing entry by index
*                                      GT_FALSE: set routing entry by message
*                                       None
*/
GT_VOID prvTgfFdbBasicIpv4UcRoutingFillFdb
(
    GT_BOOL routingByIndex
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFdbBasicIpv4UcRoutingh */


