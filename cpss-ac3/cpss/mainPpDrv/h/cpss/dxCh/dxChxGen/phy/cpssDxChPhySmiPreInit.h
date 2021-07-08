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
* @file cpssDxChPhySmiPreInit.h
*
* @brief API implementation for PHY SMI pre init configuration.
* All functions should be called after cpssDxChHwPpPhase1Init() and
* before cpssDxChPhyPortSmiInit().
*
* @version   15
********************************************************************************
*/
#ifndef __cpssDxChPhySmiPreInith
#define __cpssDxChPhySmiPreInith

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/phy/cpssGenPhySmi.h>

/**
* @enum CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT
 *
 * @brief Enumeration of Auto Poll numbers of port
*/
typedef enum{

    /** 8 ports polling */
    CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E = 8,

    /** 12 ports polling */
    CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E = 12,

    /** 16 ports polling */
    CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_16_E = 16

} CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT;


/**
* @internal cpssDxChPhyPortAddrSet function
* @endinternal
*
* @brief   Configure the port's default phy address, this function should be
*         used to change the default port's phy address.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - port number.
* @param[in] phyAddr                  - The new phy address, (APPLICABLE RANGES: 0...31).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_SUPPORTED         - for XG ports
* @retval GT_OUT_OF_RANGE          - phyAddr bigger then 31
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For flex ports this API and cpssDxChPhyPortSmiInterfaceSet() must be
*       used before any access to flex port phy related registers.
*       This function should be called after cpssDxChHwPpPhase1Init()
*       and before cpssDxChPhyPortSmiInit().
*
*/
GT_STATUS cpssDxChPhyPortAddrSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8   phyAddr
);

/**
* @internal cpssDxChPhyPortAddrGet function
* @endinternal
*
* @brief   Gets port's phy address from hardware and from database.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - port number.
*
* @param[out] phyAddFromHwPtr          - (pointer to) phy address in HW.
* @param[out] phyAddFromDbPtr          - (pointer to) phy address in DB.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_SUPPORTED         - for XG ports
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For flex ports no hardware value exists and this field should be
*       ignored.
*
*/

GT_STATUS cpssDxChPhyPortAddrGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U8   *phyAddFromHwPtr,
    OUT GT_U8   *phyAddFromDbPtr
);

/**
* @internal cpssDxChPhyAutoPollNumOfPortsSet function
* @endinternal
*
* @brief   Configure number of auto poll ports for SMI interfaces.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] autoPollNumOfPortsSmi0   - number of ports for SMI0.
* @param[in] autoPollNumOfPortsSmi1   - number of ports for SMI1.
* @param[in] autoPollNumOfPortsSmi2   - number of ports for SMI2.
*                                      (APPLICABLE DEVICES Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
* @param[in] autoPollNumOfPortsSmi3   - number of ports for SMI3.
*                                      (APPLICABLE DEVICES Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, autoPollNumOfPortsSmi0,
*                                       autoPollNumOfPortsSmi1,
*                                       autoPollNumOfPortsSmi2,
*                                       autoPollNumOfPortsSmi3
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The acceptable combinations for Auto Poll number of ports are:
*       |-----------------------|
*       |  SMI 0  |  SMI 1  |
*       |-----------|-----------|
*       |   8   |  16   |
*       |   8   |  12   |
*       |  12   |  12   |
*       |  16   |  8   |
*       |-----------------------|
*       |-----------------------|
*       |  SMI 2  |  SMI 3  |
*       |-----------|-----------|
*       |   8   |  16   |
*       |   8   |  12   |
*       |  12   |  12   |
*       |  16   |  8   |
*       |-----------------------|
*       This function should be called after cpssDxChHwPpPhase1Init()
*       and before cpssDxChPhyPortSmiInit().
*
*/
GT_STATUS cpssDxChPhyAutoPollNumOfPortsSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    autoPollNumOfPortsSmi0,
    IN  CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    autoPollNumOfPortsSmi1,
    IN  CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    autoPollNumOfPortsSmi2,
    IN  CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    autoPollNumOfPortsSmi3
);

/**
* @internal cpssDxChPhyAutoPollNumOfPortsGet function
* @endinternal
*
* @brief   Get number of auto poll ports for SMI interfaces.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2; Bobcat2; Caelum;  Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] autoPollNumOfPortsSmi0Ptr - number of ports for SMI0.
* @param[out] autoPollNumOfPortsSmi1Ptr - number of ports for SMI1.
* @param[out] autoPollNumOfPortsSmi2Ptr - number of ports for SMI2.
*                                      (APPLICABLE DEVICES Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
* @param[out] autoPollNumOfPortsSmi3Ptr - number of ports for SMI3.
*                                      (APPLICABLE DEVICES Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, autoPollNumOfPortsSmi0,
*                                       autoPollNumOfPortsSmi1,
*                                       autoPollNumOfPortsSmi2,
*                                       autoPollNumOfPortsSmi3
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhyAutoPollNumOfPortsGet
(
    IN  GT_U8                                           devNum,
    OUT CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    *autoPollNumOfPortsSmi0Ptr,
    OUT CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    *autoPollNumOfPortsSmi1Ptr,
    OUT CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    *autoPollNumOfPortsSmi2Ptr,
    OUT CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    *autoPollNumOfPortsSmi3Ptr
);

/**
* @internal cpssDxChPhyPortSmiInterfaceSet function
* @endinternal
*
* @brief   Configure port SMI interface.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - port number.
* @param[in] smiInterface             - port SMI interface.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, portNum or SMI interface.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For flex ports this API and cpssDxChPhyPortAddrSet() must be
*       used before any access to flex port PHY related registers.
*       This function should be called after cpssDxChHwPpPhase1Init()
*       and before cpssDxChPhyPortSmiInit().
*
*/
GT_STATUS cpssDxChPhyPortSmiInterfaceSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PHY_SMI_INTERFACE_ENT  smiInterface
);

/**
* @internal cpssDxChPhyPortSmiInterfaceGet function
* @endinternal
*
* @brief   Get port SMI interface.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - port number.
*
* @param[out] smiInterfacePtr          - (pointer to) port SMI interface.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, portNum.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - port SMI interface was not initialized
*                                       correctly.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhyPortSmiInterfaceGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PHY_SMI_INTERFACE_ENT  *smiInterfacePtr
);

/**
* @internal cpssDxChPhyPortSmiInterfacePortGroupSet function
* @endinternal
*
* @brief   This API maps a port to SMI interface in a multi port Groups devices.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - port number.
* @param[in] smiInterfacePortGroup    - the port group of SMI interface to configure the port.
*                                      (APPLICABLE RANGES: Lion 0..3; Lion2 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note this API doesn't access the HW but only updates an internal database.
*       After calling this API the application must access the PHYs by
*       cpssDxChPhyPortSmiRegisterRead and cpssDxChPhyPortSmiRegisterWrite
*
*/
GT_STATUS cpssDxChPhyPortSmiInterfacePortGroupSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32  smiInterfacePortGroup
);
/**
* @internal cpssDxChPhyPortSmiInterfacePortGroupGet function
* @endinternal
*
* @brief   This API gets the SMI interface connected to a port in a multi port Groups
*         devices.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - port number.
*
* @param[out] smiInterfacePortGroupPtr - (pointer to) port group of SMI interface.
*                                      (APPLICABLE RANGES: Lion 0..3; Lion2 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhyPortSmiInterfacePortGroupGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_U32  *smiInterfacePortGroupPtr
);

/**
* @internal cpssDxChPhyPortXSmiInterfaceSet function
* @endinternal
*
* @brief   Configure port XSMI interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - port number.
* @param[in] smiInterface             - port XSMI interface.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, portNum or XSMI interface.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on wrong port type
*/
GT_STATUS cpssDxChPhyPortXSmiInterfaceSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT  smiInterface
);

/**
* @internal cpssDxChPhyPortXSmiInterfaceGet function
* @endinternal
*
* @brief   Get port XSMI interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - port number.
*
* @param[out] smiInterfacePtr          - (pointer to) port XSMI interface.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, portNum or XSMI interface.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on wrong port type
*/
GT_STATUS cpssDxChPhyPortXSmiInterfaceGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PHY_XSMI_INTERFACE_ENT  *smiInterfacePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPhySmiPreInith */


