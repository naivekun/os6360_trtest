/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfFdbIpv6UcRouting.h
*
* DESCRIPTION:
*       Fdb ipv6 uc routing
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
*
*******************************************************************************/
#ifndef __prvTgfFdbBasicIpv6UcRoutingh
#define __prvTgfFdbBasicIpv6UcRoutingh

#include <common/tgfBridgeGen.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfFdbBasicIpv6UcRoutingBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfFdbBasicIpv6UcRoutingBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfFdbBasicIpv6UcRoutingRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*/
GT_VOID prvTgfFdbBasicIpv6UcRoutingRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfFdbBasicIpv6UcGenericRoutingConfigurationSet function
* @endinternal
*
* @brief   Set FDB Generic Route Configuration
*/
GT_VOID prvTgfFdbBasicIpv6UcGenericRoutingConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfFdbIpv6UcRoutingRouteByIndexConfigurationSet function
* @endinternal
*
* @brief   Set FDB Route Configuration
*
* @param[in] macEntryPtr              - (pointer to) mac entry
* @param[in] fdbEntryIndex            - FDB entry index
*                                       None
*/
GT_VOID prvTgfFdbIpv6UcRoutingRouteByIndexConfigurationSet
(
    PRV_TGF_BRG_MAC_ENTRY_STC               *macEntryPtr,
    GT_U32                                   fdbEntryIndex
);

/**
* @internal prvTgfFdbIpv6UcUpdateAddrDataLinkedList function
* @endinternal
*
* @brief   Update FDB IPv6 UC Route entries linked list,
*         that holds AddressIndex to DataIndex mapping
* @param[in] devNum                   - the device number
* @param[in] addrIndex                - FDB IPv6 UC address index bind to dataIndex
* @param[in] dataIndex                - FDB IPv6 UC data index bind to addrIndex
* @param[in] deleteElement            - GT_TRUE: delete the element if found
*                                      GT_FALSE: update the element if found or add a new one
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - on allocation fail
* @retval GT_FAIL                  - otherwise.
*
* @note adding a mapping of addrIndex and dataIndex to the Linked List must be
*       of valid entries that were added to HW.
*       if the address entry or the data entry is not added to HW this function will fail.
*       the Linked List should reflect the HW state
*
*/
GT_STATUS prvTgfFdbIpv6UcUpdateAddrDataLinkedList
(
    IN  GT_U8           devNum,
    IN  GT_U32          addrIndex,
    IN  GT_U32          dataIndex,
    IN  GT_BOOL         deleteElement
);

/**
* @internal prvTgfFdbBasicIpv6UcRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] expectTraffic            - whether to expect traffic or not
*                                       None
*/
GT_VOID prvTgfFdbBasicIpv6UcRoutingTrafficGenerate
(
    GT_BOOL  expectTraffic
);


/**
* @internal prvTgfFdbBasicIpv6UcGenericRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore generic routing test configuration
*/
GT_VOID prvTgfFdbBasicIpv6UcGenericRoutingConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfFdbBasicIpv6UcRoutingConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbBasicIpv6UcRoutingConfigRestore
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __utfTestsTypeRun 1,0,1,2,4,5,0h */



