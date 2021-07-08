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
* @file prvTgfVnt.h
*
* @brief Virtual Network Tester (VNT)
*
* @version   2.
********************************************************************************
*/
#ifndef __prvTgfVnt
#define __prvTgfVnt

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfVntOamPortPduTrapSet function
* @endinternal
*
* @brief   Set test configuration:
*         Enable OAM Port PPU Trap feature.
*/
GT_VOID prvTgfVntOamPortPduTrapSet
(
    GT_VOID
);

/**
* @internal prvTgfVntLoopbackModeConfSet function
* @endinternal
*
* @brief   Set test configuration:
*         Configure Loopback Mode om port 8.
*/
GT_VOID prvTgfVntLoopbackModeConfSet
(
    GT_VOID
);

/**
* @internal prvTgfVntOamPortPduTrapTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = 01:80:C2:00:00:02,
*         macSa = 00:00:00:00:00:02
*         Success Criteria:
*         Packet is captured on ports 0 and 18, 23 and CPU.
*/
GT_VOID prvTgfVntOamPortPduTrapTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfVntLoopbackTrafficConfGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 OAM control packet:
*         macDa = 01:80:C2:00:00:02,
*         macSa = 00:00:00:00:00:02,
*         Ether Type 8809.
*         Subtype 0x03.
*         Success Criteria:
*         Packet is captured on CPU.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         Packet is captured on port 8.
*/
GT_VOID prvTgfVntLoopbackTrafficConfGenerate
(
    GT_VOID
);

/**
* @internal prvTgfVntLoopbackConfRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfVntLoopbackConfRestore
(
    GT_VOID
);

/**
* @internal prvTgfVntOamPortPduTrapRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfVntOamPortPduTrapRestore
(
    GT_VOID
);

/**
* @internal prvTgfVntCfmEtherTypeIdentificationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine.
*         Set Pcl rule to trap packets with ethertype 3434.
*         Set CFM ethertype to 3434.
*/
GT_VOID prvTgfVntCfmEtherTypeIdentificationSet
(
    GT_VOID
);

/**
* @internal prvTgfVntCfmEtherTypeIdentificationTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         ethertype = 2222
*         Success Criteria:
*         Packet is captured on ports 0,18,23.
*         Send to device's port 0 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         ethertype = 3434
*         Success Criteria:
*         Packet is captured on CPU only.
*/
GT_VOID prvTgfVntCfmEtherTypeIdentificationTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfVntCfmEtherTypeIdentificationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfVntCfmEtherTypeIdentificationRestore
(
    GT_VOID
);

/**
* @internal prvTgfVntCfmOpcodeIdentificationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine.
*         Set Pcl rule to trap packets with ethertype 3434 and UDB 0xFF and
*         to redirect them to the send port.
*         Set CFM ethertype to 3434.
*         Set opcode to 0x7F.
*/
GT_VOID prvTgfVntCfmOpcodeIdentificationSet
(
    GT_VOID
);

/**
* @internal prvTgfVntCfmOpcodeIdentificationTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         ethertype = 3434
*         Success Criteria:
*         Packet is captured on CPU.
*         Packet is captured on port 8 with opcode 0x7F
*/
GT_VOID prvTgfVntCfmOpcodeIdentificationTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfVntCfmOpcodeIdentificationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfVntCfmOpcodeIdentificationRestore
(
    GT_VOID
);

/**
* @internal prvTgfVntCfmOpcodeChangeSet function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the TTI.
*         Set TTI rule to trap packets with ethertype 3434 and
*         to redirect them to the send port.
*         Set CFM ethertype to 3434.
*         Set opcode to 0x7F.
*/
GT_VOID prvTgfVntCfmOpcodeChangeSet
(
    GT_VOID
);

/**
* @internal prvTgfVntCfmOpcodeChangeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         ethertype = 3434
*         Success Criteria:
*         Packet is captured on CPU.
*         Packet is captured on port 8 with opcode 0x7F
*/
GT_VOID prvTgfVntCfmOpcodeChangeTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfVntCfmOpcodeChangeRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfVntCfmOpcodeChangeRestore
(
    GT_VOID
);

/**
* @internal prvTgfVntOamPortUnidirectionalTransmitModeSet function
* @endinternal
*
* @brief   Set test configuration:
*         Enable/Disable the port for unidirectional transmit.
*/
GT_VOID prvTgfVntOamPortUnidirectionalTransmitModeSet
(
    GT_VOID
);

/**
* @internal prvTgfVntOamPortUnidirectionalTransmitModeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         Packet is captured on ports 0,18,23.
*/
GT_VOID prvTgfVntOamPortUnidirectionalTransmitModeTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfVntOamPortUnidirectionalTransmitModeRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfVntOamPortUnidirectionalTransmitModeRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfVnt */


