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
* @file cpssGenSmi.h
*
* @brief API implementation for ASIC SMI controller
*
*
* @version   10
********************************************************************************
*/

#ifndef __cpssGenPpSmih
#define __cpssGenPpSmih

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/phy/cpssGenPhySmi.h>

/**
* @internal prvCpssSmiInit function
* @endinternal
*
* @brief   Initialize SMI/XSMI master instances
*
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum
* @retval GT_NO_RESOURCE           - no memory/resource
* @param[in] devNum                - device number
* @param[in] portGroupId           - port Group ID.
*/
GT_STATUS prvCpssSmiInit
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId
);

/**
* @internal cpssSmiRegisterRead function
* @endinternal
*
* @brief   The function reads register of a Marvell device, which connected to
*         SMI master controller of packet processor
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx; Pipe.
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
*
* @note Function specific for Marvell devices with 32-bit registers
*
*/
GT_STATUS cpssSmiRegisterRead
(
    IN  GT_U8   devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  CPSS_PHY_SMI_INTERFACE_ENT   smiInterface,
    IN  GT_U32  smiAddr,
    IN  GT_U32  regAddr,
    OUT GT_U32  *dataPtr
);

/**
* @internal cpssSmiRegisterWrite function
* @endinternal
*
* @brief   The function writes register of a Marvell device, which connected to SMI master
*         controller of packet processor
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx; Pipe.
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
*
* @note Function specific for Marvell devices with 32-bit registers
*
*/
GT_STATUS cpssSmiRegisterWrite
(
    IN  GT_U8   devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  CPSS_PHY_SMI_INTERFACE_ENT   smiInterface,
    IN  GT_U32  smiAddr,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
);

/**
* @internal cpssSmiRegisterReadShort function
* @endinternal
*
* @brief   The function reads register of a device, which connected to SMI master
*         controller of packet processor
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx; Pipe.
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
GT_STATUS cpssSmiRegisterReadShort
(
    IN  GT_U8   devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  CPSS_PHY_SMI_INTERFACE_ENT   smiInterface,
    IN  GT_U32  smiAddr,
    IN  GT_U32  regAddr,
    OUT GT_U16  *dataPtr
);

/**
* @internal cpssSmiRegisterWriteShort function
* @endinternal
*
* @brief   The function writes register of a device, which connected to SMI master
*         controller of packet processor
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx; Pipe.
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
GT_STATUS cpssSmiRegisterWriteShort
(
    IN  GT_U8   devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  CPSS_PHY_SMI_INTERFACE_ENT   smiInterface,
    IN  GT_U32  smiAddr,
    IN  GT_U32  regAddr,
    IN  GT_U16  data
);

/**
* @internal cpssXsmiPortGroupRegisterWrite function
* @endinternal
*
* @brief   Write value to a specified XSMI Register.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] xsmiInterface            - XSMI instance
* @param[in] xsmiAddr                 - address of configurated device on XSMI (APPLICABLE RANGES: 0..31)
* @param[in] regAddr                  - address of register of configurated device
* @param[in] phyDev                   - the PHY device to write to (APPLICABLE RANGES: 0..31).
* @param[in] data                     -  to write
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - callback not set
*/
GT_STATUS cpssXsmiPortGroupRegisterWrite
(
    IN  GT_U8   devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface,
    IN  GT_U32  xsmiAddr,
    IN  GT_U32  regAddr,
    IN  GT_U32  phyDev,
    IN  GT_U16  data
);


/**
* @internal cpssXsmiPortGroupRegisterRead function
* @endinternal
*
* @brief   Read value of a specified XSMI Register.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] xsmiInterface            - XSMI instance
* @param[in] xsmiAddr                 - address of configurated device on XSMI (APPLICABLE RANGES: 0..31)
* @param[in] regAddr                  - address of register of configurated device
* @param[in] phyDev                   - the PHY device to read from (APPLICABLE RANGES: 0..31).
*
* @param[out] dataPtr                  - (Pointer to) the read data.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - pointer to place data is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - callback not set
*/
GT_STATUS cpssXsmiPortGroupRegisterRead
(
    IN  GT_U8   devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface,
    IN  GT_U32  xsmiAddr,
    IN  GT_U32  regAddr,
    IN  GT_U32  phyDev,
    OUT GT_U16  *dataPtr
);

/**
* @internal cpssXsmiRegisterWrite function
* @endinternal
*
* @brief   Write value to a specified XSMI Register.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
* @param[in] xsmiInterface            - XSMI instance
* @param[in] xsmiAddr                 - address of configurated device on XSMI (APPLICABLE RANGES: 0..31)
* @param[in] regAddr                  - address of register of configurated device
* @param[in] phyDev                   - the PHY device to write to (APPLICABLE RANGES: 0..31).
* @param[in] data                     -  to write
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - callback not set
*/
GT_STATUS cpssXsmiRegisterWrite
(
    IN  GT_U8   devNum,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface,
    IN  GT_U32  xsmiAddr,
    IN  GT_U32  regAddr,
    IN  GT_U32  phyDev,
    IN  GT_U16  data
);


/**
* @internal cpssXsmiRegisterRead function
* @endinternal
*
* @brief   Read value of a specified XSMI Register.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
* @param[in] xsmiInterface            - XSMI instance
* @param[in] xsmiAddr                 - address of configurated device on XSMI (APPLICABLE RANGES: 0..31)
* @param[in] regAddr                  - address of register of configurated device
* @param[in] phyDev                   - the PHY device to read from (APPLICABLE RANGES: 0..31).
*
* @param[out] dataPtr                  - (Pointer to) the read data.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - pointer to place data is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - callback not set
*/
GT_STATUS cpssXsmiRegisterRead
(
    IN  GT_U8   devNum,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface,
    IN  GT_U32  xsmiAddr,
    IN  GT_U32  regAddr,
    IN  GT_U32  phyDev,
    OUT GT_U16  *dataPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssGenPpSmih */


