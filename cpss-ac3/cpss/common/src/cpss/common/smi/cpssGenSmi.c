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
* cpssGenPpSmi.c
*
* DESCRIPTION:
*       API for read/write register of device, which connected to SMI master
*           controller of packet processor
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 13 $
*
*******************************************************************************/

#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/common/smi/private/prvCpssGenSmiLog.h>

#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/phy/private/prvCpssPhy.h>
#include <cpss/common/smi/cpssGenSmi.h>

/*************************** Private definitions ******************************/

#define  PRV_CPSS_SMI_WRITE_ADDRESS_MSB_REGISTER_CNS   (0x00)
#define  PRV_CPSS_SMI_WRITE_ADDRESS_LSB_REGISTER_CNS   (0x01)
#define  PRV_CPSS_SMI_WRITE_DATA_MSB_REGISTER_CNS      (0x02)
#define  PRV_CPSS_SMI_WRITE_DATA_LSB_REGISTER_CNS      (0x03)
#define  PRV_CPSS_SMI_READ_ADDRESS_MSB_REGISTER_CNS    (0x04)
#define  PRV_CPSS_SMI_READ_ADDRESS_LSB_REGISTER_CNS    (0x05)
#define  PRV_CPSS_SMI_READ_DATA_MSB_REGISTER_CNS       (0x06)
#define  PRV_CPSS_SMI_READ_DATA_LSB_REGISTER_CNS       (0x07)
#define  PRV_CPSS_SMI_STATUS_REGISTER_CNS              (0x1f)
#define  PRV_CPSS_SMI_STATUS_WRITE_DONE_CNS            (0x02)
#define  PRV_CPSS_SMI_STATUS_READ_READY_CNS            (0x01)


/*************************** Private functions ********************************/

#define PRV_CPSS_SMI_ADDRESS_CHECK_MAC(smiAddress)     \
    if(smiAddress >= BIT_5)                                 \
    {                                                       \
        /* only 5 bits for each of those parameters */      \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG); \
    }


/*************************** Global functions ********************************/

/*TODO*/
GT_STATUS prvSmiHwDriverCreateDrv(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId,
    IN  CPSS_PHY_SMI_INTERFACE_ENT  i
);
GT_STATUS prvXSmiHwDriverCreateDrv(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT  i
);

/**
* @internal prvCpssSmiInit function
* @endinternal
*
* @brief   Initialize SMI/XSMI master instances
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - device number
* @param[in] portGroupId           - port Group ID.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum
* @retval GT_NO_RESOURCE           - no memory/resource
*/
GT_STATUS prvCpssSmiInit
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId
)
{
    GT_STATUS rc;
    GT_U32 i;

    for (i = 0; i < CPSS_PHY_SMI_INTERFACE_MAX_E; i++)
    {
        rc = prvSmiHwDriverCreateDrv(devNum, portGroupId, (CPSS_PHY_SMI_INTERFACE_ENT)i);
        if (rc != GT_OK && rc != GT_NO_RESOURCE)
            return rc;
    }

    for (i = 0; i < CPSS_PHY_XSMI_INTERFACE_MAX_E; i++)
    {
        rc = prvXSmiHwDriverCreateDrv(devNum, portGroupId, (CPSS_PHY_XSMI_INTERFACE_ENT)i);
        if (rc != GT_OK && rc != GT_NO_RESOURCE)
            return rc;
    }
    return GT_OK;
}

/*************************** Global functions ********************************/

/**
* @internal internal_cpssSmiRegisterReadShort function
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
* @param[in] smiAddr                  - address of configured device on SMI (APPLICABLE RANGES: 0..31)
* @param[in] regAddr                  - register address
*
* @param[out] dataPtr                  - pointer to place data from read operation
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, smiInterface
* @retval GT_OUT_OF_RANGE          - smiAddr is out of range
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
)
{
    GT_STATUS rc;
    GT_U32  regData;
    CPSS_HW_DRIVER_STC *drv;
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */

    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (smiInterface >= CPSS_PHY_SMI_INTERFACE_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CPSS_NULL_PTR_CHECK_MAC(dataPtr);

    if(0 == PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {   /* for non multi-port groups device the MACRO assign value */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    }
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);
    if (portGroupId == CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
        portGroupId = 0;

    PRV_CPSS_SMI_ADDRESS_CHECK_MAC(smiAddr);

    drv = PRV_CPSS_PP_MAC(devNum)->smiMasters[portGroupId][smiInterface];
    if (drv == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = drv->read(drv, smiAddr, regAddr, &regData, 1);
    if (rc == GT_OK)
    {
        *dataPtr = (GT_U16)regData;
    }
    return rc;
}

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
* @param[in] smiAddr                  - address of configured device on SMI (APPLICABLE RANGES: 0..31)
* @param[in] regAddr                  - register address
*
* @param[out] dataPtr                  - pointer to place data from read operation
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, smiInterface
* @retval GT_OUT_OF_RANGE          - smiAddr is out of range
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssSmiRegisterReadShort);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, smiInterface, smiAddr, regAddr, dataPtr));

    rc = internal_cpssSmiRegisterReadShort(devNum, portGroupsBmp, smiInterface, smiAddr, regAddr, dataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, smiInterface, smiAddr, regAddr, dataPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssSmiRegisterWriteShort function
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
* @param[in] smiAddr                  - address of configured device on SMI (APPLICABLE RANGES: 0..31)
* @param[in] regAddr                  - address of register of configured device
* @param[in] data                     -  to write
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, smiInterface
* @retval GT_OUT_OF_RANGE          - smiAddr is out of range
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
)
{
    GT_STATUS rc;
    GT_U32  regData = (GT_U32)data;
    CPSS_HW_DRIVER_STC *drv;
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */

    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    if (smiInterface >= CPSS_PHY_SMI_INTERFACE_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_SMI_ADDRESS_CHECK_MAC(smiAddr);
    if(0 == PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {   /* for non multi-port groups device assign first active port group ID */
        portGroupsBmp = (1 << PRV_CPSS_FIRST_ACTIVE_PORT_GROUP_ID_MAC(devNum));
    }

    /* loop on all active port groups in the bmp */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        drv = PRV_CPSS_PP_MAC(devNum)->smiMasters[portGroupId][smiInterface];
        if (drv == NULL)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        rc = drv->writeMask(drv, smiAddr, regAddr, &regData, 1, 0xffffffff);
        if (rc == GT_OK)
            return rc;
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    return GT_OK;
}

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
* @param[in] smiAddr                  - address of configured device on SMI (APPLICABLE RANGES: 0..31)
* @param[in] regAddr                  - address of register of configured device
* @param[in] data                     -  to write
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, smiInterface
* @retval GT_OUT_OF_RANGE          - smiAddr is out of range
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssSmiRegisterWriteShort);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, smiInterface, smiAddr, regAddr, data));

    rc = internal_cpssSmiRegisterWriteShort(devNum, portGroupsBmp, smiInterface, smiAddr, regAddr, data);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, smiInterface, smiAddr, regAddr, data));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssSmiRegisterRead function
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
* @param[in] smiAddr                  - address of configured device on SMI (APPLICABLE RANGES: 0..31)
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
static GT_STATUS internal_cpssSmiRegisterRead
(
    IN  GT_U8   devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  CPSS_PHY_SMI_INTERFACE_ENT   smiInterface,
    IN  GT_U32  smiAddr,
    IN  GT_U32  regAddr,
    OUT GT_U32  *dataPtr
)
{
    GT_U16              msb;
    GT_U16              lsb;
    GT_U32              value;
    GT_STATUS           rc;

    CPSS_NULL_PTR_CHECK_MAC(dataPtr);

    /* write addr to read */
    msb = (GT_U16)(regAddr >> 16);
    lsb = (GT_U16)(regAddr & 0xFFFF);

    if ((rc = cpssSmiRegisterWriteShort(devNum, portGroupsBmp, smiInterface, smiAddr,
                                        PRV_CPSS_SMI_READ_ADDRESS_MSB_REGISTER_CNS, msb)) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if ((rc = cpssSmiRegisterWriteShort(devNum, portGroupsBmp, smiInterface, smiAddr,
                                        PRV_CPSS_SMI_READ_ADDRESS_LSB_REGISTER_CNS, lsb)) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* read data */
    if ((rc = cpssSmiRegisterReadShort(devNum, portGroupsBmp, smiInterface, smiAddr,
                                        PRV_CPSS_SMI_READ_DATA_MSB_REGISTER_CNS, &msb)) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if ((rc = cpssSmiRegisterReadShort(devNum, portGroupsBmp, smiInterface, smiAddr,
                                        PRV_CPSS_SMI_READ_DATA_LSB_REGISTER_CNS, &lsb)) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    value = (GT_U32)msb;
    *dataPtr = (value << 16) | lsb;

    return GT_OK;
}

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
* @param[in] smiAddr                  - address of configured device on SMI (APPLICABLE RANGES: 0..31)
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssSmiRegisterRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, smiInterface, smiAddr, regAddr, dataPtr));

    rc = internal_cpssSmiRegisterRead(devNum, portGroupsBmp, smiInterface, smiAddr, regAddr, dataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, smiInterface, smiAddr, regAddr, dataPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssSmiRegisterWrite function
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
* @param[in] smiAddr                  - address of configured device on SMI (APPLICABLE RANGES: 0..31)
* @param[in] regAddr                  - address of register of configured device
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
static GT_STATUS internal_cpssSmiRegisterWrite
(
    IN  GT_U8   devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  CPSS_PHY_SMI_INTERFACE_ENT   smiInterface,
    IN  GT_U32  smiAddr,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{
    GT_U16              msb;
    GT_U16              lsb;
    GT_STATUS           rc;

    /* write addr to write */
    msb = (GT_U16)(regAddr >> 16);
    lsb = (GT_U16)(regAddr & 0xFFFF);

    if ((rc = cpssSmiRegisterWriteShort(devNum, portGroupsBmp, smiInterface, smiAddr,
                                PRV_CPSS_SMI_WRITE_ADDRESS_MSB_REGISTER_CNS, msb)) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if ((rc = cpssSmiRegisterWriteShort(devNum, portGroupsBmp, smiInterface, smiAddr,
                                PRV_CPSS_SMI_WRITE_ADDRESS_LSB_REGISTER_CNS, lsb)) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* write data to write */
    msb = (GT_U16)(data >> 16);
    lsb = (GT_U16)(data & 0xFFFF);

    if ((rc = cpssSmiRegisterWriteShort(devNum, portGroupsBmp, smiInterface, smiAddr,
                                PRV_CPSS_SMI_WRITE_DATA_MSB_REGISTER_CNS, msb)) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if ((rc = cpssSmiRegisterWriteShort(devNum, portGroupsBmp, smiInterface, smiAddr,
                                PRV_CPSS_SMI_WRITE_DATA_LSB_REGISTER_CNS, lsb)) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

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
* @param[in] smiAddr                  - address of configured device on SMI (APPLICABLE RANGES: 0..31)
* @param[in] regAddr                  - address of register of configured device
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssSmiRegisterWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, smiInterface, smiAddr, regAddr, data));

    rc = internal_cpssSmiRegisterWrite(devNum, portGroupsBmp, smiInterface, smiAddr, regAddr, data);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, smiInterface, smiAddr, regAddr, data));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssXsmiPortGroupRegisterWrite function
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
* @param[in] xsmiAddr                 - address of configured device on XSMI (APPLICABLE RANGES: 0..31)
* @param[in] regAddr                  - address of register of configured device
* @param[in] phyDev                   - the PHY device to write to (APPLICABLE RANGES: 0..31).
* @param[in] data                     -  to write
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, xsmiAddr, phyDev.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - callback not set
*/
static GT_STATUS internal_cpssXsmiPortGroupRegisterWrite
(
    IN  GT_U8   devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface,
    IN  GT_U32  xsmiAddr,
    IN  GT_U32  regAddr,
    IN  GT_U32  phyDev,
    IN  GT_U16  data
)
{
    GT_U32 value = (GT_U32)data;
    CPSS_HW_DRIVER_STC *drv;
    GT_U32 portGroupId; /*the port group Id - support multi-port-groups device */
    GT_STATUS rc;

    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (xsmiInterface >= CPSS_PHY_XSMI_INTERFACE_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    if((phyDev >= BIT_5) || (xsmiAddr >= BIT_5))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(0 == PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {   /* for non multi-port groups device assign first active port group ID */
        portGroupsBmp = (1 << PRV_CPSS_FIRST_ACTIVE_PORT_GROUP_ID_MAC(devNum));
    }

    /* loop on all active port groups in the bmp */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        drv = PRV_CPSS_PP_MAC(devNum)->xsmiMasters[portGroupId][xsmiInterface];
        if (drv == NULL)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        rc = drv->writeMask(drv, (phyDev << 8) | xsmiAddr, regAddr, &value, 1, 0xffffffff);
        if (rc != GT_OK)
            return rc;
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    return GT_OK;
}

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
* @param[in] xsmiAddr                 - address of configured device on XSMI (APPLICABLE RANGES: 0..31)
* @param[in] regAddr                  - address of register of configured device
* @param[in] phyDev                   - the PHY device to write to (APPLICABLE RANGES: 0..31).
* @param[in] data                     -  to write
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, xsmiAddr, phyDev.
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssXsmiPortGroupRegisterWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, xsmiInterface, xsmiAddr, regAddr, phyDev, data));

    rc = internal_cpssXsmiPortGroupRegisterWrite(devNum, portGroupsBmp, xsmiInterface, xsmiAddr, regAddr, phyDev, data);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, xsmiInterface, xsmiAddr, regAddr, phyDev, data));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssXsmiPortGroupRegisterRead function
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
* @param[in] xsmiAddr                 - address of configured device on XSMI (APPLICABLE RANGES: 0..31)
* @param[in] regAddr                  - address of register of configured device
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
static GT_STATUS internal_cpssXsmiPortGroupRegisterRead
(
    IN  GT_U8   devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface,
    IN  GT_U32  xsmiAddr,
    IN  GT_U32  regAddr,
    IN  GT_U32  phyDev,
    OUT GT_U16  *dataPtr
)
{
    GT_U32 value;
    CPSS_HW_DRIVER_STC *drv;
    GT_U32  portGroupId;    /*the port group Id - support multi-port-groups device */
    GT_STATUS rc;

    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (xsmiInterface >= CPSS_PHY_XSMI_INTERFACE_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CPSS_NULL_PTR_CHECK_MAC(dataPtr);

    if((phyDev >= BIT_5) || (xsmiAddr >= BIT_5))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(0 == PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {   /* for non multi-port groups device the MACRO assign value */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    }
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
            devNum, portGroupsBmp, portGroupId);
    if (portGroupId == CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
        portGroupId = 0;

    drv = PRV_CPSS_PP_MAC(devNum)->xsmiMasters[portGroupId][xsmiInterface];
    if (drv == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    rc = drv->read(drv, (phyDev << 8) | xsmiAddr, regAddr, &value, 1);
    if (rc == GT_OK)
    {
        *dataPtr = (GT_U16) value;
    }
    return rc;
}

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
* @param[in] xsmiAddr                 - address of configured device on XSMI (APPLICABLE RANGES: 0..31)
* @param[in] regAddr                  - address of register of configured device
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssXsmiPortGroupRegisterRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, xsmiInterface, xsmiAddr, regAddr, phyDev, dataPtr));

    rc = internal_cpssXsmiPortGroupRegisterRead(devNum, portGroupsBmp, xsmiInterface, xsmiAddr, regAddr, phyDev, dataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, xsmiInterface, xsmiAddr, regAddr, phyDev, dataPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssXsmiRegisterWrite function
* @endinternal
*
* @brief   Write value to a specified XSMI Register.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
* @param[in] xsmiInterface            - XSMI instance
* @param[in] xsmiAddr                 - address of configured device on XSMI (APPLICABLE RANGES: 0..31)
* @param[in] regAddr                  - address of register of configured device
* @param[in] phyDev                   - the PHY device to write to (APPLICABLE RANGES: 0..31).
* @param[in] data                     -  to write
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - callback not set
*/
static GT_STATUS internal_cpssXsmiRegisterWrite
(
    IN  GT_U8   devNum,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface,
    IN  GT_U32  xsmiAddr,
    IN  GT_U32  regAddr,
    IN  GT_U32  phyDev,
    IN  GT_U16  data
)
{
    return cpssXsmiPortGroupRegisterWrite(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                          xsmiInterface, xsmiAddr, regAddr,
                                          phyDev, data);
}

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
* @param[in] xsmiAddr                 - address of configured device on XSMI (APPLICABLE RANGES: 0..31)
* @param[in] regAddr                  - address of register of configured device
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssXsmiRegisterWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, xsmiInterface, xsmiAddr, regAddr, phyDev, data));

    rc = internal_cpssXsmiRegisterWrite(devNum, xsmiInterface, xsmiAddr, regAddr, phyDev, data);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, xsmiInterface, xsmiAddr, regAddr, phyDev, data));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssXsmiRegisterRead function
* @endinternal
*
* @brief   Read value of a specified XSMI Register.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
* @param[in] xsmiInterface            - XSMI instance
* @param[in] xsmiAddr                 - address of configured device on XSMI (APPLICABLE RANGES: 0..31)
* @param[in] regAddr                  - address of register of configured device
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
static GT_STATUS internal_cpssXsmiRegisterRead
(
    IN  GT_U8   devNum,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface,
    IN  GT_U32  xsmiAddr,
    IN  GT_U32  regAddr,
    IN  GT_U32  phyDev,
    OUT GT_U16  *dataPtr
)
{
    return cpssXsmiPortGroupRegisterRead(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                         xsmiInterface, xsmiAddr, regAddr, phyDev,
                                         dataPtr);
}

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
* @param[in] xsmiAddr                 - address of configured device on XSMI (APPLICABLE RANGES: 0..31)
* @param[in] regAddr                  - address of register of configured device
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssXsmiRegisterRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, xsmiInterface, xsmiAddr, regAddr, phyDev, dataPtr));

    rc = internal_cpssXsmiRegisterRead(devNum, xsmiInterface, xsmiAddr, regAddr, phyDev, dataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, xsmiInterface, xsmiAddr, regAddr, phyDev, dataPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/* ----------------- debug functions --------------------------- */
#define PRV_CPSS_GEN_SMI_DBG_PHY_PAGE_REG_ADDR_CNS 22 /* PHY page number register */
#define PRV_CPSS_GEN_SMI_DBG_PHY_ID_1_REG_ADDR_CNS 2 /* PHY Identifier 1 register */
#define PRV_CPSS_GEN_SMI_DBG_PHY_ID_2_REG_ADDR_CNS 3 /* PHY Identifier 2 register */
GT_VOID prvCpssGenSmiDebugSmiScan
(
    IN  GT_U8   devNum
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      portGroupId; /* local core number */
    GT_PORT_GROUPS_BMP  portGroupsBmp; /* port groups bitmap */
    CPSS_PHY_SMI_INTERFACE_ENT   smiInterface;  /* SMI i/f iterator */
    GT_U32      smiAddr;    /* SMI Address iterator */
    GT_U32      regAddr;    /* PHY register address */
    GT_U16      data=0;       /* register data */

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        portGroupsBmp = 1<<portGroupId;
        for(smiInterface = CPSS_PHY_SMI_INTERFACE_0_E;
             smiInterface < CPSS_PHY_SMI_INTERFACE_MAX_E; smiInterface++)
        {
            for(smiAddr = 0; smiAddr < 16; smiAddr++)
            {
                /* set PHY page 0 */
                regAddr = PRV_CPSS_GEN_SMI_DBG_PHY_PAGE_REG_ADDR_CNS;
                rc = cpssSmiRegisterWriteShort(devNum, portGroupsBmp,
                                               smiInterface, smiAddr, regAddr, 0);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("cpssSmiRegisterWriteShort FAIL:devNum=%d,\
portGroupId=%d,smiInterface=%d,smiAddr=%d,\
regAddr=%d,data=%d\n",
                                 devNum, portGroupId, smiInterface, smiAddr, regAddr, 0);
                    continue;
                }
                /* read PHY ID 1 */
                regAddr = PRV_CPSS_GEN_SMI_DBG_PHY_ID_1_REG_ADDR_CNS;
                rc = cpssSmiRegisterReadShort(devNum, portGroupsBmp, smiInterface,
                                              smiAddr, regAddr, &data);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("cpssSmiRegisterReadShort FAIL:devNum=%d,\
portGroupId=%d,smiInterface=%d,smiAddr=%d,\
regAddr=%d,data=%d\n",
                                 devNum, portGroupId, smiInterface, smiAddr, regAddr);
                    continue;
                }
                cpssOsPrintf("cpssSmiRegisterReadShort:devNum=%d,\
portGroupId=%d,smiInterface=%d,smiAddr=%d,\
PHY_ID_1=0x%x\n",
                             devNum, portGroupId, smiInterface, smiAddr, data);
                /* read PHY ID 2 */
                regAddr = PRV_CPSS_GEN_SMI_DBG_PHY_ID_2_REG_ADDR_CNS;
                rc = cpssSmiRegisterReadShort(devNum, portGroupsBmp, smiInterface,
                                              smiAddr, regAddr, &data);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("cpssSmiRegisterReadShort FAIL:devNum=%d,\
portGroupId=%d,smiInterface=%d,smiAddr=%d,\
regAddr=%d,data=%d\n",
                                 devNum, portGroupId, smiInterface, smiAddr, regAddr);
                    continue;
                }
                cpssOsPrintf("cpssSmiRegisterReadShort:devNum=%d,\
portGroupId=%d,smiInterface=%d,smiAddr=%d,\
PHY_ID_2=0x%x\n",
                             devNum, portGroupId, smiInterface, smiAddr, data);
            }
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    return;
}

/**
* @internal prvCpssGenXsmiInterfaceResourceTypeGet function
* @endinternal
*
* @brief   Return resource type (address space) appropriate to specified
*         XSMI interface number.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] xsmiInterface            - XSMI master interface number
*
* @param[out] resTypePtr               - (pointer to) resource type
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - on unknown xsmiInterface number
*/
GT_STATUS prvCpssGenXsmiInterfaceResourceTypeGet
(
    IN  GT_U8                         devNum,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT   xsmiInterface,
    OUT CPSS_DRV_HW_RESOURCE_TYPE_ENT *resTypePtr
)
{
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E ||
        PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        /* in bobcat3/Aldrin2 has four XSMI interfaces - one per MG unit. MG1..MG3 units have
           own address space accessed through special API. */

        switch (xsmiInterface)
        {
            case CPSS_PHY_XSMI_INTERFACE_0_E:
                *resTypePtr = CPSS_DRV_HW_RESOURCE_SWITCHING_AND_MG0_CORE_E;
                break;
            case CPSS_PHY_XSMI_INTERFACE_1_E:
                *resTypePtr = CPSS_DRV_HW_RESOURCE_MG1_CORE_E;
                break;
            case CPSS_PHY_XSMI_INTERFACE_2_E:
                *resTypePtr = CPSS_DRV_HW_RESOURCE_MG2_CORE_E;
                break;
            case CPSS_PHY_XSMI_INTERFACE_3_E:
                *resTypePtr = CPSS_DRV_HW_RESOURCE_MG3_CORE_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        *resTypePtr = CPSS_DRV_HW_RESOURCE_SWITCHING_AND_MG0_CORE_E;
    }

    return GT_OK;
}
