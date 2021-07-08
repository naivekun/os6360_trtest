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
* @file prvTgfBrgGen.h
*
* @brief Bridge Generic APIs UT.
*
* @version   2.
********************************************************************************
*/
#ifndef __prvTgfBrgGen
#define __prvTgfBrgGen

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBrgGenIeeeReservedMcastProfilesConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Create VLAN 2 on all ports (0,8,18,23)
*         Set capture to CPU
*         BridgingModeSet = CPSS_BRG_MODE_802_1Q_E
*         GenIeeeReservedMcastTrapEnable = GT_TRUE
*         For all ports (0,8,18,23):
*         For all profileIndex (0...3):
*         IeeeReservedMcastProfileIndexSet (devNum = 0,
*         portNum,
*         profileIndex)
*         For all protocols (0...255)
*         IeeeReservedMcastProtCmdSet1 (dev = 0,
*         profileIndex,
*         protocol_iterator,
*         CPSS_PACKET_CMD_TRAP_TO_CPU_E)
*         Send Packet from to portNum.
*         IeeeReservedMcastProtCmdSet1 (dev = 0,
*         profileIndex,
*         protocol_iterator,
*         CPSS_PACKET_CMD_DROP_SOFT_E)
*         Success Criteria:
*         Fisrt packet is not captured on ports 0,8,18,23 but is captured in CPU.
*         Second packet is dropped.
*/
GT_VOID prvTgfBrgGenIeeeReservedMcastProfilesConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgGenIeeeReservedMcastProfilesTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 IP4 packet:
*         000000   01 80 c2 00 00 03 00 00 00 00 00 11 81 00 00 02
*         000010 08 00 45 00 00 2a 00 00 00 00 40 ff 79 d4 00 00
*         000020  00 00 00 00 00 02 00 01 02 03 04 05 06 07 08 09
*         000030  0a 0b 0c 0d 0e 0f 10 11 12 13 14 15
*/
GT_VOID prvTgfBrgGenIeeeReservedMcastProfilesTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgGenIeeeReservedMcastProfilesConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgGenIeeeReservedMcastProfilesConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgGenBypassModeTest function
* @endinternal
*
* @brief   Bridge Bypass mode envelope
*/
GT_VOID prvTgfBrgGenBypassModeTest
(
    GT_VOID
);

/**
* @internal prvTgfBrgPhysicalPortsSetUpTest function
* @endinternal
*
* @brief   Bridge physical port setup test
*/
GT_VOID prvTgfBrgPhysicalPortsSetUpTest
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgGen */


