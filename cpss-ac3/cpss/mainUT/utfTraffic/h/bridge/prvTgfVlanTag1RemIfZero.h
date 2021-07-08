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
* @file prvTgfVlanTag1RemIfZero.h
*
* @brief Bridge VLAN Tag1 Removal If Zero UT.
*
* @version   1.0
********************************************************************************
*/
#ifndef __prvTgfVlanTag1RemIfZero
#define __prvTgfVlanTag1RemIfZero

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfVlanTag1RemIfZeroConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Generate Traffic:
*         Success Criteria:
*         Additional Configuration:
*         Generate Traffic:
*         Success Criteria:
*/
GT_VOID prvTgfVlanTag1RemIfZeroConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfVlanTag1RemIfZeroDisabledTrafficGenerate function
* @endinternal
*
* @brief   Disable VLan Tag1 Removal.
*         Configure Vlan command to Tag1 inner tag and Tag0 outer tag
*         TTI Action for Tag1 is Modify_ALL and VID = 0.
*         TTI Action for Tag0 is Modify_ALL and VID = 5.
*         Generate traffic:
*         Send from port0 single tagged packet:
*         macDa = 00:00:22:22:22:22
*         macSa = 00:00:11:11:11:11
*         Success Criteria:
*         Tag1 is not removed and it's value 0
*/
GT_VOID prvTgfVlanTag1RemIfZeroDisabledTrafficGenerate
(
    void
);

/**
* @internal prvTgfVlanTag1RemIfZeroSingleTagTrafficGenerate function
* @endinternal
*
* @brief   Enable VLan Tag1 Removal for not double tagged packets.
*         Configure Vlan command to Tag1 inner tag and Tag0 outer tag
*         TTI Action for Tag1 is Modify_ALL and VID = 0.
*         TTI Action for Tag0 is Modify_ALL and VID = 5.
*         Generate traffic:
*         Send from port0 single tagged packet:
*         macDa = 00:00:22:22:22:22
*         macSa = 00:00:11:11:11:11
*         Success Criteria:
*         Tag1 is removed
*/
GT_VOID prvTgfVlanTag1RemIfZeroSingleTagTrafficGenerate
(
    void
);

/**
* @internal prvTgfVlanTag1RemIfZeroDoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Enable VLan Tag1 Removal for all packets.
*         Configure Vlan command to Tag1 inner tag and Tag0 outer tag
*         TTI Action for Tag1 is Modify_ALL and VID = 0.
*         TTI Action for Tag0 is Modify_ALL and VID = 5.
*         Generate traffic:
*         Send from port0 double tagged packet:
*         macDa = 00:00:22:22:22:22
*         macSa = 00:00:11:11:11:11
*         Success Criteria:
*         Tag1 is removed
*/
GT_VOID prvTgfVlanTag1RemIfZeroDoubleTagTrafficGenerate
(
    void
);

/**
* @internal prvTgfVlanTag1RemIfZeroUntagTrafficGenerate function
* @endinternal
*
* @brief   Enable VLan Tag1 Removal for all packets.
*         Configure Vlan command to Tag1 inner tag and Tag0 outer tag
*         TTI Action for Tag1 is Modify_ALL and VID = 0.
*         TTI Action for Tag0 is Modify_ALL and VID = 5.
*         Generate traffic:
*         Send from port0 untagged packet:
*         macDa = 00:00:22:22:22:22
*         macSa = 00:00:11:11:11:11
*         Success Criteria:
*         Tag1 is removed
*/
GT_VOID prvTgfVlanTag1RemIfZeroUntagTrafficGenerate
(
    void
);

/**
* @internal prvTgfVlanTag1RemIfZeroDoubleTagDisableTrafficGenerate function
* @endinternal
*
* @brief   Enable VLan Tag1 Removal for not double tagged packets.
*         Configure Vlan command to Tag1 inner tag and Tag0 outer tag
*         TTI Action for Tag1 is Modify_ALL and VID = 0.
*         TTI Action for Tag0 is Modify_ALL and VID = 5.
*         Generate traffic:
*         Send from port0 double tagged packet:
*         macDa = 00:00:22:22:22:22
*         macSa = 00:00:11:11:11:11
*         Success Criteria:
*         Tag1 is not removed
*/
GT_VOID prvTgfVlanTag1RemIfZeroDoubleTagDisableTrafficGenerate
(
    void
);

/**
* @internal prvTgfVlanTag1RemIfZeroTag1AssignmentIsNotZeroTrafficGenerate function
* @endinternal
*
* @brief   Enable VLan Tag1 Removal for all packets.
*         Configure Vlan command to Tag1 inner tag and Tag0 outer tag
*         TTI Action for Tag1 is Modify_ALL and VID = 2.
*         TTI Action for Tag0 is Modify_ALL and VID = 5.
*         Generate traffic:
*         Send from port0 untagged packet:
*         macDa = 00:00:22:22:22:22
*         macSa = 00:00:11:11:11:11
*         Success Criteria:
*         Tag1 is removed
*/
GT_VOID prvTgfVlanTag1RemIfZeroTag1AssignmentIsNotZeroTrafficGenerate
(
    void
);

/**
* @internal prvTgfVlanTag1RemIfZeroConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfVlanTag1RemIfZeroConfigurationRestore
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfVlanTag1RemIfZero */



