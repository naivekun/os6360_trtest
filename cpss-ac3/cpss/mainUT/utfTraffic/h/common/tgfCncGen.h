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
* @file tgfCncGen.h
*
* @brief Generic API for CNC Technology facility API.
*
* @version   5
********************************************************************************
*/

#ifndef __tgfCncGenh
#define __tgfCncGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* get public types */
#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/common/cpssTypes.h>

/**
* @enum PRV_TGF_CNC_CLIENT_ENT
 *
 * @brief CNC clients.
*/
typedef enum{

    /** L2/L3 Ingress VLAN */
    PRV_TGF_CNC_CLIENT_L2L3_INGRESS_VLAN_E,

    /** Ingress PCL0 lookup 0 */
    PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E,

    /** @brief Ingress PCL0 lookup 1
     *  (APPLICABLE DEVICES:
     *  DxChXcat and above)
     */
    PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E,

    /** Ingress PCL1 lookup */
    PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E,

    /** Ingress VLAN Pass/Drop */
    PRV_TGF_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E,

    /** Egress VLAN Pass/Drop */
    PRV_TGF_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E,

    /** @brief Egress Queue Pass/Drop
     *  Note: For Lion and above devices
     *  the CN messages may be counted instead
     *  of Egress Queue Pass/Drop, if CN mode enabled.
     */
    PRV_TGF_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E,

    /** Egress PCL */
    PRV_TGF_CNC_CLIENT_EGRESS_PCL_E,

    /** ARP Table access */
    PRV_TGF_CNC_CLIENT_ARP_TABLE_ACCESS_E,

    /** Tunnel Start */
    PRV_TGF_CNC_CLIENT_TUNNEL_START_E,

    /** TTI */
    PRV_TGF_CNC_CLIENT_TTI_E,

    /** @brief 'source ePort' client - count per source EPort
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    PRV_TGF_CNC_CLIENT_INGRESS_SRC_EPORT_E,

    /** @brief 'taregt ePort' client - count per target EPort
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    PRV_TGF_CNC_CLIENT_EGRESS_TRG_EPORT_E,

    /** @brief TTI Parallel lookup0(TTI action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  TTI Parallel lookup2(TTI action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X)
     *  NOTE: Bobcat3; Aldrin2 uses 'TTI_PARALLEL_2' for 'sublookup2'
     */
    PRV_TGF_CNC_CLIENT_TTI_PARALLEL_0_E,

    /** @brief TTI Parallel lookup1(TTI action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  TTI Parallel lookup3(TTI action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X)
     *  NOTE: Bobcat3; Aldrin2 uses 'TTI_PARALLEL_3' for 'sublookup3'
     */
    PRV_TGF_CNC_CLIENT_TTI_PARALLEL_1_E,

    /** Ingress PCL Lookup0, Parallel sublookup0(PCL action counter index) */
    PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E,

    /** Ingress PCL Lookup0, Parallel sublookup1(PCL action counter index) */
    PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_1_E,

    /** Ingress PCL Lookup0, Parallel sublookup2(PCL action counter index) */
    PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_2_E,

    /** Ingress PCL Lookup0, Parallel sublookup3(PCL action counter index) */
    PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E,

    /** Ingress PCL Lookup1, Parallel sublookup0(PCL action counter index) */
    PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E,

    /** Ingress PCL Lookup1, Parallel sublookup1(PCL action counter index) */
    PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_1_E,

    /** Ingress PCL Lookup1, Parallel sublookup2(PCL action counter index) */
    PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_2_E,

    /** Ingress PCL Lookup1, Parallel sublookup3(PCL action counter index) */
    PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E,

    /** Ingress PCL Lookup2, Parallel sublookup0(PCL action counter index) */
    PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E,

    /** Ingress PCL Lookup2, Parallel sublookup1(PCL action counter index) */
    PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_1_E,

    /** @brief Ingress PCL Lookup2, Parallel sublookup2(PCL action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_2_E,

    /** Ingress PCL Lookup2, Parallel sublookup3(PCL action counter index) */
    PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E,

    /** Egress PCL Parallel sublookup0(PCL action counter index) */
    PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_0_E,

    /** Egress PCL Parallel sublookup1(PCL action counter index) */
    PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_1_E,

    /** Egress PCL Parallel sublookup2(PCL action counter index) */
    PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_2_E,

    /** Egress PCL Parallel sublookup3(PCL action counter index) */
    PRV_TGF_CNC_CLIENT_EGRESS_PCL_PARALLEL_3_E,

    /** Preegress counting packets by packet command. */
    PRV_TGF_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E,

    /** Traffic Manager counting packets by TM Queue Id command. */
    PRV_TGF_CNC_CLIENT_TM_PASS_DROP_E,

    /** @brief TTI Parallel lookup2(TTI action counter index)
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon)
     */
    PRV_TGF_CNC_CLIENT_TTI_PARALLEL_2_E,

    /** @brief TTI Parallel lookup2(TTI action counter index)
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon)
     */
    PRV_TGF_CNC_CLIENT_TTI_PARALLEL_3_E,

    /** amount of enum members */
    PRV_TGF_CNC_CLIENT_LAST_E

} PRV_TGF_CNC_CLIENT_ENT;

/**
* @enum PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT
 *
 * @brief CNC modes of Egress Queue counting.
*/
typedef enum{

    /** count egress queue pass and taildropped packets. */
    PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E,

    /** count Congestion Notification messages. */
    PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E

} PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT;

/**
* @enum PRV_TGF_CNC_BYTE_COUNT_MODE_ENT
 *
 * @brief CNC modes for byte count counters.
*/
typedef enum{

    /** @brief The Byte Count counter counts the
     *  entire packet byte count for all packet type
     */
    PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E,

    /** @brief Byte Count counters counts the
     *  packet L3 fields (the entire packet
     *  minus the L3 offset) and only
     *  the passenger part for tunnel-terminated
     *  packets or tunnel-start packets.
     */
    PRV_TGF_CNC_BYTE_COUNT_MODE_L3_E

} PRV_TGF_CNC_BYTE_COUNT_MODE_ENT;

/**
* @enum PRV_TGF_CNC_EGRESS_DROP_COUNT_MODE_ENT
 *
 * @brief CNC modes Egress VLAN Drop counting.
*/
typedef enum{

    /** Dropcounter counts egressfiltered and taildropped traffic */
    PRV_TGF_CNC_EGRESS_DROP_COUNT_MODE_EGRESS_FILTER_AND_TAIL_DROP_E,

    /** Dropcounter counts egress filtered traffic only */
    PRV_TGF_CNC_EGRESS_DROP_COUNT_MODE_EGRESS_FILTER_ONLY_E,

    /** Dropcounter counts taildrop only */
    PRV_TGF_CNC_EGRESS_DROP_COUNT_MODE_TAIL_DROP_ONLY_E

} PRV_TGF_CNC_EGRESS_DROP_COUNT_MODE_ENT;

/**
* @enum PRV_TGF_CNC_COUNTING_ENABLE_UNIT_ENT
 *
 * @brief CNC counting enabled unit
*/
typedef enum{

    /** @brief enable counting for TTI
     *  client.
     */
    PRV_TGF_CNC_COUNTING_ENABLE_UNIT_TTI_E,

    /** @brief enable counting for extended
     *  PCL unit (this unit includes
     *  PCL0_0, PCL0_1, PCL1 & L2/L3
     *  Ingress VLAN clients).
     */
    PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E,

    /** @brief enable Header Alternation Client.
     *  (TS and ARP headers)
     */
    PRV_TGF_CNC_COUNTING_ENABLE_UNIT_HA_E,

    /** @brief enable counting for
     *  Egress PCL client
     */
    PRV_TGF_CNC_COUNTING_ENABLE_UNIT_EGRESS_PCL_E

} PRV_TGF_CNC_COUNTING_ENABLE_UNIT_ENT;


/**
* @enum PRV_TGF_CNC_COUNTER_FORMAT_ENT
 *
 * @brief CNC modes of counter formats.
*/
typedef enum{

    /** @brief PKT_29_BC_35;
     *  Partitioning of the 64 bits entry is as following:
     *  Packets counter: 29 bits, Byte Count counter: 35 bits
     */
    PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E,

    /** @brief PKT_27_BC_37;
     *  Partitioning of the 64 bits entry is as following:
     *  Packets counter: 27 bits, Byte Count counter: 37 bits
     */
    PRV_TGF_CNC_COUNTER_FORMAT_MODE_1_E,

    /** @brief PKT_37_BC_27;
     *  Partitioning of the 64 bits entry is as following:
     *  Packets counter: 37 bits, Byte Count counter: 27 bits
     */
    PRV_TGF_CNC_COUNTER_FORMAT_MODE_2_E

} PRV_TGF_CNC_COUNTER_FORMAT_ENT;

/**
* @enum PRV_TGF_CNC_VLAN_INDEX_MODE_ENT
 *
 * @brief Index calculation modes of Ingress VLAN Pass/Drop
 * and Egress VLAN Pass/DropCNC Clients.
*/
typedef enum{

    /** original packet VID */
    PRV_TGF_CNC_VLAN_INDEX_MODE_ORIGINAL_VID_E,

    /** assigned eVid */
    PRV_TGF_CNC_VLAN_INDEX_MODE_EVID_E,

    /** tag1 VID */
    PRV_TGF_CNC_VLAN_INDEX_MODE_TAG1_VID_E

} PRV_TGF_CNC_VLAN_INDEX_MODE_ENT;

/**
* @enum PRV_TGF_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT
 *
 * @brief Index calculation modes of To CPU packets for Packet Type Pass/Drop CNC Client.
*/
typedef enum{

    /** use local physical source port */
    PRV_TGF_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_SRC_PORT_E,

    /** use CPU Code */
    PRV_TGF_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_CPU_CODE_E

} PRV_TGF_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT;

/**
* @struct PRV_TGF_CNC_COUNTER_STC
 *
 * @brief The counter entry contents.
*/
typedef struct{

    /** byte count */
    GT_U64 byteCount;

    /** @brief packets count
     *  Comment:
     *  See the possible counter HW formats of Lion and above devices in
     *  PRV_TGF_CNC_COUNTER_FORMAT_ENT description.
     *  For DxCh3 and DxChXcat devices the byte counter has 35 bits,
     *  the packets counter has 29 bits
     */
    GT_U64 packetCount;

} PRV_TGF_CNC_COUNTER_STC;

/**
* @internal prvTgfCncBlockClientEnableSet function
* @endinternal
*
* @brief   The function binds/unbinds the selected client to/from a counter block.
*
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
* @param[in] updateEnable             - the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*                                      It is forbidden to enable update the same
*                                      block by more then one client. When an
*                                      application enables some client it is responsible
*                                      to disable all other clients it enabled before
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncBlockClientEnableSet
(
    IN  GT_U32                    blockNum,
    IN  PRV_TGF_CNC_CLIENT_ENT    client,
    IN  GT_BOOL                   updateEnable
);

/**
* @internal prvTgfCncBlockClientEnableGet function
* @endinternal
*
* @brief   The function gets bind/unbind of the selected client to a counter block.
*
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   -  CNC client
*
* @param[out] updateEnablePtr          - (pointer to) the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*                                      It is forbidden to enable update the same
*                                      block by more then one client. When an
*                                      application enables some client it is responsible
*                                      to disable all other clients it enabled before
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncBlockClientEnableGet
(
    IN  GT_U32                    blockNum,
    IN  PRV_TGF_CNC_CLIENT_ENT    client,
    OUT GT_BOOL                   *updateEnablePtr
);

/**
* @internal prvTgfCncBlockClientRangesSet function
* @endinternal
*
* @brief   The function sets index ranges per CNC client and Block
*
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
* @param[in] rangesBmpArr[]           - the counter index ranges bitmap array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on not relevant bits in indexRangesBmp
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncBlockClientRangesSet
(
    IN  GT_U32                      blockNum,
    IN  PRV_TGF_CNC_CLIENT_ENT      client,
    IN  GT_U32                      rangesBmpArr[]
);

/**
* @internal prvTgfCncBlockClientRangesGet function
* @endinternal
*
* @brief   The function gets index ranges per CNC client and Block
*
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
*
* @param[out] rangesBmpArr[]           - the counter index ranges bitmap array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncBlockClientRangesGet
(
    IN  GT_U32                    blockNum,
    IN  PRV_TGF_CNC_CLIENT_ENT    client,
    OUT GT_U32                    rangesBmpArr[]
);

/**
* @internal prvTgfCncPortClientEnableSet function
* @endinternal
*
* @brief   The function sets the given client Enable counting per port.
*         Currently only L2/L3 Ingress Vlan client supported.
* @param[in] portIndex                - port index in UTF array
* @param[in] client                   - CNC client
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncPortClientEnableSet
(
    IN  GT_U8                     portIndex,
    IN  PRV_TGF_CNC_CLIENT_ENT    client,
    IN  GT_BOOL                   enable
);

/**
* @internal prvTgfCncPortClientEnableSetByDevPort function
* @endinternal
*
* @brief   The function sets the given client Enable counting per port.
*         Currently only L2/L3 Ingress Vlan and INGRESS_SRC_EPORT clients supported.
* @param[in] portNum                  - port Number
* @param[in] client                   - CNC client
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncPortClientEnableSetByDevPort
(
    IN  GT_U8                     devNum,
    IN  GT_U32                    portNum,
    IN  PRV_TGF_CNC_CLIENT_ENT    client,
    IN  GT_BOOL                   enable
);

/**
* @internal prvTgfCncPortClientEnableGet function
* @endinternal
*
* @brief   The function gets the given client Enable counting per port.
*         Currently only L2/L3 Ingress Vlan client supported.
* @param[in] portIndex                - port index in UTF array
* @param[in] client                   - CNC client
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncPortClientEnableGet
(
    IN  GT_U8                     portIndex,
    IN  PRV_TGF_CNC_CLIENT_ENT    client,
    OUT GT_BOOL                   *enablePtr
);

/**
* @internal prvTgfCncIngressVlanPassDropFromCpuCountEnableSet function
* @endinternal
*
* @brief   The function enables or disables counting of FROM_CPU packets
*         for the Ingress Vlan Pass/Drop CNC client.
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncIngressVlanPassDropFromCpuCountEnableSet
(
    IN  GT_BOOL enable
);

/**
* @internal prvTgfCncIngressVlanPassDropFromCpuCountEnableGet function
* @endinternal
*
* @brief   The function gets status of counting of FROM_CPU packets
*         for the Ingress Vlan Pass/Drop CNC client.
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncIngressVlanPassDropFromCpuCountEnableGet
(
    OUT GT_BOOL *enablePtr
);

/**
* @internal prvTgfCncClientByteCountModeSet function
* @endinternal
*
* @brief   The function sets byte count counter mode for CNC client.
*
* @param[in] client                   - CNC client
* @param[in] countMode                - count mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncClientByteCountModeSet
(
    IN  PRV_TGF_CNC_CLIENT_ENT            client,
    IN  PRV_TGF_CNC_BYTE_COUNT_MODE_ENT   countMode
);

/**
* @internal prvTgfCncClientByteCountModeGet function
* @endinternal
*
* @brief   The function gets byte count counter mode for CNC client.
*
* @param[in] client                   - CNC client
*
* @param[out] countModePtr             - (pointer to) count mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncClientByteCountModeGet
(
    IN  PRV_TGF_CNC_CLIENT_ENT            client,
    OUT PRV_TGF_CNC_BYTE_COUNT_MODE_ENT   *countModePtr
);

/**
* @internal prvTgfCncEgressVlanDropCountModeSet function
* @endinternal
*
* @brief   The function sets Egress VLAN Drop counting mode.
*
* @param[in] mode                     - Egress VLAN Drop counting mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncEgressVlanDropCountModeSet
(
    IN  PRV_TGF_CNC_EGRESS_DROP_COUNT_MODE_ENT   mode
);

/**
* @internal prvTgfCncEgressVlanDropCountModeGet function
* @endinternal
*
* @brief   The function gets Egress VLAN Drop counting mode.
*
* @param[out] modePtr                  - (pointer to) Egress VLAN Drop counting mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_STATE             - on reserved value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncEgressVlanDropCountModeGet
(
    OUT PRV_TGF_CNC_EGRESS_DROP_COUNT_MODE_ENT   *modePtr
);

/**
* @internal prvTgfCncVlanClientIndexModeSet function
* @endinternal
*
* @brief   The function sets index calculation mode
*         for the Vlan CNC clients.
* @param[in] vlanClient               - VLAN client
*                                      relevant clients:
*                                      PRV_TGF_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E
*                                      PRV_TGF_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E
* @param[in] indexMode                - index calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncVlanClientIndexModeSet
(
    IN  PRV_TGF_CNC_CLIENT_ENT            vlanClient,
    IN  PRV_TGF_CNC_VLAN_INDEX_MODE_ENT   indexMode
);

/**
* @internal prvTgfCncVlanClientIndexModeGet function
* @endinternal
*
* @brief   The function Gets the index calculation mode
*         for the Vlan CNC clients.
* @param[in] vlanClient               - VLAN client
*                                      relevant clients:
*                                      PRV_TGF_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E
*                                      PRV_TGF_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E
* @param[in] indexModePtr             - (pointer to) index calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncVlanClientIndexModeGet
(
    IN  PRV_TGF_CNC_CLIENT_ENT            vlanClient,
    OUT  PRV_TGF_CNC_VLAN_INDEX_MODE_ENT   *indexModePtr
);

/**
* @internal prvTgfCncPacketTypePassDropToCpuModeSet function
* @endinternal
*
* @brief   The function sets index calculation mode of To CPU packets
*         for the Packet Type Pass/Drop CNC client.
* @param[in] toCpuMode                - to CPU packets index calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncPacketTypePassDropToCpuModeSet
(
    IN  PRV_TGF_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT   toCpuMode
);

/**
* @internal prvTgfCncCounterClearByReadEnableSet function
* @endinternal
*
* @brief   The function enable/disables clear by read mode of CNC counters read
*         operation.
*         If clear by read mode is disable the counters after read
*         keep the current value and continue to count normally.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCounterClearByReadEnableSet
(
    IN  GT_BOOL  enable
);

/**
* @internal prvTgfCncCounterClearByReadEnableGet function
* @endinternal
*
* @brief   The function gets clear by read mode status of CNC counters read
*         operation.
*         If clear by read mode is disable the counters after read
*         keep the current value and continue to count normally.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCounterClearByReadEnableGet
(
    OUT GT_BOOL  *enablePtr
);

/**
* @internal prvTgfCncCounterClearByReadValueSet function
* @endinternal
*
* @brief   The function sets the counter clear by read globally configured value.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
* @param[in] format                   - CNC counter HW format,
*                                      relevant only for Lion and above
* @param[in] counterPtr               - (pointer to) counter contents
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCounterClearByReadValueSet
(
    IN  PRV_TGF_CNC_COUNTER_FORMAT_ENT    format,
    IN  PRV_TGF_CNC_COUNTER_STC           *counterPtr
);

/**
* @internal prvTgfCncCounterClearByReadValueGet function
* @endinternal
*
* @brief   The function gets the counter clear by read globally configured value.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
* @param[in] format                   - CNC counter HW format,
*                                      relevant only for Lion and above
*
* @param[out] counterPtr               - (pointer to) counter contents
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCounterClearByReadValueGet
(
    IN  PRV_TGF_CNC_COUNTER_FORMAT_ENT    format,
    OUT PRV_TGF_CNC_COUNTER_STC           *counterPtr
);

/**
* @internal prvTgfCncCounterWraparoundEnableSet function
* @endinternal
*
* @brief   The function enables/disables wraparound for all CNC counters
*
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCounterWraparoundEnableSet
(
    IN  GT_BOOL  enable
);

/**
* @internal prvTgfCncCounterWraparoundEnableGet function
* @endinternal
*
* @brief   The function gets status of wraparound for all CNC counters
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCounterWraparoundEnableGet
(
    OUT GT_BOOL  *enablePtr
);

/**
* @internal prvTgfCncCounterWraparoundIndexesGet function
* @endinternal
*
* @brief   The function gets the counter Wrap Around last 8 indexes
*
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in,out] indexNumPtr              - (pointer to) maximal size of array of indexes
* @param[in,out] indexNumPtr              - (pointer to) actual size of array of indexes
*
* @param[out] indexesArr[]             - (pointer to) array of indexes of counters wrapped around
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The wraparound indexes are cleared on read.
*       Since in the HW each pair of wraparound indexes resides in one register,
*       cleared by read, in case not all the 8 possible indexes are read at once,
*       some data may be lost.
*       Recommendation: Always request 8 indexes => indexNumPtr=8 as input.
*
*/
GT_STATUS prvTgfCncCounterWraparoundIndexesGet
(
    IN    GT_U32   blockNum,
    INOUT GT_U32   *indexNumPtr,
    OUT   GT_U32   indexesArr[]
);

/**
* @internal prvTgfCncCounterSet function
* @endinternal
*
* @brief   The function sets the counter contents
*
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] index                    - counter  in the block
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter HW format,
*                                      relevant only for Lion and above
* @param[in] counterPtr               - (pointer to) counter contents
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCounterSet
(
    IN  GT_U32                            blockNum,
    IN  GT_U32                            index,
    IN  PRV_TGF_CNC_COUNTER_FORMAT_ENT    format,
    IN  PRV_TGF_CNC_COUNTER_STC           *counterPtr
);

/**
* @internal prvTgfCncCounterGet function
* @endinternal
*
* @brief   The function gets the counter contents
*
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] index                    - counter  in the block
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter HW format,
*                                      relevant only for Lion and above
*
* @param[out] counterPtr               - (pointer to) received CNC counter value.
*                                      For multi port group devices
*                                      the result counter contains the sum of
*                                      counters read from all port groups.
*                                      It contains more bits than in HW.
*                                      For example the sum of 4 35-bit values may be
*                                      37-bit value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCounterGet
(
    IN  GT_U32                            blockNum,
    IN  GT_U32                            index,
    IN  PRV_TGF_CNC_COUNTER_FORMAT_ENT    format,
    OUT PRV_TGF_CNC_COUNTER_STC           *counterPtr
);

/**
* @internal prvTgfCncBlockUploadTrigger function
* @endinternal
*
* @brief   The function triggers the Upload of the given counters block.
*         The function checks that there is no unfinished CNC and FDB upload (FU).
*         Also the function checks that all FU messages of previous FU were retrieved
*         by cpssDxChBrgFdbFuMsgBlockGet.
*         An application may check that CNC upload finished by
*         prvTgfCncBlockUploadInProcessGet.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - if the previous CNC upload or FU in process or
*                                       FU is finished but all FU messages were not
*                                       retrieved yet by cpssDxChBrgFdbFuMsgBlockGet.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device use same resource the FDB upload queue (FUQ) for both CNC and
*       FDB uploads. The function cpssDxChHwPpPhase2Init configures parameters for
*       FUQ. The fuqUseSeparate (ppPhase2ParamsPtr) parameter must be set GT_TRUE
*       to enable CNC upload.
*       There are limitations for FDB and CNC uploads if an application use
*       both of them:
*       1. After triggering of FU and before start of CNC upload an application
*       must retrieve all FU messages from FUQ by cpssDxChBrgFdbFuMsgBlockGet.
*       2. After start of CNC upload and before triggering of FU an application
*       must retrieve all CNC messages from FUQ by prvTgfCncUploadedBlockGet.
*
*/
GT_STATUS prvTgfCncBlockUploadTrigger
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      blockNum
);

/**
* @internal prvTgfCncBlockUploadInProcessGet function
* @endinternal
*
* @brief   The function gets bitmap of numbers of such counters blocks that upload
*         of them yet in process. The HW cannot keep more then one block in such
*         state, but an API matches the original HW representation of the state.
* @param[in] devNum                   - device number
*
* @param[out] inProcessBlocksBmpPtr    - (pointer to) bitmap of in-process blocks
*                                      value 1 of the bit#n means that CNC upload
*                                      is not finished yet for block n
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncBlockUploadInProcessGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *inProcessBlocksBmpPtr
);

/**
* @internal prvTgfCncUploadedBlockGet function
* @endinternal
*
* @brief   The function return a block (array) of CNC counter values,
*         the maximal number of elements defined by the caller.
*         The CNC upload may triggered by prvTgfCncBlockUploadTrigger.
*         The CNC upload transfers whole CNC block (2K CNC counters)
*         to FDB Upload queue. An application must get all transferred counters.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*         The device may transfer only part of CNC block because of FUQ full. In
*         this case the prvTgfCncUploadedBlockGet may return only part of the
*         CNC block with return GT_OK. An application must to call
*         prvTgfCncUploadedBlockGet one more time to get rest of the block.
* @param[in] devNum                   - device number
* @param[in,out] numOfCounterValuesPtr    - (pointer to) maximal number of CNC counters
*                                      values to get.This is the size of
* @param[in] counterValuesPtr         array allocated by caller.
* @param[in] format                   - CNC counter HW format,
*                                      relevant only for Lion and above
* @param[in,out] numOfCounterValuesPtr    - (pointer to) actual number of CNC
*                                      counters values in counterValuesPtr.
*
* @param[out] counterValuesPtr         - (pointer to) array that holds received CNC
*                                      counters values. Array must be allocated by
*                                      caller. For multi port group devices
*                                      the result counters contain the sum of
*                                      counters read from all port groups.
*                                      It contains more bits than in HW.
*                                      For example the sum of 4 35-bit values may be
*                                      37-bit value.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       CNC counter value
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - if the previous FU messages were not
*                                       retrieved yet by cpssDxChBrgFdbFuMsgBlockGet.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device use same resource the FDB upload queue (FUQ) for both CNC and
*       FDB uploads. The function cpssDxChHwPpPhase2Init configures parameters for
*       FUQ. The fuqUseSeparate (ppPhase2ParamsPtr) parameter must be set GT_TRUE
*       to enable CNC upload.
*       There are limitations for FDB and CNC uploads if an application use
*       both of them:
*       1. After triggering of FU and before start of CNC upload an application
*       must retrieve all FU messages from FUQ by cpssDxChBrgFdbFuMsgBlockGet.
*       2. After start of CNC upload and before triggering of FU an application
*       must retrieve all CNC messages from FUQ by prvTgfCncUploadedBlockGet.
*
*/
GT_STATUS prvTgfCncUploadedBlockGet
(
    IN  GT_U8                                devNum,
    INOUT  GT_U32                            *numOfCounterValuesPtr,
    IN     PRV_TGF_CNC_COUNTER_FORMAT_ENT    format,
    OUT    PRV_TGF_CNC_COUNTER_STC           *counterValuesPtr
);

/**
* @internal prvTgfCncCountingEnableSet function
* @endinternal
*
* @brief   The function enables counting on selected cnc unit.
*
* @param[in] cncUnit                  - selected unit for enable\disable counting
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCountingEnableSet
(
    IN  PRV_TGF_CNC_COUNTING_ENABLE_UNIT_ENT    cncUnit,
    IN  GT_BOOL                                 enable
);

/**
* @internal prvTgfCncCountingEnableGet function
* @endinternal
*
* @brief   The function gets enable counting status on selected cnc unit.
*
* @param[in] cncUnit                  - selected unit for enable\disable counting
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCountingEnableGet
(
    IN  PRV_TGF_CNC_COUNTING_ENABLE_UNIT_ENT    cncUnit,
    OUT GT_BOOL                                 *enablePtr
);

/**
* @internal prvTgfCncCounterFormatSet function
* @endinternal
*
* @brief   The function sets format of CNC counter
*
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCounterFormatSet
(
    IN  GT_U32                            blockNum,
    IN  PRV_TGF_CNC_COUNTER_FORMAT_ENT    format
);

/**
* @internal prvTgfCncCounterFormatGet function
* @endinternal
*
* @brief   The function gets format of CNC counter
*
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
*
* @param[out] formatPtr                - (pointer to) CNC counter format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       on not supported client for device.
*/
GT_STATUS prvTgfCncCounterFormatGet
(
    IN  GT_U32                            blockNum,
    OUT PRV_TGF_CNC_COUNTER_FORMAT_ENT    *formatPtr
);

/**
* @internal prvTgfCncEgressQueueClientModeSet function
* @endinternal
*
* @brief   The function sets Egress Queue client counting mode
*
* @param[in] mode                     - Egress Queue client counting mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncEgressQueueClientModeSet
(
    IN  PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT   mode
);

/**
* @internal prvTgfCncEgressQueueClientModeGet function
* @endinternal
*
* @brief   The function gets Egress Queue client counting mode
*
* @param[out] modePtr                  - (pointer to) Egress Queue client counting mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncEgressQueueClientModeGet
(
    OUT PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT   *modePtr
);

/**
* @internal prvTgfCncCpuAccessStrictPriorityEnableSet function
* @endinternal
*
* @brief   The function enables strict priority of CPU access to counter blocks
*         Disable: Others clients have strict priority
*         Enable: CPU has strict priority
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCpuAccessStrictPriorityEnableSet
(
    IN  GT_BOOL   enable
);

/**
* @internal prvTgfCncCpuAccessStrictPriorityEnableGet function
* @endinternal
*
* @brief   The function gets enable status of strict priority of
*         CPU access to counter blocks
*         Disable: Others clients have strict priority
*         Enable: CPU has strict priority
*
* @param[out] enablePtr                - (pointer to) GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncCpuAccessStrictPriorityEnableGet
(
    OUT GT_BOOL   *enablePtr
);

/**
* @internal prvTgfCncPortGroupBlockClientEnableSet function
* @endinternal
*
* @brief   The function binds/unbinds the selected client to/from a counter block.
*
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
* @param[in] updateEnable             - the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*                                      It is forbidden to enable update the same
*                                      block by more then one client. When an
*                                      application enables some client it is responsible
*                                      to disable all other clients it enabled before
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncPortGroupBlockClientEnableSet
(
    IN  GT_PORT_GROUPS_BMP        portGroupsBmp,
    IN  GT_U32                    blockNum,
    IN  PRV_TGF_CNC_CLIENT_ENT    client,
    IN  GT_BOOL                   updateEnable
);

/**
* @internal prvTgfCncPortGroupBlockClientEnableGet function
* @endinternal
*
* @brief   The function gets bind/unbind of the selected client to a counter block.
*
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   -  CNC client
*
* @param[out] updateEnablePtr          - (pointer to) the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*                                      It is forbidden to enable update the same
*                                      block by more then one client. When an
*                                      application enables some client it is responsible
*                                      to disable all other clients it enabled before
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncPortGroupBlockClientEnableGet
(
    IN  GT_PORT_GROUPS_BMP        portGroupsBmp,
    IN  GT_U32                    blockNum,
    IN  PRV_TGF_CNC_CLIENT_ENT    client,
    OUT GT_BOOL                   *updateEnablePtr
);

/**
* @internal prvTgfCncPortGroupBlockClientRangesSet function
* @endinternal
*
* @brief   The function sets index ranges per CNC client and Block
*
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
* @param[in] indexRangesBmp           - bitmap of counter index ranges
*                                      DxCh3 and DxChXcat devices have 8 ranges of counter indexes,
*                                      each 2048 indexes (0..(2K-1), 2K..(4K-1), ..., 14K..(16K-1))
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (2048 counters)
*                                      The Lion devices has 64 ranges (512 indexes each).
*                                      Lion2 devices have 32 ranges (512 indexes each).
*                                      Allowed to map more then one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on not relevant bits in indexRangesBmp
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncPortGroupBlockClientRangesSet
(
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      blockNum,
    IN  PRV_TGF_CNC_CLIENT_ENT      client,
    IN  GT_U64                      indexRangesBmp
);

/**
* @internal prvTgfCncPortGroupBlockClientRangesGet function
* @endinternal
*
* @brief   The function gets index ranges per CNC client and Block
*
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
*
* @param[out] indexRangesBmpPtr        - (pointer to) bitmap counter index ranges
*                                      DxCh3 and DxChXcat devices have 8 ranges of counter indexes,
*                                      each 2048 indexes (0..(2K-1), 2K..(4K-1), ..., 14K..(16K-1))
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (2048 counters)
*                                      The Lion devices has 64 ranges (512 indexes each).
*                                      Lion2 devices have 32 ranges (512 indexes each).
*                                      Allowed to map more then one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncPortGroupBlockClientRangesGet
(
    IN  GT_PORT_GROUPS_BMP        portGroupsBmp,
    IN  GT_U32                    blockNum,
    IN  PRV_TGF_CNC_CLIENT_ENT    client,
    OUT GT_U64                    *indexRangesBmpPtr
);

/**
* @internal prvTgfCncPortGroupCounterWraparoundIndexesGet function
* @endinternal
*
* @brief   The function gets the counter Wrap Around last 8 indexes
*
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in,out] indexNumPtr              - (pointer to) maximal size of array of indexes
* @param[in,out] indexNumPtr              - (pointer to) actual size of array of indexes
*
* @param[out] portGroupIdArr[]         (pointer to) array of port group Ids of
*                                      counters wrapped around.
*                                      The NULL pointer supported.
* @param[out] indexesArr[]             - (pointer to) array of indexes of counters
*                                      wrapped around
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The wraparound indexes are cleared on read.
*       Since in the HW each pair of wraparound indexes resides in one register,
*       cleared by read, in case not all the 8 possible indexes are read at once,
*       some data may be lost.
*       Recommendation: Always request 8 indexes => indexNumPtr=8 as input.
*
*/
GT_STATUS prvTgfCncPortGroupCounterWraparoundIndexesGet
(
    IN    GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN    GT_U32                  blockNum,
    INOUT GT_U32                  *indexNumPtr,
    OUT   GT_U32                  portGroupIdArr[],
    OUT   GT_U32                  indexesArr[]
);

/**
* @internal prvTgfCncPortGroupCounterSet function
* @endinternal
*
* @brief   The function sets the counter contents
*
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] index                    - counter  in the block
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter HW format,
*                                      relevant only for Lion and above
* @param[in] counterPtr               - (pointer to) counter contents
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncPortGroupCounterSet
(
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  GT_U32                            blockNum,
    IN  GT_U32                            index,
    IN  PRV_TGF_CNC_COUNTER_FORMAT_ENT    format,
    IN  PRV_TGF_CNC_COUNTER_STC           *counterPtr
);

/**
* @internal prvTgfCncPortGroupCounterGet function
* @endinternal
*
* @brief   The function gets the counter contents
*
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] index                    - counter  in the block
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter HW format,
*                                      relevant only for Lion and above
*
* @param[out] counterPtr               - (pointer to) received CNC counter value.
*                                      For multi port group devices
*                                      the result counter contains the sum of
*                                      counters read from all port groups.
*                                      It contains more bits than in HW.
*                                      For example the sum of 4 35-bit values may be
*                                      37-bit value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncPortGroupCounterGet
(
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  GT_U32                            blockNum,
    IN  GT_U32                            index,
    IN  PRV_TGF_CNC_COUNTER_FORMAT_ENT    format,
    OUT PRV_TGF_CNC_COUNTER_STC           *counterPtr
);

/**
* @internal prvTgfCncPortGroupBlockUploadTrigger function
* @endinternal
*
* @brief   The function triggers the Upload of the given counters block.
*         The function checks that there is no unfinished CNC and FDB upload (FU).
*         Also the function checks that all FU messages of previous FU were retrieved
*         by cpssDxChBrgFdbFuMsgBlockGet.
*         An application may check that CNC upload finished by
*         prvTgfCncPortGroupBlockUploadInProcessGet.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - if the previous CNC upload or FU in process or
*                                       FU is finished but all FU messages were not
*                                       retrieved yet by cpssDxChBrgFdbFuMsgBlockGet.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device use same resource the FDB upload queue (FUQ) for both CNC and
*       FDB uploads. The function cpssDxChHwPpPhase2Init configures parameters for
*       FUQ. The fuqUseSeparate (ppPhase2ParamsPtr) parameter must be set GT_TRUE
*       to enable CNC upload.
*       There are limitations for FDB and CNC uploads if an application use
*       both of them:
*       1. After triggering of FU and before start of CNC upload an application
*       must retrieve all FU messages from FUQ by cpssDxChBrgFdbFuMsgBlockGet.
*       2. After start of CNC upload and before triggering of FU an application
*       must retrieve all CNC messages from FUQ by prvTgfCncPortGroupUploadedBlockGet.
*
*/
GT_STATUS prvTgfCncPortGroupBlockUploadTrigger
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      blockNum
);

/**
* @internal prvTgfCncPortGroupBlockUploadInProcessGet function
* @endinternal
*
* @brief   The function gets bitmap of numbers of such counters blocks that upload
*         of them yet in process. The HW cannot keep more then one block in such
*         state, but an API matches the original HW representation of the state.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      Checks all active port groups of the bitmap.
*
* @param[out] inProcessBlocksBmpPtr    - (pointer to) bitmap of in-process blocks
*                                      value 1 of the bit#n means that CNC upload
*                                      is not finished yet for block n
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncPortGroupBlockUploadInProcessGet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    OUT GT_U32                      *inProcessBlocksBmpPtr
);

/**
* @internal prvTgfCncPortGroupUploadedBlockGet function
* @endinternal
*
* @brief   The function return a block (array) of CNC counter values,
*         the maximal number of elements defined by the caller.
*         The CNC upload may triggered by prvTgfCncPortGroupBlockUploadTrigger.
*         The CNC upload transfers whole CNC block (2K CNC counters)
*         to FDB Upload queue. An application must get all transferred counters.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*         The device may transfer only part of CNC block because of FUQ full. In
*         this case the prvTgfCncPortGroupUploadedBlockGet may return only part of the
*         CNC block with return GT_OK. An application must to call
*         prvTgfCncPortGroupUploadedBlockGet one more time to get rest of the block.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in,out] numOfCounterValuesPtr    - (pointer to) maximal number of CNC counters
*                                      values to get.This is the size of
* @param[in] counterValuesPtr         array allocated by caller.
* @param[in] format                   - CNC counter HW format,
*                                      relevant only for Lion and above
* @param[in,out] numOfCounterValuesPtr    - (pointer to) actual number of CNC
*                                      counters values in counterValuesPtr.
*
* @param[out] counterValuesPtr         - (pointer to) array that holds received CNC
*                                      counters values. Array must be allocated by
*                                      caller. For multi port group devices
*                                      the result counters contain the sum of
*                                      counters read from all port groups.
*                                      It contains more bits than in HW.
*                                      For example the sum of 4 35-bit values may be
*                                      37-bit value.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       CNC counter value
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - if the previous FU messages were not
*                                       retrieved yet by cpssDxChBrgFdbFuMsgBlockGet.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device use same resource the FDB upload queue (FUQ) for both CNC and
*       FDB uploads. The function cpssDxChHwPpPhase2Init configures parameters for
*       FUQ. The fuqUseSeparate (ppPhase2ParamsPtr) parameter must be set GT_TRUE
*       to enable CNC upload.
*       There are limitations for FDB and CNC uploads if an application use
*       both of them:
*       1. After triggering of FU and before start of CNC upload an application
*       must retrieve all FU messages from FUQ by cpssDxChBrgFdbFuMsgBlockGet.
*       2. After start of CNC upload and before triggering of FU an application
*       must retrieve all CNC messages from FUQ by prvTgfCncPortGroupUploadedBlockGet.
*
*/
GT_STATUS prvTgfCncPortGroupUploadedBlockGet
(
    IN     GT_U8                             devNum,
    IN     GT_PORT_GROUPS_BMP                portGroupsBmp,
    INOUT  GT_U32                            *numOfCounterValuesPtr,
    IN     PRV_TGF_CNC_COUNTER_FORMAT_ENT    format,
    OUT    PRV_TGF_CNC_COUNTER_STC           *counterValuesPtr
);

/**
* @internal prvTgfCncPortGroupCounterFormatSet function
* @endinternal
*
* @brief   The function sets format of CNC counter
*
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCncPortGroupCounterFormatSet
(
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  GT_U32                            blockNum,
    IN  PRV_TGF_CNC_COUNTER_FORMAT_ENT    format
);

/**
* @internal prvTgfCncPortGroupCounterFormatGet function
* @endinternal
*
* @brief   The function gets format of CNC counter
*
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
*
* @param[out] formatPtr                - (pointer to) CNC counter format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       on not supported client for device.
*/
GT_STATUS prvTgfCncPortGroupCounterFormatGet
(
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  GT_U32                            blockNum,
    OUT PRV_TGF_CNC_COUNTER_FORMAT_ENT    *formatPtr
);

/**
* @internal prvTgfCncFineTuningBlockAmountGet function
* @endinternal
*
* @brief   The function gets Amount of Cnc Blocks per device.
*         if Cnc not supported - 0 returned.
*/
GT_U32 prvTgfCncFineTuningBlockAmountGet
(
    GT_VOID
);

/**
* @internal prvTgfCncFineTuningBlockSizeGet function
* @endinternal
*
* @brief   The function gets Amount of Cnc Blocks per device.
*         if Cnc not supported - 0 returned.
*/
GT_U32 prvTgfCncFineTuningBlockSizeGet
(
    GT_VOID
);

/*========= Common scenarios ==============*/

/**
* @struct PRV_TGF_CNC_CONFIGURATION_STC
 *
 * @brief The CNC configuration.
*/
typedef struct
{
    /* common parametrs for all fuctions where relevant */
    PRV_TGF_CNC_COUNTING_ENABLE_UNIT_ENT         countingUnit;
    PRV_TGF_CNC_CLIENT_ENT                       clientType;
    GT_U32                                       blockNum;
    GT_U32                                       counterNum;
    PRV_TGF_CNC_COUNTER_FORMAT_ENT               counterFormat;
    /* prvTgfCncCountingEnableSet */
    GT_BOOL                                      configureCountingUnitEnable;
    GT_BOOL                                      countingUnitEnable;
    /* prvTgfCncBlockClientEnableSet */
    GT_BOOL                                      configureClientEnable;
    GT_BOOL                                      clientEnable;
    /* prvTgfCncBlockClientRangesBmpArrSet */
    GT_BOOL                                      configureIndexRangeBitMap;
    GT_U32                                       indexRangeBitMap[4];
    /* prvTgfCncCounterFormatSet */
    GT_BOOL                                      configureCounterFormat;
    /* prvTgfCncCounterSet */
    GT_BOOL                                      configureCounterValue;
    PRV_TGF_CNC_COUNTER_STC                      counterValue;
    /* prvTgfCncCounterWraparoundEnableSet */
    GT_BOOL                                      configureWraparoundEnable;
    GT_BOOL                                      wraparoundEnable;
    /* prvTgfCncCounterClearByReadEnableSet */
    GT_BOOL                                      configureClearByReadEnable;
    GT_BOOL                                      clearByReadEnable;
    /* prvTgfCncCounterClearByReadValueSet */
    GT_BOOL                                      configureClearByReadValue;
    PRV_TGF_CNC_COUNTER_STC                      clearByReadValue;
    /* prvTgfCncClientByteCountModeSet */
    GT_BOOL                                      configureByteCountMode;
    PRV_TGF_CNC_BYTE_COUNT_MODE_ENT              byteCountMode;
    /* prvTgfCncEgressVlanDropCountModeSet */
    GT_BOOL                                      configureEgressVlanDropCountMode;
    PRV_TGF_CNC_EGRESS_DROP_COUNT_MODE_ENT       egressVlanDropCountMode;
    /* prvTgfCncEgressQueueClientModeSet */
    GT_BOOL                                      configureEgressQueueClientMode;
    PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT     egressQueueClientMode;
} PRV_TGF_CNC_CONFIGURATION_STC;

/**
* @struct PRV_TGF_CNC_CHECK_STC
 *
 * @brief The CNC check.
*/
typedef struct{

    GT_U32 blockNum;

    GT_U32 counterNum;

    PRV_TGF_CNC_COUNTER_FORMAT_ENT counterFormat;

    GT_BOOL compareCounterValue;

    PRV_TGF_CNC_COUNTER_STC counterValue;

    GT_BOOL compareWraparoundIndexes;

    GT_U32 wraparoundIndexesNum;

    GT_U32 wraparoundIndexesArr[8];

} PRV_TGF_CNC_CHECK_STC;

/**
* @internal prvTgfCncGenConfigure function
* @endinternal
*
* @brief   This function sets typical Cnc configuration
*
* @param[in] cncConfigurationPtr      - (pointer to)CNC configuration data
* @param[in] stopOnErrors             - to stop after first fail (GT_TRUE, used for configuration)
*                                      or to continue (GT_FALS, use for restore).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
GT_STATUS prvTgfCncGenConfigure
(
    IN PRV_TGF_CNC_CONFIGURATION_STC     *cncConfigurationPtr,
    IN GT_BOOL                           stopOnErrors
);

/**
* @internal prvTgfCncGenCheck function
* @endinternal
*
* @brief   This function sets typical Cnc check
*
* @param[in] cncCheckPtr              - (pointer to)CNC check data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
GT_STATUS prvTgfCncGenCheck
(
    IN PRV_TGF_CNC_CHECK_STC     *cncCheckPtr
);

/**
* @internal prvTgfCncTxqClientIndexGet function
* @endinternal
*
* @brief   calculate TXQ client index.
*
* @param[in] devNum                   - device number
* @param[in] cnMode                   - GT_TRUE - CN mode, GT_FALSE - Tail Drop mode
* @param[in] droppedPacket            - 1-dropped, 0-passed
* @param[in] portNum                  - port number
* @param[in] tc                       - traffic class (0..7)
* @param[in] dp                       - drop priority (0..2)
*                                       calculated index or 0xFFFFFFFF on bad parameters
*/
GT_U32 prvTgfCncTxqClientIndexGet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL cnMode,
    IN  GT_U32  droppedPacket,
    IN  GT_U32  portNum,
    IN  GT_U32  tc,
    IN  GT_U32  dp
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfCncGenh */




