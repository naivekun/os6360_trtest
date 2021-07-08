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
* @file prvTgfFdbAging.h
*
* @brief FDB Aging tests
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfFdbAging
#define __prvTgfFdbAging

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfFdbAgeBitDaRefreshConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Enable automatic aging.
*         Add to FDB table non-static entry with
*         MACADDR: 00:00:00:00:00:22 on port 23.
*         Change Aging time to 10 seconds (real aging will be after
*         210 second).
*         Success Criteria:
*         After 20 seconds packets captured on ports 8 and 18
*         Add to FDB table non-static entry with
*         MACADDR: 00:00:00:00:00:22 on port 23.
*         Enable Aged bit DA refresh.
*         Success Criteria:
*         After 20 seconds there will no packets captured on
*         ports 8 and 18.
*/
GT_VOID prvTgfFdbAgeBitDaRefreshConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfFdbAgeBitDaRefreshTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:22,
*         macSa = 00:00:00:00:00:11,
*/
GT_VOID prvTgfFdbAgeBitDaRefreshTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfFdbAgeBitDaRefreshConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbAgeBitDaRefreshConfigurationRestore
(
    GT_VOID
);


/**
* @internal prvTgfFdbAgingInMultiPortGroupDeviceConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*/
GT_VOID prvTgfFdbAgingInMultiPortGroupDeviceConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfFdbAgingInMultiPortGroupDeviceTrafficGenerate function
* @endinternal
*
* @brief   a.    Into port group 0 : Set 8 FDB entries associated with port N1..N8 (as dynamic entry)
*         i.    Trigger aging twice. (to get AA messages)
*         ii.    Check that got only single AA message from port group 0.
*         b.    Into port group 1 : Set 8 FDB entries associated with port N1..N8 (as dynamic entry)
*         i.    Trigger aging twice. (to get AA messages)
*         ii.    Check that got only single AA message from port group 1.
*         c.    Into port group 2 : Set 8 FDB entries associated with port N1..N8 (as dynamic entry)
*         i.    Trigger aging twice. (to get AA messages)
*         ii.    Check that got only single AA message from port group 2.
*         d.    ... in the same logic do for port groups 3..6
*         e.    Into port group 7 : Set 8 FDB entries associated with port N1..N8 (as dynamic entry)
*         i.    Trigger aging twice. (to get AA messages)
*         ii.    Check that got only single AA message from port group 7.
*         Use next 8 trunks for the test: 0xC,0x1C,0x2C,0x3C,0x4C,0x5C,0x6C,0x7C.
*         a.    Into port group 0 : Set 8 FDB entries associated with trunks (as dynamic entry)
*         i.    Trigger aging twice. (to get AA messages)
*         ii.    Check that got 8 AA (for each and every entry).
*         b.    ... in the same logic do for port groups 2..6
*         c.    Into port group 7 : Set 8 FDB entries associated with trunks (as dynamic entry)
*         i.    Trigger aging twice. (to get AA messages)
*         ii.    Check that got 8 AA (for each and every entry).
*/
GT_VOID prvTgfFdbAgingInMultiPortGroupDeviceTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfFdbAgingInMultiPortGroupDeviceConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbAgingInMultiPortGroupDeviceConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfFdbAuMessageFilteringConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Enable automatic aging.
*         Add to FDB table non-static entry with
*         MACADDR: 00:00:00:00:00:22 on port 23.
*         Change Aging time to 1 second (real aging will be after
*         2 seconds).
*/
GT_VOID prvTgfFdbAuMessageFilteringConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfFdbAuMessageFilteringTrafficGenerate function
* @endinternal
*
* @brief   Sleep 1 sec.
*         See only 1 AA message
*/
GT_VOID prvTgfFdbAuMessageFilteringTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfFdbAuMessageFilteringConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbAuMessageFilteringConfigurationRestore
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

/**
* @internal prvTgfFdbAddEntry function
* @endinternal
*
* @brief   Add FDB entry.
*
* @param[in] vlanId                   -  to be configured
* @param[in] portIdx                  - port index
* @param[in] macAddr                  - MAC address
*                                       None
*/
GT_VOID prvTgfFdbAddEntry
(
    IN GT_U16           vlanId,
    IN GT_U8            portIdx,
    IN TGF_MAC_ADDR     macAddr
);

/**
* @internal prvTgfFdbAgeBitDaRefreshTestInit function
* @endinternal
*
* @brief   Set VLAN entry.
*
* @param[in] vlanId                   -  to be configured
*                                       None
*/
GT_VOID prvTgfFdbAgeBitDaRefreshTestInit
(
    IN GT_U16           vlanId,
    IN TGF_MAC_ADDR     macAddr
);

#endif /* __prvTgfFdbAging */



