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
* @file cpssDxChBrgNestVlan.h
*
* @brief CPSS DxCh Nested VLANs facility implementation.
*
* @version   9
********************************************************************************
*/

#ifndef __cpssDxChBrgNestVlanh
#define __cpssDxChBrgNestVlanh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>

/**
* @internal cpssDxChBrgNestVlanAccessPortSet function
* @endinternal
*
* @brief   Configure given port as Nested VLAN access port.
*         The VID of all the packets received on Nested VLAN access port is
*         discarded and they are assigned with the Port VID that set by
*         cpssDxChBrgVlanPortVidSet(). This VLAN assignment may be subsequently
*         overridden by the protocol based VLANs or policy based VLANs algorithms.
*         When a packet received on an access port is transmitted via a core port
*         or a cascading port, a VLAN tag is added to the packet, in addition to
*         any existing VLAN tag.
*         The 802.1p User Priority field of this added tag may be one of the
*         following, based on the ModifyUP QoS parameter set to the packet at
*         the end of the Ingress pipe:
*         - If ModifyUP is "Modify", it is the UP extracted from the
*         QoS Profile table entry that configured by the
*         cpssDxChCosProfileEntrySet().
*         - If ModifyUP is "Not Modify", it is the original packet
*         802.1p User Priority field, if it is tagged. If untagged,
*         it is ingress port's default user priority that configured by
*         cpssDxChPortDefaultUPSet().
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] portNum                  - port number or CPU port number.
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   -  GT_TRUE  - port is Nested VLAN Access Port.
*                                      GT_FALSE - port is usual port (Core or Customer).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To complete Access Port Configuration do the following:
*       For DxCh1, DxCh2 and DxCh3 devices:
*       - by API cpssDxChBrgNestVlanEtherTypeSelectSet(), set
*       the Port VLAN Selection to the CPSS_VLAN_ETHERTYPE0_E;
*       - update egress Port VLAN EtherType;
*       - by API cpssDxChBrgVlanForcePvidEnable(), set
*       Port VID Assignment mode.
*       To complete Core Port Configuration, select CPSS_VLAN_ETHERTYPE1_E
*       for Usual Port.
*       To complete Customers Bridges(bridges that don't employ Nested VLANs)
*       Configuration, select CPSS_VLAN_ETHERTYPE0_E for Usual Port.
*       For xCat and above devices:
*       See CPSS user guide how to configure Nested VLAN or TR101 features.
*
*/
GT_STATUS cpssDxChBrgNestVlanAccessPortSet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    IN  GT_BOOL         enable
);

/**
* @internal cpssDxChBrgNestVlanAccessPortGet function
* @endinternal
*
* @brief   Gets configuration of the given port (Nested VLAN access port
*         or Core/Customer port).
*         The VID of all the packets received on Nested VLAN access port is
*         discarded and they are assigned with the Port VID that set by
*         cpssDxChBrgVlanPortVidSet(). This VLAN assignment may be subsequently
*         overridden by the protocol based VLANs or policy based VLANs algorithms.
*         When a packet received on an access port is transmitted via a core port
*         or a cascading port, a VLAN tag is added to the packet, in addition to
*         any existing VLAN tag.
*         The 802.1p User Priority field of this added tag may be one of the
*         following, based on the ModifyUP QoS parameter set to the packet at
*         the end of the Ingress pipe:
*         - If ModifyUP is "Modify", it is the UP extracted from the
*         QoS Profile table entry that configured by the
*         cpssDxChCosProfileEntrySet().
*         - If ModifyUP is "Not Modify", it is the original packet
*         802.1p User Priority field, if it is tagged. If untagged,
*         it is ingress port's default user priority that configured by
*         cpssDxChPortDefaultUPSet().
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] portNum                  - physical or CPU port number.
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - Pointer to the Boolean value:
*                                      GT_TRUE   - port is Nested VLAN Access Port.
*                                      GT_FALSE  - port is usual port:
*                                      - Core Port in case of
*                                      CPSS_VLAN_ETHERTYPE1_E selected;
*                                      - Customers Bridges (bridges that don't
*                                      employ Nested VLANs) in case of
*                                      CPSS_VLAN_ETHERTYPE0_E selected;
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum values.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgNestVlanAccessPortGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChBrgNestVlanEtherTypeSelectSet function
* @endinternal
*
* @brief   Sets selected port VLAN EtherType.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - PP's device number.
* @param[in] portNum                  - physical or CPU port number.
* @param[in] ingressVlanSel           - selected port VLAN EtherType.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum or ingressVlanSel values.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For the VLAN EtherType coherency between Ingress and Egress,
*       the VLAN EtherType shall be configured to match the ingressVlanSel.
*
*/
GT_STATUS cpssDxChBrgNestVlanEtherTypeSelectSet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             portNum,
    IN  CPSS_ETHER_MODE_ENT     ingressVlanSel
);

/**
* @internal cpssDxChBrgNestVlanEtherTypeSelectGet function
* @endinternal
*
* @brief   Gets selected port VLAN EtherType.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - PP's device number.
* @param[in] portNum                  - physical or CPU port number.
*
* @param[out] ingressVlanSelPtr        - pointer to selected port VLAN EtherType.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum values.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgNestVlanEtherTypeSelectGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             portNum,
    OUT CPSS_ETHER_MODE_ENT     *ingressVlanSelPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChBrgNestVlanh */

