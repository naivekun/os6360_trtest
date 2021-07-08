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
* @file cpssDxChPhySmiPreInit.c
*
* @brief API implementation for PHY SMI pre init configuration.
* All functions should be called after cpssDxChHwPpPhase1Init() and
* before cpssDxChPhyPortSmiInit().
*
* @version   29
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmiPreInit.h>
/*#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>*/
#include <cpss/generic/phy/private/prvCpssGenPhySmi.h>
#include <cpss/dxCh/dxChxGen/phy/private/prvCpssDxChPhySmi.h>
#include <cpss/dxCh/dxChxGen/phy/prvCpssDxChSmiUnitDrv.h>
#include <cpss/dxCh/dxChxGen/phy/private/prvCpssDxChPhyLog.h>

/***************Private functions ********************************************/

/* Convert SMI Auto Poll number of ports software value to hardware */
#define PRV_CPSS_DXCH_SMI_AUTO_POLL_NUM_OF_PORTS_SW_TO_HW_CONVERT_MAC(_autoPollNumOfPorts,_value) \
    switch (_autoPollNumOfPorts)                            \
    {                                                       \
        case CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E:  \
            _value = 0x0;                                   \
            break;                                          \
        case CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E: \
            _value = 0x1;                                   \
            break;                                          \
        case CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_16_E: \
            _value = 0x2;                                   \
            break;                                          \
        default:                                            \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                            \
    }


/* Convert SMI Auto Poll number of ports hardware value to software  */
#define PRV_CPSS_DXCH_SMI_AUTO_POLL_NUM_OF_PORTS_HW_TO_SW_CONVERT_MAC(_value, _autoPollNumOfPorts) \
    switch (_value)                                                                 \
    {                                                                               \
        case 0x0:                                                                   \
            _autoPollNumOfPorts = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E;     \
            break;                                                                  \
        case 0x1:                                                                   \
            _autoPollNumOfPorts = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E;    \
            break;                                                                  \
        case 0x2:                                                                   \
            _autoPollNumOfPorts = CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_16_E;    \
            break;                                                                  \
        default:                                                                    \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                    \
    }

extern GT_STATUS prvCpssDxChEArchPhyAutoPollNumOfPortsGet
(
    IN  GT_U8                                           devNum,
    OUT CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    *autoPollNumOfPortsSmi0Ptr,
    OUT CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    *autoPollNumOfPortsSmi1Ptr,
    OUT CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    *autoPollNumOfPortsSmi2Ptr,
    OUT CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    *autoPollNumOfPortsSmi3Ptr
);

extern GT_STATUS prvCpssDxChEArchPhyPortSMIGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  portMacNum,
    OUT GT_U32 *smiInstancePtr,
    OUT GT_U32 *smiLoclaPortPtr
);


/**
* @internal prvCpssDxChEArchPhyPortAddrSet function
* @endinternal
*
* @brief   Configure the port's default phy address, this function should be
*         used to change the default port's phy address.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - port number.
* @param[in] phyAddr                  - The new phy address, (APPLICABLE RANGES: 0...31).
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChEArchPhyPortAddrSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8   phyAddr
)
{
    GT_STATUS rc = GT_OK;       /* return status */
    GT_U32  phyAddress;
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;/* local port - support multi-port-groups device */
    GT_U32  portMacNum;         /* MAC number */
    GT_U32 smiInstance;
    GT_U32 smiLocalPort;


    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    phyAddress = (GT_U32)phyAddr;

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portMacNum);

    /* the Bocat2, Caelum devices has PHY polling support (Out-Of-Band autonegotiation)
       for first 48 ports only */
    if (localPort < PRV_CPSS_DXCH_E_ARCH_SMI_PPU_PORTS_NUM_CNS)
    {
        /* array of numbers of ports par SMI interface */
        rc = prvCpssDxChEArchPhyPortSMIGet(devNum,portMacNum,/*OUT*/&smiInstance,&smiLocalPort);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChSMIPortPhyAddSet(devNum,portGroupId,smiInstance,smiLocalPort,phyAddress);
        if (rc != GT_OK)
        {
            return rc;
        }
        PRV_CPSS_PHY_SMI_PORT_ADDR_MAC(devNum, portMacNum) = phyAddr;
    }
    else
    {
        /* ports above 47 */
        PRV_CPSS_PHY_SMI_PORT_ADDR_MAC(devNum, portMacNum) = phyAddr;
    }

    return rc;
}

/**
* @internal internal_cpssDxChPhyPortAddrSet function
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
static GT_STATUS internal_cpssDxChPhyPortAddrSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8                phyAddr
)
{
    GT_U32  regOffset;              /* Phy address register offset.     */
    GT_U32  fieldOffset;            /* Phy address reg. field offset.   */
    GT_U32  regAddr;                /* Register address.                */
    GT_U32  phyAddress;
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;/* local port - support multi-port-groups device */
    GT_STATUS rc = GT_OK;     /* return status */
    GT_U32  portMacNum;         /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if(phyAddr >= BIT_5)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        return prvCpssDxChEArchPhyPortAddrSet(devNum, portNum, phyAddr);
    }

    /* XG ports are not supported */
    if ((GT_FALSE == PRV_CPSS_DXCH_IS_FLEX_LINK_MAC(devNum,portMacNum)) &&
        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portType == PRV_CPSS_PORT_XG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    phyAddress = (GT_U32)phyAddr;

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portMacNum);

    /* the DxCh devices has PHY polling support (Out-Of-Band autonegotiation)
       for first 24 ports only. Some xCat devices has Flex ports (24-27).
       This ports has no PHY polling support. */
    if (localPort < PRV_CPSS_DXCH_SMI_PPU_PORTS_NUM_CNS)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->smiRegs.smiPhyAddr;

        /* calc the reg and bit offset for the port */
        regOffset   = (localPort / 6) * CPSS_DX_PHY_ADDR_REG_OFFSET_CNS;
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            /* the units steps are 0x01000000 (not CPSS_DX_PHY_ADDR_REG_OFFSET_CNS)*/
            regOffset *= 2;
        }
        fieldOffset = (localPort % 6) * 5;

        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId,
                                  regAddr + regOffset,
                                  fieldOffset,
                                  5,
                                  phyAddress);
        if (rc != GT_OK)
        {
            return rc;
        }
        PRV_CPSS_PHY_SMI_PORT_ADDR_MAC(devNum,portNum) = (GT_U8)phyAddress;
    }
    else if (PRV_CPSS_DXCH_IS_FLEX_LINK_MAC(devNum,portMacNum))
    {
        /* flex port */
        PRV_CPSS_PHY_SMI_PORT_ADDR_MAC(devNum,portMacNum) = phyAddr;
    }
    else
    {
        rc = GT_NOT_SUPPORTED;
    }

    return rc;
}

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
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8                phyAddr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhyPortAddrSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, phyAddr));

    rc = internal_cpssDxChPhyPortAddrSet(devNum, portNum, phyAddr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, phyAddr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChEArchPhyPortAddrGet function
* @endinternal
*
* @brief   Get the port's default phy address.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - port number.
*
* @param[out] phyAddFromHwPtr          - (pointer to) phy address in HW.
* @param[out] phyAddFromDbPtr          - (pointer to) phy address in DB.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChEArchPhyPortAddrGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U8   *phyAddFromHwPtr,
    OUT GT_U8   *phyAddFromDbPtr
)
{
    GT_STATUS rc = GT_OK;       /* return status */
    GT_U32  phyAddress;
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;/* local port - support multi-port-groups device */
    GT_U32  portMacNum;         /* MAC number */
    GT_U32  smiInstance;
    GT_U32  smiLocalPort;

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portMacNum);

    /* the Bocat2, Caelum devices has PHY polling support (Out-Of-Band autonegotiation)
       for first 48 ports only */
    if (localPort < PRV_CPSS_DXCH_E_ARCH_SMI_PPU_PORTS_NUM_CNS)
    {
        /* array of numbers of ports par SMI interface */
        rc = prvCpssDxChEArchPhyPortSMIGet(devNum,portMacNum,/*OUT*/&smiInstance,&smiLocalPort);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChSMIPortPhyAddGet(devNum,portGroupId,smiInstance,smiLocalPort,/*OUT*/&phyAddress);
        if (rc != GT_OK)
        {
            return rc;
        }
        *phyAddFromHwPtr = (GT_U8)phyAddress;
        *phyAddFromDbPtr = (GT_U8)PRV_CPSS_PHY_SMI_PORT_ADDR_MAC(devNum,portNum);
    }
    else
    {
        /* ports above 47 */
        *phyAddFromDbPtr = (GT_U8)PRV_CPSS_PHY_SMI_PORT_ADDR_MAC(devNum, portMacNum);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPhyPortAddrGet function
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
static GT_STATUS internal_cpssDxChPhyPortAddrGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U8   *phyAddFromHwPtr,
    OUT GT_U8   *phyAddFromDbPtr
)
{
    GT_U32  regOffset;              /* Phy address register offset.     */
    GT_U32  fieldOffset;            /* Phy address reg. field offset.   */
    GT_U32  regAddr;                /* Register address.                */
    GT_U32  phyAddress;
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;/* local port - support multi-port-groups device */
    GT_STATUS rc = GT_OK;     /* return status */
    GT_U32  portMacNum;         /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(phyAddFromHwPtr);
    CPSS_NULL_PTR_CHECK_MAC(phyAddFromDbPtr);

    (*phyAddFromHwPtr) = (GT_U8)0xFF;/* not all cases get from HW .. so init it here */

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        return prvCpssDxChEArchPhyPortAddrGet(devNum, portNum,
                                              phyAddFromHwPtr, phyAddFromDbPtr);
    }

    /* XG ports are not supported */
    if ((GT_FALSE == PRV_CPSS_DXCH_IS_FLEX_LINK_MAC(devNum,portMacNum)) &&
        (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portType == PRV_CPSS_PORT_XG_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portMacNum);

    /* the DxCh devices has PHY polling support (Out-Of-Band autonegotiation)
       for first 24 ports only. Some xCat devices has Flex ports (24-27).
       This ports has no PHY polling support. */
    if (localPort < PRV_CPSS_DXCH_SMI_PPU_PORTS_NUM_CNS)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->smiRegs.smiPhyAddr;

        /* calc the reg and bit offset for the port */
        regOffset   = (localPort / 6) * CPSS_DX_PHY_ADDR_REG_OFFSET_CNS;
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            /* the units steps are 0x01000000 (not CPSS_DX_PHY_ADDR_REG_OFFSET_CNS)*/
            regOffset *= 2;
        }
        fieldOffset = (localPort % 6) * 5;

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,portGroupId,
                                  regAddr + regOffset,
                                  fieldOffset,
                                  5,
                                  &phyAddress);
        if( rc != GT_OK )
        {
            return rc;
        }
        *phyAddFromHwPtr = (GT_U8)phyAddress;
    }
    else if (PRV_CPSS_DXCH_IS_FLEX_LINK_MAC(devNum,portMacNum))
    {
        /* flex port */
        *phyAddFromDbPtr = (GT_U8)PRV_CPSS_PHY_SMI_PORT_ADDR_MAC(devNum,portNum);
    }
    else
        rc = GT_NOT_SUPPORTED;

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhyPortAddrGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, phyAddFromHwPtr, phyAddFromDbPtr));

    rc = internal_cpssDxChPhyPortAddrGet(devNum, portNum, phyAddFromHwPtr, phyAddFromDbPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, phyAddFromHwPtr, phyAddFromDbPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChEArchPhyAutoPollNumOfPortsSet function
* @endinternal
*
* @brief   Configure number of auto poll ports for SMI interfaces
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] autoPollNumOfPortsSmi0   - number of ports for SMI0.
* @param[in] autoPollNumOfPortsSmi1   - number of ports for SMI1.
* @param[in] autoPollNumOfPortsSmi2   - number of ports for SMI2.
* @param[in] autoPollNumOfPortsSmi3   - number of ports for SMI3.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong autoPollNumOfPortsSmi0,
*                                       autoPollNumOfPortsSmi1,
*                                       autoPollNumOfPortsSmi2,
*                                       autoPollNumOfPortsSmi3
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChEArchPhyAutoPollNumOfPortsSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    autoPollNumOfPortsSmi0,
    IN  CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    autoPollNumOfPortsSmi1,
    IN  CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    autoPollNumOfPortsSmi2,
    IN  CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    autoPollNumOfPortsSmi3
)
{
    GT_STATUS       rc;             /* return code      */
    GT_U32          smiInstance;
    GT_U32          autoPollNumOfPortsSmi[CPSS_PHY_SMI_INTERFACE_MAX_E];
    PRV_CPSS_DXCH_PP_CONFIG_STC *pDev;

    pDev = PRV_CPSS_DXCH_PP_MAC(devNum);

    if (pDev->hwInfo.smi_support.numberOfSmiIf != 4)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* check that the combination is acceptable */
    if (((autoPollNumOfPortsSmi0 == CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E)  && (autoPollNumOfPortsSmi1 == CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_16_E)) ||
        ((autoPollNumOfPortsSmi0 == CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E) && (autoPollNumOfPortsSmi1 == CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E)) ||
        ((autoPollNumOfPortsSmi0 == CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_16_E) && (autoPollNumOfPortsSmi1 == CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E)) ||
        ((autoPollNumOfPortsSmi0 == CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E)  && (autoPollNumOfPortsSmi1 == CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E)))
    {
        /* valid combination, do nothing */
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (((autoPollNumOfPortsSmi2 == CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E)  && (autoPollNumOfPortsSmi3 == CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_16_E)) ||
        ((autoPollNumOfPortsSmi2 == CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E) && (autoPollNumOfPortsSmi3 == CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E)) ||
        ((autoPollNumOfPortsSmi2 == CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_16_E) && (autoPollNumOfPortsSmi3 == CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E)) ||
        ((autoPollNumOfPortsSmi2 == CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E)  && (autoPollNumOfPortsSmi3 == CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E)))
    {
        /* valid combination, do nothing */
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    autoPollNumOfPortsSmi[CPSS_PHY_SMI_INTERFACE_0_E] = autoPollNumOfPortsSmi0;
    autoPollNumOfPortsSmi[CPSS_PHY_SMI_INTERFACE_1_E] = autoPollNumOfPortsSmi1;
    autoPollNumOfPortsSmi[CPSS_PHY_SMI_INTERFACE_2_E] = autoPollNumOfPortsSmi2;
    autoPollNumOfPortsSmi[CPSS_PHY_SMI_INTERFACE_3_E] = autoPollNumOfPortsSmi3;

    for (smiInstance = 0 ; smiInstance < CPSS_PHY_SMI_INTERFACE_MAX_E; smiInstance++)
    {
        rc = prvCpssDxChSMIAutoPollNumOfPortsSet(devNum,0,smiInstance,autoPollNumOfPortsSmi[smiInstance]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChEArchPhyAutoPollNumOfPortsGet function
* @endinternal
*
* @brief   Get number of auto poll ports for SMI interfaces.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] autoPollNumOfPortsSmi0Ptr - number of ports for SMI0.
* @param[out] autoPollNumOfPortsSmi1Ptr - number of ports for SMI1.
* @param[out] autoPollNumOfPortsSmi2Ptr - number of ports for SMI2.
* @param[out] autoPollNumOfPortsSmi3Ptr - number of ports for SMI3.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChEArchPhyAutoPollNumOfPortsGet
(
    IN  GT_U8                                           devNum,
    OUT CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    *autoPollNumOfPortsSmi0Ptr,
    OUT CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    *autoPollNumOfPortsSmi1Ptr,
    OUT CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    *autoPollNumOfPortsSmi2Ptr,
    OUT CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    *autoPollNumOfPortsSmi3Ptr
)
{
    GT_STATUS       rc;             /* return code      */
    GT_U32          value[CPSS_PHY_SMI_INTERFACE_MAX_E];
    GT_U32          smiInterface;

    PRV_CPSS_DXCH_PP_CONFIG_STC *pDev;

    pDev = PRV_CPSS_DXCH_PP_MAC(devNum);

    if (pDev->hwInfo.smi_support.numberOfSmiIf != 4)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    for (smiInterface = CPSS_PHY_SMI_INTERFACE_0_E; smiInterface < CPSS_PHY_SMI_INTERFACE_MAX_E; smiInterface++)
    {
        rc = prvCpssDxChSMIAutoPollNumOfPortsGet(devNum,0,smiInterface,&value[smiInterface]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    *autoPollNumOfPortsSmi0Ptr = (CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT)value[0];
    *autoPollNumOfPortsSmi1Ptr = (CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT)value[1];
    *autoPollNumOfPortsSmi2Ptr = (CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT)value[2];
    *autoPollNumOfPortsSmi3Ptr = (CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT)value[3];
    return GT_OK;
}


/**
* @internal internal_cpssDxChPhyAutoPollNumOfPortsSet function
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
static GT_STATUS internal_cpssDxChPhyAutoPollNumOfPortsSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    autoPollNumOfPortsSmi0,
    IN  CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    autoPollNumOfPortsSmi1,
    IN  CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    autoPollNumOfPortsSmi2,
    IN  CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    autoPollNumOfPortsSmi3
)
{
    GT_U32          regAddr;        /* register address */
    GT_U32          regValue;       /* hw value         */
    GT_STATUS       rc;             /* return code      */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E
                    | CPSS_LION2_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        return prvCpssDxChEArchPhyAutoPollNumOfPortsSet(devNum,
                                                        autoPollNumOfPortsSmi0,
                                                        autoPollNumOfPortsSmi1,
                                                        autoPollNumOfPortsSmi2,
                                                        autoPollNumOfPortsSmi3);
    }

    /* check that the combination is acceptable */
    if (((autoPollNumOfPortsSmi0 == CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E)  && (autoPollNumOfPortsSmi1 == CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_16_E)) ||
        ((autoPollNumOfPortsSmi0 == CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E) && (autoPollNumOfPortsSmi1 == CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E)) ||
        ((autoPollNumOfPortsSmi0 == CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_16_E) && (autoPollNumOfPortsSmi1 == CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E)) ||
        ((autoPollNumOfPortsSmi0 == CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_8_E)  && (autoPollNumOfPortsSmi1 == CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E)))
    {
        /* valid combination, do nothing */
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_SMI_AUTO_POLL_NUM_OF_PORTS_SW_TO_HW_CONVERT_MAC(
                                                    autoPollNumOfPortsSmi0,
                                                    regValue)

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->smiRegs.lms0MiscConfig;

    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 18, 2, regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_DXCH_SMI_AUTO_POLL_NUM_OF_PORTS_SW_TO_HW_CONVERT_MAC(
                                                    autoPollNumOfPortsSmi1,
                                                    regValue)

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->smiRegs.lms1MiscConfig;

    return prvCpssDrvHwPpSetRegField(devNum, regAddr, 18, 2, regValue);

}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhyAutoPollNumOfPortsSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, autoPollNumOfPortsSmi0, autoPollNumOfPortsSmi1, autoPollNumOfPortsSmi2, autoPollNumOfPortsSmi3));

    rc = internal_cpssDxChPhyAutoPollNumOfPortsSet(devNum, autoPollNumOfPortsSmi0, autoPollNumOfPortsSmi1, autoPollNumOfPortsSmi2, autoPollNumOfPortsSmi3);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, autoPollNumOfPortsSmi0, autoPollNumOfPortsSmi1, autoPollNumOfPortsSmi2, autoPollNumOfPortsSmi3));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhyAutoPollNumOfPortsGet function
* @endinternal
*
* @brief   Get number of auto poll ports for SMI interfaces.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
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
static GT_STATUS internal_cpssDxChPhyAutoPollNumOfPortsGet
(
    IN  GT_U8                                           devNum,
    OUT CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    *autoPollNumOfPortsSmi0Ptr,
    OUT CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    *autoPollNumOfPortsSmi1Ptr,
    OUT CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    *autoPollNumOfPortsSmi2Ptr,
    OUT CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT    *autoPollNumOfPortsSmi3Ptr
)
{
    GT_U32          regAddr;        /* register address */
    GT_U32          regValue;       /* hw value         */
    GT_STATUS       rc;             /* return code      */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E
                    | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(autoPollNumOfPortsSmi0Ptr);
    CPSS_NULL_PTR_CHECK_MAC(autoPollNumOfPortsSmi1Ptr);
    CPSS_NULL_PTR_CHECK_MAC(autoPollNumOfPortsSmi2Ptr);
    CPSS_NULL_PTR_CHECK_MAC(autoPollNumOfPortsSmi3Ptr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        return prvCpssDxChEArchPhyAutoPollNumOfPortsGet(devNum,
                                        autoPollNumOfPortsSmi0Ptr,
                                        autoPollNumOfPortsSmi1Ptr,
                                        autoPollNumOfPortsSmi2Ptr,
                                        autoPollNumOfPortsSmi3Ptr);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->smiRegs.lms0MiscConfig;

    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 18, 2, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_DXCH_SMI_AUTO_POLL_NUM_OF_PORTS_HW_TO_SW_CONVERT_MAC(
                                                    regValue,
                                                    *autoPollNumOfPortsSmi0Ptr)

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->smiRegs.lms1MiscConfig;


    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 18, 2, &regValue);

    PRV_CPSS_DXCH_SMI_AUTO_POLL_NUM_OF_PORTS_HW_TO_SW_CONVERT_MAC(
                                                    regValue,
                                                    *autoPollNumOfPortsSmi1Ptr)
    return rc;
}

/**
* @internal cpssDxChPhyAutoPollNumOfPortsGet function
* @endinternal
*
* @brief   Get number of auto poll ports for SMI interfaces.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhyAutoPollNumOfPortsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, autoPollNumOfPortsSmi0Ptr, autoPollNumOfPortsSmi1Ptr, autoPollNumOfPortsSmi2Ptr, autoPollNumOfPortsSmi3Ptr));

    rc = internal_cpssDxChPhyAutoPollNumOfPortsGet(devNum, autoPollNumOfPortsSmi0Ptr, autoPollNumOfPortsSmi1Ptr, autoPollNumOfPortsSmi2Ptr, autoPollNumOfPortsSmi3Ptr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, autoPollNumOfPortsSmi0Ptr, autoPollNumOfPortsSmi1Ptr, autoPollNumOfPortsSmi2Ptr, autoPollNumOfPortsSmi3Ptr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhyPortSmiInterfaceSet function
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
static GT_STATUS internal_cpssDxChPhyPortSmiInterfaceSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PHY_SMI_INTERFACE_ENT  smiInterface
)
{
    GT_STATUS rc;
    GT_U32  portMacNum;             /* MAC number */
    GT_U32  smiMngmtRegAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if (smiInterface >= CPSS_PHY_SMI_INTERFACE_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssDxChSMIDrvManagementRegAddrGet(devNum,smiInterface,/*OUT*/&smiMngmtRegAddr);
        if (rc != GT_OK)
        {
            return rc;
        }
        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].smiIfInfo.smiInterface   = smiInterface;
        return GT_OK;
    }
    else
    {
        switch(smiInterface)
        {
            case CPSS_PHY_SMI_INTERFACE_0_E:
                /* use SMI#0 controller to access port's PHY*/
                PRV_CPSS_PHY_SMI_INSTANCE_MAC(devNum,portMacNum) = CPSS_PHY_SMI_INTERFACE_0_E;
                break;
            case CPSS_PHY_SMI_INTERFACE_1_E:
                PRV_CPSS_PHY_SMI_INSTANCE_MAC(devNum,portMacNum) = CPSS_PHY_SMI_INTERFACE_1_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhyPortSmiInterfaceSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, smiInterface));

    rc = internal_cpssDxChPhyPortSmiInterfaceSet(devNum, portNum, smiInterface);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, smiInterface));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhyPortSmiInterfaceGet function
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
static GT_STATUS internal_cpssDxChPhyPortSmiInterfaceGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PHY_SMI_INTERFACE_ENT  *smiInterfacePtr
)
{
    GT_U32 portMacNum;              /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(smiInterfacePtr);

    *smiInterfacePtr = PRV_CPSS_PHY_SMI_INSTANCE_MAC(devNum,portMacNum);

    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhyPortSmiInterfaceGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, smiInterfacePtr));

    rc = internal_cpssDxChPhyPortSmiInterfaceGet(devNum, portNum, smiInterfacePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, smiInterfacePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChPhyPortSmiInterfacePortGroupSet function
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
static GT_STATUS internal_cpssDxChPhyPortSmiInterfacePortGroupSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32  smiInterfacePortGroup
)
{
    GT_STATUS   rc = GT_OK; /* return status */
    GT_U32      portMacNum; /* MAC to which mapped port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                          | CPSS_XCAT2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if((CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
            && (/*(smiInterfacePortGroup < 0) || */ (smiInterfacePortGroup > 7)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else if((CPSS_PP_FAMILY_DXCH_LION_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
            && (/*(smiInterfacePortGroup < 0) || */(smiInterfacePortGroup > 3)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_PHY_SMI_GROUP_PORT_MAC(devNum, portMacNum) = smiInterfacePortGroup;

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhyPortSmiInterfacePortGroupSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, smiInterfacePortGroup));

    rc = internal_cpssDxChPhyPortSmiInterfacePortGroupSet(devNum, portNum, smiInterfacePortGroup);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, smiInterfacePortGroup));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhyPortSmiInterfacePortGroupGet function
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
static GT_STATUS internal_cpssDxChPhyPortSmiInterfacePortGroupGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_U32  *smiInterfacePortGroupPtr
)
{
    GT_STATUS   rc = GT_OK; /* return status */
    GT_U32      portMacNum; /* MAC to which mapped port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                                          CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
                                          CPSS_XCAT2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    CPSS_NULL_PTR_CHECK_MAC(smiInterfacePortGroupPtr);

    *smiInterfacePortGroupPtr = PRV_CPSS_PHY_SMI_GROUP_PORT_MAC(devNum, portMacNum);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhyPortSmiInterfacePortGroupGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, smiInterfacePortGroupPtr));

    rc = internal_cpssDxChPhyPortSmiInterfacePortGroupGet(devNum, portNum, smiInterfacePortGroupPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, smiInterfacePortGroupPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhyPortXSmiInterfaceSet function
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
static GT_STATUS internal_cpssDxChPhyPortXSmiInterfaceSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT  smiInterface
)
{
    GT_U32  portMacNum;         /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E |
         CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portType < PRV_CPSS_PORT_XG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    switch(smiInterface)
    {
        case CPSS_PHY_XSMI_INTERFACE_0_E:
        case CPSS_PHY_XSMI_INTERFACE_1_E:
            break;
        case CPSS_PHY_XSMI_INTERFACE_2_E:
        case CPSS_PHY_XSMI_INTERFACE_3_E:
            if (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Set port XSMI interface */
    PRV_CPSS_PHY_XSMI_PORT_INTERFACE_MAC(devNum, portMacNum) = smiInterface;

    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhyPortXSmiInterfaceSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, smiInterface));

    rc = internal_cpssDxChPhyPortXSmiInterfaceSet(devNum, portNum, smiInterface);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, smiInterface));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhyPortXSmiInterfaceGet function
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
static GT_STATUS internal_cpssDxChPhyPortXSmiInterfaceGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PHY_XSMI_INTERFACE_ENT  *smiInterfacePtr
)
{
    GT_U32  portMacNum;         /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E |
         CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portType < PRV_CPSS_PORT_XG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* Get port XSMI interface */
    *smiInterfacePtr = PRV_CPSS_PHY_XSMI_PORT_INTERFACE_MAC(devNum, portMacNum);

    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhyPortXSmiInterfaceGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, smiInterfacePtr));

    rc = internal_cpssDxChPhyPortXSmiInterfaceGet(devNum, portNum, smiInterfacePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, smiInterfacePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

