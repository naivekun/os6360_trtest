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
* @file cpssDxChCnc.h
*
* @brief CPSS DxCh Centralized Counters (CNC) API.
*
* @version   33
********************************************************************************
*/

#ifndef __cpssDxChCnch
#define __cpssDxChCnch

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* get public types */
#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/common/cpssTypes.h>

/**
* @enum CPSS_DXCH_CNC_CLIENT_ENT
 *
 * @brief CNC clients.
*/
typedef enum{

    /** L2/L3 Ingress VLAN */
    CPSS_DXCH_CNC_CLIENT_L2L3_INGRESS_VLAN_E,

    /** @brief Ingress PCL0 lookup 0
     *  For Bobcat2, Caelum, Bobcat3, Aldrin, AC3X relates parallel sublookup0.
     */
    CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E,

    /** @brief Ingress PCL0 lookup 1
     *  (APPLICABLE DEVICES: xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  For Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X relates parallel sublookup0.
     */
    CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E,

    /** @brief Ingress PCL1 lookup
     *  For Bobcat2, Caelum, Bobcat3, Aldrin, AC3X relates parallel sublookup0.
     */
    CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E,

    /** Ingress VLAN Pass/Drop */
    CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E,

    /** Egress VLAN Pass/Drop */
    CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E,

    /** @brief Egress Queue Pass/Drop
     *  Note: For Lion and above devices
     *  the CN messages may be counted instead
     *  of Egress Queue Pass/Drop, if CN mode enabled.
     */
    CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E,

    /** @brief Egress PCL
     *  For Bobcat2, Caelum, Bobcat3, Aldrin, AC3X relates parallel sublookup0.
     */
    CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_E,

    /** ARP Table access */
    CPSS_DXCH_CNC_CLIENT_ARP_TABLE_ACCESS_E,

    /** Tunnel Start */
    CPSS_DXCH_CNC_CLIENT_TUNNEL_START_E,

    /** @brief TTI (TTI action index)
     *  For Bobcat2, Caelum, Bobcat3, Aldrin, AC3X relates parallel sublookup0.
     */
    CPSS_DXCH_CNC_CLIENT_TTI_E,

    /** @brief 'source ePort' client - count per source EPort
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CNC_CLIENT_INGRESS_SRC_EPORT_E,

    /** @brief 'taregt ePort' client - count per target EPort
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CNC_CLIENT_EGRESS_TRG_EPORT_E,

    /** @brief TTI Parallel lookup0(TTI action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  TTI Parallel lookup2(TTI action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X)
     *  NOTE: Bobcat3; Aldrin2 uses 'TTI_PARALLEL_2' for 'sublookup2'
     */
    CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_0_E,

    /** @brief TTI Parallel lookup1(TTI action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  TTI Parallel lookup3(TTI action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X)
     *  NOTE: Bobcat3; Aldrin2 uses 'TTI_PARALLEL_3' for 'sublookup3'
     */
    CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_1_E,

    /** @brief Ingress PCL Lookup0, Parallel sublookup0(PCL action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E,

    /** @brief Ingress PCL Lookup0, Parallel sublookup1(PCL action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_1_E,

    /** @brief Ingress PCL Lookup0, Parallel sublookup2(PCL action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_2_E,

    /** @brief Ingress PCL Lookup0, Parallel sublookup3(PCL action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E,

    /** @brief Ingress PCL Lookup1, Parallel sublookup0(PCL action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E,

    /** @brief Ingress PCL Lookup1, Parallel sublookup1(PCL action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_1_E,

    /** @brief Ingress PCL Lookup1, Parallel sublookup2(PCL action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_2_E,

    /** @brief Ingress PCL Lookup1, Parallel sublookup3(PCL action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E,

    /** @brief Ingress PCL Lookup2, Parallel sublookup0(PCL action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E,

    /** @brief Ingress PCL Lookup2, Parallel sublookup1(PCL action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_1_E,

    /** @brief Ingress PCL Lookup2, Parallel sublookup2(PCL action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_2_E,

    /** @brief Ingress PCL Lookup2, Parallel sublookup3(PCL action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E,

    /** @brief Egress PCL Parallel sublookup0(PCL action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_0_E,

    /** @brief Egress PCL Parallel sublookup1(PCL action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_1_E,

    /** @brief Egress PCL Parallel sublookup2(PCL action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_2_E,

    /** @brief Egress PCL Parallel sublookup3(PCL action counter index)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_3_E,

    /** @brief Preegress counting packets by packet command.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E,

    /** @brief Traffic Manager counting packets by TM Queue Id command.
     *  (APPLICABLE DEVICES: Bobcat2)
     */
    CPSS_DXCH_CNC_CLIENT_TM_PASS_DROP_E,

    /** @brief TTI Parallel lookup2(TTI action counter index)
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_2_E,

    /** @brief TTI Parallel lookup2(TTI action counter index)
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_3_E,

    /** amount of enum members */
    CPSS_DXCH_CNC_CLIENT_LAST_E

} CPSS_DXCH_CNC_CLIENT_ENT;

/**
* @enum CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT
 *
 * @brief CNC modes of Egress Queue counting.
*/
typedef enum{

    /** count egress queue pass and taildropped packets. */
    CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E,

    /** count Congestion Notification messages. */
    CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E,

    /** @brief count egress queue pass and taildropped packets with unaware DP.
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_REDUCED_E

} CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT;

/**
* @enum CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT
 *
 * @brief CNC modes for byte count counters.
*/
typedef enum{

    /** @brief The Byte Count counter counts the
     *  entire packet byte count for all packet type
     */
    CPSS_DXCH_CNC_BYTE_COUNT_MODE_L2_E,

    /** @brief Byte Count counters counts the
     *  packet L3 fields (the entire packet
     *  minus the L3 offset) and only
     *  the passenger part for tunnel-terminated
     *  packets or tunnel-start packets.
     */
    CPSS_DXCH_CNC_BYTE_COUNT_MODE_L3_E

} CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT;

/**
* @enum CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT
 *
 * @brief CNC modes Egress VLAN Drop counting.
*/
typedef enum{

    /** Dropcounter counts egressfiltered and taildropped traffic */
    CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_EGRESS_FILTER_AND_TAIL_DROP_E,

    /** Dropcounter counts egress filtered traffic only */
    CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_EGRESS_FILTER_ONLY_E,

    /** Dropcounter counts taildrop only */
    CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_TAIL_DROP_ONLY_E

} CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT;

/**
* @enum CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT
 *
 * @brief CNC counting enabled unit
*/
typedef enum{

    /** @brief enable counting for TTI
     *  client.
     */
    CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_TTI_E,

    /** @brief enable counting for extended
     *  PCL unit (this unit includes
     *  PCL0_0, PCL0_1, PCL1 & L2/L3
     *  Ingress VLAN clients).
     */
    CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_PCL_E

} CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT;


/**
* @enum CPSS_DXCH_CNC_COUNTER_FORMAT_ENT
 *
 * @brief CNC modes of counter formats.
*/
typedef enum{

    /** @brief PKT_29_BC_35;
     *  Partitioning of the 64 bits entry is as following:
     *  Packets counter: 29 bits, Byte Count counter: 35 bits
     */
    CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E,

    /** @brief PKT_27_BC_37;
     *  Partitioning of the 64 bits entry is as following:
     *  Packets counter: 27 bits, Byte Count counter: 37 bits
     */
    CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_1_E,

    /** @brief PKT_37_BC_27;
     *  Partitioning of the 64 bits entry is as following:
     *  Packets counter: 37 bits, Byte Count counter: 27 bits
     */
    CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_2_E,

    /** @brief PKT_64_BC_0;
     *  Partitioning of the 64 bits entry is as following:
     *  Packets counter: 64 bits, Byte Count counter: 0 bits
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_3_E,

    /** @brief PKT_0_BC_64
     *  Partitioning of the 64 bits entry is as following:
     *  Packets counter: 0 bits, Byte Count counter: 64 bits
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E

} CPSS_DXCH_CNC_COUNTER_FORMAT_ENT;

/**
* @enum CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT
 *
 * @brief Index calculation modes of Ingress VLAN Pass/Drop
 * and Egress VLAN Pass/DropCNC Clients.
*/
typedef enum{

    /** original packet VID */
    CPSS_DXCH_CNC_VLAN_INDEX_MODE_ORIGINAL_VID_E,

    /** assigned eVid */
    CPSS_DXCH_CNC_VLAN_INDEX_MODE_EVID_E,

    /** tag1 VID */
    CPSS_DXCH_CNC_VLAN_INDEX_MODE_TAG1_VID_E

} CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT;

/**
* @enum CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT
 *
 * @brief Index calculation modes of To CPU packets for Packet Type Pass/Drop CNC Client.
*/
typedef enum{

    /** use local physical source port */
    CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_SRC_PORT_E,

    /** use CPU Code */
    CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_CPU_CODE_E

} CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT;

/**
* @enum CPSS_DXCH_CNC_TM_INDEX_MODE_ENT
 *
 * @brief Index calculation modes for Packet Traffic Manager CNC Client.
*/
typedef enum{

    /** Mode0: index is (queue_id[13:0],pass/drop) */
    CPSS_DXCH_CNC_TM_INDEX_MODE_0_E,

    /** Mode1: index is (queue_id[12:0],DP[0],pass/drop) */
    CPSS_DXCH_CNC_TM_INDEX_MODE_1_E,

    /** Mode2: index is (queue_id[11:0],DP[1:0],pass/drop) */
    CPSS_DXCH_CNC_TM_INDEX_MODE_2_E,

    /** Mode3: index is (queue_id[12:0],DP[1],pass/drop) */
    CPSS_DXCH_CNC_TM_INDEX_MODE_3_E

} CPSS_DXCH_CNC_TM_INDEX_MODE_ENT;

/**
* @struct CPSS_DXCH_CNC_COUNTER_STC
 *
 * @brief The counter entry contents.
*/
typedef struct{

    /** byte count */
    GT_U64 byteCount;

    /** @brief packets count
     *  Comment:
     *  See the possible counter HW formats of Lion and above devices in
     *  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT description.
     *  For DxCh3,xCat and xCat3 devices the byte counter has 35 bits,
     *  the packets counter has 29 bits
     */
    GT_U64 packetCount;

} CPSS_DXCH_CNC_COUNTER_STC;

/**
* @internal cpssDxChCncBlockClientEnableSet function
* @endinternal
*
* @brief   The function binds/unbinds the selected client to/from a counter block.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
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
GT_STATUS cpssDxChCncBlockClientEnableSet
(
    IN  GT_U8                     devNum,
    IN  GT_U32                    blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT  client,
    IN  GT_BOOL                   updateEnable
);

/**
* @internal cpssDxChCncBlockClientEnableGet function
* @endinternal
*
* @brief   The function gets bind/unbind of the selected client to a counter block.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
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
GT_STATUS cpssDxChCncBlockClientEnableGet
(
    IN  GT_U8                     devNum,
    IN  GT_U32                    blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT  client,
    OUT GT_BOOL                   *updateEnablePtr
);

/**
* @internal cpssDxChCncBlockClientRangesSet function
* @endinternal
*
* @brief   The function sets index ranges per CNC client and Block
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
*                                      (APPLICABLE DEVICES DxCh3; xCat; xCat3; Lion; xCat2; Lion2).
* @param[in] indexRangesBmp           - the counter index ranges bitmap
*                                      DxCh3,xCat and xCat3 devices have 10 ranges of counter indexes,
*                                      each 2048 indexes (APPLICABLE RANGES: 0..2K-1, 2K..4K-1, ,18K..20K-1)
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (2048 counters)
*                                      Lion devices has 64 ranges (512 indexes each).
*                                      xCat2 devices has 64 ranges (256 indexes each).
*                                      Lion2 devices have 32 ranges (512 indexes each).
*                                      Allowed to map more then one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
*                                      For Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X (APPLICABLE RANGES: 0..1K, 1K..2K-1,  ,63K..64K-1).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on not relevant bits in indexRangesBmp
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncBlockClientRangesSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT    client,
    IN  GT_U64                      indexRangesBmp
);

/**
* @internal cpssDxChCncBlockClientRangesGet function
* @endinternal
*
* @brief   The function gets index ranges per CNC client and Block
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
*                                      (APPLICABLE DEVICES DxCh3; xCat; xCat3; Lion; xCat2; Lion2).
*
* @param[out] indexRangesBmpPtr        - (pointer to) the counter index ranges bitmap
*                                      DxCh3,xCat and xCat3 devices have 10 ranges of counter indexes,
*                                      each 2048 indexes (APPLICABLE RANGES: 0..2K-1, 2K..4K-1, ,18K..20K-1)
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (2048 counters)
*                                      Lion devices has 64 ranges (512 indexes each).
*                                      xCat2 devices has 64 ranges (256 indexes each).
*                                      Lion2 devices have 32 ranges (512 indexes each).
*                                      Allowed to map more then one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
*                                      For Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X (APPLICABLE RANGES: 0..1K, 1K..2K-1,  ,63K..64K-1).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncBlockClientRangesGet
(
    IN  GT_U8                     devNum,
    IN  GT_U32                    blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT  client,
    OUT GT_U64                    *indexRangesBmpPtr
);

/**
* @internal cpssDxChCncPortClientEnableSet function
* @endinternal
*
* @brief   The function sets the given client Enable counting per port.
*         Currently only L2/L3 Ingress Vlan client supported.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] client                   - CNC client
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncPortClientEnableSet
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT  client,
    IN  GT_BOOL                   enable
);

/**
* @internal cpssDxChCncPortClientEnableGet function
* @endinternal
*
* @brief   The function gets the given client Enable counting per port.
*         Currently only L2/L3 Ingress Vlan client supported.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
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
GT_STATUS cpssDxChCncPortClientEnableGet
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT  client,
    OUT GT_BOOL                   *enablePtr
);

/**
* @internal cpssDxChCncIngressVlanPassDropFromCpuCountEnableSet function
* @endinternal
*
* @brief   The function enables or disables counting of FROM_CPU packets
*         for the Ingress Vlan Pass/Drop CNC client.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncIngressVlanPassDropFromCpuCountEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enable
);

/**
* @internal cpssDxChCncIngressVlanPassDropFromCpuCountEnableGet function
* @endinternal
*
* @brief   The function gets status of counting of FROM_CPU packets
*         for the Ingress Vlan Pass/Drop CNC client.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
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
GT_STATUS cpssDxChCncIngressVlanPassDropFromCpuCountEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChCncVlanClientIndexModeSet function
* @endinternal
*
* @brief   The function sets index calculation mode
*         for the Vlan CNC clients.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanClient               - VLAN client
*                                      relevant clients:
*                                      CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E
*                                      CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E
* @param[in] indexMode                - index calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncVlanClientIndexModeSet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT          vlanClient,
    IN  CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT indexMode
);

/**
* @internal cpssDxChCncVlanClientIndexModeGet function
* @endinternal
*
* @brief   The function gets index calculation mode
*         for the Vlan CNC clients.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanClient               - VLAN client
*                                      relevant clients:
*                                      CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E
*                                      CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E
*
* @param[out] indexModePtr             - (pointer to)index calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_STATE             - enexpected HW value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncVlanClientIndexModeGet
(
    IN   GT_U8                             devNum,
    IN   CPSS_DXCH_CNC_CLIENT_ENT          vlanClient,
    OUT  CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT *indexModePtr
);

/**
* @internal cpssDxChCncPacketTypePassDropToCpuModeSet function
* @endinternal
*
* @brief   The function sets index calculation mode of To CPU packets
*         for the Packet Type Pass/Drop CNC client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] toCpuMode                - to CPU packets index calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncPacketTypePassDropToCpuModeSet
(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT toCpuMode
);

/**
* @internal cpssDxChCncPacketTypePassDropToCpuModeGet function
* @endinternal
*
* @brief   The function gets index calculation mode of To CPU packets
*         for the Packet Type Pass/Drop CNC client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] toCpuModePtr             - to CPU packets index calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncPacketTypePassDropToCpuModeGet
(
    IN  GT_U8                                               devNum,
    OUT CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT *toCpuModePtr
);

/**
* @internal cpssDxChCncTmClientIndexModeSet function
* @endinternal
*
* @brief   The function sets index calculation mode
*         for the Traffic manager CNC clients.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] indexMode                - index calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncTmClientIndexModeSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_CNC_TM_INDEX_MODE_ENT indexMode
);

/**
* @internal cpssDxChCncTmClientIndexModeGet function
* @endinternal
*
* @brief   The function gets index calculation mode
*         for the Traffic manager CNC clients.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
*
* @param[out] indexModePtr             - (pointer to)index calculation mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncTmClientIndexModeGet
(
    IN   GT_U8                           devNum,
    OUT  CPSS_DXCH_CNC_TM_INDEX_MODE_ENT *indexModePtr
);

/**
* @internal cpssDxChCncClientByteCountModeSet function
* @endinternal
*
* @brief   The function sets byte count counter mode for CNC client.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] client                   - CNC client
* @param[in] countMode                - count mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncClientByteCountModeSet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT          client,
    IN  CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT countMode
);

/**
* @internal cpssDxChCncClientByteCountModeGet function
* @endinternal
*
* @brief   The function gets byte count counter mode for CNC client.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
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
GT_STATUS cpssDxChCncClientByteCountModeGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT          client,
    OUT CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT *countModePtr
);

/**
* @internal cpssDxChCncEgressVlanDropCountModeSet function
* @endinternal
*
* @brief   The function sets Egress VLAN Drop counting mode.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - Egress VLAN Drop counting mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncEgressVlanDropCountModeSet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT mode
);

/**
* @internal cpssDxChCncEgressVlanDropCountModeGet function
* @endinternal
*
* @brief   The function gets Egress VLAN Drop counting mode.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Falcon.
*
* @param[in] devNum                   - device number
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
GT_STATUS cpssDxChCncEgressVlanDropCountModeGet
(
    IN  GT_U8                                    devNum,
    OUT CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT *modePtr
);

/**
* @internal cpssDxChCncCounterClearByReadEnableSet function
* @endinternal
*
* @brief   The function enable/disables clear by read mode of CNC counters read
*         operation.
*         If clear by read mode is disable the counters after read
*         keep the current value and continue to count normally.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncCounterClearByReadEnableSet
(
    IN  GT_U8    devNum,
    IN  GT_BOOL  enable
);

/**
* @internal cpssDxChCncCounterClearByReadEnableGet function
* @endinternal
*
* @brief   The function gets clear by read mode status of CNC counters read
*         operation.
*         If clear by read mode is disable the counters after read
*         keep the current value and continue to count normally.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
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
GT_STATUS cpssDxChCncCounterClearByReadEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChCncCounterClearByReadValueSet function
* @endinternal
*
* @brief   The function sets the counter clear by read globally configured value.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion and above
* @param[in] counterPtr               - (pointer to) counter contents
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncCounterClearByReadValueSet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    IN  CPSS_DXCH_CNC_COUNTER_STC         *counterPtr
);

/**
* @internal cpssDxChCncCounterClearByReadValueGet function
* @endinternal
*
* @brief   The function gets the counter clear by read globally configured value.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] format                   - CNC counter HW format
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
GT_STATUS cpssDxChCncCounterClearByReadValueGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    OUT CPSS_DXCH_CNC_COUNTER_STC         *counterPtr
);

/**
* @internal cpssDxChCncCounterWraparoundEnableSet function
* @endinternal
*
* @brief   The function enables/disables wraparound for all CNC counters
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncCounterWraparoundEnableSet
(
    IN  GT_U8    devNum,
    IN  GT_BOOL  enable
);

/**
* @internal cpssDxChCncCounterWraparoundEnableGet function
* @endinternal
*
* @brief   The function gets status of wraparound for all CNC counters
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
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
GT_STATUS cpssDxChCncCounterWraparoundEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChCncCounterWraparoundIndexesGet function
* @endinternal
*
* @brief   The function gets the counter Wrap Around last 8 indexes
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
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
GT_STATUS cpssDxChCncCounterWraparoundIndexesGet
(
    IN    GT_U8    devNum,
    IN    GT_U32   blockNum,
    INOUT GT_U32   *indexNumPtr,
    OUT   GT_U32   indexesArr[]
);

/**
* @internal cpssDxChCncCounterSet function
* @endinternal
*
* @brief   The function sets the counter contents
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] index                    - counter  in the block
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion and above
* @param[in] counterPtr               - (pointer to) counter contents
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncCounterSet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            blockNum,
    IN  GT_U32                            index,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    IN  CPSS_DXCH_CNC_COUNTER_STC         *counterPtr
);

/**
* @internal cpssDxChCncCounterGet function
* @endinternal
*
* @brief   The function gets the counter contents
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] index                    - counter  in the block
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter HW format
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
GT_STATUS cpssDxChCncCounterGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            blockNum,
    IN  GT_U32                            index,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    OUT CPSS_DXCH_CNC_COUNTER_STC         *counterPtr
);

/**
* @internal cpssDxChCncBlockUploadTrigger function
* @endinternal
*
* @brief   The function triggers the Upload of the given counters block.
*         The function checks that there is no unfinished CNC and FDB upload (FU).
*         Also the function checks that all FU messages of previous FU were retrieved
*         by cpssDxChBrgFdbFuMsgBlockGet.
*         An application may check that CNC upload finished by
*         cpssDxChCncBlockUploadInProcessGet.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*         Note: Bobcat3 has CNC unit per port group.
*         Triggering upload from both CNC ports groups in parallel is not supported.
*         Application must choose a single Port-Group at a time.
*         (need to use cpssDxChCncPortGroupBlockUploadTrigger(...))
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Bobcat3.
*
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
*       must retrieve all CNC messages from FUQ by cpssDxChCncUploadedBlockGet.
*
*/
GT_STATUS cpssDxChCncBlockUploadTrigger
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      blockNum
);

/**
* @internal cpssDxChCncBlockUploadInProcessGet function
* @endinternal
*
* @brief   The function gets bitmap of numbers of such counters blocks that upload
*         of them yet in process. The HW cannot keep more then one block in such
*         state, but an API matches the original HW representation of the state.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
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
GT_STATUS cpssDxChCncBlockUploadInProcessGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                     *inProcessBlocksBmpPtr
);

/**
* @internal cpssDxChCncUploadedBlockGet function
* @endinternal
*
* @brief   The function return a block (array) of CNC counter values,
*         the maximal number of elements defined by the caller.
*         The CNC upload may triggered by cpssDxChCncBlockUploadTrigger.
*         The CNC upload transfers whole CNC block (2K CNC counters)
*         to FDB Upload queue. An application must get all transferred counters.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*         The device may transfer only part of CNC block because of FUQ full. In
*         this case the cpssDxChCncUploadedBlockGet may return only part of the
*         CNC block with return GT_OK. An application must to call
*         cpssDxChCncUploadedBlockGet one more time to get rest of the block.
*         Note: Bobcat3 has CNC unit per port group.
*         Getting messages from both CNC ports groups in parallel is not supported.
*         Application must choose a single Port-Group at a time.
*         (need to use cpssDxChCncPortGroupUploadedBlockGet(...))
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Bobcat3.
*
* @param[in] devNum                   - the device number from which FU are taken
* @param[in,out] numOfCounterValuesPtr    - (pointer to) maximal number of CNC counters
*                                      values to get.This is the size of
* @param[in] counterValuesPtr         array allocated by caller.
* @param[in] format                   - CNC counter HW format
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
* @retval GT_NOT_READY             - Upload started after upload trigger or
*                                       continued after queue rewind but yet
*                                       not paused due to queue full and yet not finished.
*                                       The part of queue memory does not contain uploaded counters yet.
*                                       No counters retrieved.
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - on wrong devNum
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
*       must retrieve all CNC messages from FUQ by cpssDxChCncUploadedBlockGet.
*
*/
GT_STATUS cpssDxChCncUploadedBlockGet
(
    IN     GT_U8                             devNum,
    INOUT  GT_U32                            *numOfCounterValuesPtr,
    IN     CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    OUT    CPSS_DXCH_CNC_COUNTER_STC         *counterValuesPtr
);

/**
* @internal cpssDxChCncCountingEnableSet function
* @endinternal
*
* @brief   The function enables counting on selected cnc unit.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] cncUnit                  - selected unit for enable\disable counting
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncCountingEnableSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT  cncUnit,
    IN  GT_BOOL                                 enable
);

/**
* @internal cpssDxChCncCountingEnableGet function
* @endinternal
*
* @brief   The function gets enable counting status on selected cnc unit.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
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
GT_STATUS cpssDxChCncCountingEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT  cncUnit,
    OUT GT_BOOL                                 *enablePtr
);

/**
* @internal cpssDxChCncCounterFormatSet function
* @endinternal
*
* @brief   The function sets format of CNC counter
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncCounterFormatSet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            blockNum,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format
);

/**
* @internal cpssDxChCncCounterFormatGet function
* @endinternal
*
* @brief   The function gets format of CNC counter
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
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
GT_STATUS cpssDxChCncCounterFormatGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            blockNum,
    OUT CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  *formatPtr
);

/**
* @internal cpssDxChCncEgressQueueClientModeSet function
* @endinternal
*
* @brief   The function sets Egress Queue client counting mode
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - Egress Queue client counting mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncEgressQueueClientModeSet
(
    IN  GT_U8                                      devNum,
    IN  CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT mode
);

/**
* @internal cpssDxChCncEgressQueueClientModeGet function
* @endinternal
*
* @brief   The function gets Egress Queue client counting mode
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) Egress Queue client counting mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncEgressQueueClientModeGet
(
    IN  GT_U8                                      devNum,
    OUT CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT *modePtr
);

/**
* @internal cpssDxChCncCpuAccessStrictPriorityEnableSet function
* @endinternal
*
* @brief   The function enables strict priority of CPU access to counter blocks
*         Disable: Others clients have strict priority
*         Enable: CPU has strict priority
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncCpuAccessStrictPriorityEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
);

/**
* @internal cpssDxChCncCpuAccessStrictPriorityEnableGet function
* @endinternal
*
* @brief   The function gets enable status of strict priority of
*         CPU access to counter blocks
*         Disable: Others clients have strict priority
*         Enable: CPU has strict priority
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncCpuAccessStrictPriorityEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr
);

/**
* @internal cpssDxChCncPortGroupBlockClientEnableSet function
* @endinternal
*
* @brief   The function binds/unbinds the selected client to/from a counter block.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
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
GT_STATUS cpssDxChCncPortGroupBlockClientEnableSet
(
    IN  GT_U8                     devNum,
    IN  GT_PORT_GROUPS_BMP        portGroupsBmp,
    IN  GT_U32                    blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT  client,
    IN  GT_BOOL                   updateEnable
);

/**
* @internal cpssDxChCncPortGroupBlockClientEnableGet function
* @endinternal
*
* @brief   The function gets bind/unbind of the selected client to a counter block.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
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
GT_STATUS cpssDxChCncPortGroupBlockClientEnableGet
(
    IN  GT_U8                     devNum,
    IN  GT_PORT_GROUPS_BMP        portGroupsBmp,
    IN  GT_U32                    blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT  client,
    OUT GT_BOOL                   *updateEnablePtr
);

/**
* @internal cpssDxChCncPortGroupBlockClientRangesSet function
* @endinternal
*
* @brief   The function sets index ranges per CNC client and Block
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
*                                      (APPLICABLE DEVICES DxCh3; xCat; xCat3; Lion; xCat2; Lion2).
* @param[in] indexRangesBmp           - bitmap of counter index ranges
*                                      DxCh3,xCat and xCat3 devices have 10 ranges of counter indexes,
*                                      each 2048 indexes (APPLICABLE RANGES: 0..2K-1, 2K..4K-1, ,18K..20K-1)
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (2048 counters)
*                                      Lion devices has 64 ranges (512 indexes each).
*                                      xCat2 devices has 64 ranges (256 indexes each).
*                                      Lion2 devices have 32 ranges (512 indexes each).
*                                      Allowed to map more then one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
*                                      For Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X (APPLICABLE RANGES: 0..1K, 1K..2K-1,  ,63K..64K-1).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on not relevant bits in indexRangesBmp
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncPortGroupBlockClientRangesSet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT    client,
    IN  GT_U64                      indexRangesBmp
);

/**
* @internal cpssDxChCncPortGroupBlockClientRangesGet function
* @endinternal
*
* @brief   The function gets index ranges per CNC client and Block
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
*                                      (APPLICABLE DEVICES DxCh3; xCat; xCat3; Lion; xCat2; Lion2).
*
* @param[out] indexRangesBmpPtr        - (pointer to) bitmap counter index ranges
*                                      DxCh3,xCat and xCat3 devices have 10 ranges of counter indexes,
*                                      each 2048 indexes (APPLICABLE RANGES: 0..2K-1, 2K..4K-1, ,18K..20K-1)
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (2048 counters)
*                                      Lion devices has 64 ranges (512 indexes each).
*                                      xCat2 devices has 64 ranges (256 indexes each).
*                                      Lion2 devices have 32 ranges (512 indexes each).
*                                      Allowed to map more then one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
*                                      For Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X (APPLICABLE RANGES: 0..1K, 1K..2K-1,  ,63K..64K-1).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncPortGroupBlockClientRangesGet
(
    IN  GT_U8                     devNum,
    IN  GT_PORT_GROUPS_BMP        portGroupsBmp,
    IN  GT_U32                    blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT  client,
    OUT GT_U64                    *indexRangesBmpPtr
);

/**
* @internal cpssDxChCncPortGroupCounterWraparoundIndexesGet function
* @endinternal
*
* @brief   The function gets the counter Wrap Around last 8 indexes
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
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
GT_STATUS cpssDxChCncPortGroupCounterWraparoundIndexesGet
(
    IN    GT_U8                   devNum,
    IN    GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN    GT_U32                  blockNum,
    INOUT GT_U32                  *indexNumPtr,
    OUT   GT_U32                  portGroupIdArr[],
    OUT   GT_U32                  indexesArr[]
);

/**
* @internal cpssDxChCncPortGroupCounterSet function
* @endinternal
*
* @brief   The function sets the counter contents
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] index                    - counter  in the block
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion and above
* @param[in] counterPtr               - (pointer to) counter contents
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCncPortGroupCounterSet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  GT_U32                            blockNum,
    IN  GT_U32                            index,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    IN  CPSS_DXCH_CNC_COUNTER_STC         *counterPtr
);

/**
* @internal cpssDxChCncPortGroupCounterGet function
* @endinternal
*
* @brief   The function gets the counter contents
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] index                    - counter  in the block
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter HW format
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
GT_STATUS cpssDxChCncPortGroupCounterGet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  GT_U32                            blockNum,
    IN  GT_U32                            index,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    OUT CPSS_DXCH_CNC_COUNTER_STC         *counterPtr
);

/**
* @internal cpssDxChCncPortGroupBlockUploadTrigger function
* @endinternal
*
* @brief   The function triggers the Upload of the given counters block.
*         The function checks that there is no unfinished CNC and FDB upload (FU).
*         Also the function checks that all FU messages of previous FU were retrieved
*         by cpssDxChBrgFdbFuMsgBlockGet.
*         An application may check that CNC upload finished by
*         cpssDxChCncPortGroupBlockUploadInProcessGet.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      Note: Bobcat3 has CNC unit per port group.
*                                      Triggering upload from both CNC ports groups in parallel is not supported.
*                                      Application must choose a single Port-Group at a time.
*                                      meaning that only value 'BIT_0' or 'BIT_1' are supported.
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
*       must retrieve all CNC messages from FUQ by cpssDxChCncPortGroupUploadedBlockGet.
*
*/
GT_STATUS cpssDxChCncPortGroupBlockUploadTrigger
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      blockNum
);

/**
* @internal cpssDxChCncPortGroupBlockUploadInProcessGet function
* @endinternal
*
* @brief   The function gets bitmap of numbers of such counters blocks that upload
*         of them yet in process. The HW cannot keep more then one block in such
*         state, but an API matches the original HW representation of the state.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
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
GT_STATUS cpssDxChCncPortGroupBlockUploadInProcessGet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    OUT GT_U32                      *inProcessBlocksBmpPtr
);

/**
* @internal cpssDxChCncPortGroupUploadedBlockGet function
* @endinternal
*
* @brief   The function return a block (array) of CNC counter values,
*         the maximal number of elements defined by the caller.
*         The CNC upload may triggered by cpssDxChCncPortGroupBlockUploadTrigger.
*         The CNC upload transfers whole CNC block (2K CNC counters)
*         to FDB Upload queue. An application must get all transferred counters.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*         The device may transfer only part of CNC block because of FUQ full. In
*         this case the cpssDxChCncPortGroupUploadedBlockGet may return only part of the
*         CNC block with return GT_OK. An application must to call
*         cpssDxChCncPortGroupUploadedBlockGet one more time to get rest of the block.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - the device number from which FU are taken
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      Note: Bobcat3 has CNC unit per port group.
*                                      Getting messages from both CNC ports groups in parallel is not supported.
*                                      Application must choose a single Port-Group at a time.
*                                      meaning that only value 'BIT_0' or 'BIT_1' are supported.
* @param[in,out] numOfCounterValuesPtr    - (pointer to) maximal number of CNC counters
*                                      values to get.This is the size of
* @param[in] counterValuesPtr         array allocated by caller.
* @param[in] format                   - CNC counter HW format
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
* @retval GT_NOT_READY             - Upload started after upload trigger or
*                                       continued after queue rewind but yet
*                                       not paused due to queue full and yet not finished.
*                                       The part of queue memory does not contain uploaded counters yet.
*                                       No counters retrieved.
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - on wrong devNum
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
*       must retrieve all CNC messages from FUQ by cpssDxChCncPortGroupUploadedBlockGet.
*
*/
GT_STATUS cpssDxChCncPortGroupUploadedBlockGet
(
    IN     GT_U8                             devNum,
    IN     GT_PORT_GROUPS_BMP                portGroupsBmp,
    INOUT  GT_U32                            *numOfCounterValuesPtr,
    IN     CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    OUT    CPSS_DXCH_CNC_COUNTER_STC         *counterValuesPtr
);

/**
* @internal cpssDxChCncPortGroupCounterFormatSet function
* @endinternal
*
* @brief   The function sets format of CNC counter
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
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
GT_STATUS cpssDxChCncPortGroupCounterFormatSet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  GT_U32                            blockNum,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format
);

/**
* @internal cpssDxChCncPortGroupCounterFormatGet function
* @endinternal
*
* @brief   The function gets format of CNC counter
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
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
GT_STATUS cpssDxChCncPortGroupCounterFormatGet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  GT_U32                            blockNum,
    OUT CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  *formatPtr
);

/**
* @internal cpssDxChCncOffsetForNatClientSet function
* @endinternal
*
* @brief   Set the CNC offset for NAT client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] offset                   - CNC  for NAT client
*                                      (APPLICABLE RANGES: 0..65535)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - on out off range offset
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The CNC clients ARP and NAT are muxed (since for a
*       given packet the user can access ARP entry or NAT entry).
*       This offset is added to the NAT when sending the pointer to the CNC.
*
*/
GT_STATUS cpssDxChCncOffsetForNatClientSet
(
    IN   GT_U8    devNum,
    IN   GT_U32   offset
);

/**
* @internal cpssDxChCncOffsetForNatClientGet function
* @endinternal
*
* @brief   Get the CNC offset for NAT client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] offsetPtr                - (pointer to) CNC offset for NAT client
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - on illegal pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The CNC clients ARP and NAT are muxed between them (since for a
*       given packet the user can access ARP entry or NAT entry).
*       This offset is added to the NAT when sending the pointer to the CNC.
*
*/
GT_STATUS cpssDxChCncOffsetForNatClientGet
(
    IN   GT_U8    devNum,
    OUT  GT_U32   *offsetPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChCnch */

