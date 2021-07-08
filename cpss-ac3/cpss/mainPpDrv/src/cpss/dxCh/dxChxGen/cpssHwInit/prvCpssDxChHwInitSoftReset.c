/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChHwInitSoftRest.c
*       Internal function used for Soft Reset testing
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitSoftReset.h>
#include <cpss/common/config/private/prvCpssCommonRegs.h>

/* for SIP6 The prvCpssDrvHwPpApi.c functions         */
/* subsitute CPSS_HW_DRIVER_AS_SWITCHING_E instead of */
/* CPSS_HW_DRIVER_AS_RESET_AND_INIT_CONTROLLER_E and  */
/* CPSS_HW_DRIVER_AS_DFX_E that is the same value 8   */
/* So DFX registers can be accessed using function    */
/* prvCpssDrvHwPpPortGroupWriteRegister and other     */
/* Switching core oriented functions                  */
/* see                                                */
/* prvCpssDrvHwPpWriteRegBitMaskDrv and               */
/* prvCpssDrvHwPpDoReadOpDrv                          */

/**
* @internal prvCpssDxChHwInitSoftResetSkipInitMatrixRegAddrGet function
* @endinternal
*
* @brief   Get address of register from Skip Initialization Marix by type.
*          For SIP6 this address of the register in DFX of Tile0 Eagle.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] skipType                 - the type of Skip Initialization Marix
*                                       see comments in PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT.
*
* @param[out] regAddrPtr              - pointer to register address
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device - not supported DFX server or
*                                    specified type of Skip Initialization Marix
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvCpssDxChHwInitSoftResetSkipInitMatrixRegAddrGet
(
    IN  GT_U8                                          devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT  skipType,
    OUT GT_U32                                         *regAddrPtr
)
{
    GT_U32 regAddr;
    PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC *dfxRegsAddrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);

    if (PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    dfxRegsAddrPtr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum);
    switch (skipType)
    {
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_REGISTERS_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.configSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_SRR_LOAD_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.SRRSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_BIST_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.BISTSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_SOFT_REPAIR_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.SoftRepairSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_RAM_INIT_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.RAMInitSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_REGS_GENX_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.RegxGenxSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_MULTI_ACTION_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.MultiActionSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_TABLES_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.tableSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_SERDES_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.SERDESSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_EEPROM_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.EEPROMSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_PCIE_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.PCIeSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_DEVICE_EEPROM_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DeviceEEPROMSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_D2D_LINK_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.D2DLinkSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_DFX_REGISTERS_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DFXRegistersSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_DFX_PIPE_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DFXPipeSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_TILE_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.Core_TileSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_MNG_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.Core_MngSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_D2D_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.Core_D2DSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_RAVEN_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.Core_RavenSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_D2D_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.GOP_D2DSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_MAIN_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.GOP_MainSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_CNM_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.GOP_CNMSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_GW_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.GOP_GWSkipInitializationMatrix;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    *regAddrPtr = regAddr;
    return GT_OK;
}

/**
* @internal prvCpssDxChHwInitSoftResetSkipTypeSw2HwConvert function
* @endinternal
*
* @brief   Convert SW type of Skip Initialization Domain to sequence of HW types.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] skipType                 - the type of Skip Initialization Marix
*                                       see comments in CPSS_HW_PP_RESET_SKIP_TYPE_ENT.
* @param[in] index                    - index in the type sequence.
*
* @param[out] hwSkipTypePtr           - pointer to HW Skip Type
*                                       see comments in PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT.
*
* @retval GT_OK                    - on success,
* @retval GT_NO_MORE               - index is out of sequence length
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvCpssDxChHwInitSoftResetSkipTypeSw2HwConvert
(
    IN  CPSS_HW_PP_RESET_SKIP_TYPE_ENT                  skipType,
    IN  GT_U32                                          index,
    OUT  PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT  *hwSkipTypePtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(hwSkipTypePtr);

    switch (skipType)
    {
        case CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E:
            if (index > 0) CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
            *hwSkipTypePtr = PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_REGISTERS_E;
            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E:
            switch (index)
            {
                case 0:
                    *hwSkipTypePtr = PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_RAM_INIT_E;
                    break;
                case 1:
                    *hwSkipTypePtr = PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_TABLES_E;
                    break;
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E:
            if (index > 0) CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
            *hwSkipTypePtr = PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_EEPROM_E;
            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E:
            if (index > 0) CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
            *hwSkipTypePtr = PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_PCIE_E;
            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_CHIPLETS_E:
            if (index > 0) CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
            *hwSkipTypePtr = PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_RAVEN_E;
            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E:
            if (index > 0) CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
            *hwSkipTypePtr = PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_SERDES_E;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6DfxRegAddrConvert function
* @endinternal
*
* @brief   Get address of register instance in given SIP6 Tile or in given Raven.
*          .
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] dfxType               - dfx Type - core or GOP
* @param[in] tileIndex             - index of tile 0..3
* @param[in] gopIndex              - index of GOP 0..3 - for GOP connected to the given tile.
*                                    relevant only when dfx Type is GOP
* @param[in] regAddr               - pattern register address used to take offset from DFX base.
* @param[out] portGroupPtr         - pointer to port group address
* @param[out] regAddrPtr           - pointer to register address
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6DfxRegAddrConvert
(
    IN  GT_U8                                 devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT dfxType,
    IN  GT_U32                                tileIndex,
    IN  GT_U32                                gopIndex,
    IN  GT_U32                                regAddr,
    OUT GT_U32                                *portGroupPtr,
    OUT GT_U32                                *regAddrPtr
)
{
    GT_U32        numPipesPerTile;
    GT_U32        dfxAddressMask   = 0x000FFFFF;
    GT_U32        dfxInGopLocaBase = 0x00700000;
    GT_U32        relativeDfxBase;
    GT_BOOL       error;

    PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC *dfxRegsAddrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    CPSS_NULL_PTR_CHECK_MAC(portGroupPtr);
    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);
    if (PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    if (tileIndex >= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (gopIndex >= FALCON_RAVENS_PER_TILE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    numPipesPerTile =
        PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes /
            PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
    *portGroupPtr = (tileIndex * numPipesPerTile);
    switch (dfxType)
    {
        case PRV_CPSS_DXCH_SOFT_RESET_DFX_CORE_E:
            /* Processing Core DFX */
            dfxRegsAddrPtr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum);
            relativeDfxBase =
                (dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceResetCtrl
                 & (~ dfxAddressMask));
            *regAddrPtr = relativeDfxBase | (regAddr & dfxAddressMask);
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_DFX_GOP_E:
            /* GOP DFX */
            relativeDfxBase =
                (prvCpssDxChHwUnitBaseAddrGet(
                    devNum, (PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + gopIndex), &error)
                 + dfxInGopLocaBase);
            *regAddrPtr = relativeDfxBase | (regAddr & dfxAddressMask);
            if (error != GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6DfxRegWriteBitMask function
* @endinternal
*
* @brief   Write DFX Register bit mask to register instance in given Falcon Tile or in given Raven.
*          .
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] dfxType               - dfx Type - core or GOP
* @param[in] tileIndex             - index of tile 0..3
* @param[in] gopIndex              - index of GOP 0..3 - for GOP connected to the given tile.
*                                    relevant only when dfx Type is GOP
* @param[in] regAddr               - pattern register address used to take offset from DFX base.
* @param[in] mask                  - Mask for selecting the written bits.
* @param[in] value                 - Data to be written to register.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6DfxRegWriteBitMask
(
    IN  GT_U8                                          devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT          dfxType,
    IN  GT_U32                                         tileIndex,
    IN  GT_U32                                         gopIndex,
    IN  GT_U32                                         regAddr,
    IN  GT_U32                                         mask,
    IN  GT_U32                                         data
)
{
    GT_STATUS    rc;
    GT_U32       portGroup;
    GT_U32       convertedRegAddr;

    rc = prvCpssDxChHwInitSoftResetSip6DfxRegAddrConvert(
        devNum, dfxType, tileIndex, gopIndex, regAddr,
        &portGroup, &convertedRegAddr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    return prvCpssDrvHwPpPortGroupWriteRegBitMask(
        devNum, portGroup, convertedRegAddr, mask, data);
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6DfxRegRead function
* @endinternal
*
* @brief   Read DFX register instance in given Falcon Tile or in given Raven.
*          .
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] dfxType               - dfx Type - core or GOP
* @param[in] tileIndex             - index of tile 0..3
* @param[in] gopIndex              - index of GOP 0..3 - for GOP connected to the given tile.
*                                    relevant only when dfx Type is GOP
* @param[in] regAddr               - pattern register address used to take offset from DFX base.
* @param[out] dataPtr              - pointer to data read from register
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6DfxRegRead
(
    IN  GT_U8                                          devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT          dfxType,
    IN  GT_U32                                         tileIndex,
    IN  GT_U32                                         gopIndex,
    IN  GT_U32                                         regAddr,
    OUT GT_U32                                         *dataPtr
)
{
    GT_STATUS    rc;
    GT_U32       portGroup;
    GT_U32       convertedRegAddr;

    rc = prvCpssDxChHwInitSoftResetSip6DfxRegAddrConvert(
        devNum, dfxType, tileIndex, gopIndex, regAddr,
        &portGroup, &convertedRegAddr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    return prvCpssDrvHwPpPortGroupReadRegister(
        devNum, portGroup, convertedRegAddr, dataPtr);
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6SkipInitMatrixWriteBitMask function
* @endinternal
*
* @brief   Write bit mask to given Skip Init Matrix register instance
*          in given Falcon Tile or in given Raven.
*          .
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] dfxType               - dfx Type - core or GOP
* @param[in] tileIndex             - index of tile 0..3
* @param[in] gopIndex              - index of GOP 0..3 - for GOP connected to the given tile.
*                                    relevant only when dfx Type is GOP
* @param[in] skipType              - the type of Skip Initialization Marix
*                                    see comments in PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT.
* @param[in] mask                  - Mask for selecting the written bits.
* @param[in] value                 - Data to be written to register.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6SkipInitMatrixWriteBitMask
(
    IN  GT_U8                                          devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT          dfxType,
    IN  GT_U32                                         tileIndex,
    IN  GT_U32                                         gopIndex,
    IN  PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT  skipType,
    IN  GT_U32                                         mask,
    IN  GT_U32                                         data
)
{
    GT_STATUS    rc;
    GT_U32       dbRegAddr;

    rc = prvCpssDxChHwInitSoftResetSkipInitMatrixRegAddrGet(
        devNum, skipType, &dbRegAddr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    return prvCpssDxChHwInitSoftResetSip6DfxRegWriteBitMask(
        devNum, dfxType, tileIndex, gopIndex, dbRegAddr,
        mask, data);
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6SkipInitMatrixRead function
* @endinternal
*
* @brief   Read given Skip Init Matrix register instance
*          in given Falcon Tile or in given Raven.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] dfxType               - dfx Type - core or GOP
* @param[in] tileIndex             - index of tile 0..3
* @param[in] gopIndex              - index of GOP 0..3 - for GOP connected to the given tile.
*                                    relevant only when dfx Type is GOP
* @param[in] skipType              - the type of Skip Initialization Marix
*                                    see comments in PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT.
* @param[out] dataPtr              - pointer to data read from register
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6SkipInitMatrixRead
(
    IN  GT_U8                                          devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT          dfxType,
    IN  GT_U32                                         tileIndex,
    IN  GT_U32                                         gopIndex,
    IN  PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT  skipType,
    OUT GT_U32                                         *dataPtr
)
{
    GT_STATUS    rc;
    GT_U32       dbRegAddr;

    rc = prvCpssDxChHwInitSoftResetSkipInitMatrixRegAddrGet(
        devNum, skipType, &dbRegAddr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    return prvCpssDxChHwInitSoftResetSip6DfxRegRead(
        devNum, dfxType, tileIndex, gopIndex, dbRegAddr, dataPtr);
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6ResetTriggerRegWriteBitMask function
* @endinternal
*
* @brief   Write bit mask to reset trigger register instance
*          in given Falcon Tile or in given Raven.
*          .
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] dfxType               - dfx Type - core or GOP
* @param[in] tileIndex             - index of tile 0..3
* @param[in] gopIndex              - index of GOP 0..3 - for GOP connected to the given tile.
*                                    relevant only when dfx Type is GOP
* @param[in] mask                  - Mask for selecting the written bits.
* @param[in] value                 - Data to be written to register.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6ResetTriggerRegWriteBitMask
(
    IN  GT_U8                                          devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT          dfxType,
    IN  GT_U32                                         tileIndex,
    IN  GT_U32                                         gopIndex,
    IN  GT_U32                                         mask,
    IN  GT_U32                                         data
)
{
    GT_U32       dbRegAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    dbRegAddr =
        PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceResetCtrl;

    return prvCpssDxChHwInitSoftResetSip6DfxRegWriteBitMask(
        devNum, dfxType, tileIndex, gopIndex, dbRegAddr,
        mask, data);
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6CnmSystemResetTrigger function
* @endinternal
*
* @brief   Trigger Falcon CNM System Soff Reset.
*          .
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6CnmSystemResetTrigger
(
    IN  GT_U8                                          devNum
)
{
    GT_U32    unitBase;
    GT_BOOL   error;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    error = GT_FALSE;
    unitBase = prvCpssDxChHwUnitBaseAddrGet(
            devNum, PRV_CPSS_DXCH_UNIT_CNM_RFU_E, &error);
    if (error != GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* 0-th CNM unit connected to all tiles */
    /* prvCpssDrvHwPpPortGroupWriteInternalPciReg - not applicable for Falcon CNM */
    return prvCpssDrvHwPpPortGroupWriteRegBitMask(
        devNum, 0 /*portGroupId*/,
        (unitBase + 0x44)  /*regAddr - System Soft Reset*/,
        1/*mask*/, 1 /*data*/);
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6AllSubunitsSkipInitMatrixWriteBitMask function
* @endinternal
*
* @brief   Write bit mask to given Skip Init Matrix register instance
*          in all Falcon Tiles and Ravens relvant to given Skip Init Matrix.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] skipType              - the type of Skip Initialization Marix
*                                    see comments in PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT.
* @param[in] mask                  - Mask for selecting the written bits.
* @param[in] value                 - Data to be written to register.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6AllSubunitsSkipInitMatrixWriteBitMask
(
    IN  GT_U8                                          devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT  skipType,
    IN  GT_U32                                         mask,
    IN  GT_U32                                         data
)
{
    GT_STATUS                               rc;
    GT_BOOL                                 perGop;
    GT_BOOL                                 perTile;
    PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT   dfxType;
    GT_U32                                  tileIndex;
    GT_U32                                  gopIndex;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    switch (skipType)
    {
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_TILE_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_MNG_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_D2D_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_RAVEN_E:
            perGop   = GT_FALSE;
            perTile  = GT_TRUE;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_D2D_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_MAIN_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_CNM_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_GW_E:
            perGop   = GT_TRUE;
            perTile  = GT_FALSE;
            break;
        default:
            perGop   = GT_TRUE;
            perTile  = GT_TRUE;
            break;
    }

    for (tileIndex = 0; (tileIndex < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles); tileIndex++)
    {
        if (perTile != GT_FALSE)
        {
            dfxType = PRV_CPSS_DXCH_SOFT_RESET_DFX_CORE_E;
            gopIndex = 0;
            rc = prvCpssDxChHwInitSoftResetSip6SkipInitMatrixWriteBitMask(
                devNum, dfxType, tileIndex, gopIndex, skipType, mask, data);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
        if (perGop != GT_FALSE)
        {
            for (gopIndex = 0; (gopIndex < FALCON_RAVENS_PER_TILE); gopIndex++)
            {
                dfxType = PRV_CPSS_DXCH_SOFT_RESET_DFX_GOP_E;
                rc = prvCpssDxChHwInitSoftResetSip6SkipInitMatrixWriteBitMask(
                    devNum, dfxType, tileIndex, gopIndex, skipType, mask, data);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6GopSetSkipInitMatrixWriteBitMask function
* @endinternal
*
* @brief   Write bit mask to given Skip Init Matrix register instance
*          in given Ravens Set.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] gopBitmap             - bitmap of GOP global indexes
*                                    CPSS_CHIPLETS_UNAWARE_MODE_CNS for all chiplets of the device
* @param[in] skipType              - the type of Skip Initialization Marix
*                                    see comments in PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT.
*                                    Skip types relevant to Main Die only ignored. GT_OK returned.
* @param[in] mask                  - Mask for selecting the written bits.
* @param[in] value                 - Data to be written to register.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6GopSetSkipInitMatrixWriteBitMask
(
    IN  GT_U8                                          devNum,
    IN  GT_CHIPLETS_BMP                                gopBitmap,
    IN  PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT  skipType,
    IN  GT_U32                                         mask,
    IN  GT_U32                                         data
)
{
    GT_STATUS                               rc;
    PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT   dfxType;
    GT_U32                                  tileIndex;
    GT_U32                                  gopIndex;
    GT_U32                                  gopGlobalIndex;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    if (gopBitmap == CPSS_CHIPLETS_UNAWARE_MODE_CNS)
    {
        gopBitmap = 0xFFFFFFFF; /* rize all bits */
    }
    switch (skipType)
    {
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_TILE_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_MNG_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_D2D_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_RAVEN_E:
            return GT_OK; /* ignored for GOP, relevant to Tile */
        default: break;
    }

    dfxType = PRV_CPSS_DXCH_SOFT_RESET_DFX_GOP_E;
    for (tileIndex = 0; (tileIndex < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles); tileIndex++)
    {
        for (gopIndex = 0; (gopIndex < FALCON_RAVENS_PER_TILE); gopIndex++)
        {
            gopGlobalIndex =
                ((PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles * tileIndex)
                 + gopIndex);
            if ((gopBitmap & (1 << gopGlobalIndex)) == 0) continue;
            rc = prvCpssDxChHwInitSoftResetSip6SkipInitMatrixWriteBitMask(
                devNum, dfxType, tileIndex, gopIndex, skipType, mask, data);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
    }

    return GT_OK;
}


