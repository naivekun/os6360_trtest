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
* @file cpssDxChPhySmi.h
*
* @brief API implementation for port Core Serial Management Interface facility.
*
* @version   22
********************************************************************************
*/
#ifndef __cpssDxChPhySmih
#define __cpssDxChPhySmih

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/phy/cpssGenPhySmi.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmiPreInit.h>


/**
* @internal cpssDxChPhyPortSmiInit function
* @endinternal
*
* @brief   Initialiaze the SMI control register port, Check all GE ports and
*         activate the errata initialization fix.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhyPortSmiInit
(
    IN GT_U8 devNum
);

/**
* @internal cpssDxChPhyPortSmiRegisterRead function
* @endinternal
*
* @brief   Read specified SMI Register on a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] phyReg                   - SMI register (APPLICABLE RANGES: 0..31)
*
* @param[out] dataPtr                  - (pointer to) the read data.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_OUT_OF_RANGE          - phyAddr bigger then 31
* @retval GT_NOT_SUPPORTED         - for XG ports
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - Flex port SMI or PHY address were not set
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhyPortSmiRegisterRead
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U8     phyReg,
    OUT GT_U16    *dataPtr
);

/**
* @internal cpssDxChPhyPortSmiRegisterWrite function
* @endinternal
*
* @brief   Write value to specified SMI Register on a specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number.
* @param[in] phyReg                   - The new phy address, (APPLICABLE RANGES: 0...31).
* @param[in] data                     - Data to write.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_SUPPORTED         - for XG ports
* @retval GT_OUT_OF_RANGE          - phyAddr bigger then 31
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - Flex port SMI or PHY address were not set
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhyPortSmiRegisterWrite
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U8     phyReg,
    IN  GT_U16    data
);

/**
* @internal cpssDxChPhyPort10GSmiRegisterRead function
* @endinternal
*
* @brief   Read specified SMI Register and PHY device of specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] phyId                    - ID of external PHY (APPLICABLE RANGES: 0..31).
* @param[in] useExternalPhy           - boolean variable, defines if to use external PHY
*                                      NOTE: for DxCh3 and above only GT_TRUE supported !
* @param[in] phyReg                   - SMI register, the register of PHY to read from
* @param[in] phyDev                   - the PHY device to read from (APPLICABLE RANGES: 0..31).
*
* @param[out] dataPtr                  - (Pointer to) the read data.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_OUT_OF_RANGE          - phyAddr bigger then 31
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If useExternalPhy flag is GT_FALSE, the phyId is being ignored
*
*/
GT_STATUS cpssDxChPhyPort10GSmiRegisterRead
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U8                               phyId,
    IN  GT_BOOL                             useExternalPhy,
    IN  GT_U16                              phyReg,
    IN  GT_U8                               phyDev,
    OUT GT_U16                              *dataPtr
);

/**
* @internal cpssDxChPhyPort10GSmiRegisterWrite function
* @endinternal
*
* @brief   Write value to a specified SMI Register and PHY device of
*         specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number.
* @param[in] phyId                    - ID of external PHY (APPLICABLE RANGES: 0...31).
* @param[in] useExternalPhy           - Boolean variable, defines if to use external PHY
*                                      NOTE: for DxCh3 and above only GT_TRUE supported !
* @param[in] phyReg                   - SMI register, the register of PHY to read from
* @param[in] phyDev                   - the PHY device to read from (APPLICABLE RANGES: 0..31).
* @param[in] data                     - Data to write.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_OUT_OF_RANGE          - phyAddr bigger then 31
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If useExternalPhy flag is GT_FALSE, the phyId is being ignored
*
*/
GT_STATUS cpssDxChPhyPort10GSmiRegisterWrite
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U8                               phyId,
    IN  GT_BOOL                             useExternalPhy,
    IN  GT_U16                              phyReg,
    IN  GT_U8                               phyDev,
    IN  GT_U16                              data
);

/**
* @internal cpssDxChPhyAutonegSmiGet function
* @endinternal
*
* @brief   This function gets the auto negotiation status between the PP and PHY.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum;  Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Aldrin.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - port number
*
* @param[out] enabledPtr               - (Pointer to) the auto negotiation process state between
*                                      PP and Phy:
*                                      0 = Auto-Negotiation process is preformed.
*                                      1 = Auto-Negotiation process is not preforme
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Although the device ignores the information read from the PHY
*       registers, it keeps polling those registers.
*
*/
GT_STATUS cpssDxChPhyAutonegSmiGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL *enabledPtr
);

/**
* @internal cpssDxChPhyAutonegSmiSet function
* @endinternal
*
* @brief   This function sets the auto negotiation process, between
*         the PP and PHY, to enable/disable.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Aldrin.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - port number
* @param[in] enable                   - 0 = Auto-Negotiation process is preformed.
*                                      1 = Auto-Negotiation process is not preforme
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.Although the device ignores the information read from
*       the PHY registers, it keeps polling those registers.
*       2.Set affects all ports using SMI interface of this port
*
*/
GT_STATUS cpssDxChPhyAutonegSmiSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChPhySmiAutoMediaSelectSet function
* @endinternal
*
* @brief   This function gets a bit for port
*         indicating whether this port is connected to a dual-media PHY,
*         such as Marvell 88E1112, that is able to perform Auto-media select
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Aldrin.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - port number: applicable ports from 0-23 (single LMS) 0-47 (2 LMS or 4 SMI units)
* @param[in] autoMediaSelect          - 0 = Port is not connected to dual-media PHY
*                                      1 = Port is connected to dual-media PHY
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_OUT_OF_RANGE          - phyAddr bigger then 31
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhySmiAutoMediaSelectSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32   autoMediaSelect
);

/**
* @internal cpssDxChPhySmiAutoMediaSelectGet function
* @endinternal
*
* @brief   This function gets a bit for given port
*         indicating whether this port is connected to a dual-media PHY,
*         such as Marvell 88E1112, that is able to perform Auto-media select
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Aldrin.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - port number: applicable ports from 0-23 (single LMS) 0-47 (2 LMS or 4 SMI units)
*
* @param[out] autoMediaSelectPtr       - (Pointer to) port state
*                                      0 = Port is not connected to dual-media PHY
*                                      1 = Port is connected to dual-media PHY
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhySmiAutoMediaSelectGet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32   *autoMediaSelectPtr
);

/**
* @internal cpssDxChPhyXsmiMdcDivisionFactorSet function
* @endinternal
*
* @brief   This function sets MDC frequency for Master XSMI Interface
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
* @param[in] xsmiInterface            - XSMI master interface number
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
* @param[in] divisionFactor           - Division factor of the core clock to get the MDC
*                                      (Serial Management Interface Clock).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, xsmiInterface or divisionFactor
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhyXsmiMdcDivisionFactorSet
(
    IN GT_U8 devNum,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT    xsmiInterface,
    IN CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactor
);

/**
* @internal cpssDxChPhyXsmiMdcDivisionFactorGet function
* @endinternal
*
* @brief   This function gets MDC frequency for Master XSMI Interface
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
* @param[in] xsmiInterface            - XSMI master interface number
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @param[out] divisionFactorPtr        - (Pointer to) Division factor of the core clock
*                                      to get the MDC (Serial Management Interface Clock).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, xsmiInterface or divisionFactorPtr
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhyXsmiMdcDivisionFactorGet
(
    IN  GT_U8 devNum,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT    xsmiInterface,
    OUT CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT *divisionFactorPtr
);

/**
* @internal cpssDxChPhySmiMdcDivisionFactorSet function
* @endinternal
*
* @brief   This function sets Fast MDC Division Selector
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - port number.
* @param[in] divisionFactor           - Division factor of the core clock to get the MDC
*                                      (Serial Management Interface Clock).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or divisionFactor
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhySmiMdcDivisionFactorSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactor
);

/**
* @internal cpssDxChPhySmiMdcDivisionFactorGet function
* @endinternal
*
* @brief   This function gets Fast MDC Division Selector
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - port number.
*
* @param[out] divisionFactorPtr        - (Pointer to) Division factor of the core clock
*                                      to get the MDC (Serial Management Interface Clock).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or divisionFactor
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhySmiMdcDivisionFactorGet
(
    IN  GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT *divisionFactorPtr
);

/**
* @internal cpssDxChPhyBobcat2SmiXsmiMuxSet function
* @endinternal
*
* @brief   Bobcat2 has XSMI#1 and SMI#3 but mutual exclusive. Both of them use same
*         pins. This means either XSMI#1 or SMI#3 may be used on board.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number.
* @param[in] enableXsmi               - XSMI master interface number:
*                                      GT_TRUE - use XSMI#1
*                                      GT_FALSE - use SMI#3
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhyBobcat2SmiXsmiMuxSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enableXsmi
);

/**
* @internal cpssDxChPhyBobcat2SmiXsmiMuxGet function
* @endinternal
*
* @brief   Get what interface is active XSMI#1 or SMI#3.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] enableXsmiPtr            - XSMI master interface number:
*                                      GT_TRUE - use XSMI#1
*                                      GT_FALSE - use SMI#3
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - enableXsmiPtr == NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhyBobcat2SmiXsmiMuxGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enableXsmiPtr
);


/**
* @internal cpssDxChPhyXSmiMDCInvertSet function
* @endinternal
*
* @brief   Set Invert or Do Not Invert XSmi MDC.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] invertMDC                - flag - invert or do not invert XSMI MDC
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note This function changed bit in the register.
*
*/
GT_STATUS cpssDxChPhyXSmiMDCInvertSet
(
    IN  GT_U8    devNum,
    IN  GT_BOOL  invertMDC
);

/**
* @internal cpssDxChPhyXSmiMDCInvertGet function
* @endinternal
*
* @brief   Get XSmi MDC setting (Invert or Do Not Invert)
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] invertMDCPtr             - Flag - Invert or Do Not Invert XSMI MDC
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note This function read bit in the register.
*
*/
GT_STATUS cpssDxChPhyXSmiMDCInvertGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL  *invertMDCPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPhySmih */

