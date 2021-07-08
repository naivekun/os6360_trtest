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
* @file prvTgfOamGeneral.h
*
* @brief Common OAM features testing
*
* @version   9
********************************************************************************
*/
#ifndef __prvTgfOamGeneralh
#define __prvTgfOamGeneralh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTrunkGen.h>
#include <common/tgfPclGen.h>
#include <common/tgfPolicerGen.h>
#include <common/tgfOamGen.h>
#include <common/tgfPtpGen.h>

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS                  5

/* Trunk ID */
#define PRV_TGF_TRUNK_ID_CNS                6

/* ether type used for "vlan tag" */
#define TGF_ETHERTYPE_8100_VLAN_TAG_CNS     0x8100

/* CFM Ethernet Type */
#define TGF_ETHERTYPE_8902_CFM_TAG_CNS      0x8902

/* port number to forward packet to */
#define PRV_TGF_FDB_PORT_0_IDX_CNS          0

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS           1

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS        2

/* port number to forward packet to */
#define PRV_TGF_FDB_PORT_2_IDX_CNS          2

/* port number to forward packet to */
#define PRV_TGF_FDB_PORT_3_IDX_CNS          3

/* CPU port */
#define PRV_TGF_CPU_PORT_IDX_CNS            4

/* Number of ports used in egress PCL*/
#define PRV_TGF_EPCL_PORTS_NUM_CNS          5

/* default number of packets to send */
#define PRV_TGF_OAM_BURST_COUNT_CNS         1

/* default sleep time between packets to send in milliseconds */
#define PRV_TGF_OAM_NO_DELAY_CNS            0

/* count of packets to send */
#define PRV_TGF_SEND_COUNT_CNS              2

/* billing counters index  */
#define PRV_TGF_BILLING_COUNTERS_INDEX_CNS  1
/* IPCL rule index */
#define PRV_TGF_CNC_GEN_IPCL_RULE_INDEX_CNS 3
/* EPCL rule index */
#define PRV_TGF_CNC_GEN_EPCL_RULE_INDEX_CNS  prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(30)

#define POLICER_ID_MASK    \
    (PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum) ? BIT_MASK_MAC(14) : BIT_MASK_MAC(13))

#define POLICER_MASK(value)         ((value) & POLICER_ID_MASK)

/* OAM base flow ID */
#define PRV_TGF_BASEFLOW_ID_CNS             1024
/* IPCL Action Flow ID*/
#define PRV_TGF_PCL_ACTION_FLOW_ID_CNS      1025
/* EPCL Action Flow ID*/
#define PRV_TGF_EPCL_ACTION_FLOW_ID_CNS     1035

/* Number of IPCL/EPCL rules */
#define PRV_TGF_IOAM_PCL_RULE_NUM_STC       6
#define PRV_TGF_EOAM_DM_PCL_RULE_NUM_STC    4
#define PRV_TGF_EOAM_LM_PCL_RULE_NUM_STC    4

/* Ingress OAM start entry index - source interface port */
#define PRV_IOAM_ENTRY_START_INDEX_CNS      PRV_TGF_PCL_ACTION_FLOW_ID_CNS - PRV_TGF_BASEFLOW_ID_CNS
/* Egress OAM start entry index - DM messages */
#define PRV_EOAM_DM_ENTRY_START_INDEX_CNS   PRV_TGF_EPCL_ACTION_FLOW_ID_CNS - PRV_TGF_BASEFLOW_ID_CNS
/* Egress OAM start entry index - LM messages */
#define PRV_EOAM_LM_ENTRY_START_INDEX_CNS   PRV_EOAM_DM_ENTRY_START_INDEX_CNS + PRV_TGF_EOAM_DM_PCL_RULE_NUM_STC

/* Ingress OAM entry index - source interface trunk */
#define PRV_OAM_ENTRY_SRC_TRUNK_INDEX_CNS   PRV_IOAM_ENTRY_START_INDEX_CNS + 3

/* OAM profile index */
#define PRV_TGF_OAM_PROFILE_INDEX_CNS       1

/* OAM entry index */
#define PRV_TGF_OAM_ENTRY_INDEX_CNS         1

/* PCL IDs */
#define PRV_TGF_IPCL_PCL_ID_0_CNS           0
#define PRV_TGF_EPCL_PCL_ID_0_CNS           10

/* MEG Level Byte == 3 */
#define PRV_TGF_MEL_BYTE_3_CNS              0x3
/* MEG Level Byte == 5 */
#define PRV_TGF_MEL_BYTE_5_CNS              0x5

/* Don't care MEG Level Byte */
#define PRV_TGF_MEL_BYTE_DONT_CARE_CNS      0x00

/* LM Opcode Byte == 40 DMM */
#define PRV_TGF_LM_OPCODE_BYTE_40_CNS       0x28
/* LM Opcode Byte == 41 DMM */
#define PRV_TGF_LM_OPCODE_BYTE_41_CNS       0x29
/* LM Opcode Byte == 42 DMM */
#define PRV_TGF_LM_OPCODE_BYTE_42_CNS       0x2A
/* LM Opcode Byte == 43 DMR */
#define PRV_TGF_LM_OPCODE_BYTE_43_CNS       0x2B
/* Dual-ended LM Opcode Byte == 1 */
#define PRV_TGF_LM_OPCODE_BYTE_01_CNS       0x1

/* DM Opcode Byte == 45 DMM */
#define PRV_TGF_DM_OPCODE_BYTE_45_CNS       0x2D
/* DM Opcode Byte == 46 DMR */
#define PRV_TGF_DM_OPCODE_BYTE_46_CNS       0x2E
/* DM Opcode Byte == 47 DMM */
#define PRV_TGF_DM_OPCODE_BYTE_47_CNS       0x2F

/* Ude Packet key UDB index */
#define PRV_TGF_PCL_UDE_UDB_INDEX_CNS       15

/* From CPU0 Egresss Port */
#define PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS   PRV_TGF_SEND_PORT_IDX_CNS
/* From CPU1 Egresss Port */
#define PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS   PRV_TGF_RECEIVE_PORT_IDX_CNS

/* OAM Opcode base value */
#define PRV_TGF_OPCODE_BASE_VALUE_CNS       0x3C

/* Packet commands */
#define PRV_TGF_PKT_CMD_TO_CPU_CNS          0
#define PRV_TGF_PKT_CMD_FROM_CPU_CNS        1

/* Ethertype offset */
#define PRV_TGF_OAM_ETHERTYPE_OFFSET_STC    0
/* MEG Level offset */
#define PRV_TGF_OAM_MEL_OFFSET_STC          0
/* Opcode offset */
#define PRV_TGF_OAM_OPCODE_OFFSET_STC       1
/* Flags offset */
#define PRV_TGF_OAM_FLAGS_OFFSET_STC        2
/* TLV offset */
#define PRV_TGF_OAM_TLV_OFFSET_STC          3
/* TLV data */
#define PRV_TGF_OAM_TLV_DATA_OFFSET_STC     4
/* TLV data size */
#define PRV_TGF_OAM_TLV_DATA_SIZE_STC       70

/* RDI UDB value */
#define PRV_TGF_OAM_RDI_UDB_VALUE_STC       0x0
/* RDI Profile 0 UDB mask */
#define PRV_TGF_OAM_RDI_PROFILE_0_UDB_MASK_STC 0x80
/* RDI Profile 1 UDB mask */
#define PRV_TGF_OAM_RDI_PROFILE_1_UDB_MASK_STC 0xFF

/* Source MAC check byte index - for IPCL/EPCL rules */
#define PRV_TGF_OAM_SRC_MAC_BYTE_INDEX_STC  4

/**
* @struct PRV_TGF_OAM_PDU_HEADER_DATA_STC
 *
 * @brief OAM PDU header structure
*/
typedef struct{

    /** @brief MEG level is a 3
     *  It contains an integer value that identifies MEG level of OAM PDU.
     *  Value ranges from 0 to 7 .
     */
    GT_U8 megLevel;

    /** @brief Version is a 5
     *  that identifies the OAM protocol version.
     */
    GT_U8 version;

    /** @brief OpCode is a 1
     *  that identifies an OAM PDU type. OpCode is used to identify
     *  the remaining content of an OAM PDU.
     */
    GT_U8 opcode;

    /** @brief Flags is an 8
     *  Use of the bits in this field is depend on the OAM PDU type.
     */
    GT_U8 flags;

    /** @brief The value of this field is associated with an OAM PDU type.
     *  When the TLV offset is 0, it points to the first octet following the TLV offset field.
     */
    GT_U8 tlvOffset;

    GT_U8 tlvDataArr[PRV_TGF_OAM_TLV_DATA_SIZE_STC];

} PRV_TGF_OAM_PDU_HEADER_DATA_STC;

/**
* @struct PRV_TGF_OAM_COMMON_CONFIG_STC
 *
 * @brief OAM common configuration
*/
typedef struct{

    /** enable/disable OAM process */
    GT_BOOL oamEnable;

    /** OAM entry index */
    GT_U32 oamEntryIndex;

    /** OAM entry */
    PRV_TGF_OAM_ENTRY_STC oamEntry;

    /** OAM exception type */
    PRV_TGF_OAM_EXCEPTION_TYPE_ENT oamExceptionType;

    /** OAM exception configuration */
    PRV_TGF_OAM_EXCEPTION_CONFIG_STC oamExceptionConfig;

} PRV_TGF_OAM_COMMON_CONFIG_STC;

/**
* @struct PRV_TGF_OAM_PCL_RULE_CONFIG_STC
 *
 * @brief OAM PCL keys and rules info
*/
typedef struct{

    /** @brief source interface type port/trunk
     *  srcTrgPortTrunk   - source/target interface port/trunk
     */
    CPSS_INTERFACE_TYPE_ENT type;

    GT_U32 srcPortTrunk;

    /** PDU header byte value */
    GT_U8 pduHeaderByteValue;

} PRV_TGF_OAM_PCL_RULE_CONFIG_STC;

/**
* @struct PRV_TGF_OAM_EPCL_RULE_CONFIG_STC
 *
 * @brief OAM EPCL keys and rules info
*/
typedef struct{

    /** source interface type port/trunk */
    CPSS_INTERFACE_TYPE_ENT type;

    /** target interface port/trunk */
    GT_U32 trgPortTrunk;

    /** @brief PDU header byte value
     *  pcktType      - packet type
     */
    GT_U8 pduHeaderByteValue;

    PRV_TGF_PCL_EGRESS_PKT_TYPE_ENT pktType;

} PRV_TGF_OAM_EPCL_RULE_CONFIG_STC;

/**
* @internal prvTgfOamConfigurationSet function
* @endinternal
*
* @brief   Set OAM common configuration
*
* @param[in] oamConfigPtr             - pointer to common OAM configuration
*                                       None
*/
GT_VOID prvTgfOamConfigurationSet
(
    IN PRV_TGF_OAM_COMMON_CONFIG_STC * oamConfigPtr
);

/**
* @internal prvTgfOamConfigurationGet function
* @endinternal
*
* @brief   Get OAM common configuration
*
* @param[in] oamConfigPtr             - pointer to common OAM configuration
*
* @param[out] oamConfigPtr             - pointer to OAM entry.
*                                       None
*/
GT_VOID prvTgfOamConfigurationGet
(
    OUT PRV_TGF_OAM_COMMON_CONFIG_STC * oamConfigPtr
);

/**
* @internal prvTgfOamTransmitSetup function
* @endinternal
*
* @brief   Set transmission parameters
*
* @param[in] pduHeaderDataPtr         - pointer to PDU header data structure
* @param[in] sendPort                 - send port
*                                       None
*/
GT_VOID prvTgfOamTransmitSetup
(
    IN  PRV_TGF_OAM_PDU_HEADER_DATA_STC  * pduHeaderDataPtr,
    IN  GT_U8 sendPort
);

/**
* @internal prvTgfOamTransmitPacket function
* @endinternal
*
* @brief   Set transmission parameters and send packets
*
* @param[in] pduHeaderDataPtr         - pointer to PDU header data structure
* @param[in] sendPort                 - send port
* @param[in] burstCount               - packet burst count
* @param[in] sleepTime                - transmit timings in milliseconds
*                                       None
*/
GT_VOID prvTgfOamTransmitPacket
(
    IN  PRV_TGF_OAM_PDU_HEADER_DATA_STC  * pduHeaderDataPtr,
    IN  GT_U8 sendPort,
    IN  GT_U32 burstCount,
    IN  GT_U32 sleepTime
);

/**
* @internal prvTgfOamRxPduHeaderParse function
* @endinternal
*
* @brief   Parse OAM PDU header from Rx table
*
* @param[in] rxPacketPtr              - pointer to Rx packet
* @param[in,out] pduHeaderDataPtr         - pointer to PDU header data structure
* @param[in,out] pduHeaderDataPtr         - pointer to PDU header data structure
*                                       None
*/
GT_VOID prvTgfOamRxPduHeaderParse
(
    IN    TGF_PACKET_STC * rxPacketPtr,
    INOUT PRV_TGF_OAM_PDU_HEADER_DATA_STC   * pduHeaderDataPtr
);

/**
* @internal prvTgfDefOamGeneralConfigurationSet function
* @endinternal
*
* @brief   Set Default OAM general configuration.
*/
GT_VOID prvTgfDefOamGeneralConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfDefOamGeneralConfigurationRestore function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfDefOamGeneralConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfDefOamGeneralTrafficCheck function
* @endinternal
*
* @brief   Check traffic results.
*
* @param[in] sendPort                 - send interface index
* @param[in] exceptionType            - exception type
* @param[in] expectedFlood            - flooded packets
* @param[in] expectedTrap             - trapped packets
* @param[in] expectedException        - number of exceptions
* @param[in] expectedBilling          - number of excpected billing counters
*                                       None
*/
GT_VOID prvTgfDefOamGeneralTrafficCheck
(
    IN GT_U32 sendPort,
    IN PRV_TGF_OAM_EXCEPTION_TYPE_ENT exceptionType,
    IN GT_U32 expectedFlood,
    IN GT_U32 expectedTrap,
    IN GT_U32 expectedException,
    IN GT_U32 expectedBilling
);

/**
* @internal prvTgfDefOamGeneralMelTrafficTest function
* @endinternal
*
* @brief   MEG level check
*/
GT_VOID prvTgfDefOamGeneralMelTrafficTest
(
    GT_VOID
);

/**
* @internal prvTgfDefOamGeneralSourceInterfaceTest function
* @endinternal
*
* @brief   Source Interface Verification
*/
GT_VOID prvTgfDefOamGeneralSourceInterfaceTest
(
    GT_VOID
);

/**
* @internal prvTgfDefOamGeneralDualEndedLossMeasurementTest function
* @endinternal
*
* @brief   Dual-Ended Loss Measurement Packets Test
*/
GT_VOID prvTgfDefOamGeneralDualEndedLossMeasurementTest
(
    GT_VOID
);

/**
* @internal prvTgfOamPacketHeaderDataSet function
* @endinternal
*
* @brief   Set OAM packet header part
*
* @param[in] packetHeaderPartType     - packet header part type
* @param[in,out] packetHeaderPartPtr      - pointer to packet header part
*                                       None
*/
GT_VOID prvTgfOamPacketHeaderDataSet
(
    IN    TGF_PACKET_PART_ENT  packetHeaderPartType,
    INOUT GT_VOID             *packetHeaderPartPtr
);

/**
* @internal prvTgfOamPacketHeaderDataReset function
* @endinternal
*
* @brief   Reset OAM packet header part
*/
GT_VOID prvTgfOamPacketHeaderDataReset
(
    GT_VOID
);

/**
* @internal prvTgfOamPacketHeaderMacSwap function
* @endinternal
*
* @brief   Swap MAC addresses in packet header
*/
GT_VOID prvTgfOamPacketHeaderMacSwap
(
     GT_VOID
);

/**
* @internal prvTgfOamPacketHeaderSrcMacByteSet function
* @endinternal
*
* @brief   Set source MAC addresses in packet header
*/
GT_VOID prvTgfOamPacketHeaderSrcMacByteSet
(
     GT_U32     byteIndex,
     GT_U8      byteValue
);

/**
* @internal prvTgfOamGenericUseTtiForOam function
* @endinternal
*
* @brief   Set mode for the test: using TTI/IPCL for the OAM profile and flowId assignment
*         in any case the IPCL is used for other parameters setting.
* @param[in] useTtiForOam             - GT_TRUE - use TTI  for the OAM profile and flowId assignment
*                                      GT_TRUE - use IPCL for the OAM profile and flowId assignment
*                                       None
*/
GT_VOID prvTgfOamGenericUseTtiForOam
(
    IN GT_BOOL  useTtiForOam
);

/**
* @internal prvTgfOamBridgeConfigurationSet function
* @endinternal
*
* @brief   Set OAM test bridge configuration
*/
GT_VOID prvTgfOamBridgeConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfOamBridgeConfigurationRestore function
* @endinternal
*
* @brief   Restore OAM test bridge configuration
*/
GT_VOID prvTgfOamBridgeConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfOamGeneralh */



