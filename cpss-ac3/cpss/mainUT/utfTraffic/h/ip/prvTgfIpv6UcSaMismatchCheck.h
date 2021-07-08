/*******************************************************************************
*              (c), Copyright 2016, Marvell International Ltd.                 *
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
* @file prvTgfIpv6UcSaMismatchCheck.h
*
* @brief Unicast SIP/SA Check
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIpv6UcSaMismatchCheckh
#define __prvTgfIpv6UcSaMismatchCheckh

#include <utf/utfMain.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfIpv6UcSaMismatchCheckConfig function
* @endinternal
*
* @brief   AUTODOC: Set Configuration:
*         1. Create two VLANs SENDER and NEXTHOP
*         2. Save unicast routing state for sender port
*         3. Enable unicast routing on sender port
*         4. Assign virtual router to sender VLAN
*         5. Enable unicast routing in sender VLAN
*         6. Create FDB entries for DA/SA MACs
*         7. Save ARP table entries
*         8. Write ARP table entries for DA/SA MACs
*         9. Save route entries
*         10. Set up route entries to route packets between two VLANs
*         11. Add prefix rules for SIP and DIP which use the route entries
*         12. Save SA check service state
*         13. Enable SA check service
*/
GT_VOID prvTgfIpv6UcSaMismatchCheckConfig
(
    GT_VOID
);

/**
* @internal prvTgfIpv6UcSaMismatchCheckGenerate function
* @endinternal
*
* @brief   AUTODOC: Perform the test:
*         SA check enabled, ARP entry correct   - packet passed
*         SA check enabled, ARP entry incorrect  - packet dropped
*         SA check disabled, ARP entry correct  - packet passed
*         SA check disabled, ARP entry incorrect - packet passed
*/
GT_VOID prvTgfIpv6UcSaMismatchCheckGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIpv6UcSaMismatchCheckRestore function
* @endinternal
*
* @brief   AUTODOC: Restore previous configuration
*         1. Restore SA check state
*         2. Delete prefix rules
*         3. Restore route entries
*         4. Restore ARP entries
*         5. Flush FDB
*         6. Disable routing in sender VLAN
*         7. Restore routing enabled state for sender port
*         8. Invalidate VLANs
*/
GT_VOID prvTgfIpv6UcSaMismatchCheckRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIpv6UcSaMismatchCheckh */


