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
* @file prvTgfVlanKeepVlan1.h
*
* @brief Keep Vlan1 UT.
*
* @version   1.0
********************************************************************************
*/
#ifndef __prvTgfVlanKeepVlan1
#define __prvTgfVlanKeepVlan1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfVlanKeepVlan1ConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Generate Traffic:
*         Success Criteria:
*         Additional Configuration:
*         Generate Traffic:
*         Success Criteria:
*/
GT_VOID prvTgfVlanKeepVlan1ConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfVlanKeepVlan1EnTagStateVlan1Vlan0DoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Enable Keep Vlan1 for destination port 18 and up 4.
*         Configure Vlan command to (Tag1, Tag0)
*         Generate traffic:
*         Send from port0 double tagged packet, where Tag0 = 2 and Tag1 = 7:
*         macDa = 00:00:22:22:22:22 (port 18)
*         macSa = 00:00:11:11:11:11 (port 0)
*         Success Criteria:
*         Tag1 is not removed and it's value 7
*/
GT_VOID prvTgfVlanKeepVlan1EnTagStateVlan1Vlan0DoubleTagTrafficGenerate
(
    void
);

/**
* @internal prvTgfVlanKeepVlan1EnTagStateVlan0DoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Enable Keep Vlan1 for destination port 18 and up 6.
*         Configure Vlan command to inner Tag0
*         Generate traffic:
*         Send from port0 double tagged packet, where Tag0 = 2 and Tag1 = 7:
*         macDa = 00:00:22:22:22:22 (port 18)
*         macSa = 00:00:11:11:11:11 (port 0)
*         Success Criteria:
*         Tag1 is not removed and it's value 7
*/
GT_VOID prvTgfVlanKeepVlan1EnTagStateVlan0DoubleTagTrafficGenerate
(
    void
);

/**
* @internal prvTgfVlanKeepVlan1EnTagStateUntaggedDoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Enable Keep Vlan1 for destination port 18 and up 4.
*         Configure Vlan command to inner Tag0
*         Generate traffic:
*         Send from port0 double tagged packet, where Tag0 = 2 and Tag1 = 7:
*         macDa = 00:00:22:22:22:22 (port 18)
*         macSa = 00:00:11:11:11:11 (port 0)
*         Success Criteria:
*         Tag1 is not removed and it's value 7
*/
GT_VOID prvTgfVlanKeepVlan1EnTagStateUntaggedDoubleTagTrafficGenerate
(
    void
);

/**
* @internal prvTgfVlanKeepVlan1EnTagStateVlan0SingleTagTrafficGenerate function
* @endinternal
*
* @brief   Enable Keep Vlan1 for destination port 18 and up 6.
*         Configure Vlan command to inner Tag0
*         Generate traffic:
*         Send from port0 single tagged packet, where Tag0 = 2:
*         macDa = 00:00:22:22:22:22 (port 18)
*         macSa = 00:00:11:11:11:11 (port 0)
*         Success Criteria:
*         Tag1 is removed
*/
GT_VOID prvTgfVlanKeepVlan1EnTagStateVlan0SingleTagTrafficGenerate
(
    void
);

/**
* @internal prvTgfVlanKeepVlan1DisTagStateVlan0DoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Disable Keep Vlan1 for destination port 18 and up 4.
*         Configure Vlan command to inner Tag0
*         Generate traffic:
*         Send from port0 double tagged packet, where Tag0 = 2 and Tag1 = 7:
*         macDa = 00:00:22:22:22:22 (port 18)
*         macSa = 00:00:11:11:11:11 (port 0)
*         Success Criteria:
*         Tag1 is removed
*/
GT_VOID prvTgfVlanKeepVlan1DisTagStateVlan0DoubleTagTrafficGenerate
(
    void
);

/**
* @internal prvTgfVlanKeepVlan1DisTagStateUntaggedDoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Disable Keep Vlan1 for destination port 18 and up 4.
*         Configure Vlan command to untagged
*         Generate traffic:
*         Send from port0 double tagged packet, where Tag0 = 2 and Tag1 = 7:
*         macDa = 00:00:22:22:22:22 (port 18)
*         macSa = 00:00:11:11:11:11 (port 0)
*         Success Criteria:
*         Tag1 is removed
*/
GT_VOID prvTgfVlanKeepVlan1DisTagStateUntaggedDoubleTagTrafficGenerate
(
    void
);

/**
* @internal prvTgfVlanKeepVlan1ConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfVlanKeepVlan1ConfigurationRestore
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfVlanKeepVlan1 */



