/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file cpssDxChCscd.h
*
* @brief CPSS DXCH Cascading API.
*
* @version   40
********************************************************************************
*/

#ifndef __cpssDxChCscdh
#define __cpssDxChCscdh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cscd/cpssGenCscd.h>
#include <cpss/generic/cos/cpssCosTypes.h>
#include <cpss/generic/port/cpssPortCtrl.h>
#include <cpss/common/config/cpssGenCfg.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/generic/port/cpssPortTx.h>

/**
* @enum CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT
 *
 * @brief egress QoS remap mode type
*/
typedef enum{

    /** disable QOS remapping */
    CPSS_DXCH_CSCD_QOS_REMAP_DISABLED_E,

    /** enable QOS remap for control traffic */
    CPSS_DXCH_CSCD_QOS_REMAP_CNTRL_ONLY_E,

    /** enable QOS remap for data traffic */
    CPSS_DXCH_CSCD_QOS_REMAP_DATA_ONLY_E,

    /** @brief enable QOS remap for both control
     *  and data traffic
     */
    CPSS_DXCH_CSCD_QOS_REMAP_ALL_E

} CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT;


/**
* @enum CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_ENT
 *
 * @brief HyperG.Stack Port Speed
*/
typedef enum{

    /** @brief 10 Gbps - each of the XAUI
     *  four SERDES lanes operates at 3.125 Gbps.
     */
    CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_STANDART_E,

    /** @brief 12 Gbps - each of the XAUI
     *  four SERDES lanes operates at 3.75 Gbps.
     */
    CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_ACCELERATED_E

} CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_ENT;

/**
* @enum CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT
 *
 * @brief type of the load balancing trunk hash for packets forwarded
 * via trunk cascade link
 * Enabled to set the load balancing trunk hash for packets
 * forwarded via an trunk uplink to be based on the packet's source
 * port, and not on the packets data.
*/
typedef enum{

    /** @brief Load balancing trunk hash is based on the ingress pipe
     *  hash mode as configured by function
     *  cpssDxChTrunkHashModeSet(...)
     */
    CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E,

    /** @brief Load balancing trunk
     *  hash for this cascade trunk interface is based on the packet's
     *  source port, regardless of the ingress pipe hash mode.
     */
    CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E,

    /** @brief Load balancing trunk
     *  hash for this cascade trunk interface is based on the packet's
     *  destination port, regardless of the ingress pipe hash mode.
     *  Relevant only for Lion and above.
     */
    CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_DST_PORT_E

} CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT;

/**
* @enum CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT
 *
 * @brief TX/RX HyperG.link CRC Mode
*/
typedef enum{

    /** one byte CRC mode */
    CPSS_DXCH_CSCD_PORT_CRC_ONE_BYTE_E,

    CPSS_DXCH_CSCD_PORT_CRC_TWO_BYTES_E,

    CPSS_DXCH_CSCD_PORT_CRC_THREE_BYTES_E,

    /** four bytes CRC mode */
    CPSS_DXCH_CSCD_PORT_CRC_FOUR_BYTES_E,

    /** @brief NO CRC added
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_CSCD_PORT_CRC_ZERO_BYTES_E

} CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT;


/**
* @enum CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT
 *
 * @brief Describe Trust Forward DSA modes configured for cascade ports.
*/
typedef enum{

    /** @brief global QoS mode: this is the legacy mode where the packet QoS
     *  profile index is copied from the incoming packet DSA tag<QoS Profile>.
     *  The QoS Profile Table maps QoS profile to the associated TC & DP.
     *  This mode supports up to 128 QoS profiles globally.
     */
    CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_GLOBAL_E,

    /** @brief extended QoS mode: DSA tag <QoS Profile> is used to assign
     *  packet's TC and DP. QoS Profile Table is bypassed in this mode.
     */
    CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_EXTENDED_E

} CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT;




/**
* @struct CPSS_DXCH_CSCD_QOS_TC_REMAP_STC
 *
 * @brief Structure for remapping TC to new TCs, for each DSA tag
 * and source port type.
*/
typedef struct{

    /** @brief Traffic Class for DSA tag FORWARD and for
     *  local to stack port
     */
    GT_U32 forwardLocalTc;

    /** @brief Traffic Class for DSA tag FORWARD and for
     *  stack to stack port
     */
    GT_U32 forwardStackTc;

    /** @brief Traffic Class for DSA tag TO_ANALYZER and
     *  for local to stack port
     */
    GT_U32 toAnalyzerLocalTc;

    /** @brief Traffic Class for DSA tag TO_ANALYZER and
     *  for stack to stack port
     */
    GT_U32 toAnalyzerStackTc;

    /** @brief Traffic Class for DSA tag TO_CPU and for
     *  local to stack port
     */
    GT_U32 toCpuLocalTc;

    /** @brief Traffic Class for DSA tag TO_CPU and for
     *  stack to stack port
     */
    GT_U32 toCpuStackTc;

    /** @brief Traffic Class for DSA tag FROM_CPU and for
     *  local to stack port
     */
    GT_U32 fromCpuLocalTc;

    /** @brief Traffic Class for DSA tag FROM_CPU and for
     *  stack to stack port
     */
    GT_U32 fromCpuStackTc;

} CPSS_DXCH_CSCD_QOS_TC_REMAP_STC;

/**
* @struct CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC
 *
 * @brief This structure describes tc,dp remapping index in tc,dp remapping table.
 * The 8 bit index is a result of:
 * 3 bit tc,1 bit isStack,2 bit dsaCmd,2 bit dp
 * APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X.
 * The 12 bits index is a result of:
 * 3 bit tc, 2 bit dp, 2 bit dsaCmd, 1 bit packetIsMultiDestination, 2 bit targetPortTcProfile, 2 bit isStack
 * APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon.
*/
typedef struct{

    /** QoS profile Traffic Class.(APPLICABLE RANGES: 0..7) */
    GT_U32 tc;

    /** QoS profile Drop Precedence. */
    CPSS_DP_LEVEL_ENT dp;

    /** @brief source physical port type (stack port or network port):
     *  APPLICABLE VALUES:
     *  CPSS_DXCH_PORT_PROFILE_NETWORK_E  - network port TC profile used in index calculation.
     *  CPSS_DXCH_PORT_PROFILE_CSCD_E    - cascade port TC profile used in index calculation.
     *  APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X.
     *  APPLICABLE VALUES:
     *  All values are valid.
     *  APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon.
     */
    CPSS_DXCH_PORT_PROFILE_ENT isStack;

    /** dsa tag command. */
    CPSS_DXCH_NET_DSA_CMD_ENT dsaTagCmd;

    /** @brief target physical port TC profile type
     *  APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon.
     */
    CPSS_DXCH_PORT_PROFILE_ENT targetPortTcProfile;

    /** @brief packet destination type
     *  GT_TRUE   - multicast destination packet.
     *  GT_FALSE  - single destination packet.
     *  APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon.
     */
    GT_BOOL packetIsMultiDestination;

} CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC;


/**
* @enum CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT
 *
 * @brief Device Map lookup mode.
 * The mode defines index calculation that is used to
 * access Device Map table.
*/
typedef enum{

    /** Target Device used as index. */
    CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E,

    /** @brief Both Target Device and Target Port are used to get index.
     *  Index bits representation is Target Device[4:0], Target Port[5:0].
     *  for eArch (<TrgDev>[5:0], Target Port[5:0])
     */
    CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E,

    /** @brief Both Target Device and Source Device are used to get index.
     *  Index bits representation is Target Device[4:0], Source Device[4:0].
     */
    CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E,

    /** @brief Both Target Device and Source Port are used to get index.
     *  Index bits representation is Target Device[4:0], Source Port[5:0].
     */
    CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E,

    /** @brief Target Device Source Device and Source Port are used to get index.
     *  Index bits representation is Target Device[4:0],
     *  Source Device[0], Source Port[4:0].
     */
    CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_1BIT_SRC_DEV_SRC_PORT_E,

    /** @brief Target Device Source Device and Source Port are used to get index.
     *  Index bits representation is Target Device[4:0],
     *  Source Device[1:0], Source Port[3:0].
     */
    CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_2BITS_SRC_DEV_SRC_PORT_E,

    /** @brief Target Device Source Device and Source Port are used to get index.
     *  Index bits representation is Target Device[4:0],
     *  Source Device[2:0], Source Port[2:0].
     */
    CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_3BITS_SRC_DEV_SRC_PORT_E,

    /** @brief Target Device Source Device and Source Port are used to get index.
     *  Index bits representation is Target Device[4:0],
     *  Source Device[3:0], Source Port[1:0].
     */
    CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_4BITS_SRC_DEV_SRC_PORT_E,

    /** @brief Target Device Source Device and Source Port are used to get index.
     *  Index bits representation is Target Device[4:0],
     *  Source Device[4:0], Source Port[0].
     */
    CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E

} CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT;

/**
* @enum CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT
 *
 * @brief Ingress physical port assignment mode
*/
typedef enum{

    /** No mapping is performed: Source Physical Port = My Physical Port */
    CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DISABLED_E,

    /** @brief This is an Interlaken interface. Based on the Rx ILK interface, the
     *  packet is mapped to an ingress physical port number on this device
     */
    CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_INTERLAKEN_E,

    /** @brief This is a cascaded DSA interface. Based on the DSA source
     *  information, the packet is mapped to an ingress physical port number
     *  on this device
     */
    CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DSA_E,

     /** @brief This is a E-Tagged interface. Based on the E-CID
     *   value in the packet E-Tag, The E-Tagged packet is mapped to
     *   ingress physical port number on this device
     */
    CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_E_TAG_E,

    /** @brief This is a VLAN interface. Based on the VID value
     *  in the outermost VLAN Tag, The packet is mapped to ingress
     *  physical port number on  this devic
     */
    CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_VLAN_TAG_E

} CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT;

/**
* @internal cpssDxChCscdPortTypeSet function
* @endinternal
*
* @brief   Configure a PP port to be a cascade port. Application is responsible
*         for setting the default values of the port.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum  - physical device number
*         portNum - The port to be configured as cascade
*         portDirection  - port's direction.
*         (APPLICABLE DEVICES Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
*         portType - cascade type DSA tag port or network port
*         OUTPUTS:
*         None
*         RETURNS:
*         GT_OK          - on success
*         GT_BAD_PARAM       - on wrong port number or device
*         GT_HW_ERROR       - on hardware error
*         GT_NOT_APPLICABLE_DEVICE - on not applicable device
*         COMMENTS:
*         1. This function also configures insertion of DSA tag for PFC frames.
*         Relevant to egress direction.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - The port to be configured as cascade
* @param[in] portDirection            - port's direction.
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.)
* @param[in] portType                 - cascade  type DSA tag port or network port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. This function also configures insertion of DSA tag for PFC frames.
*       Relevant to egress direction.
*       APPLICABLE DEVICES: Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*/
GT_STATUS cpssDxChCscdPortTypeSet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN CPSS_PORT_DIRECTION_ENT      portDirection,
    IN CPSS_CSCD_PORT_TYPE_ENT      portType
);

/**
* @internal cpssDxChCscdPortTypeGet function
* @endinternal
*
* @brief   Retrieve a PP port cascade port configuration.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum  - physical device number
*         portNum - port number or CPU port
*         portDirection  - port's direction.
*         (APPLICABLE DEVICES Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
*         OUTPUTS:
*         portTypePtr - (pointer to) cascade type DSA tag port or network port.
*         RETURNS:
*         GT_OK          - on success
*         GT_BAD_PARAM       - on wrong port number or device
*         GT_BAD_STATE       - when portDirection = 'both' but ingress value
*         conflicts the egress value.
*         GT_HW_ERROR       - on hardware error
*         GT_BAD_PTR        - one of the parameters is NULL pointer
*         GT_NOT_APPLICABLE_DEVICE - on not applicable device
*         COMMENTS:
*         1. When portDirection = 'Ingress direction' , the (portTypePtr) will get value of:
*         CPSS_CSCD_PORT_NETWORK_E - network port indication.
*         CPSS_CSCD_PORT_DSA_MODE_1_WORD_E - cascade port indication.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number or CPU port
* @param[in] portDirection            - port's direction.
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.)
*
* @param[out] portTypePtr              - (pointer to) cascade type DSA tag port or network port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_STATE             - when portDirection = 'both' but ingress value
*                                       conflicts the egress value.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. When portDirection = 'Ingress direction' , the (portTypePtr) will get value of:
*       CPSS_CSCD_PORT_NETWORK_E - network port indication.
*       CPSS_CSCD_PORT_DSA_MODE_1_WORD_E - cascade port indication.
*       APPLICABLE DEVICES: Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*/
GT_STATUS cpssDxChCscdPortTypeGet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN CPSS_PORT_DIRECTION_ENT      portDirection,
    OUT CPSS_CSCD_PORT_TYPE_ENT     *portTypePtr
);

/**
* @internal cpssDxChCscdDevMapTableSet function
* @endinternal
*
* @brief   Set the cascade map table . the map table define the local port or
*         trunk that packets destined to a destination device
*         should be transmitted to.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] targetHwDevNum           - the HW device to be reached via cascade (APPLICABLE RANGES: 0..31)
* @param[in] sourceHwDevNum           - source HW device number (APPLICABLE RANGES: 0..31)
*                                      (APPLICABLE DEVICES Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Relevant only for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_1BIT_SRC_DEV_SRC_PORT_E
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_2BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_3BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_4BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E,
*                                      modes, otherwise ignored.
* @param[in] portNum                  - target / source port number
*                                      (APPLICABLE DEVICES Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Target for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E mode,
*                                      Source for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_1BIT_SRC_DEV_SRC_PORT_E
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_2BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_3BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_4BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E,
*                                      modes, otherwise ignored.
* @param[in] cascadeLinkPtr           - (pointer to)A structure holding the cascade link type
*                                      (port (APPLICABLE RANGES: 0..63)
*                                      or trunk(APPLICABLE RANGES: 0..127)) and the link number
*                                      leading to the target device.
* @param[in] srcPortTrunkHashEn       - Relevant when (cascadeLinkPtr->linkType ==
*                                      CPSS_CSCD_LINK_TYPE_TRUNK_E)
*                                      Enabled to set the load balancing trunk hash for packets
*                                      forwarded via an trunk uplink to be based on the
*                                      packet's source port, and not on the packets data.
*                                      Indicates the type of uplink.
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E -
*                                      Load balancing trunk hash is based on the ingress pipe
*                                      hash mode as configured by function
*                                      cpssDxChTrunkHashGeneralModeSet(...)
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E - Load
*                                      balancing trunk hash for this cascade trunk interface is
*                                      based on the packet's source port, regardless of the
*                                      ingress pipe hash mode
*                                      NOTE : this parameter is relevant only to DXCH2
*                                      and above devices
* @param[in] egressAttributesLocallyEn - Determines whether the egress attributes
*                                      are determined by the target port even if the target
*                                      device is not the local device.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or target device or source device,
*                                       or port number or bad trunk hash mode
* @retval GT_BAD_STATE             - the trunk is in bad state , one of:
*                                       1. empty trunk (APPLICABLE DEVICES : Lion2)
*                                       2. hold members from no local device (APPLICABLE DEVICES : Lion2)
*                                       3. hold members from more then single hemisphere (APPLICABLE DEVICES : Lion2)
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdDevMapTableSet
(
    IN GT_U8                        devNum,
    IN GT_HW_DEV_NUM                targetHwDevNum,
    IN GT_HW_DEV_NUM                sourceHwDevNum,
    IN GT_PORT_NUM                  portNum,
    IN CPSS_CSCD_LINK_TYPE_STC      *cascadeLinkPtr,
    IN CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT srcPortTrunkHashEn,
    IN GT_BOOL                      egressAttributesLocallyEn
);

/**
* @internal cpssDxChCscdDevMapTableGet function
* @endinternal
*
* @brief   Get the cascade map table . the map table define the local port or
*         trunk that packets destined to a destination device
*         should be transmitted to.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] targetHwDevNum           - the HW device to be reached via cascade (APPLICABLE RANGES: 0..31)
* @param[in] sourceHwDevNum           - source HW device number (APPLICABLE RANGES: 0..31)
*                                      (APPLICABLE DEVICES Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Relevant only for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_1BIT_SRC_DEV_SRC_PORT_E
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_2BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_3BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_4BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E,
*                                      modes, otherwise ignored.
* @param[in] portNum                  - target / source port number
*                                      (APPLICABLE DEVICES Lion, Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Target for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E mode,
*                                      Source for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_1BIT_SRC_DEV_SRC_PORT_E
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_2BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_3BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_4BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E,
*                                      modes, otherwise ignored.
*
* @param[out] cascadeLinkPtr           - (pointer to)A structure holding the cascade link type
*                                      (port (APPLICABLE RANGES: 0..63)
*                                      or trunk(APPLICABLE RANGES: 0..127)) and the link number
*                                      leading to the target device.
* @param[out] srcPortTrunkHashEnPtr    - Relevant when (cascadeLinkPtr->linkType ==
*                                      CPSS_CSCD_LINK_TYPE_TRUNK_E)
*                                      (pointer to) Enabled to set the load balancing trunk
*                                      hash for packets forwarded via an trunk uplink to be
*                                      based on the packet's source port, and not on the
*                                      packets data.
*                                      Indicates the type of uplink.
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E -
*                                      Load balancing trunk hash is based on the ingress pipe
*                                      hash mode as configured by function
*                                      cpssDxChTrunkHashGeneralModeSet(...)
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E - Load
*                                      balancing trunk hash for this cascade trunk interface is
*                                      based on the packet's source port, regardless of the
*                                      ingress pipe hash mode
*                                      NOTE : this parameter is relevant only to DXCH2 and
*                                      above devices
* @param[out] egressAttributesLocallyEnPtr - Determines whether the egress attributes
*                                      are determined by the target port even if the target
*                                      device is not the local device.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or target device or source device,
*                                       or port number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on bad value in HW
*/
GT_STATUS cpssDxChCscdDevMapTableGet
(
    IN GT_U8                         devNum,
    IN GT_HW_DEV_NUM                 targetHwDevNum,
    IN GT_HW_DEV_NUM                 sourceHwDevNum,
    IN GT_PORT_NUM                   portNum,
    OUT CPSS_CSCD_LINK_TYPE_STC      *cascadeLinkPtr,
    OUT CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT *srcPortTrunkHashEnPtr,
    OUT GT_BOOL                      *egressAttributesLocallyEnPtr
);

/**
* @internal cpssDxChCscdRemapQosModeSet function
* @endinternal
*
* @brief   Enables/disables remapping of Tc and Dp for Data and Control Traffic
*         on a port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] remapType                - traffic type to remap
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdRemapQosModeSet
(
   IN GT_U8                             devNum,
   IN GT_PHYSICAL_PORT_NUM              portNum,
   IN CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT remapType
);

/**
* @internal cpssDxChCscdRemapQosModeGet function
* @endinternal
*
* @brief   Get remapping status of Tc and Dp for Data and Control Traffic
*         on a port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] remapTypePtr             - traffic type to remap
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChCscdRemapQosModeGet
(
   IN  GT_U8                             devNum,
   IN  GT_PHYSICAL_PORT_NUM              portNum,
   OUT CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT *remapTypePtr
);

/**
* @internal cpssDxChCscdCtrlQosSet function
* @endinternal
*
* @brief   Set control packets TC and DP if Control Remap QoS enabled on a port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] ctrlTc                   - The TC assigned to control packets forwarded to
*                                      cascading ports (APPLICABLE RANGES: 0..7)
* @param[in] ctrlDp                   - The DP assigned to CPU-to-network control traffic or
*                                      network-to-CPU traffic.
*                                      DxCh devices support only :
*                                      CPSS_DP_RED_E and CPSS_DP_GREEN_E
* @param[in] cpuToCpuDp               - The DP assigned to CPU-to-CPU control traffic
*                                      DxCh devices support only :
*                                      CPSS_DP_RED_E and CPSS_DP_GREEN_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_VALUE             - on wrong ctrlDp or cpuToCpuDp level value or tc value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCtrlQosSet
(
   IN GT_U8             devNum,
   IN GT_U8             ctrlTc,
   IN CPSS_DP_LEVEL_ENT ctrlDp,
   IN CPSS_DP_LEVEL_ENT cpuToCpuDp
);

/**
* @internal cpssDxChCscdCtrlQosGet function
* @endinternal
*
* @brief   Get control packets TC and DP if Control Remap QoS enabled on a port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] ctrlTcPtr                - The TC assigned to control packets forwarded to
*                                      cascading ports
* @param[out] ctrlDpPtr                - The DP assigned to CPU-to-network control traffic or
*                                      network-to-CPU traffic.
*                                      DxCh devices support only :
*                                      CPSS_DP_RED_E and CPSS_DP_GREEN_E
* @param[out] cpuToCpuDpPtr            - The DP assigned to CPU-to-CPU control traffic
*                                      DxCh devices support only :
*                                      CPSS_DP_RED_E and CPSS_DP_GREEN_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_VALUE             - on wrong ctrlDp or cpuToCpuDp level value or tc value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChCscdCtrlQosGet
(
   IN  GT_U8             devNum,
   OUT GT_U8             *ctrlTcPtr,
   OUT CPSS_DP_LEVEL_ENT *ctrlDpPtr,
   OUT CPSS_DP_LEVEL_ENT *cpuToCpuDpPtr
);

/**
* @internal cpssDxChCscdRemapDataQosTblSet function
* @endinternal
*
* @brief   Set table to remap Data packets QoS parameters
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] tc                       - original packet TC (APPLICABLE RANGES: 0..7)
* @param[in] dp                       - original packet DP
*                                      DxCh devices support only :
*                                      CPSS_DP_RED_E and CPSS_DP_GREEN_E
* @param[in] remapTc                  - TC assigned to data packets with DP and TC (APPLICABLE RANGES: 0..7)
* @param[in] remapDp                  - DP assigned to data packets with DP and TC
*                                      DxCh devices support only :
*                                      CPSS_DP_RED_E and CPSS_DP_GREEN_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_VALUE             - on wrong DP or dp level value or tc value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdRemapDataQosTblSet
(
   IN GT_U8              devNum,
   IN GT_U8              tc,
   IN CPSS_DP_LEVEL_ENT  dp,
   IN GT_U8              remapTc,
   IN CPSS_DP_LEVEL_ENT  remapDp
);

/**
* @internal cpssDxChCscdRemapDataQosTblGet function
* @endinternal
*
* @brief   Get QoS parameters from table to remap Data packets
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] tc                       - original packet TC (APPLICABLE RANGES: 0..7)
* @param[in] dp                       - original packet DP
*                                      DxCh devices support only :
*                                      CPSS_DP_RED_E and CPSS_DP_GREEN_E
*
* @param[out] remapTcPtr               - TC assigned to data packets with DP and TC
* @param[out] remapDpPtr               - DP assigned to data packets with DP and TC
*                                      DxCh devices support only :
*                                      CPSS_DP_RED_E and CPSS_DP_GREEN_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_VALUE             - on wrong DP or dp level value or tc value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChCscdRemapDataQosTblGet
(
   IN  GT_U8              devNum,
   IN  GT_U8              tc,
   IN  CPSS_DP_LEVEL_ENT  dp,
   OUT GT_U8              *remapTcPtr,
   OUT CPSS_DP_LEVEL_ENT  *remapDpPtr
);

/**
* @internal cpssDxChCscdDsaSrcDevFilterSet function
* @endinternal
*
* @brief   Enable/Disable filtering the ingress DSA tagged packets in which
*         source device (Src Dev) equals to local device number.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] enableOwnDevFltr         - enable/disable ingress DSA loop filter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdDsaSrcDevFilterSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enableOwnDevFltr
);

/**
* @internal cpssDxChCscdDsaSrcDevFilterGet function
* @endinternal
*
* @brief   get value of Enable/Disable filtering the ingress DSA tagged packets in which
*         source device (Src Dev) equals to local device number.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enableOwnDevFltrPtr      - (pointer to) enable/disable ingress DSA loop filter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdDsaSrcDevFilterGet
(
    IN GT_U8        devNum,
    OUT GT_BOOL      *enableOwnDevFltrPtr
);

/**
* @internal cpssDxChCscdHyperGInternalPortModeSet function
* @endinternal
*
* @brief   Change HyperG.stack internal port speed.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - cascade HyperG port to be configured
* @param[in] mode                     - HyperG.Stack Port Speed 10Gbps or 12Gbps
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - the request is not supported
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdHyperGInternalPortModeSet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_ENT mode
);

/**
* @internal cpssDxChCscdHyperGInternalPortModeGet function
* @endinternal
*
* @brief   Get status of HyperG port mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - cascade HyperG port to be configured
*
* @param[out] modePtr                  - HyperG.Stack Port Speed 10Gbps or 12Gbps
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - the request is not supported
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChCscdHyperGInternalPortModeGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    OUT CPSS_DXCH_CSCD_HYPER_G_PORT_MODE_ENT *modePtr
);

/**
* @internal cpssDxChCscdHyperGPortCrcModeSet function
* @endinternal
*
* @brief   Set CRC mode to be standard 4 bytes or proprietary one byte CRC mode.
*
* @note   APPLICABLE DEVICES:      DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] portDirection            - TX/RX cascade port direction
* @param[in] crcMode                  - TX/RX HyperG.link CRC Mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdHyperGPortCrcModeSet
(
    IN GT_U8                             devNum,
    IN GT_PHYSICAL_PORT_NUM              portNum,
    IN CPSS_PORT_DIRECTION_ENT           portDirection,
    IN CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT  crcMode
);

/**
* @internal cpssDxChCscdHyperGPortCrcModeGet function
* @endinternal
*
* @brief   Get CRC mode (standard 4 bytes or proprietary one byte).
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] portDirection            - TX/RX cascade port direction (ingress or egress)
*
* @param[out] crcModePtr               - TX/RX HyperG.link CRC Mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChCscdHyperGPortCrcModeGet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    IN  CPSS_PORT_DIRECTION_ENT           portDirection,
    OUT CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT  *crcModePtr
);

/**
* @internal cpssDxChCscdFastFailoverFastStackRecoveryEnableSet function
* @endinternal
*
* @brief   Enable/Disable fast stack recovery.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:  fast stack recovery
*                                      GT_FALSE: disable fast stack recovery
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdFastFailoverFastStackRecoveryEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChCscdFastFailoverFastStackRecoveryEnableGet function
* @endinternal
*
* @brief   Get the status of fast stack recovery (Enabled/Disabled).
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) fast stack recovery status
*                                      GT_TRUE: fast stack recovery enabled
*                                      GT_FALSE: fast stack recovery disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdFastFailoverFastStackRecoveryEnableGet
(
    IN  GT_U8        devNum,
    OUT GT_BOOL      *enablePtr
);

/**
* @internal cpssDxChCscdFastFailoverSecondaryTargetPortMapSet function
* @endinternal
*
* @brief   Set secondary target port map.
*         If the device is the device where the ring break occured, the
*         packet is looped on the ingress port and is egressed according to
*         Secondary Target Port Map. Also "packetIsLooped" bit is
*         set in DSA tag.
*         If the device receives a packet with "packetIsLooped" bit is set
*         in DSA tag, the packet is forwarded according to Secondary Target Port
*         Map.
*         Device MAP table (cpssDxChCscdDevMapTableSet) is not used in the case.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - ingress port number
* @param[in] secondaryTargetPort      - secondary target port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - when secondaryTargetPort is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdFastFailoverSecondaryTargetPortMapSet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN GT_PHYSICAL_PORT_NUM         secondaryTargetPort
);

/**
* @internal cpssDxChCscdFastFailoverSecondaryTargetPortMapGet function
* @endinternal
*
* @brief   Get Secondary Target Port Map for given device.
*         If the device is the device where the ring break occured, the
*         packet is looped on the ingress port and is egressed according to
*         Secondary Target Port Map. Also "packetIsLooped" bit is
*         set in DSA tag.
*         If the device receives a packet with "packetIsLooped" bit is set
*         in DSA tag, the packet is forwarded according to Secondary Target Port
*         Map.
*         Device MAP table (cpssDxChCscdDevMapTableSet) is not used in the case.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - ingress port number
*
* @param[out] secondaryTargetPortPtr   - (pointer to) secondary target port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdFastFailoverSecondaryTargetPortMapGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_PHYSICAL_PORT_NUM        *secondaryTargetPortPtr
);

/**
* @internal cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableSet function
* @endinternal
*
* @brief   Enable/Disable fast failover loopback termination
*         for single-destination packets.
*         There are two configurable options for forwarding single-destination
*         packets that are looped-back across the ring:
*         - Termination Disabled.
*         Unconditionally forward the looped-back packet to the configured
*         backup ring port (for the given ingress ring port) on all the ring
*         devices until it reaches the far-end device where it is again
*         internally looped-back on the ring port and then forward it normally.
*         - Termination Enabled.
*         The looped-back packet passes through the
*         ring until it reaches the target device where it is egressed on its
*         target port.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:  terminating local loopback
*                                      for fast stack recovery
*                                      GT_FALSE: disable terminating local loopback
*                                      for fast stack recovery
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
);


/**
* @internal cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet function
* @endinternal
*
* @brief   Get the status of fast failover loopback termination
*         for single-destination packets (Enabled/Disabled).
*         There are two configurable options for forwarding single-destination
*         packets that are looped-back across the ring:
*         - Termination Disabled.
*         Unconditionally forward the looped-back packet to the configured
*         backup ring port (for the given ingress ring port) on all the ring
*         devices until it reaches the far-end device where it is again
*         internally looped-back on the ring port and then forward it normally.
*         - Termination Enabled.
*         The looped-back packet passes through the
*         ring until it reaches the target device where it is egressed on its
*         target port.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE: terminating local loopback
*                                      for fast stack recovery enabled
*                                      GT_FALSE: terminating local loopback
*                                      for fast stack recovery disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet
(
    IN  GT_U8        devNum,
    OUT GT_BOOL      *enablePtr
);

/**
* @internal cpssDxChCscdFastFailoverPortIsLoopedSet function
* @endinternal
*
* @brief   Enable/disable Fast Failover on a failed port.
*         When port is looped and get packet with DSA tag <Packet is Looped> = 0,
*         then device do next:
*         - set DSA tag <Packet is Looped> = 1
*         - bypass ingress and egress processing
*         - send packet through egress port that defined in secondary target
*         port map (cpssDxChCscdFastFailoverSecondaryTargetPortMapSet).
*         When port is looped and get packet with DSA tag <Packet is Looped> = 1,
*         then device do next:
*         - set DSA tag <Packet is Looped> = 0
*         - Apply usual ingress and egress processing
*         When port is not looped and get packet then device do next:
*         - Apply usual ingress and egress processing
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (including CPU)
* @param[in] isLooped                 -  GT_FALSE - Port is not looped.
*                                      GT_TRUE - Port is looped.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdFastFailoverPortIsLoopedSet
(
  IN GT_U8                     devNum,
  IN GT_PHYSICAL_PORT_NUM      portNum,
  IN GT_BOOL                   isLooped
);

/**
* @internal cpssDxChCscdFastFailoverPortIsLoopedGet function
* @endinternal
*
* @brief   Get status (Enable/Disable) of Fast Failover on the failed port.
*         When port is looped and get packet with DSA tag <Packet is Looped> = 0,
*         then device do next:
*         - set DSA tag <Packet is Looped> = 1
*         - bypass ingress and egress processing
*         - send packet through egress port that defined in secondary target
*         port map (cpssDxChCscdFastFailoverSecondaryTargetPortMapSet).
*         When port is looped and get packet with DSA tag <Packet is Looped> = 1,
*         then device do next:
*         - set DSA tag <Packet is Looped> = 0
*         - Apply usual ingress and egress processing
*         When port is not looped and get packet then device do next:
*         - Apply usual ingress and egress processing
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (including CPU)
*
* @param[out] isLoopedPtr              - (pointer to) Is Looped
*                                      GT_FALSE - Port is not looped.
*                                      GT_TRUE - Port is looped.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdFastFailoverPortIsLoopedGet
(
  IN GT_U8                  devNum,
  IN GT_PHYSICAL_PORT_NUM   portNum,
  OUT GT_BOOL               *isLoopedPtr
);

/**
* @internal cpssDxChCscdQosPortTcRemapEnableSet function
* @endinternal
*
* @brief   Enable/Disable Traffic Class Remapping on cascading port.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - cascading port number or CPU port
* @param[in] enable                   - GT_TRUE:  Traffic Class remapping
*                                      GT_FALSE: disable Traffic Class remapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdQosPortTcRemapEnableSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL              enable
);

/**
* @internal cpssDxChCscdQosPortTcRemapEnableGet function
* @endinternal
*
* @brief   Get the status of Traffic Class Remapping on cascading port
*         (Enabled/Disabled).
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - cascading port number or CPU port
*
* @param[out] enablePtr                - GT_TRUE: Traffic Class remapping enabled
*                                      GT_FALSE: Traffic Class remapping disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdQosPortTcRemapEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal cpssDxChCscdPortQosDsaModeSet function
* @endinternal
*
* @brief   Configures DSA tag QoS trust mode for cascade port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - cascading port number.
* @param[in] portQosDsaMode           - DSA tag QoS mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, port,portQosDsaTrustMode.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChCscdPortQosDsaModeSet
(
    IN GT_U8                         devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    IN CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT  portQosDsaMode
);


/**
* @internal cpssDxChCscdPortQosDsaModeGet function
* @endinternal
*
* @brief   Get DSA tag QoS trust mode for cascade port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - cascading port number.
*
* @param[out] portQosDsaTrustModePtr   - pointer to DSA tag QoS trust mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, port.
* @retval GT_BAD_PTR               - portQosDsaTrustModePtr is NULL pointer.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChCscdPortQosDsaModeGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    OUT CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT  *portQosDsaTrustModePtr
);


/**
* @internal cpssDxChCscdQosTcRemapTableSet function
* @endinternal
*
* @brief   Remap Traffic Class on cascading port to new Traffic Classes,
*         for each DSA tag type and for source port type (local or cascading).
*         If the source port is enabled for Traffic Class remapping, then traffic
*         will egress with remapped Traffic Class.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] tc                       - Traffic Class of the packet on the source port (APPLICABLE RANGES: 0..7)
* @param[in] tcMappingsPtr            - (pointer to )remapped traffic Classes
*                                      for ingress Traffic Class
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdQosTcRemapTableSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  tc,
    IN  CPSS_DXCH_CSCD_QOS_TC_REMAP_STC         *tcMappingsPtr
);

/**
* @internal cpssDxChCscdQosTcRemapTableGet function
* @endinternal
*
* @brief   Get the remapped Traffic Classes for given Traffic Class.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] tc                       - Traffic Class of the packet on the source port (APPLICABLE RANGES: 0..7)
*
* @param[out] tcMappingsPtr            - (pointer to )remapped Traffic Classes
*                                      for ingress Traffic Class
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdQosTcRemapTableGet
(
    IN   GT_U8                                  devNum,
    IN   GT_U32                                 tc,
    OUT  CPSS_DXCH_CSCD_QOS_TC_REMAP_STC        *tcMappingsPtr
);


/**
* @internal cpssDxChCscdQosTcDpRemapTableSet function
* @endinternal
*
* @brief   Set remaping of (packet TC, packet ingress Port Type, packet DSA cmd,
*         packet drop precedence and packet multi ot single-destination) on cascading port
*         to new priority queue for enqueuing the packet, new drop precedence
*         and new priority queue for PFC assigned to this packet.
*         APPLICABLE DEVICES:
*         Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*         INPUTS:
*         devNum     - device number
*         tcDpRemappingPtr  - (pointer to )tc,dp remapping index structure.
*         newTc   - new priority queue assigned to the packet. (APPLICABLE RANGES: 0..7).
*         newDp   - new drop precedence assigned to the packet.
*         newPfcTc - new priority queue for PFC assigned to the packet.
*         (APPLICABLE RANGES: 0..7
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] tcDpRemappingPtr         - (pointer to )tc,dp remapping index structure.
* @param[in] newTc                    - new priority queue assigned to the packet.
*                                       (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2 0..7).
*                                       (APPLICABLE RANGES: Falcon 0..15).
* @param[in] newDp                    - new drop precedence assigned to the packet.
* @param[in] newPfcTc                 - new priority queue for PFC assigned to the packet.
*                                      (APPLICABLE RANGES: 0..7
*                                      APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon).
* @param[in]  mcastPriority            -defines whether a Multicast packet has guarantee delivery (e.g. IPTv,
*                                                             ARP-BC) or best effort delivery (e.g. flood).This indication has meaning only if the
*                                                             packet is Multicast.(APPLICABLE DEVICES: Falcon).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdQosTcDpRemapTableSet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC *tcDpRemappingPtr,
    IN  GT_U32                                   newTc,
    IN  CPSS_DP_LEVEL_ENT                        newDp,
    IN  GT_U32                                   newPfcTc,
    IN  CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT     mcastPriority
);


/**
* @internal cpssDxChCscdQosTcDpRemapTableGet function
* @endinternal
*
* @brief   Get the remapped value of priority queue, drop precedence and priority queue for PFC
*         assigned to the packet for given (packet TC, packet ingress Port Type,
*         packet DSA cmd, packet drop precedence and packet multi or single-destination) on cascading port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] tcDpRemappingPtr         - (pointer to )tc,dp remapping index structure.
*
* @param[out] remappedTcPtr            - (pointer to) priority queue assigned to the packet.
* @param[out] remappedDpPtr            - (pointer to) drop precedence assigned to the packet.
* @param[out] remappedPfcTcPtr         - (pointer to) priority queue for PFC assigned to the packet.
*                                      (APPLICABLE DEVICES Bobcat3; Aldrin2; Falcon).
* @param[out]  remappedMcastPriorityPtr            -(pointer to)defines whether a Multicast packet has guarantee delivery (e.g. IPTv,
*                                                             ARP-BC) or best effort delivery (e.g. flood).This indication has meaning only if the
*                                                             packet is Multicast.(APPLICABLE DEVICES: Falcon).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdQosTcDpRemapTableGet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC *tcDpRemappingPtr,
    OUT  GT_U32                                  *remappedTcPtr,
    OUT  CPSS_DP_LEVEL_ENT                       *remappedDpPtr,
    OUT  GT_U32                                  *remappedPfcTcPtr,
    OUT  CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT    *remappedMcastPriorityPtr
);

/**
* @internal cpssDxChCscdPortBridgeBypassEnableSet function
* @endinternal
*
* @brief   The function enables/disables bypass of the bridge engine per port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -  PP's device number.
* @param[in] portNum                  -  port number or CPU port
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   -  GT_TRUE  - Enable bypass of the bridge engine.
*                                      GT_FALSE - Disable bypass of the bridge engine.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If the DSA tag is not extended Forward, the bridging decision
*       is performed regardless of the setting.
*       When bypass enabled the Bridge engine still learn MAC source addresses,
*       but will not modify the packet command, attributes (or forwarding
*       decision).
*
*/
GT_STATUS cpssDxChCscdPortBridgeBypassEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    IN  GT_BOOL     enable
);

/**
* @internal cpssDxChCscdPortBridgeBypassEnableGet function
* @endinternal
*
* @brief   The function gets bypass of the bridge engine per port
*         configuration status.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] portNum                  - port number or CPU port
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                -  pointer to bypass of the bridge engine per port
*                                      configuration status.
*                                      GT_TRUE  - Enable bypass of the bridge engine.
*                                      GT_FALSE - Disable bypass of the bridge engine.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdPortBridgeBypassEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChCscdOrigSrcPortFilterEnableSet function
* @endinternal
*
* @brief   Enable/Disable filtering the multi-destination packet that was received
*         by the local device, sent to another device, and sent back to this
*         device, from being sent back to the network port at which it was
*         initially received.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - filter and drop the packet
*                                      - GT_FALSE - don't filter the packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdOrigSrcPortFilterEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChCscdOrigSrcPortFilterEnableGet function
* @endinternal
*
* @brief   Get the status of filtering the multi-destination packet that was
*         received by the local device, sent to another device, and sent back to
*         this device, from being sent back to the network port at which it was
*         initially received.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE - filter and drop the packet
*                                      - GT_FALSE - don't filter the packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdOrigSrcPortFilterEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChCscdDevMapLookupModeSet function
* @endinternal
*
* @brief   Set lookup mode for accessing the Device Map table.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - device Map lookup mode
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdDevMapLookupModeSet
(
    IN GT_U8                               devNum,
    IN CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT   mode
);

/**
* @internal cpssDxChCscdDevMapLookupModeGet function
* @endinternal
*
* @brief   Get lookup mode for accessing the Device Map table.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - pointer to device Map lookup mode
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdDevMapLookupModeGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT   *modePtr
);

/**
* @internal cpssDxChCscdPortLocalDevMapLookupEnableSet function
* @endinternal
*
* @brief   Enable / Disable the local target port for device map lookup
*         for local device.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number or CPU port
* @param[in] portDirection            - port's direction:
*                                      CPSS_DIRECTION_INGRESS_E - source port
*                                      CPSS_DIRECTION_EGRESS_E  -  target port
*                                      CPSS_DIRECTION_BOTH_E    - both source and target ports
* @param[in] enable                   - GT_TRUE  - the port is enabled for device map lookup
*                                      for local device.
*                                      - GT_FALSE - No access to Device map table for
*                                      local devices.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or port number or portDirection
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To enable access to the Device Map Table for the local target devices
*       - Enable the local source port for device map lookup
*       - Enable the local target port for device map lookup
*
*/
GT_STATUS cpssDxChCscdPortLocalDevMapLookupEnableSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_DIRECTION_ENT   portDirection,
    IN GT_BOOL              enable
);

/**
* @internal cpssDxChCscdPortLocalDevMapLookupEnableGet function
* @endinternal
*
* @brief   Get status of enabling / disabling the local target port
*         for device map lookup for local device.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number or CPU port
* @param[in] portDirection            - port's direction:
*                                      CPSS_DIRECTION_INGRESS_E - source port
*                                      CPSS_DIRECTION_EGRESS_E  -  target port
*                                      CPSS_DIRECTION_BOTH_E    - both source and target ports
*
* @param[out] enablePtr                - pointer to status of enabling / disabling the local
*                                      target port for device map lookup for local device.
*                                      - GT_TRUE  - target port is enabled for device map lookup
*                                      for local device.
*                                      - GT_FALSE - No access to Device map table for
*                                      local devices.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or port number or portDirection
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdPortLocalDevMapLookupEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DIRECTION_ENT      portDirection,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal cpssDxChCscdCentralizedChassisModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable initial local source port assignment from DSA tag, used
*         for centralized chassis.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - Centralized Chassis Mode
*                                      GT_FALSE: Ingress port is not connected to a line-card
*                                      device in a centralized chassis system
*                                      GT_TRUE:  Ingress port is connected to a line-card
*                                      device in a centralized chassis system
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisModeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             enable
);

/**
* @internal cpssDxChCscdCentralizedChassisModeEnableGet function
* @endinternal
*
* @brief   Get initial local source port assignment from DSA tag, used
*         for centralized chassis.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) Centralized Chassis Mode
*                                      GT_FALSE: Ingress port is not connected to a line-card
*                                      device in a centralized chassis system
*                                      GT_TRUE:  Ingress port is connected to a line-card
*                                      device in a centralized chassis system
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisModeEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_BOOL                             *enablePtr
);



/**
* @internal cpssDxChCscdCentralizedChassisReservedDevNumSet function
* @endinternal
*
* @brief   Configures a value representing the local device in centralized chassis
*         and used as the source device in the outgoing FORWARD DSA tag.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] reservedHwDevNum         - reserved hw dev num
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisReservedDevNumSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_HW_DEV_NUM                       reservedHwDevNum
);

/**
* @internal cpssDxChCscdCentralizedChassisReservedDevNumGet function
* @endinternal
*
* @brief   Gets a value representing the local device in centralized chassis
*         and used as the source device in the outgoing FORWARD DSA tag.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] reservedHwDevNumPtr      - (pointer to) reserved hw dev num
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisReservedDevNumGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_HW_DEV_NUM                       *reservedHwDevNumPtr
);

/**
* @internal cpssDxChCscdCentralizedChassisSrcIdSet function
* @endinternal
*
* @brief   Configured what is the source ID used by the line card directly
*         attached to the Centralized chassis port
*         Relevant only when <CC Mode Enable> = Enable
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] srcId                    - Src Id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisSrcIdSet
(
    IN  GT_U8                        devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    IN  GT_U32                       srcId
);

/**
* @internal cpssDxChCscdCentralizedChassisSrcIdGet function
* @endinternal
*
* @brief   Get CC Src ID
*         Relevant only when <CC Mode Enable> = Enable
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] srcIdPtr                 - (pointer to) Src Id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisSrcIdGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_U32                              *srcIdPtr
);

/**
* @internal cpssDxChCscdDbRemoteHwDevNumModeSet function
* @endinternal
*
* @brief   Set single/dual HW device number mode to remote HW device number.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] hwDevNum                 - HW device number (APPLICABLE RANGES: 0..(4K-1)).
* @param[in] hwDevMode                - single/dual HW device number mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - hwDevNum is odd number and hwDevMode is
*                                       CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E
* @retval GT_OUT_OF_RANGE          - when hwDevNum is out of range
*
* @note 1. Only even device numbers allowed to be marked as "dual HW device"
*       2. "Dual HW device" mode must be configured before any other
*       configuration that uses hwDevNum.
*       3. There are no restrictions on SW devNum for dual mode devices.
*
*/
GT_STATUS cpssDxChCscdDbRemoteHwDevNumModeSet
(
    IN GT_HW_DEV_NUM                    hwDevNum,
    IN CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT hwDevMode
);

/**
* @internal cpssDxChCscdDbRemoteHwDevNumModeGet function
* @endinternal
*
* @brief   Get single/dual HW device number mode to remote HW device number.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] hwDevNum                 - HW device number (APPLICABLE RANGES: 0..(4K-1)).
*
* @param[out] hwDevModePtr             - (pointer to) single/dual HW device number mode.
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - when hwDevNum is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssDxChCscdDbRemoteHwDevNumModeGet
(
    IN  GT_HW_DEV_NUM                    hwDevNum,
    OUT CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT *hwDevModePtr
);


/**
* @internal cpssDxChCscdPortStackAggregationEnableSet function
* @endinternal
*
* @brief   Enable/disable stack aggregation per port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE -  stack aggregation
*                                      GT_FALSE - disable stack aggregation
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 device: applicable starting from revision B0
*
*/
GT_STATUS cpssDxChCscdPortStackAggregationEnableSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL              enable
);


/**
* @internal cpssDxChCscdPortStackAggregationEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of stack aggregation per port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - aggregator port number
*
* @param[out] enablePtr                - (pointer to)GT_TRUE - enable stack aggregation
*                                      GT_FALSE - disable stack aggregation
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 device: applicable starting from revision B0
*
*/
GT_STATUS cpssDxChCscdPortStackAggregationEnableGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
);


/**
* @internal cpssDxChCscdPortStackAggregationConfigSet function
* @endinternal
*
* @brief   Set stack aggregation configuration per egress port.
*         All Forward DSA tagged packets transmitted from cascaded port with
*         these configurable source device number, source port numbers and
*         source IDs when feature enabled by
*         cpssDxChCscdPortStackAggregationEnableSet().
*         Configuration should be applied on aggregator device only.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] aggHwDevNum              - aggregator HW device number
* @param[in] aggPortNum               - aggregator port number
* @param[in] aggSrcId                 - aggregator source ID
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portNum
* @retval GT_OUT_OF_RANGE          - on wrong aggHwDevNum, aggPortNum, aggSrcId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 device: applicable starting from revision B0
*
*/
GT_STATUS cpssDxChCscdPortStackAggregationConfigSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_HW_DEV_NUM        aggHwDevNum,
    IN  GT_PORT_NUM          aggPortNum,
    IN  GT_U32               aggSrcId
);


/**
* @internal cpssDxChCscdPortStackAggregationConfigGet function
* @endinternal
*
* @brief   Get stack aggregation configuration per egress port.
*         All Forward DSA tagged packets transmitted from cascaded port with
*         these configurable source device number, source port numbers and
*         source IDs when feature enabled by
*         cpssDxChCscdPortStackAggregationEnableSet().
*         Configuration should be applied on aggregator device only.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] aggHwDevNumPtr           - (pointer to) aggregator HW device number
* @param[out] aggPortNumPtr            - (pointer to) aggregator port number
* @param[out] aggSrcIdPtr              - (pointer to) aggregator source ID
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 device: applicable starting from revision B0
*
*/
GT_STATUS cpssDxChCscdPortStackAggregationConfigGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_HW_DEV_NUM        *aggHwDevNumPtr,
    OUT GT_PORT_NUM          *aggPortNumPtr,
    OUT GT_U32               *aggSrcIdPtr
);

/**
* @internal cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet function
* @endinternal
*
* @brief   Set initial default ePort mapping assignment on ingress centralized chassis
*         enabled ports, when packets are received from line-card port/trunk accordingly.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portBaseEport            - base value used for default ePort mapping on
*                                      ingress CC enabled ports, when packets are
*                                      received from line-card port.
*                                      (APPLICABLE RANGES: 0..0x1FFFF)
* @param[in] trunkBaseEport           base value used for default ePort mapping on
*                                      ingress CC enabled ports, when packets are
*                                      received from line-card trunk.
*                                      (APPLICABLE RANGES: 0..0x1FFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
GT_STATUS cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_PORT_NUM                         portBaseEport,
    IN  GT_PORT_NUM                         trunkBaseEport
);

/**
* @internal cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet function
* @endinternal
*
* @brief   Get initial default ePort mapping assignment on ingress centralized chassis
*         enabled ports, when packets are received from line-card port/trunk accordingly.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] portBaseEportPtr         - (pointer to) base value used for default
*                                      ePort mapping on ingress CC enabled ports,
*                                      when packets are received from line-card port.
*                                      (APPLICABLE RANGES: 0..0x1FFFF)
* @param[out] trunkBaseEportPtr        - (pointer to) base value used for default
*                                      ePort mapping on ingress CC enabled ports,
*                                      when packets are received from line-card trunk.
*                                      (APPLICABLE RANGES: 0..0x1FFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_PORT_NUM                         *portBaseEportPtr,
    OUT GT_PORT_NUM                         *trunkBaseEportPtr
);

/**
* @internal cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet function
* @endinternal
*
* @brief   Set the amount of least significant bits taken from DSA tag
*         for assigning a default source ePort on CC ports, for packets received
*         from line-card device trunks/physical ports accordingly.
*         Relevant only when <CC Mode Enable> = Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] srcTrunkLsbAmount        - amount of least significant bits taken from DSA<SrcTrunk>
*                                      for assigning a default source ePort on CC ports, for
*                                      packets received from line-card device trunks.
*                                      Relevant only for packet DSA<SrcIsTrunk> = 1.
*                                      (APPLICABLE RANGES: 0..7)
* @param[in] srcPortLsbAmount         - amount of least significant bits taken from DSA<SrcPort>
*                                      for assigning a default source ePort on CC ports, for
*                                      packets received from line-card device physical ports.
*                                      Relevant only for packet DSA<SrcIsTrunk> = 0.
*                                      (APPLICABLE RANGES: 0..6)
* @param[in] srcDevLsbAmount          - amount of least significant bits taken from DSA<SrcDev>
*                                      for assigning a default source ePort on CC ports, for
*                                      packets received from line-card device physical ports.
*                                      Relevant only for packet DSA<SrcIsTrunk> = 0.
*                                      (APPLICABLE RANGES: 0..5)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
GT_STATUS cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              srcTrunkLsbAmount,
    IN  GT_U32                              srcPortLsbAmount,
    IN  GT_U32                              srcDevLsbAmount
);

/**
* @internal cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet function
* @endinternal
*
* @brief   Get the amount of least significant bits taken from DSA tag
*         for assigning a default source ePort on CC ports, for packets received
*         from line-card device trunks/physical ports accordingly.
*         Relevant only when <CC Mode Enable> = Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] srcTrunkLsbAmountPtr     - (pointer to) amount of least significant bits taken
*                                      from DSA<SrcTrunk> for assigning a default source
*                                      ePort on CC ports, for packets received from
*                                      line-card device trunks.
*                                      Relevant only for packet DSA<SrcIsTrunk> = 1.
*                                      (APPLICABLE RANGES: 0..7)
* @param[out] srcPortLsbAmountPtr      - (pointer to) amount of least significant bits taken
*                                      from DSA<SrcPort> for assigning a default source
*                                      ePort on CC ports, for packets received from
*                                      line-card device physical ports.
*                                      Relevant only for packet DSA<SrcIsTrunk> = 0.
*                                      (APPLICABLE RANGES: 0..6)
* @param[out] srcDevLsbAmountPtr       - (pointer to) amount of least significant bits taken
*                                      from DSA<SrcDev> for assigning a default source
*                                      ePort on CC ports, for packets received from
*                                      line-card device physical ports.
*                                      Relevant only for packet DSA<SrcIsTrunk> = 0.
*                                      (APPLICABLE RANGES: 0..5)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet
(
    IN  GT_U8                               devNum,
    OUT GT_U32                              *srcTrunkLsbAmountPtr,
    OUT GT_U32                              *srcPortLsbAmountPtr,
    OUT GT_U32                              *srcDevLsbAmountPtr
);

/**
* @internal cpssDxChCscdPortMruCheckOnCascadeEnableSet function
* @endinternal
*
* @brief   Enable/Disable MRU Check On Cascade Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number or CPU port
* @param[in] enable                   - MRU check Mode
*                                      GT_FALSE: Disable MRU Check On Cascade Port
*                                      GT_TRUE:  Enable MRU Check On Cascade Port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdPortMruCheckOnCascadeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             enable
);

/**
* @internal cpssDxChCscdPortMruCheckOnCascadeEnableGet function
* @endinternal
*
* @brief   Get (Enable/Disable) MRU Check On Cascade Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number or CPU port
*
* @param[out] enablePtr                - (pointer to) MRU check Mode
*                                      GT_FALSE: Disable MRU Check On Cascade Port
*                                      GT_TRUE:  Enable MRU Check On Cascade Port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdPortMruCheckOnCascadeEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_BOOL                             *enablePtr
);

/**
* @internal cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet function
* @endinternal
*
* @brief   Set the number of LSB bits taken from the DSA tag <source device> and
*         <source port>, that are used to map the My Physical port to a physical
*         port.
*         Relevant when <Assign Physical Port From DSA Enable> is enabled in the
*         My Physical Port Table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] srcDevLsbAmount          - the number of bits from the DSA <source device>
*                                      used for the mapping
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] srcPortLsbAmount         - the number of bits from the DSA <source port> used
*                                      for the mapping
*                                      (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
GT_STATUS cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet
(
    IN GT_U8        devNum,
    IN GT_U32       srcDevLsbAmount,
    IN GT_U32       srcPortLsbAmount
);

/**
* @internal cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet function
* @endinternal
*
* @brief   Get the number of LSB bits taken from the DSA tag <source device> and
*         <source port>, that are used to map the My Physical port to a physical
*         port.
*         Relevant when <Assign Physical Port From DSA Enable> is enabled in the
*         My Physical Port Table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] srcDevLsbAmountPtr       - (pointer to) the number of bits from the DSA
*                                      <source device> used for the mapping
* @param[out] srcPortLsbAmountPtr      - (pointer to) the number of bits from the DSA
*                                      <source port> used for the mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *srcDevLsbAmountPtr,
    OUT GT_U32      *srcPortLsbAmountPtr
);



/**
* @internal cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet function
* @endinternal
*
* @brief   Set the ingress physical port assignment mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
* @param[in] assignmentMode           - the physical port assignment mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet
(
    IN GT_U8                                                        devNum,
    IN GT_PHYSICAL_PORT_NUM                                         portNum,
    IN CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT     assignmentMode
);

/**
* @internal cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet function
* @endinternal
*
* @brief   Get the ingress physical port assignment mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
*
* @param[out] assignmentModePtr        - (pointer to) the physical port assignment mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet
(
    IN  GT_U8                                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                        portNum,
    OUT CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT    *assignmentModePtr
);

/**
* @internal cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet function
* @endinternal
*
* @brief   Set the ingress physical port base. It's used to map the My Physical
*         port to a physical port.
*         Relevant only when the ingress Physical Port assignment mode is
*         configured to Interlaken or DSA mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
* @param[in] physicalPortBase         - the physical port base (APPLICABLE RANGES: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
GT_STATUS cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   physicalPortBase
);

/**
* @internal cpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet function
* @endinternal
*
* @brief   Get the ingress physical port base. It's used to map the My Physical
*         port to a physical port.
*         Relevant only when the ingress Physical Port assignment mode is
*         configured to Interlaken or DSA mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
*
* @param[out] physicalPortBasePtr      - (pointer to) the physical port base
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *physicalPortBasePtr
);

/**
* @internal cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet function
* @endinternal
*
* @brief   Enable the assignment of the packet source device to be the local device
*         number
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
* @param[in] enable                   -  the assignment
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  enable
);

/**
* @internal cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet function
* @endinternal
*
* @brief   Get the enabling status of the assignment of the packet source device to
*         be the local device number
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
*
* @param[out] enablePtr                - (pointer to) the enabling status of the assignment
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet function
* @endinternal
*
* @brief   Enable mapping of target physical port to a remote physical port that
*         resides over a DSA-tagged interface
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - target device number
* @param[in] portNum                  - target physical port
* @param[in] enable                   -  mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);

/**
* @internal cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet function
* @endinternal
*
* @brief   Get the enabling status of the mapping of target physical port to a
*         remote physical port that resides over a DSA-tagged interface
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - target device number
* @param[in] portNum                  - target physical port
*
* @param[out] enablePtr                - (pointer to) the enabling status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    OUT GT_BOOL                   *enablePtr
);

/**
* @internal cpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet function
* @endinternal
*
* @brief   Map a target physical port to a remote physical port that resides over
*         a DSA-tagged interface
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - target device number
* @param[in] portNum                  - target physical port
* @param[in] remotePhysicalHwDevNum   - the remote physical HW device
*                                      (APPLICABLE RANGES: 0..4095)
* @param[in] remotePhysicalPortNum    - the remote physical port
*                                      (APPLICABLE RANGES: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The mapping should be enabled in order to take effect. To enable the
*       mapping use cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet
*
*/
GT_STATUS cpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_HW_DEV_NUM           remotePhysicalHwDevNum,
    IN  GT_PHYSICAL_PORT_NUM    remotePhysicalPortNum
);

/**
* @internal cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet function
* @endinternal
*
* @brief   Get the mapping of a target physical port to a remote physical port that
*         resides over a DSA-tagged interface
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - target device number
* @param[in] portNum                  - target physical port
*
* @param[out] remotePhysicalHwDevNumPtr - (pointer to) the remote physical HW device
* @param[out] remotePhysicalPortNumPtr - (pointer to) the remote physical port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The mapping takes effect only if it's enabled. To check whether it's
*       enabled use cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet
*
*/
GT_STATUS cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_HW_DEV_NUM           *remotePhysicalHwDevNumPtr,
    OUT GT_PHYSICAL_PORT_NUM    *remotePhysicalPortNumPtr
);

/**
* @internal cpssDxChCscdHashInDsaEnableSet function
* @endinternal
*
* @brief   Enable a packet's hash to be extracted from(rx) and/or inserted into(tx)
*         DSA tag. Relevant for FORWARD DSA tagged packets only.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - port's  (rx/tx/both)
* @param[in] enable                   - GT_TRUE:  enable
*                                      GT_FALSE: disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdHashInDsaEnableSet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN CPSS_PORT_DIRECTION_ENT direction,
    IN GT_BOOL                 enable
);

/**
* @internal cpssDxChCscdHashInDsaEnableGet function
* @endinternal
*
* @brief   Get value of flag determining whether a packet's hash should be
*         extracted from(rx)/inserted info(tx) DSA tag.
*         Relevand for FORWARD DSA tagged packets only.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - port's  (rx or tx. Not both!)
*
* @param[out] enablePtr                - (pointer to) flag value
*                                      GT_TRUE:  use DSA to extract/store hash.
*                                      GT_FALSE: don't use DSA to extract/store hash.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdHashInDsaEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal cpssDxChCscdPortTcProfiletSet function
* @endinternal
*
* @brief   Sets port TC profile on source/target port.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] portDirection            - port's direction.
* @param[in] portProfile              - port's profile values.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdPortTcProfiletSet
(
    IN    GT_U8                      devNum,
    IN    GT_PHYSICAL_PORT_NUM       portNum,
    IN    CPSS_PORT_DIRECTION_ENT    portDirection,
    IN    CPSS_DXCH_PORT_PROFILE_ENT portProfile
);

/**
* @internal cpssDxChCscdPortTcProfiletGet function
* @endinternal
*
* @brief   Gets port TC profile on source/target port.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] portDirection            - port's direction.
*
* @param[out] portProfilePtr           - (pointer to) port's profile values.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdPortTcProfiletGet
(
    IN    GT_U8                      devNum,
    IN    GT_PHYSICAL_PORT_NUM       portNum,
    IN    CPSS_PORT_DIRECTION_ENT    portDirection,
    OUT   CPSS_DXCH_PORT_PROFILE_ENT * portProfilePtr
);

/**
* @internal cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet function
* @endinternal
*
* @brief   Configure port mode regarding skipping of SA lookup.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] acceptSkipSaLookup    - Port's SA lookup skip mode.
*                                  - GT_TRUE:  Do not accept the <Skip FDB SA lookup> field in the eDSA tag
*                                  - GT_FALSE: Accept the <Skip FDB SA lookup> field in the eDSA tag
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet
(
    IN    GT_U8                   devNum,
    IN    GT_PHYSICAL_PORT_NUM    portNum,
    IN   GT_BOOL                 acceptSkipSaLookup
);

/**
* @internal cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet function
* @endinternal
*
* @brief   Gets port mode regarding skipping of SA lookup.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @param[out] acceptSkipSaLookup   - (pointer to) port's skip SA lookup skip mode.
*                                  - GT_TRUE:  Do not accept the <Skip FDB SA lookup> field in the eDSA tag
*                                  - GT_FALSE: Accept the <Skip FDB SA lookup> field in the eDSA tag
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet
(
    IN    GT_U8                   devNum,
    IN    GT_PHYSICAL_PORT_NUM    portNum,
    OUT   GT_BOOL                 *acceptSkipSaLookupPtr
);


/**
* @internal
*           cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidGet
*           function
* @endinternal
*
* @brief   Get TPID value of the VLAN Tag taken from the outermost VLAN Tag TPID port.
*           Relevant when enabling VLAN Tag based remote physical ports
*           mapping, this field specifies the TPID value of the VLAN Tag
*
* @note   APPLICABLE DEVICES:    Falcon.
* @note   NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - device number
*
* @param[out] vlanTagTpidPtr       - (pointer to) TPID value of the VLAN Tag
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidGet
(
  IN GT_U8        devNum,
  OUT GT_U32      *vlanTagTpidPtr
);



/**
* @internal
*           cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet
*           function
* @endinternal
*
* @brief   Set TPID value of the VLAN Tag taken from the
*           outermost VLAN Tag TPID port. Relevant when enabling
*           VLAN Tag based remote physical ports mapping, this
*           field specifies the TPID value of the VLAN Tag
*
* @note   APPLICABLE DEVICES:    Falcon.
* @note   NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum            - device number
* @param[in] vlanTagTpid       -  TPID value of the VLAN Tag.
*                                 (APPLICABLE RANGES:0..0xffff)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      vlanTagTpid
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChCscdh */


