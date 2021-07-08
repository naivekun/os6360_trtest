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
* @file cpssDxChBrgE2Phy.h
*
* @brief Eport to Physical port tables facility CPSS DxCh implementation.
*
* @version   5
********************************************************************************
*/
#ifndef __cpssDxChBrgE2Phy
#define __cpssDxChBrgE2Phy

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnelTypes.h>

/**
* @struct CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC
 *
 * @brief Egress port info
*/
typedef struct{

    /** @brief GT_TRUE: redirected to Tunnel Egress Interface
     *  GT_FALSE: redirected to non-Tunnel Egress Interface
     */
    GT_BOOL tunnelStart;

    /** @brief pointer to the Tunnel Start entry. valid if <tunnelStart>
     *  is GT_TRUE
     */
    GT_U32 tunnelStartPtr;

    /** @brief Type of passenger packet for packet redirected
     *  to Tunnel-Start.
     */
    CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT tsPassengerPacketType;

    /** pointer to the ARP entry */
    GT_U32 arpPtr;

    /** @brief Indicates that the MAC SA of the packet is changed
     *  to the address of the current device.
     *  Relevant only when the <TS> flag is disabled.
     */
    GT_BOOL modifyMacSa;

    /** @brief Indicates that the MAC DA of the packet should
     *  be changed according to the <ARP pointer> field.
     *  Relevant only when the <TS> flag is disabled.
     */
    GT_BOOL modifyMacDa;

    /** @brief Determines whether the HA egress ePort table
     *  tunnel start / ARP related fields
     *  (i.e. modify MAC DA / SA, TS pointer etc')
     *  affects the TS/ARP assignment logic.
     *  False: Do not force HA egress ePort
     *  table TS/ARP decision
     *  True: Force HA egress ePort table
     *  TS/ARP decision
     */
    GT_BOOL forceArpTsPortDecision;

} CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC;

/**
* @internal cpssDxChBrgEportToPhysicalPortTargetMappingTableSet function
* @endinternal
*
* @brief   Set physical info for the given ePort in the E2PHY mapping table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] physicalInfoPtr          - (pointer to) physical Information related to the ePort
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note E2PHY table is accessed by the packet's target ePort
*
*/
GT_STATUS cpssDxChBrgEportToPhysicalPortTargetMappingTableSet
(
    IN GT_U8                            devNum,
    IN GT_PORT_NUM                      portNum,
    IN CPSS_INTERFACE_INFO_STC          *physicalInfoPtr
);

/**
* @internal cpssDxChBrgEportToPhysicalPortTargetMappingTableGet function
* @endinternal
*
* @brief   Get physical info for the given ePort in the E2PHY mapping table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] physicalInfoPtr          - (pointer to) physical Information related to the ePort
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note E2PHY table is accessed by the packet's target ePort
*
*/
GT_STATUS cpssDxChBrgEportToPhysicalPortTargetMappingTableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT CPSS_INTERFACE_INFO_STC             *physicalInfoPtr
);

/**
* @internal cpssDxChBrgEportToPhysicalPortEgressPortInfoSet function
* @endinternal
*
* @brief   Set physical info for the given ePort in the HA Egress port table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] egressInfoPtr            - (pointer to) physical information related to the egress port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEportToPhysicalPortEgressPortInfoSet
(
    IN GT_U8                                devNum,
    IN GT_PORT_NUM                          portNum,
    IN CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC   *egressInfoPtr
);

/**
* @internal cpssDxChBrgEportToPhysicalPortEgressPortInfoGet function
* @endinternal
*
* @brief   Get physical info for the given ePort in the HA Egress port table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] egressInfoPtr            - (pointer to) physical Information related to the egress port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgEportToPhysicalPortEgressPortInfoGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC  *egressInfoPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChBrgE2Phy */


