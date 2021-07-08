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
* @file cpssDxChBrgCount.h
*
* @brief CPSS DxCh Ingress Bridge Counters facility implementation.
*
* @version   12
********************************************************************************
*/

#ifndef __cpssDxChBrgCounth
#define __cpssDxChBrgCounth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/bridge/cpssGenBrgGen.h>

/**
* @enum CPSS_DXCH_BRIDGE_DROP_CNTR_MODE_ENT
 *
 * @brief Enumeration of Bridge Drop Counter reasons to be counted.
*/
typedef enum{

    /** Count All. */
    CPSS_DXCH_BRG_DROP_CNTR_COUNT_ALL_E = 0,

    /** FDB Entry command Drop. */
    CPSS_DXCH_BRG_DROP_CNTR_FDB_ENTRY_CMD_E,

    /** Unknown MAC SA Drop. */
    CPSS_DXCH_BRG_DROP_CNTR_UNKNOWN_MAC_SA_E,

    /** Invalid SA Drop. */
    CPSS_DXCH_BRG_DROP_CNTR_INVALID_SA_E,

    /** VLAN not valid Drop. */
    CPSS_DXCH_BRG_DROP_CNTR_INVALID_VLAN_E,

    /** @brief Port not Member in VLAN
     *  Drop.
     */
    CPSS_DXCH_BRG_DROP_CNTR_PORT_NOT_IN_VLAN_E,

    /** VLAN Range Drop. */
    CPSS_DXCH_BRG_DROP_CNTR_VLAN_RANGE_E,

    /** Moved Static Address Drop. */
    CPSS_DXCH_BRG_DROP_CNTR_MOVED_STATIC_ADDR_E,

    /** ARP SA mismatch Drop. */
    CPSS_DXCH_BRG_DROP_CNTR_ARP_SA_MISMATCH_E,

    /** SYN with data Drop. */
    CPSS_DXCH_BRG_DROP_CNTR_SYN_WITH_DATA_E,

    /** @brief TCP over MCAST or BCAST
     *  Drop.
     */
    CPSS_DXCH_BRG_DROP_CNTR_TCP_OVER_MC_OR_BC_E,

    /** Bridge Access Matrix Drop. */
    CPSS_DXCH_BRG_DROP_CNTR_ACCESS_MATRIX_E,

    /** Secure Learning Drop. */
    CPSS_DXCH_BRG_DROP_CNTR_SEC_LEARNING_E,

    /** @brief Acceptable Frame Type
     *  Drop.
     */
    CPSS_DXCH_BRG_DROP_CNTR_ACCEPT_FRAME_TYPE_E,

    /** Fragmented ICMP Drop. */
    CPSS_DXCH_BRG_DROP_CNTR_FRAG_ICMP_E,

    /** TCP Flags Zero Drop. */
    CPSS_DXCH_BRG_DROP_CNTR_TCP_FLAGS_ZERO_E,

    /** @brief TCP Flags FIN, URG and PSH
     *  are all set Drop.
     */
    CPSS_DXCH_BRG_DROP_CNTR_TCP_FLAGS_FUP_SET_E,

    /** @brief TCP Flags SYN and FIN are
     *  set Drop.
     */
    CPSS_DXCH_BRG_DROP_CNTR_TCP_FLAGS_SF_SET_E,

    /** @brief TCP Flags SYN and RST are
     *  set Drop.
     */
    CPSS_DXCH_BRG_DROP_CNTR_TCP_FLAGS_SR_SET_E,

    /** @brief TCP/UDP Source or
     *  Destination Port is Zero
     *  Drop.
     */
    CPSS_DXCH_BRG_DROP_CNTR_TCP_UDP_PORT_ZERO_E,

    /** VLAN MRU Drop. */
    CPSS_DXCH_BRG_DROP_CNTR_VLAN_MRU_E,

    /** Rate Limiting Drop. */
    CPSS_DXCH_BRG_DROP_CNTR_RATE_LIMIT_E,

    /** Local Port Drop. */
    CPSS_DXCH_BRG_DROP_CNTR_LOCAL_PORT_E,

    /** @brief Spanning Tree port state
     *  Drop.
     */
    CPSS_DXCH_BRG_DROP_CNTR_SPAN_TREE_PORT_ST_E,

    /** IP Multicast Drop. */
    CPSS_DXCH_BRG_DROP_CNTR_IP_MC_E,

    /** NON-IP Multicast Drop. */
    CPSS_DXCH_BRG_DROP_CNTR_NON_IP_MC_E,

    /** @brief DSATag due to Local dev
     *  Drop.
     */
    CPSS_DXCH_BRG_DROP_CNTR_DSATAG_LOCAL_DEV_E,

    /** IEEE Reserved Drop. */
    CPSS_DXCH_BRG_DROP_CNTR_IEEE_RESERVED_E,

    /** @brief Unregistered L2 NON-IPM
     *  Multicast Drop.
     */
    CPSS_DXCH_BRG_DROP_CNTR_UREG_L2_NON_IPM_MC_E,

    /** @brief Unregistered L2 IPv6
     *  Multicast Drop.
     */
    CPSS_DXCH_BRG_DROP_CNTR_UREG_L2_IPV6_MC_E,

    /** @brief Unregistered L2 IPv4
     *  Multicast Drop.
     */
    CPSS_DXCH_BRG_DROP_CNTR_UREG_L2_IPV4_MC_E,

    /** Unknown L2 Unicast Drop. */
    CPSS_DXCH_BRG_DROP_CNTR_UNKNOWN_L2_UC_E,

    /** @brief Unregistered L2 IPv4
     *  Brodcast Drop.
     */
    CPSS_DXCH_BRG_DROP_CNTR_UREG_L2_IPV4_BC_E,

    /** @brief Unregistered L2 NON-IPv4
     *  Brodcast Drop.
     *  ###########################################################################
     *  ## The following Drop Counter Modes are not supported by Dx Ch device. ##
     *  ###########################################################################
     *  -----------------------------------------------------------------------
     */
    CPSS_DXCH_BRG_DROP_CNTR_UREG_L2_NON_IPV4_BC_E

} CPSS_DXCH_BRIDGE_DROP_CNTR_MODE_ENT;


/**
* @enum CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT
 *
 * @brief Enumeration of counter set number.
*/
typedef enum{

    /** id for counter set 0 */
    CPSS_DXCH_BRG_CNT_SET_ID_0_E = 0,

    /** id for counter set 1 */
    CPSS_DXCH_BRG_CNT_SET_ID_1_E

} CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT;

/**
* @struct CPSS_DXCH_BRIDGE_HOST_CNTR_STC
 *
 * @brief Structure of Host Group counters is maintained for a CPU-
 * configured MAC source address and MAC destination address.
*/
typedef struct{

    /** @brief number of paskets (good packets only) with a
     *  MAC DA matching the CPU-configured MAC DA.
     */
    GT_U32 gtHostInPkts;

    /** @brief number of paskets (good packets only) with a
     *  MAC SA matching the CPU-configured MAC SA.
     */
    GT_U32 gtHostOutPkts;

    /** @brief number of Broadcast paskets (good packets only)
     *  with a MAC SA matching the CPU-configured MAC SA.
     */
    GT_U32 gtHostOutBroadcastPkts;

    /** @brief number of Multicast paskets (good packets only)
     *  with a MAC SA matching the CPU-configured MAC SA.
     *  Note:
     *  The "Good Packets" are error-free Ethernet packetdfs
     *  that have a valid frame length, per RFC 2819.
     */
    GT_U32 gtHostOutMulticastPkts;

} CPSS_DXCH_BRIDGE_HOST_CNTR_STC;

/**
* @internal cpssDxChBrgCntDropCntrModeSet function
* @endinternal
*
* @brief   Sets Drop Counter Mode (configures a Bridge Drop Counter "reason").
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
* @param[in] dropMode                 - Drop Counter mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum or dropMode
* @retval GT_NOT_SUPPORTED         - on non-supported dropMode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgCntDropCntrModeSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_BRIDGE_DROP_CNTR_MODE_ENT dropMode
);

/**
* @internal cpssDxChBrgCntDropCntrModeGet function
* @endinternal
*
* @brief   Gets the Drop Counter Mode (Bridge Drop Counter "reason").
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] dropModePtr              - pointer to the Drop Counter mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_SUPPORTED         - on non-supported dropMode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgCntDropCntrModeGet
(
    IN   GT_U8                                   devNum,
    OUT  CPSS_DXCH_BRIDGE_DROP_CNTR_MODE_ENT     *dropModePtr
);

/**
* @internal cpssDxChBrgCntMacDaSaSet function
* @endinternal
*
* @brief   Sets a specific MAC DA and SA to be monitored by Host
*         and Matrix counter groups on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
* @param[in] saAddrPtr                - source MAC address (MAC SA).
* @param[in] daAddrPtr                - destination MAC address (MAC DA).
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgCntMacDaSaSet
(
    IN  GT_U8           devNum,
    IN  GT_ETHERADDR    *saAddrPtr,
    IN  GT_ETHERADDR    *daAddrPtr
);

/**
* @internal cpssDxChBrgCntMacDaSaGet function
* @endinternal
*
* @brief   Gets a MAC DA and SA are monitored by Host
*         and Matrix counter groups on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] saAddrPtr                - source MAC address (MAC SA).
* @param[out] daAddrPtr                - destination MAC address (MAC DA).
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgCntMacDaSaGet
(
    IN   GT_U8           devNum,
    OUT  GT_ETHERADDR    *saAddrPtr,
    OUT  GT_ETHERADDR    *daAddrPtr
);

/**
* @internal cpssDxChBrgCntBridgeIngressCntrModeSet function
* @endinternal
*
* @brief   Configures a specified Set of Bridge Ingress
*         counters to work in requested mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrSetId                - Counter Set ID
* @param[in] setMode                  - count mode of specified Set of Bridge Ingress Counters.
* @param[in] port                     -  number monitored by Set of Counters.
*                                      This parameter is applied upon CPSS_BRG_CNT_MODE_1_E and
*                                      CPSS_BRG_CNT_MODE_3_E counter modes.
* @param[in] vlan                     - VLAN ID monitored by Counters Set.
*                                      This parameter is applied upon CPSS_BRG_CNT_MODE_2_E and
*                                      CPSS_BRG_CNT_MODE_3_E counter modes.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum, setMode or cntrSetId.
* @retval GT_OUT_OF_RANGE          - on port number or vlan out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgCntBridgeIngressCntrModeSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT    cntrSetId,
    IN  CPSS_BRIDGE_INGR_CNTR_MODES_ENT     setMode,
    IN  GT_PORT_NUM                         port,
    IN  GT_U16                              vlan
);

/**
* @internal cpssDxChBrgCntBridgeIngressCntrModeGet function
* @endinternal
*
* @brief   Gets the mode (port number and VLAN Id as well) of specified
*         Bridge Ingress counters Set.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrSetId                - Counter Set ID
*
* @param[out] setModePtr               - pointer to the count mode of specified Set of Bridge
*                                      Ingress Counters.
* @param[out] portPtr                  - pointer to the port number monitored by Set of Counters.
*                                      This parameter is applied upon CPSS_BRG_CNT_MODE_1_E and
*                                      CPSS_BRG_CNT_MODE_3_E counter modes.
*                                      This parameter can be NULL;
* @param[out] vlanPtr                  - pointer to the VLAN ID monitored by Counters Set.
*                                      This parameter is applied upon CPSS_BRG_CNT_MODE_2_E and
*                                      CPSS_BRG_CNT_MODE_3_E counter modes.
*                                      This parameter can be NULL;
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum or counter set number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgCntBridgeIngressCntrModeGet
(
    IN   GT_U8                               devNum,
    IN   CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT    cntrSetId,
    OUT  CPSS_BRIDGE_INGR_CNTR_MODES_ENT     *setModePtr,
    OUT  GT_PORT_NUM                         *portPtr,
    OUT  GT_U16                              *vlanPtr
);

/**
* @internal cpssDxChBrgCntDropCntrGet function
* @endinternal
*
* @brief   Gets the Bridge Ingress Drop Counter of specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] dropCntPtr               - pointer to the number of packets that were dropped
*                                      due to drop reason configured
*                                      by the cpssDxChBrgCntDropCntrModeSet().
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgCntDropCntrGet
(
    IN   GT_U8      devNum,
    OUT  GT_U32     *dropCntPtr
);

/**
* @internal cpssDxChBrgCntDropCntrSet function
* @endinternal
*
* @brief   Sets the Bridge Ingress Drop Counter of specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
* @param[in] dropCnt                  - number of packets that were dropped due to drop reason
*                                      configured by the cpssDxChBrgCntDropCntrModeSet().
*                                      The parameter enables an application to initialize the counter,
*                                      for the desired counter value.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum or invalid Drop Counter Value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgCntDropCntrSet
(
    IN  GT_U8      devNum,
    IN  GT_U32     dropCnt
);

/**
* @internal cpssDxChBrgCntHostGroupCntrsGet function
* @endinternal
*
* @brief   Gets Bridge Host group counters value of specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] hostGroupCntPtr          - structure with current counters value.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Bridge Host group of counters are clear-on-read.
*
*/
GT_STATUS cpssDxChBrgCntHostGroupCntrsGet
(
    IN   GT_U8                              devNum,
    OUT  CPSS_DXCH_BRIDGE_HOST_CNTR_STC     *hostGroupCntPtr
);

/**
* @internal cpssDxChBrgCntMatrixGroupCntrsGet function
* @endinternal
*
* @brief   Gets Bridge Matrix counter value of specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] matrixCntSaDaPktsPtr     - number of packets (good only) with a MAC SA/DA
*                                      matching of the CPU-configured MAC SA/DA.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Bridge Matrix counter is clear-on-read.
*
*/
GT_STATUS cpssDxChBrgCntMatrixGroupCntrsGet
(
    IN   GT_U8      devNum,
    OUT  GT_U32     *matrixCntSaDaPktsPtr
);

/**
* @internal cpssDxChBrgCntBridgeIngressCntrsGet function
* @endinternal
*
* @brief   Gets a Bridge ingress Port/VLAN/Device counters from
*         specified counter set.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
* @param[in] cntrSetId                - counter set number.
*
* @param[out] ingressCntrPtr           - structure of bridge ingress counters current values.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum, on counter set number
* @retval that is out of range of [0 -1]
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Bridge Ingress group of counters are clear-on-read.
*
*/
GT_STATUS cpssDxChBrgCntBridgeIngressCntrsGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT    cntrSetId,
    OUT CPSS_BRIDGE_INGRESS_CNTR_STC        *ingressCntrPtr
);

/**
* @internal cpssDxChBrgCntLearnedEntryDiscGet function
* @endinternal
*
* @brief   Gets the total number of source addresses the were
*         not learned due to bridge internal congestion.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] countValuePtr            - The value of the counter.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hw error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Learned Entry Discards Counter is clear-on-read.
*
*/
GT_STATUS cpssDxChBrgCntLearnedEntryDiscGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *countValuePtr
);

/**
* @internal cpssDxChBrgCntPortGroupDropCntrGet function
* @endinternal
*
* @brief   Gets the Bridge Ingress Drop Counter of specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] dropCntPtr               - pointer to the number of packets that were dropped
*                                      due to drop reason configured
*                                      by the cpssDxChBrgCntDropCntrModeSet().
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgCntPortGroupDropCntrGet
(
    IN   GT_U8               devNum,
    IN   GT_PORT_GROUPS_BMP  portGroupsBmp,
    OUT  GT_U32              *dropCntPtr
);

/**
* @internal cpssDxChBrgCntPortGroupDropCntrSet function
* @endinternal
*
* @brief   Sets the Bridge Ingress Drop Counter of specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] dropCnt                  - number of packets that were dropped due to drop reason
*                                      configured by the cpssDxChBrgCntDropCntrModeSet().
*                                      The parameter enables an application to initialize the counter,
*                                      for the desired counter value.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum or invalid Drop Counter Value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgCntPortGroupDropCntrSet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_GROUPS_BMP   portGroupsBmp,
    IN  GT_U32               dropCnt
);

/**
* @internal cpssDxChBrgCntPortGroupHostGroupCntrsGet function
* @endinternal
*
* @brief   Gets Bridge Host group counters value of specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] hostGroupCntPtr          - structure with current counters value.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Bridge Host group of counters are clear-on-read.
*
*/
GT_STATUS cpssDxChBrgCntPortGroupHostGroupCntrsGet
(
    IN   GT_U8                              devNum,
    IN   GT_PORT_GROUPS_BMP                 portGroupsBmp,
    OUT  CPSS_DXCH_BRIDGE_HOST_CNTR_STC     *hostGroupCntPtr
);

/**
* @internal cpssDxChBrgCntPortGroupMatrixGroupCntrsGet function
* @endinternal
*
* @brief   Gets Bridge Matrix counter value of specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] matrixCntSaDaPktsPtr     - number of packets (good only) with a MAC SA/DA
*                                      matching of the CPU-configured MAC SA/DA.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Bridge Matrix counter is clear-on-read.
*
*/
GT_STATUS cpssDxChBrgCntPortGroupMatrixGroupCntrsGet
(
    IN   GT_U8                     devNum,
    IN   GT_PORT_GROUPS_BMP        portGroupsBmp,
    OUT  GT_U32                    *matrixCntSaDaPktsPtr
);

/**
* @internal cpssDxChBrgCntPortGroupBridgeIngressCntrsGet function
* @endinternal
*
* @brief   Gets a Bridge ingress Port/VLAN/Device counters from
*         specified counter set.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] cntrSetId                - counter set number.
*
* @param[out] ingressCntrPtr           - structure of bridge ingress counters current values.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum, on counter set number
* @retval that is out of range of [0 -1]
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Bridge Ingress group of counters are clear-on-read.
*
*/
GT_STATUS cpssDxChBrgCntPortGroupBridgeIngressCntrsGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT    cntrSetId,
    OUT CPSS_BRIDGE_INGRESS_CNTR_STC        *ingressCntrPtr
);

/**
* @internal cpssDxChBrgCntPortGroupLearnedEntryDiscGet function
* @endinternal
*
* @brief   Gets the total number of source addresses the were
*         not learned due to bridge internal congestion.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in FDB APIs for Falcon
*
* @param[out] countValuePtr            - The value of the counter.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hw error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Learned Entry Discards Counter is clear-on-read.
*
*/
GT_STATUS cpssDxChBrgCntPortGroupLearnedEntryDiscGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *countValuePtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChBrgCounth */

