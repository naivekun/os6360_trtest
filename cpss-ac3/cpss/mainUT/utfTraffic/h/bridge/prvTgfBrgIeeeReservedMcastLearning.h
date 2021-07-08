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
* @file prvTgfBrgIeeeReservedMcastLearning.h
*
* @brief Bridge Generic Port IEEE Reserved Mcast Learning UT.
*
* @version   1.0
********************************************************************************
*/
#ifndef __prvTgfBrgIeeeReservedMcastLearning
#define __prvTgfBrgIeeeReservedMcastLearning

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBrgGenIeeeReservedMcastLearningConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Create VLAN 2 on all ports (0,8,18,23)
*/
GT_VOID prvTgfBrgGenIeeeReservedMcastLearningConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgGenIeeeReservedMcastLearningEnableTrafficGenerate function
* @endinternal
*
* @brief   Enable IEEE Reserved Multicast Learning for trapped packets.
*         Set Profile index 2 for port1.
*         Set command TRAP_TO_CPU for profile index 2 and protocol 0x55.
*         Enable trap to CPU for IEEE Reserved Multicast packets.
*         Generate traffic:
*         Send from port0 single tagged packet:
*         macDa = 01:80:C2:00:00:55
*         macSa = 00:00:00:11:44:77
*         Success Criteria:
*         Packet is trapped to CPU and Learned on FDB.
*/
GT_VOID prvTgfBrgGenIeeeReservedMcastLearningEnableTrafficGenerate
(
    void
);

/**
* @internal prvTgfBrgGenIeeeReservedMcastLearningDisableTrafficGenerate function
* @endinternal
*
* @brief   Disable IEEE Reserved Multicast Learning for trapped packets.
*         Set Profile index 2 for port1.
*         Set command TRAP_TO_CPU for profile index 2 and protocol 0x55.
*         Enable trap to CPU for IEEE Reserved Multicast packets.
*         Generate traffic:
*         Send from port0 single tagged packet:
*         macDa = 01:80:C2:00:00:55
*         macSa = 00:00:00:11:44:77
*         Success Criteria:
*         Packet is trapped to CPU and is not Learned on FDB.
*/
GT_VOID prvTgfBrgGenIeeeReservedMcastLearningDisableTrafficGenerate
(
    void
);

/**
* @internal prvTgfBrgGenIeeeReservedMcastLearningDisableTrapDisableTrafficGenerate function
* @endinternal
*
* @brief   Disable IEEE Reserved Multicast Learning for trapped packets.
*         Set Profile index 2 for port1.
*         Set command CPSS_PACKET_CMD_FORWARD_E for profile index 2 and
*         protocol 0x55.
*         Disable trap to CPU for IEEE Reserved Multicast packets.
*         Generate traffic:
*         Send from port0 single tagged packet:
*         macDa = 01:80:C2:00:00:55
*         macSa = 00:00:00:11:44:77
*         Success Criteria:
*         Packet is not trapped to CPU and Learned on FDB.
*/
GT_VOID prvTgfBrgGenIeeeReservedMcastLearningDisableTrapDisableTrafficGenerate
(
    void
);

/**
* @internal prvTgfBrgGenIeeeReservedMcastLearningConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgGenIeeeReservedMcastLearningConfigurationRestore
(
    void
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgIeeeReservedMcastLearning */



