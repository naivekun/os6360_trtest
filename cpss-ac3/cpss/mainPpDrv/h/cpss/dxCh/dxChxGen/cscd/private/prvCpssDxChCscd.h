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
* @file prvCpssDxChCscd.h
*
* @brief CPSS DXCH Cascading private API.
*
* @version   11
********************************************************************************
*/
#ifndef __prvCpssDxChCscdh
#define __prvCpssDxChCscdh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cscd/cpssGenCscd.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>

#define PRV_CPSS_DXCH_CSCD_DEV_MAP_INDEX_ARRAY_MAX_SIZE 4
/* max number of port groups to use for trunk WA "cascade trunk : all ports in the same hemisphere "*/
#define PRV_CPSS_DXCH_CSCD_DEV_MAP_TRUNK_WA_PORT_GROUPS_MAX_CNS    2

/**
* @internal prvCpssDxChCscdDevMapTableEntryHwFormatGet function
* @endinternal
*
* @brief   Get Cascade Device Map table entry in HW format
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] cascadeLinkPtr           - (pointer to)A structure holding the cascade link type
*                                      (port (APPLICABLE RANGES: 0..63) or
*                                      trunk(APPLICABLE RANGES: 0..127)) and the link number
*                                      leading to the target device.
* @param[in] srcPortTrunkHashEn       - Relevant when (cascadeLinkPtr->linkType ==
*                                      CPSS_CSCD_LINK_TYPE_TRUNK_E)
*                                      Enabled to set the load balancing trunk hash for packets
*                                      forwarded via an trunk uplink to be based on the
*                                      packet’s source port, and not on the packets data.
*                                      Indicates the type of uplink.
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E -
*                                      Load balancing trunk hash is based on the ingress pipe
*                                      hash mode as configured by function
*                                      cpssDxChTrunkHashGeneralModeSet(...)
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E - Load
*                                      balancing trunk hash for this cascade trunk interface is
*                                      based on the packet’s source port, regardless of the
*                                      ingress pipe hash mode
*                                      NOTE : this parameter is relevant only to DXCH2
*                                      and above devices
* @param[in] egressAttributesLocallyEn - Determines whether the egress attributes
*                                      are determined by the target ePort even if the target
*                                      device is not the local device.
*                                      (APPLICABLE DEVICES Lion2)
*
* @param[out] hwDataPtr                - Pointer to Device Map Table entry format data.
* @param[out] numBitsPtr               - Pointer to number of bits in the entry.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad trunk hash mode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCscdDevMapTableEntryHwFormatGet
(
    IN  GT_U8                        devNum,
    IN  CPSS_CSCD_LINK_TYPE_STC      *cascadeLinkPtr,
    IN  CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT srcPortTrunkHashEn,
    IN GT_BOOL                      egressAttributesLocallyEn,
    OUT GT_U32                       *hwDataPtr,
    OUT GT_U32                       *numBitsPtr
);

/**
* @internal prvCpssDxChCscdDevMapTableEntryGet function
* @endinternal
*
* @brief   Get the cascade map table entry data from hardware format.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] hwData                   - hardware format data
* @param[in] hemisphereNum            - hemisphere Number (relevant only to dual TXQs devices)
*
* @param[out] cascadeLinkPtr           - (pointer to)A structure holding the cascade link type
*                                      (port (APPLICABLE RANGES: 0..63)
*                                      or trunk(APPLICABLE RANGES: 0..127)) and the link number
*                                      leading to the target device.
* @param[out] srcPortTrunkHashEnPtr    - Relevant when (cascadeLinkPtr->linkType ==
*                                      CPSS_CSCD_LINK_TYPE_TRUNK_E)
*                                      (pointer to) Enabled to set the load balancing trunk
*                                      hash for packets forwarded via an trunk uplink to be
*                                      based on the packet’s source port, and not on the
*                                      packets data.
*                                      Indicates the type of uplink.
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E -
*                                      Load balancing trunk hash is based on the ingress pipe
*                                      hash mode as configured by function
*                                      cpssDxChTrunkHashGeneralModeSet(...)
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E - Load
*                                      balancing trunk hash for this cascade trunk interface is
*                                      based on the packet’s source port, regardless of the
*                                      ingress pipe hash mode
*                                      NOTE : this parameter is relevant only to DXCH2 and
*                                      above devices
* @param[out] egressAttributesLocallyEnPtr - Determines whether the egress attributes
*                                      are determined by the target port even if the target
*                                      device is not the local device.
*                                      (APPLICABLE DEVICES Lion2)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on bad value in HW
*/
GT_STATUS prvCpssDxChCscdDevMapTableEntryGet
(
    IN GT_U8                         devNum,
    IN GT_U32                        hwData,
    IN GT_U32                        hemisphereNum,
    OUT CPSS_CSCD_LINK_TYPE_STC      *cascadeLinkPtr,
    OUT CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT *srcPortTrunkHashEnPtr,
    OUT GT_BOOL                      *egressAttributesLocallyEnPtr
);

/**
* @internal prvCpssDxChCscdDevMapByArrayIndexesTableSet function
* @endinternal
*
* @brief   Set the 'device map table' (used for cascaded system) by pre-calculated indexes.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] numOfIndexes             - number of indexes in indexArrPtr. (value 1..PRV_CPSS_DXCH_CSCD_DEV_MAP_INDEX_ARRAY_MAX_SIZE)
* @param[in] indexesArr[PRV_CPSS_DXCH_CSCD_DEV_MAP_INDEX_ARRAY_MAX_SIZE] - pointer to list of indexes in Device Map table.
* @param[in] cascadeLinkPtr           - (pointer to)A structure holding the cascade link type
*                                      (port (APPLICABLE RANGES: 0..63)
*                                      or trunk(APPLICABLE RANGES: 0..127)) and the link number
*                                      leading to the target device.
* @param[in] srcPortTrunkHashEn       - Relevant when (cascadeLinkPtr->linkType ==
*                                      CPSS_CSCD_LINK_TYPE_TRUNK_E)
*                                      Enabled to set the load balancing trunk hash for packets
*                                      forwarded via an trunk uplink to be based on the
*                                      packet’s source port, and not on the packets data.
*                                      Indicates the type of uplink.
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E -
*                                      Load balancing trunk hash is based on the ingress pipe
*                                      hash mode as configured by function
*                                      cpssDxChTrunkHashGeneralModeSet(...)
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E - Load
*                                      balancing trunk hash for this cascade trunk interface is
*                                      based on the packet’s source port, regardless of the
*                                      ingress pipe hash mode
*                                      NOTE : this parameter is relevant only to DXCH2
*                                      and above devices
* @param[in] egressAttributesLocallyEn - Determines whether the egress attributes
*                                      are determined by the target port even if the target
*                                      device is not the local device.
*                                      (APPLICABLE DEVICES Lion2)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or target device or bad trunk hash
*                                       mode or bad target Port
* @retval GT_BAD_STATE             - the trunk is in bad state , one of:
*                                       1. empty trunk (APPLICABLE DEVICES Lion2)
*                                       2. hold members from no local device (APPLICABLE DEVICES Lion2)
*                                       3. hold members from more then single hemisphere (APPLICABLE DEVICES Lion2)
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCscdDevMapByArrayIndexesTableSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       numOfIndexes,
    IN GT_U32                       indexesArr[PRV_CPSS_DXCH_CSCD_DEV_MAP_INDEX_ARRAY_MAX_SIZE],
    IN CPSS_CSCD_LINK_TYPE_STC      *cascadeLinkPtr,
    IN CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT srcPortTrunkHashEn,
    IN GT_BOOL                      egressAttributesLocallyEn
);

/**
* @internal prvCpssDxChCscdPortSet function
* @endinternal
*
* @brief   Set ingress and egress port as cascaded/non-cascade .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - The physical port number.
*                                      NOTE: this port is not covered and is NOT checked for 'mapping'
* @param[in] portRxDmaNum             - The RXDMA number for cascading.
* @param[in] doIngress                - indication to configure the ingress part
* @param[in] doEgress                 - indication to configure the egress part
* @param[in] portType                 - cascade  type regular/extended DSA tag port or network port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCscdPortSet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN GT_U32                       portRxDmaNum,
    IN GT_U32                       doIngress,
    IN GT_U32                       doEgress,
    IN CPSS_CSCD_PORT_TYPE_ENT      portType
);

/**
* @internal prvCpssDxChCscdMyPhysicalPortAttributesRegAddressGet function
* @endinternal
*
* @brief   Get address of myPhysicalPort Attributes register by myPhysicalPort number.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] myPhysicalPortNum        - my physical port number assigned by RxDMA unit.
*                                      Assumed that physical port (used as cascade)
*                                      with such Id mapped.
*
* @param[out] regAddrPtr               - (pointer to) register address.
* @param[out] portGroupNumPtr          - (pointer to) the port group Id that the
*                                         'physical port' belong to
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*/
GT_STATUS prvCpssDxChCscdMyPhysicalPortAttributesRegAddressGet
(
    IN   GT_U8                    devNum,
    IN   GT_PHYSICAL_PORT_NUM     myPhysicalPortNum,
    OUT  GT_U32                   *regAddrPtr,
    OUT  GT_U32                   *portGroupNumPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChCscdh */


