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
* @file prvCpssDxChPhySmi.h
*
* @brief Private definitions for PHY SMI.
*
* @version   7
********************************************************************************
*/
#ifndef __prvCpssDxChPhySmih
#define __prvCpssDxChPhySmih

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/phy/cpssGenPhySmi.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* offset of PHY SMI Group 1 registers */
#define CPSS_DX_PHY_ADDR_REG_OFFSET_CNS     0x0800000

/* offset of SMI Control registers */
#define CPSS_DX_SMI_MNG_CNTRL_OFFSET_CNS    0x1000000

/* number of tri-speed or FE ports with PHY poling support */
#define PRV_CPSS_DXCH_SMI_PPU_PORTS_NUM_CNS     24

/* number of network ports per SMI interface */
#define PRV_CPSS_DXCH_E_ARCH_SMI_PORTS_NUM_CNS          24

/* number of network ports with PHY poling support */
#define PRV_CPSS_DXCH_E_ARCH_SMI_PPU_PORTS_NUM_CNS      48

/**
* @internal prvCpssDxChPhySmiObjInit function
* @endinternal
*
* @brief   Initialise SMI service function pointers : SMI Ctrl Reg. Read/Write.
*         The generic SMI functions cpssSmiRegisterReadShort,
*         cpssSmiRegisterWriteShort use these pointers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
*                                       none.
*/
GT_VOID prvCpssDxChPhySmiObjInit
(
    IN  GT_U8     devNum
);

/**
* @internal prvCpssDxChPortSMIAutoPollingMappingGet function
* @endinternal
*
* @brief   this function returns SMI instance , SMI local port and autopolling
*         automaton at SMI controller
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - port number.
*
* @param[out] smiInterfacePtr          - smi IF
* @param[out] smiLocalPortPtr          - smi local port (logical)
* @param[out] autoPollingPlacePtr      - hw place at SMI controller (physical)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. applicable to BC2-B0 and higher, just for SMI controller (i.e. SMI unit and not LMS unit)
*       2. BC2 B0/CAELUM : at SMI IF 0,2 autopolling place equal to smi logical port
*       at SMI IF 1,3 autopolling place is (16 - number of autopolling ports + logical) (adjusted to 16
*       ex : autopolling = 8 , than HW places are 8-15 corrsponts to logical 0-7
*
*/
GT_STATUS prvCpssDxChPortSMIAutoPollingMappingGet
(
    IN  GT_U8                        devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    OUT CPSS_PHY_SMI_INTERFACE_ENT  *smiInterfacePtr,
    OUT GT_U32                      *smiLocalPortPtr,
    OUT GT_U32                      *autoPollingPlacePtr
);

/**
* @internal internal_cpssSmiRegisterReadShort function
* @endinternal
*
* @brief   The function reads register of a device, which connected to SMI master
*         controller of packet processor
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] smiInterface             - SMI master interface Id
* @param[in] smiAddr                  - address of configurated device on SMI (APPLICABLE RANGES: 0..31)
* @param[in] regAddr                  - register address
*
* @param[out] dataPtr                  - pointer to place data from read operation
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, smiInterface
* @retval GT_BAD_PTR               - pointer to place data is NULL
* @retval GT_NOT_READY             - smi is busy
* @retval GT_HW_ERROR              - hw error
* @retval GT_NOT_INITIALIZED       - smi ctrl register callback not registered
*/
GT_STATUS internal_cpssSmiRegisterReadShort
(
    IN  GT_U8   devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  CPSS_PHY_SMI_INTERFACE_ENT   smiInterface,
    IN  GT_U32  smiAddr,
    IN  GT_U32  regAddr,
    OUT GT_U16  *dataPtr
);
/**
* @internal internal_cpssSmiRegisterWriteShort function
* @endinternal
*
* @brief   The function writes register of a device, which connected to SMI master
*         controller of packet processor
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] smiInterface             - SMI master interface Id
* @param[in] smiAddr                  - address of configurated device on SMI (APPLICABLE RANGES: 0..31)
* @param[in] regAddr                  - address of register of configurated device
* @param[in] data                     -  to write
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, smiInterface
* @retval GT_NOT_READY             - smi is busy
* @retval GT_HW_ERROR              - hw error
* @retval GT_NOT_INITIALIZED       - smi ctrl register callback not registered
*/
GT_STATUS internal_cpssSmiRegisterWriteShort
(
    IN  GT_U8   devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  CPSS_PHY_SMI_INTERFACE_ENT   smiInterface,
    IN  GT_U32  smiAddr,
    IN  GT_U32  regAddr,
    IN  GT_U16  data
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChPhySmih */


