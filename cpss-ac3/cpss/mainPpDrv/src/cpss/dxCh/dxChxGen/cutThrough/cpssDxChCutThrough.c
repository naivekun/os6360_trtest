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
* @file cpssDxChCutThrough.c
*
* @brief CPSS DXCH Cut Through facility facility implementation.
*
* @version   28
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/cutThrough/cpssDxChCutThrough.h>
#include <cpss/dxCh/dxChxGen/cutThrough/private/prvCpssDxChCutThrough.h>
#include <cpss/dxCh/dxChxGen/cutThrough/private/prvCpssDxChCutThroughLog.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/cscd/private/prvCpssDxChCscd.h>

/**
* @internal internal_cpssDxChCutThroughPortEnableSet function
* @endinternal
*
* @brief   Enable/Disable Cut Through forwarding for packets received on the port.
*         For Bobcat3 device should be initialized with PHASE1_INIT_INFO.cutThroughEnable == GT_TRUE.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number including CPU port.
* @param[in] enable                   - GT_TRUE:  Enable the port for Cut Through.
*                                      GT_FALSE: Disable the port for Cut Through.
* @param[in] untaggedEnable           -  GT_TRUE:  Enable Cut Through forwarding for
*                                      untagged packets received on the port.
*                                      GT_FALSE: Disable Cut Through forwarding for
*                                      untagged packets received on the port.
*                                      Used only if enable == GT_TRUE.
*                                      Untagged packets for Cut Through purposes - packets
*                                      that don't have VLAN tag or its ethertype isn't equal
*                                      to one of two configurable VLAN ethertypes.
*                                      See cpssDxChCutThroughVlanEthertypeSet.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_BAD_STATE             - on enabling cut-through disabled on initialization
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Cut Through forwarding for tagged packets is enabled
*       per source port and UP. See cpssDxChCutThroughUpEnableSet.
*
*/
static GT_STATUS internal_cpssDxChCutThroughPortEnableSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL  enable,
    IN GT_BOOL  untaggedEnable
)
{
    GT_U32    regAddr;           /* register address */
    GT_U32    fieldOffset1;       /* The start bit number in the register - for enable*/
    GT_U32    fieldOffset2;       /* The start bit number in the register - for untaggedEnable*/
    GT_U32    data;              /* reg sub field data */
    GT_U32    mask;              /* Mask for selecting the written bits */
    GT_U32    portGroupId;       /* the port group Id - support multi port group device */
    GT_U32     localPort;         /* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_RXDMA_NUM_GET_MAC(devNum, portNum, portNum);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi port groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portNum);

    if ((PRV_CPSS_DXCH_PP_MAC(devNum)->cutThroughEnable == GT_FALSE)
        && (enable != GT_FALSE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_MAC(devNum).
            configs.cutThrough.channelCTConfig[localPort];
        fieldOffset1 = 0; /*enable*/
        fieldOffset2 = 1; /*untaggedEnable*/
    }
    else
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* each RXDMA port hold it's own registers , so bit index is not parameter of port number */
        fieldOffset1 = 7; /*enable*/
        fieldOffset2 = 8; /*untaggedEnable*/

        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
            singleChannelDMAConfigs.SCDMAConfig0[localPort];
    }
    else
    {
        if(CPSS_CPU_PORT_NUM_CNS == localPort)
        {
            fieldOffset1 = 15;    /*enable*/
        }
        else
        {
            fieldOffset1 = localPort;   /*enable*/
        }

        fieldOffset2 = fieldOffset1 + 16; /*untaggedEnable*/

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                           cutThroughRegs.ctEnablePerPortReg;
    }

    data = (enable == GT_TRUE) ? (1 << fieldOffset1) : 0;
    mask = 1 << fieldOffset1;

    /* Set Cut Through forwarding data for untagged packets received on the port */
    data |= (untaggedEnable == GT_TRUE) ? (1 << fieldOffset2) : 0;
    mask |= 1 << fieldOffset2;

    /* Enable/Disable Cut Through forwarding */
    return prvCpssHwPpPortGroupWriteRegBitMask(devNum, portGroupId,
                                             regAddr, mask, data);
}

/**
* @internal cpssDxChCutThroughPortEnableSet function
* @endinternal
*
* @brief   Enable/Disable Cut Through forwarding for packets received on the port.
*         Bobcat3 device should be initialized with PHASE1_INIT_INFO.cutThroughEnable == GT_TRUE.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number including CPU port.
* @param[in] enable                   - GT_TRUE:  Enable the port for Cut Through.
*                                      GT_FALSE: Disable the port for Cut Through.
* @param[in] untaggedEnable           -  GT_TRUE:  Enable Cut Through forwarding for
*                                      untagged packets received on the port.
*                                      GT_FALSE: Disable Cut Through forwarding for
*                                      untagged packets received on the port.
*                                      Used only if enable == GT_TRUE.
*                                      Untagged packets for Cut Through purposes - packets
*                                      that don't have VLAN tag or its ethertype isn't equal
*                                      to one of two configurable VLAN ethertypes.
*                                      See cpssDxChCutThroughVlanEthertypeSet.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_BAD_STATE             - on enabling cut-through disabled on initialization
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Cut Through forwarding for tagged packets is enabled
*       per source port and UP. See cpssDxChCutThroughUpEnableSet.
*
*/
GT_STATUS cpssDxChCutThroughPortEnableSet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL  enable,
    IN GT_BOOL  untaggedEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughPortEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable, untaggedEnable));

    rc = internal_cpssDxChCutThroughPortEnableSet(devNum, portNum, enable, untaggedEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable, untaggedEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCutThroughPortEnableGet function
* @endinternal
*
* @brief   Get Cut Through forwarding mode on the specified port.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number including CPU port.
*
* @param[out] enablePtr                - pointer to Cut Through forwarding mode status:
*                                      - GT_TRUE:  Enable the port for Cut Through.
*                                      GT_FALSE: Disable the port for Cut Through.
* @param[out] untaggedEnablePtr        - pointer to Cut Through forwarding mode status
*                                      for untagged packets.
*                                      Used only if enablePtr == GT_TRUE.
*                                      GT_TRUE:  Enable Cut Through forwarding for
*                                      untagged packets received on the port.
*                                      GT_FALSE: Disable Cut Through forwarding for
*                                      untagged packets received on the port.
*                                      Untagged packets for Cut Through purposes - packets
*                                      that don't have VLAN tag or its ethertype isn't equal
*                                      to one of two configurable VLAN ethertypes.
*                                      See cpssDxChCutThroughVlanEthertypeSet.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCutThroughPortEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL  *enablePtr,
    OUT GT_BOOL  *untaggedEnablePtr
)
{
    GT_U32    regAddr;           /* register address */
    GT_U32    fieldOffset1;       /* The start bit number in the register - for enable*/
    GT_U32    fieldOffset2;       /* The start bit number in the register - for untaggedEnable*/
    GT_U32    data;              /* reg data */
    GT_U32    portGroupId;       /* the port group Id - support multi port group device */
    GT_U32     localPort;         /* local port - support multi-port-groups device */
    GT_STATUS rc;              /* return status                */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_RXDMA_NUM_GET_MAC(devNum, portNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(untaggedEnablePtr);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi port groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portNum);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_MAC(devNum).
            configs.cutThrough.channelCTConfig[localPort];
        fieldOffset1 = 0; /*enable*/
        fieldOffset2 = 1; /*untaggedEnable*/
    }
    else
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* each RXDMA port hold it's own registers , so bit index is not parameter of port number */
        fieldOffset1 = 7; /*enable*/
        fieldOffset2 = 8; /*untaggedEnable*/

        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
            singleChannelDMAConfigs.SCDMAConfig0[localPort];
    }
    else
    {
        if(CPSS_CPU_PORT_NUM_CNS == localPort)
        {
            fieldOffset1 = 15;    /*enable*/
        }
        else
        {
            fieldOffset1 = localPort;   /*enable*/
        }

        fieldOffset2 = fieldOffset1 + 16; /*untaggedEnable*/

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                           cutThroughRegs.ctEnablePerPortReg;
    }

    /* Get Enable/Disable Cut Through forwarding status */
    rc = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId, regAddr, &data);
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (((data >> fieldOffset1) & 0x1) == 0) ? GT_FALSE : GT_TRUE;

    /* Get Cut Through forwarding status for untagged packets received on the port */
    *untaggedEnablePtr = (((data >> fieldOffset2)  & 0x1) == 0) ? GT_FALSE : GT_TRUE;

    return GT_OK;
}

/**
* @internal cpssDxChCutThroughPortEnableGet function
* @endinternal
*
* @brief   Get Cut Through forwarding mode on the specified port.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number including CPU port.
*
* @param[out] enablePtr                - pointer to Cut Through forwarding mode status:
*                                      - GT_TRUE:  Enable the port for Cut Through.
*                                      GT_FALSE: Disable the port for Cut Through.
* @param[out] untaggedEnablePtr        - pointer to Cut Through forwarding mode status
*                                      for untagged packets.
*                                      Used only if enablePtr == GT_TRUE.
*                                      GT_TRUE:  Enable Cut Through forwarding for
*                                      untagged packets received on the port.
*                                      GT_FALSE: Disable Cut Through forwarding for
*                                      untagged packets received on the port.
*                                      Untagged packets for Cut Through purposes - packets
*                                      that don't have VLAN tag or its ethertype isn't equal
*                                      to one of two configurable VLAN ethertypes.
*                                      See cpssDxChCutThroughVlanEthertypeSet.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughPortEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL  *enablePtr,
    OUT GT_BOOL  *untaggedEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughPortEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr, untaggedEnablePtr));

    rc = internal_cpssDxChCutThroughPortEnableGet(devNum, portNum, enablePtr, untaggedEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr, untaggedEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCutThroughUpEnableSet function
* @endinternal
*
* @brief   Enable / Disable tagged packets with the specified UP
*         to be Cut Through.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] up                       - user priority of a VLAN or DSA tagged
*                                      packet (APPLICABLE RANGES: 0..7).
* @param[in] enable                   - GT_TRUE:  tagged packets, with the specified UP
*                                      may be subject to Cut Through forwarding.
*                                      GT_FALSE: tagged packets, with the specified UP
*                                      aren't subject to Cut Through forwarding.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or up.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The ingress port should be enabled for Cut Through forwarding.
*       Use cpssDxChCutThroughPortEnableSet for it.
*
*/
static GT_STATUS internal_cpssDxChCutThroughUpEnableSet
(
    IN GT_U8    devNum,
    IN GT_U8    up,
    IN GT_BOOL  enable
)
{
    GT_U32    regAddr;           /* register address */
    GT_U32    data;              /* reg sub field data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_XCAT2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    if(up > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    data = (enable == GT_TRUE) ? 1 : 0;

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_MAC(devNum).configs.cutThrough.CTUPEnable;
    }
    else
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.globalConfigCutThrough.CTUPEnable;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->cutThroughRegs.ctUpEnableReg;
    }

    /* Enable / Disable tagged packets, with the specified UP to be Cut Through. */
    return prvCpssHwPpSetRegField(devNum, regAddr, up, 1, data);

}

/**
* @internal cpssDxChCutThroughUpEnableSet function
* @endinternal
*
* @brief   Enable / Disable tagged packets with the specified UP
*         to be Cut Through.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] up                       - user priority of a VLAN or DSA tagged
*                                      packet (APPLICABLE RANGES: 0..7).
* @param[in] enable                   - GT_TRUE:  tagged packets, with the specified UP
*                                      may be subject to Cut Through forwarding.
*                                      GT_FALSE: tagged packets, with the specified UP
*                                      aren't subject to Cut Through forwarding.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or up.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The ingress port should be enabled for Cut Through forwarding.
*       Use cpssDxChCutThroughPortEnableSet for it.
*
*/
GT_STATUS cpssDxChCutThroughUpEnableSet
(
    IN GT_U8    devNum,
    IN GT_U8    up,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughUpEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, up, enable));

    rc = internal_cpssDxChCutThroughUpEnableSet(devNum, up, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, up, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCutThroughUpEnableGet function
* @endinternal
*
* @brief   Get Cut Through forwarding mode for tagged packets
*         with the specified UP.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] up                       - user priority of a VLAN or DSA tagged
*                                      packet (APPLICABLE RANGES: 0..7).
*
* @param[out] enablePtr                - pointer to Cut Through forwarding mode status
*                                      for tagged packets, with the specified UP.:
*                                      - GT_TRUE:  tagged packets, with the specified UP
*                                      may be subject to Cut Through forwarding.
*                                      GT_FALSE: tagged packets, with the specified UP
*                                      aren't subject to Cut Through forwarding.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or up
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCutThroughUpEnableGet
(
    IN  GT_U8    devNum,
    IN GT_U8     up,
    OUT GT_BOOL  *enablePtr
)
{
    GT_U32    regAddr;     /* register address */
    GT_U32    data;        /* reg sub field data */
    GT_STATUS rc;          /* return status                */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_XCAT2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(up > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_MAC(devNum).configs.cutThrough.CTUPEnable;
    }
    else
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.globalConfigCutThrough.CTUPEnable;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->cutThroughRegs.ctUpEnableReg;
    }

    /* Enable / Disable tagged packets, with the specified UP to be Cut Through. */
    rc =  prvCpssHwPpGetRegField(devNum, regAddr, up, 1, &data);
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (data == 0) ? GT_FALSE : GT_TRUE;

    return GT_OK;
}

/**
* @internal cpssDxChCutThroughUpEnableGet function
* @endinternal
*
* @brief   Get Cut Through forwarding mode for tagged packets
*         with the specified UP.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] up                       - user priority of a VLAN or DSA tagged
*                                      packet (APPLICABLE RANGES: 0..7).
*
* @param[out] enablePtr                - pointer to Cut Through forwarding mode status
*                                      for tagged packets, with the specified UP.:
*                                      - GT_TRUE:  tagged packets, with the specified UP
*                                      may be subject to Cut Through forwarding.
*                                      GT_FALSE: tagged packets, with the specified UP
*                                      aren't subject to Cut Through forwarding.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or up
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughUpEnableGet
(
    IN  GT_U8    devNum,
    IN GT_U8     up,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughUpEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, up, enablePtr));

    rc = internal_cpssDxChCutThroughUpEnableGet(devNum, up, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, up, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCutThroughVlanEthertypeSet function
* @endinternal
*
* @brief   Set VLAN Ethertype in order to identify tagged packets.
*         A packed is identified as VLAN tagged for cut-through purposes.
*         Packet considered as tagged if packet's Ethertype equals to one of two
*         configurable VLAN Ethertypes.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] etherType0               - VLAN EtherType0 (APPLICABLE RANGES: 0..0xFFFF)
* @param[in] etherType1               - VLAN EtherType1 (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong etherType0 or etherType1
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCutThroughVlanEthertypeSet
(
    IN GT_U8    devNum,
    IN GT_U32   etherType0,
    IN GT_U32   etherType1
)
{
    GT_U32    regAddr;           /* register address */
    GT_U32    data;              /* reg sub field data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_XCAT2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_FALCON_E);

    if((etherType0 > 0xFFFF) || (etherType1 > 0xFFFF))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    data = etherType0 | etherType1 << 16;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.globalConfigCutThrough.CTEtherTypeIdentification;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->cutThroughRegs.ctEthertypeReg;
    }

    /* Set VLAN Ethertype in order to identify tagged packets. */
    return prvCpssHwPpWriteRegister(devNum, regAddr, data);
}

/**
* @internal cpssDxChCutThroughVlanEthertypeSet function
* @endinternal
*
* @brief   Set VLAN Ethertype in order to identify tagged packets.
*         A packed is identified as VLAN tagged for cut-through purposes.
*         Packet considered as tagged if packet's Ethertype equals to one of two
*         configurable VLAN Ethertypes.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] etherType0               - VLAN EtherType0 (APPLICABLE RANGES: 0..0xFFFF)
* @param[in] etherType1               - VLAN EtherType1 (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong etherType0 or etherType1
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughVlanEthertypeSet
(
    IN GT_U8    devNum,
    IN GT_U32   etherType0,
    IN GT_U32   etherType1
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughVlanEthertypeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, etherType0, etherType1));

    rc = internal_cpssDxChCutThroughVlanEthertypeSet(devNum, etherType0, etherType1);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, etherType0, etherType1));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCutThroughVlanEthertypeGet function
* @endinternal
*
* @brief   Get VLAN Ethertype in order to identify tagged packets.
*         A packed is identified as VLAN tagged for cut-through purposes.
*         Packet considered as tagged if packet's Ethertype equals to one of two
*         configurable VLAN Ethertypes.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Falcon.
*
* @param[in] devNum                   - device number.
*
* @param[out] etherType0Ptr            - Pointer to VLAN EtherType0
* @param[out] etherType1Ptr            - Pointer to VLAN EtherType1.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCutThroughVlanEthertypeGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *etherType0Ptr,
    OUT GT_U32   *etherType1Ptr
)
{
    GT_U32    regAddr;     /* register address   */
    GT_U32    data;        /* reg data */
    GT_STATUS rc;          /* return status      */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_XCAT2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(etherType0Ptr);
    CPSS_NULL_PTR_CHECK_MAC(etherType1Ptr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.globalConfigCutThrough.CTEtherTypeIdentification;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->cutThroughRegs.ctEthertypeReg;
    }

    /* Get VLAN Ethertype in order to identify tagged packets. */
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &data);
    if(rc != GT_OK)
    {
        return rc;
    }

    *etherType0Ptr = data & 0xFFFF;
    *etherType1Ptr = (data >> 16) & 0xFFFF;

    return GT_OK;
}

/**
* @internal cpssDxChCutThroughVlanEthertypeGet function
* @endinternal
*
* @brief   Get VLAN Ethertype in order to identify tagged packets.
*         A packed is identified as VLAN tagged for cut-through purposes.
*         Packet considered as tagged if packet's Ethertype equals to one of two
*         configurable VLAN Ethertypes.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Falcon.
*
* @param[in] devNum                   - device number.
*
* @param[out] etherType0Ptr            - Pointer to VLAN EtherType0
* @param[out] etherType1Ptr            - Pointer to VLAN EtherType1.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughVlanEthertypeGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *etherType0Ptr,
    OUT GT_U32   *etherType1Ptr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughVlanEthertypeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, etherType0Ptr, etherType1Ptr));

    rc = internal_cpssDxChCutThroughVlanEthertypeGet(devNum, etherType0Ptr, etherType1Ptr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, etherType0Ptr, etherType1Ptr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCutThroughMinimalPacketSizeSet function
* @endinternal
*
* @brief   Set minimal packet size that is enabled for Cut Through.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] size                     - minimal packet  in bytes for Cut Through (APPLICABLE RANGES: 129..16376).
*                                      Granularity - 8 bytes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong size
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When using cut-through to 1G port, and bypassing
*       of Router And Ingress Policer engines is disabled,
*       the minimal packet size should be 512 bytes.
*       When bypassing of Router And Ingress Policer engines is enabled,
*       the minimal cut-through packet size should be:
*       - for 10G or faster ports - at least 257 bytes.
*       - for ports slower than 10 G - at least 513 bytes
*
*/
static GT_STATUS internal_cpssDxChCutThroughMinimalPacketSizeSet
(
    IN GT_U8    devNum,
    IN GT_U32   size
)
{
    GT_U32    regAddr;           /* register address   */
    GT_U32    data;              /* reg sub field data */
    GT_U32    hwValue;           /* HW value           */
    GT_U32    hwMask;            /* HW mask            */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_XCAT2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    if((size < 0x81) || (size > 0x3FF8))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
        cutThroughRegs.ctPacketIndentificationReg;

    /* The field is defined in "Number of transactions" in resolution of 8 bytes.
       For example: 17 transactions (default value) means a packet whose
       byte count is 129B up to 136B */
    data = size / 8;
    if((size % 8) != 0)
    {
        data++;
    }

    hwValue = data | (data << 11);
    hwMask  = 0x003FFFFF;

    /* Set minimal packet size that is enabled for Cut Through. */
    return prvCpssHwPpWriteRegBitMask(devNum, regAddr, hwMask, hwValue);
}

/**
* @internal cpssDxChCutThroughMinimalPacketSizeSet function
* @endinternal
*
* @brief   Set minimal packet size that is enabled for Cut Through.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] size                     - minimal packet  in bytes for Cut Through (APPLICABLE RANGES: 129..16376).
*                                      Granularity - 8 bytes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong size
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When using cut-through to 1G port, and bypassing
*       of Router And Ingress Policer engines is disabled,
*       the minimal packet size should be 512 bytes.
*       When bypassing of Router And Ingress Policer engines is enabled,
*       the minimal cut-through packet size should be:
*       - for 10G or faster ports - at least 257 bytes.
*       - for ports slower than 10 G - at least 513 bytes
*
*/
GT_STATUS cpssDxChCutThroughMinimalPacketSizeSet
(
    IN GT_U8    devNum,
    IN GT_U32   size
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughMinimalPacketSizeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, size));

    rc = internal_cpssDxChCutThroughMinimalPacketSizeSet(devNum, size);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, size));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCutThroughMinimalPacketSizeGet function
* @endinternal
*
* @brief   Get minimal packet size that is enabled for Cut Through.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] sizePtr                  - pointer to minimal packet size in bytes for Cut Through.
*                                      Granularity:
*                                      (APPLICABLE VALUES: Lion; Lion2 8 bytes)
*                                      (APPLICABLE VALUES: Bobcat3; Aldrin2; Falcon 128 bytes)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCutThroughMinimalPacketSizeGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *sizePtr
)
{
    GT_U32    regAddr;     /* register address */
    GT_U32    data;        /* reg sub field data */
    GT_STATUS rc;          /* return status                */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_XCAT2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(sizePtr);


    /* Get minimal packet size that is enabled for Cut Through. */
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_MAC(devNum).
            configs.cutThrough.channelCTConfig[0];
        /*14 bits value*/
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 2, 14, &data);
        if(rc != GT_OK)
        {
            return rc;
        }
        *sizePtr = data;
    }
    else
    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
            singleChannelDMAConfigs.SCDMACTPktIndentification[0];

        rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 7, &data);
        if(rc != GT_OK)
        {
            return rc;
        }
        *sizePtr = data * 128;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            cutThroughRegs.ctPacketIndentificationReg;

        rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 11, &data);
        if(rc != GT_OK)
        {
            return rc;
        }
        *sizePtr = data * 8;
    }
    return GT_OK;
}

/**
* @internal cpssDxChCutThroughMinimalPacketSizeGet function
* @endinternal
*
* @brief   Get minimal packet size that is enabled for Cut Through.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] sizePtr                  - pointer to minimal packet size in bytes for Cut Through.
*                                      Granularity:
*                                      (APPLICABLE VALUES: Lion; Lion2 8 bytes)
*                                      (APPLICABLE VALUES: Bobcat3; Aldrin2; Falcon 128 bytes)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughMinimalPacketSizeGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *sizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughMinimalPacketSizeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sizePtr));

    rc = internal_cpssDxChCutThroughMinimalPacketSizeGet(devNum, sizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCutThroughMemoryRateLimitSet function
* @endinternal
*
* @brief   Set rate limiting of read operations from the memory
*         per target port in Cut Through mode according to the port speed.
*         To prevent congestion in egress pipe, buffer memory read operations
*         are rate limited according to the target port speed.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2;
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number including CPU port.
* @param[in] enable                   - GT_TRUE - rate limiting is enabled.
*                                      - GT_FALSE - rate limiting is disabled.
* @param[in] portSpeed                - port speed.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, portNum or portSpeed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Rate limit is recommended be enabled
*       on all egress ports for cut-through traffic.
*       Rate limit to the CPU port should be configured as a 1G port.
*       Bobcat3 : rate limit configuration is part of cpssDxChPortModeSpeedSet() API
*
*/
static GT_STATUS internal_cpssDxChCutThroughMemoryRateLimitSet
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL enable,
    IN CPSS_PORT_SPEED_ENT  portSpeed
)
{
    GT_U32    regAddr;           /* register address */
    GT_U32    data;              /* reg sub field data */
    GT_U32    fieldOffset;       /* The start bit number in the register */
    GT_U32    portGroupId;       /* the port group Id - support multi port group device */
    GT_U32     localPort;         /* local port - support multi-port-groups device */
    GT_U32    coreClock;         /* core clock frequency */
    GT_U32    rc;                /* return status */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_XCAT2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);


    /* convert port number to txdma number, port number is lost !!! */
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_TXDMA_NUM_GET_MAC(devNum, portNum, portNum);

    coreClock = PRV_CPSS_PP_MAC(devNum)->coreClock;

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi port groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portNum);

    if(enable == GT_FALSE)
    {
        data = 0;
    }
    else
    {
        switch(portSpeed)
        {
            case CPSS_PORT_SPEED_1000_E:
                data = coreClock;
                break;
            case CPSS_PORT_SPEED_2500_E:
                data = coreClock * 10 / 25;
                break;
            case CPSS_PORT_SPEED_5000_E:
               data = coreClock  / 5;
               break;
            case CPSS_PORT_SPEED_10000_E:
                data = coreClock / 10;
                break;
            case CPSS_PORT_SPEED_12000_E:
                data = coreClock / 12;
                break;
            case CPSS_PORT_SPEED_20000_E:
                data = coreClock / 20;
                break;
            case CPSS_PORT_SPEED_25000_E:
                data = coreClock / 25;
                break;
            case CPSS_PORT_SPEED_40000_E:
                data = coreClock / 40;
                break;
            case CPSS_PORT_SPEED_50000_E:
                data = coreClock / 50;
                break;
            case CPSS_PORT_SPEED_100G_E:
                data = coreClock / 100;
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    if (data > 0)
    {
        /* the value should be decremented to avoid losses under full wire speed */
        data -= 1;

        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
        {
            if (coreClock == 360)
            {
                switch (portSpeed)
                {
                    case CPSS_PORT_SPEED_1000_E:  data = 359; break;
                    case CPSS_PORT_SPEED_10000_E: data = 35; break;
                    case CPSS_PORT_SPEED_20000_E: data = 17; break;
                    case CPSS_PORT_SPEED_40000_E: data = 4;  break;
                    default:
                    {
                        break;
                    }
                }
            }
        }
    }

    if(CPSS_CPU_PORT_NUM_CNS == localPort)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            cutThroughRegs.ctCpuPortMemoryRateLimitThresholdReg;
        fieldOffset = 0;
    }
    else
    {
            /* fix Coverity warning OVERRUN_STATIC for array:
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->cutThroughRegs.ctPortMemoryRateLimitThresholdReg
        */
        if (localPort >= 12)
        {
            /* it's should never happen because
            PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC does not allows
            such ports to be accepted  */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            cutThroughRegs.ctPortMemoryRateLimitThresholdReg[localPort / 2];
        fieldOffset = (localPort % 2) * 10;
    }
    rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
            fieldOffset, 10, data);

    return rc;
}

/**
* @internal cpssDxChCutThroughMemoryRateLimitSet function
* @endinternal
*
* @brief   Set rate limiting of read operations from the memory
*         per target port in Cut Through mode according to the port speed.
*         To prevent congestion in egress pipe, buffer memory read operations
*         are rate limited according to the target port speed.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number including CPU port.
* @param[in] enable                   - GT_TRUE - rate limiting is enabled.
*                                      - GT_FALSE - rate limiting is disabled.
* @param[in] portSpeed                - port speed.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, portNum or portSpeed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Rate limit is recommended be enabled
*       on all egress ports for cut-through traffic.
*       Rate limit to the CPU port should be configured as a 1G port.
*       Bobcat3 : rate limit configuration is part of cpssDxChPortModeSpeedSet() API
*
*/
GT_STATUS cpssDxChCutThroughMemoryRateLimitSet
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL enable,
    IN CPSS_PORT_SPEED_ENT  portSpeed
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughMemoryRateLimitSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable, portSpeed));

    rc = internal_cpssDxChCutThroughMemoryRateLimitSet(devNum, portNum, enable, portSpeed);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable, portSpeed));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCutThroughMemoryRateLimitGet function
* @endinternal
*
* @brief   Get rate limiting of read operations from the memory
*         per target port in Cut Through mode.
*         To prevent congestion in egress pipe, buffer memory read operations
*         are rate limited according to the target port speed.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number including CPU port.
*
* @param[out] enablePtr                - pointer to rate limiting mode status.
*                                      - GT_TRUE - rate limiting is enabled.
*                                      - GT_FALSE - rate limiting is disabled.
* @param[out] portSpeedPtr             - pointer to port speed.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCutThroughMemoryRateLimitGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *enablePtr,
    OUT CPSS_PORT_SPEED_ENT  *portSpeedPtr
)
{
    GT_U32    regAddr;           /* register address */
    GT_U32    data;              /* reg sub field data */
    GT_U32    fieldOffset;       /* The start bit number in the register */
    GT_U32    portGroupId;       /* the port group Id - support multi port group device */
    GT_U32     localPort;         /* local port - support multi-port-groups device */
    GT_STATUS rc;                /* return status                */
    GT_U32    coreClock;         /* core clock frequency */
    GT_U32    speedCoef;         /* speed coefficient */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_XCAT2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_TXDMA_NUM_GET_MAC(devNum, portNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(portSpeedPtr);

    coreClock = PRV_CPSS_PP_MAC(devNum)->coreClock;

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi port groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portNum);

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXDMA_MAC(devNum).txDMAPerSCDMAConfigs.
            SCDMAConfigs[localPort];
        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
            20, 10, &data);
    }
    else
    {
        if(CPSS_CPU_PORT_NUM_CNS == localPort)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                cutThroughRegs.ctCpuPortMemoryRateLimitThresholdReg;
            fieldOffset = 0;
        }
        else
        {
            /* fix Coverity warning OVERRUN_STATIC for array:
             PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->cutThroughRegs.ctPortMemoryRateLimitThresholdReg
            */
            if (localPort >= 12)
            {
                /* it's should never happen because
                PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC does not allows
                such ports to be accepted  */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                cutThroughRegs.ctPortMemoryRateLimitThresholdReg[localPort / 2];
            fieldOffset = (localPort % 2) * 10;
        }
        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
            fieldOffset, 10, &data);
    }
    if(rc != GT_OK)
    {
        return rc;
    }
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E && coreClock == 360)
    {
        switch (data)
        {
            case 359: *portSpeedPtr = CPSS_PORT_SPEED_1000_E;  break;
            case 35:  *portSpeedPtr = CPSS_PORT_SPEED_10000_E; break;
            case 17:  *portSpeedPtr = CPSS_PORT_SPEED_20000_E; break;
            case 4:   *portSpeedPtr = CPSS_PORT_SPEED_40000_E; break;
            default:
            {
                goto regular_processing;
            }
        }
        *enablePtr = GT_TRUE;
        return GT_OK;
    }
regular_processing:
    if (data > 0)
    {
        /* restoring the value that was decremented to avoid losses under full wire speed */
        data += 1;
    }
    if(data == 0)
    {
        *enablePtr = GT_FALSE;
    }
    else
    {
        speedCoef = coreClock * 10 / data;
        switch(speedCoef)
        {
            case 10:
                *portSpeedPtr = CPSS_PORT_SPEED_1000_E;
                break;
            case 100:
                *portSpeedPtr = CPSS_PORT_SPEED_10000_E;
                break;
            case 25:
                *portSpeedPtr = CPSS_PORT_SPEED_2500_E;
                break;
            case 50:
                *portSpeedPtr = CPSS_PORT_SPEED_5000_E;
                break;
            case 120:
                *portSpeedPtr = CPSS_PORT_SPEED_12000_E;
                break;
            case 200:
                *portSpeedPtr = CPSS_PORT_SPEED_20000_E;
                break;
            case 250:
                *portSpeedPtr = CPSS_PORT_SPEED_25000_E;
                break;
            case 400:
            case 403: /* For 525MHz */
                *portSpeedPtr = CPSS_PORT_SPEED_40000_E;
                break;
            case 525:
                *portSpeedPtr = CPSS_PORT_SPEED_50000_E;
                break;
            case 1050:
                *portSpeedPtr = CPSS_PORT_SPEED_100G_E;
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        *enablePtr = GT_TRUE;
    }
    return GT_OK;
}

/**
* @internal cpssDxChCutThroughMemoryRateLimitGet function
* @endinternal
*
* @brief   Get rate limiting of read operations from the memory
*         per target port in Cut Through mode.
*         To prevent congestion in egress pipe, buffer memory read operations
*         are rate limited according to the target port speed.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number including CPU port.
*
* @param[out] enablePtr                - pointer to rate limiting mode status.
*                                      - GT_TRUE - rate limiting is enabled.
*                                      - GT_FALSE - rate limiting is disabled.
* @param[out] portSpeedPtr             - pointer to port speed.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughMemoryRateLimitGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL *enablePtr,
    OUT CPSS_PORT_SPEED_ENT  *portSpeedPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughMemoryRateLimitGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr, portSpeedPtr));

    rc = internal_cpssDxChCutThroughMemoryRateLimitGet(devNum, portNum, enablePtr, portSpeedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr, portSpeedPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCutThroughBypassRouterAndPolicerEnableSet function
* @endinternal
*
* @brief   Enable / Disable bypassing the Router and Ingress Policer engines.
*
* @note   APPLICABLE DEVICES:      Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE -  bypassing of Router and Ingress Policer
*                                      engines is enabled.
*                                      - GT_FALSE - bypassing of Router and Ingress Policer
*                                      engines is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCutThroughBypassRouterAndPolicerEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
{
    GT_U32    regAddr;           /* register address */
    GT_U32    data;              /* reg sub field data */
    GT_U32    fieldOffset;       /* The start bit number in the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    data = (enable == GT_TRUE) ? 1 : 0;
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeGlobalConfig1;
        fieldOffset = 7;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            bridgeRegs.bridgeGlobalConfigRegArray[2];
        fieldOffset = 20;
    }

    /* Enable / Disable bypassing the Router and Ingress Policer engines.  */
    return prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 1, data);

}

/**
* @internal cpssDxChCutThroughBypassRouterAndPolicerEnableSet function
* @endinternal
*
* @brief   Enable / Disable bypassing the Router and Ingress Policer engines.
*
* @note   APPLICABLE DEVICES:      Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE -  bypassing of Router and Ingress Policer
*                                      engines is enabled.
*                                      - GT_FALSE - bypassing of Router and Ingress Policer
*                                      engines is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughBypassRouterAndPolicerEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughBypassRouterAndPolicerEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChCutThroughBypassRouterAndPolicerEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCutThroughBypassRouterAndPolicerEnableGet function
* @endinternal
*
* @brief   Get mode of bypassing the Router and Ingress Policer engines.
*
* @note   APPLICABLE DEVICES:      Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - pointer to bypassing the Router and Ingress
*                                      Policer engines status.
*                                      - GT_TRUE  -  bypassing of Router and Ingress Policer
*                                      engines is enabled.
*                                      - GT_FALSE -  bypassing of Router and Ingress Policer
*                                      engines is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCutThroughBypassRouterAndPolicerEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_U32    regAddr;     /* register address */
    GT_U32    data;        /* reg sub field data */
    GT_STATUS rc;          /* return status */
    GT_U32    fieldOffset; /* The start bit number in the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeGlobalConfig1;
        fieldOffset = 7;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            bridgeRegs.bridgeGlobalConfigRegArray[2];
        fieldOffset = 20;
    }

    /* Get mode of bypassing the Router and Ingress Policer engines. */
    rc =  prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, 1, &data);
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (data == 0) ? GT_FALSE : GT_TRUE;

    return GT_OK;

}

/**
* @internal cpssDxChCutThroughBypassRouterAndPolicerEnableGet function
* @endinternal
*
* @brief   Get mode of bypassing the Router and Ingress Policer engines.
*
* @note   APPLICABLE DEVICES:      Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - pointer to bypassing the Router and Ingress
*                                      Policer engines status.
*                                      - GT_TRUE  -  bypassing of Router and Ingress Policer
*                                      engines is enabled.
*                                      - GT_FALSE -  bypassing of Router and Ingress Policer
*                                      engines is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughBypassRouterAndPolicerEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughBypassRouterAndPolicerEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChCutThroughBypassRouterAndPolicerEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCutThroughBypassModeSet function
* @endinternal
*
* @brief   Set Bypass mode.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] bypassModePtr            - pointer to Bypass mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, Bypass Ingress PCL mode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCutThroughBypassModeSet
(
    IN GT_U8                             devNum,
    IN CPSS_DXCH_CUT_THROUGH_BYPASS_STC  *bypassModePtr
)
{
    GT_U32    regAddr;           /* register address */
    GT_U32    value;             /* register field value */
    GT_U32    startBit;          /* start bit of the field */
    GT_U32    i;                 /* iterator */
    GT_STATUS rc;
    PRV_CPSS_DXCH_MODULE_CONFIG_STC *moduleCfgPtr;/* pointer to the module
                                                configure of the PP's database*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_XCAT2_E | CPSS_LION_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(bypassModePtr);

    /* Configure the module configuration struct.   */
    moduleCfgPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg);

    if ((PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE) ||
        (bypassModePtr->bypassRouter == GT_TRUE))
    {
        /* Enable the router glue engine when IPCL is used for routing. */
        value = 0;
    }
    else
    {
        switch(moduleCfgPtr->ip.routingMode)
        {
            case CPSS_DXCH_POLICY_BASED_ROUTING_ONLY_E:
                value = 0;
                break;
            case CPSS_DXCH_TCAM_ROUTER_BASED_E:
                value = 1;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        value |= (BOOL2BIT_MAC(bypassModePtr->bypassRouter)) << 27;
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->IPvX.routerGlobalCtrl.routerGlobalCtrl1;
        rc = prvCpssHwPpWriteRegBitMask(devNum, regAddr, 0x08000000, value);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        value |= (BOOL2BIT_MAC(bypassModePtr->bypassRouter)) << 30;
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.routerAdditionalCtrlReg;
        rc = prvCpssHwPpWriteRegBitMask(devNum, regAddr, 0x40000001, value);
        if(rc != GT_OK)
        {
            return rc;
        }
    }


    for (i = 0; i <= CPSS_DXCH_POLICER_STAGE_EGRESS_E; i++)
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,i).policerCtrl1;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[i].policerControl1Reg;
        }

        switch((CPSS_DXCH_POLICER_STAGE_TYPE_ENT)i)
        {
            case CPSS_DXCH_POLICER_STAGE_INGRESS_0_E:
                value = BOOL2BIT_MAC(bypassModePtr->bypassIngressPolicerStage0);
                break;

            case CPSS_DXCH_POLICER_STAGE_INGRESS_1_E:
                value = BOOL2BIT_MAC(bypassModePtr->bypassIngressPolicerStage1);
                break;
            case CPSS_DXCH_POLICER_STAGE_EGRESS_E:
                value = BOOL2BIT_MAC(bypassModePtr->bypassEgressPolicer);
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 6, 1, value);
        if(rc != GT_OK)
        {
            return rc;
        }

    }

    value = BOOL2BIT_MAC(bypassModePtr->bypassEgressPcl);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* in Bobcat2 and above moved from HA to EPCL */
        regAddr = PRV_DXCH_REG1_UNIT_EPCL_MAC(devNum).egrPolicyGlobalConfig;
        startBit = 12;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltGlobalConfig;
        startBit = 29;
    }

    rc = prvCpssHwPpSetRegField(devNum, regAddr, startBit, 1, value);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(bypassModePtr->bypassIngressPcl)
    {
        case CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_NO_BYPASS_E:
            if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
            {
                value = 0;
            }
            else
            {
                /* This is the default value in Falcon */
                value = 2;
            }
            break;
        case CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_ONE_LOOKUP_E:
            if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
            {
                value = 1;
            }
            else
            {
                /* No lookups left in Falcon */
                value = 3;
            }
            break;
        case CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_TWO_LOOKUP_E:
            if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
            {
                value = 2;
            }
            else
            {
                /* Only one lookup in Falcon */
                value = 1;
            }
            break;
        case CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_FULL_BYPASS_E:
            value = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).ingrPolicyGlobalConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.control;
    }

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 15, 2, value);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        /* Bypass Ingress OAM */
        regAddr = PRV_DXCH_REG1_UNIT_OAM_MAC(devNum, 0).OAMGlobalCtrl1;
        value = BOOL2BIT_MAC(bypassModePtr->bypassIngressOam);
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 2, 1, value);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Bypass Egress OAM */
        regAddr = PRV_DXCH_REG1_UNIT_OAM_MAC(devNum, 1).OAMGlobalCtrl1;
        value = BOOL2BIT_MAC(bypassModePtr->bypassEgressOam);
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 2, 1, value);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Bypass MLL */
        regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLGlobalCtrl.
            MLLGlobalCtrl;
        value = BOOL2BIT_MAC(bypassModePtr->bypassMll);
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 20, 1, value);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    return rc;

}

/**
* @internal cpssDxChCutThroughBypassModeSet function
* @endinternal
*
* @brief   Set Bypass mode.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] bypassModePtr            - pointer to Bypass mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, Bypass Ingress PCL mode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughBypassModeSet
(
    IN GT_U8                             devNum,
    IN CPSS_DXCH_CUT_THROUGH_BYPASS_STC  *bypassModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughBypassModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, bypassModePtr));

    rc = internal_cpssDxChCutThroughBypassModeSet(devNum, bypassModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, bypassModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCutThroughBypassModeGet function
* @endinternal
*
* @brief   Get Bypass mode.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] bypassModePtr            - pointer to Bypass mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCutThroughBypassModeGet
(
    IN  GT_U8                              devNum,
    OUT CPSS_DXCH_CUT_THROUGH_BYPASS_STC   *bypassModePtr
)
{
    GT_U32    regAddr;           /* register address */
    GT_U32    value;             /* register field value */
    GT_U32    startBit;          /* start bit of the field */
    GT_U32    i;                 /* iterator */
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_XCAT2_E | CPSS_LION_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(bypassModePtr);

    cpssOsMemSet(bypassModePtr, 0, sizeof(CPSS_DXCH_CUT_THROUGH_BYPASS_STC));

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->IPvX.routerGlobalCtrl.routerGlobalCtrl1;
        startBit = 27;
    }
    else
    {
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.routerAdditionalCtrlReg;
        startBit = 30;
    }
    rc = prvCpssHwPpGetRegField(devNum, regAddr, startBit, 1, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    bypassModePtr->bypassRouter = BIT2BOOL_MAC(value);

    for (i = 0; i <= CPSS_DXCH_POLICER_STAGE_EGRESS_E; i++)
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,i).policerCtrl1;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[i].policerControl1Reg;
        }

        rc = prvCpssHwPpGetRegField(devNum, regAddr, 6, 1, &value);
        if(rc != GT_OK)
        {
            return rc;
        }

        switch((CPSS_DXCH_POLICER_STAGE_TYPE_ENT)i)
        {
            case CPSS_DXCH_POLICER_STAGE_INGRESS_0_E:
                bypassModePtr->bypassIngressPolicerStage0 = BIT2BOOL_MAC(value);
                break;

            case CPSS_DXCH_POLICER_STAGE_INGRESS_1_E:
                bypassModePtr->bypassIngressPolicerStage1 = BIT2BOOL_MAC(value);
                break;
            case CPSS_DXCH_POLICER_STAGE_EGRESS_E:
                bypassModePtr->bypassEgressPolicer = BIT2BOOL_MAC(value);
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* in Bobcat2 abd above moved from HA to EPCL */
        regAddr = PRV_DXCH_REG1_UNIT_EPCL_MAC(devNum).egrPolicyGlobalConfig;
        startBit = 12;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltGlobalConfig;
        startBit = 29;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, startBit, 1, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    bypassModePtr->bypassEgressPcl = BIT2BOOL_MAC(value);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).ingrPolicyGlobalConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.control;
    }
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 15, 2, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(value)
    {
        case 0:
            bypassModePtr->bypassIngressPcl = CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_NO_BYPASS_E;
            break;
        case 1:
            if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
            {
                bypassModePtr->bypassIngressPcl = CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_ONE_LOOKUP_E;
            }
            else
            {
                /* Only one lookup in Falcon */
                bypassModePtr->bypassIngressPcl = CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_TWO_LOOKUP_E;
            }
            break;
        case 2:
            if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
            {
                bypassModePtr->bypassIngressPcl = CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_TWO_LOOKUP_E;
            }
            else
            {
                /* This is the default value in Falcon */
                bypassModePtr->bypassIngressPcl = CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_NO_BYPASS_E;
            }
            break;
        case 3:
            bypassModePtr->bypassIngressPcl = CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_FULL_BYPASS_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    }

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        /* Bypass Ingress OAM */
        regAddr = PRV_DXCH_REG1_UNIT_OAM_MAC(devNum, 0).OAMGlobalCtrl1;
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 2, 1, &value);
        if(rc != GT_OK)
        {
            return rc;
        }
        bypassModePtr->bypassIngressOam = BIT2BOOL_MAC(value);

        /* Bypass Egress OAM */
        regAddr = PRV_DXCH_REG1_UNIT_OAM_MAC(devNum, 1).OAMGlobalCtrl1;
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 2, 1, &value);
        if(rc != GT_OK)
        {
            return rc;
        }
        bypassModePtr->bypassEgressOam = BIT2BOOL_MAC(value);

        /* Bypass MLL */
        regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLGlobalCtrl.
            MLLGlobalCtrl;
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 20, 1, &value);
        if(rc != GT_OK)
        {
            return rc;
        }
        bypassModePtr->bypassMll = BIT2BOOL_MAC(value);
    }
    return rc;
}

/**
* @internal cpssDxChCutThroughBypassModeGet function
* @endinternal
*
* @brief   Get Bypass mode.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] bypassModePtr            - pointer to Bypass mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughBypassModeGet
(
    IN  GT_U8                              devNum,
    OUT CPSS_DXCH_CUT_THROUGH_BYPASS_STC   *bypassModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughBypassModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, bypassModePtr));

    rc = internal_cpssDxChCutThroughBypassModeGet(devNum, bypassModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, bypassModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCutThroughPortGroupMaxBuffersLimitSet function
* @endinternal
*
* @brief   Set max buffers limit for Cut-Through packets.
*         Packets are handled as non-Cut-Through when number of allocated buffers more then limit.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] buffersLimit             - Buffers limit enable.
*                                      - GT_TRUE  - enable.
*                                      - GT_FALSE - disable.
* @param[in] buffersLimit             - Max buffers limit for Cut-Through traffic.
*                                      The resolution is one buffer.
*                                      (APPLICABLE RANGES: Lion2 0..0x3FFF)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong maxSharedBuffersLimit
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 device: applicable starting from revision B1
*       buffersLimit = (buffers number per port group) – (guard band).
*       where guard band is (MTU) (number of cut-through ports in port group)
*
*/
static GT_STATUS internal_cpssDxChCutThroughPortGroupMaxBuffersLimitSet
(
    IN GT_U8                            devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN GT_BOOL                          buffersLimitEnable,
    IN GT_U32                           buffersLimit
)
{
    GT_U32    regAddr;           /* register address */
    GT_U32    portGroupId;       /* the port group Id */
    GT_U32    value;             /* register value */
    GT_U32    mask;              /* register mask */
    GT_STATUS rc;                /* return code */
    GT_U32    ii;                /* loop iterator */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_FALCON_E);
    if(!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);
        if (PRV_CPSS_DXCH_LION2_B1_AND_ABOVE_CHECK_MAC(devNum) == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
        }
    }

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        if (buffersLimit > 0xFFFF)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        if(GT_FALSE == buffersLimitEnable)
        {
            buffersLimit = 0xFFFF;
        }

        for(ii = 0 ; ii <= 12 ; ii++)
        {
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                singleChannelDMAConfigs.SCDMAConfig0[ii];
            rc = prvCpssHwPpSetRegField(devNum, regAddr, 12, 16,
                buffersLimit);
            if(GT_OK != rc)
            {
                return rc;
            }
        }
    }
    else
    {
        if (buffersLimit > BIT_14)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        value = ((BOOL2BIT_MAC(buffersLimitEnable)) << 31) | buffersLimit;
        mask  = 0x80003FFF;

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.bufMngSharedBufConfigReg;

        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
        {
            rc = prvCpssHwPpPortGroupWriteRegBitMask(
                devNum, portGroupId, regAddr, mask, value);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
    }

    return GT_OK;
}

/**
* @internal cpssDxChCutThroughPortGroupMaxBuffersLimitSet function
* @endinternal
*
* @brief   Set max buffers limit for Cut-Through packets.
*         Packets are handled as non-Cut-Through when number of allocated buffers more then limit.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] buffersLimit             - Buffers limit enable.
*                                      - GT_TRUE  - enable.
*                                      - GT_FALSE - disable.
* @param[in] buffersLimit             - Max buffers limit for Cut-Through traffic.
*                                      The resolution is one buffer.
*                                      (APPLICABLE RANGES: Lion2 0..0x3FFF)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong maxSharedBuffersLimit
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 device: applicable starting from revision B1
*       buffersLimit = (buffers number per port group) – (guard band).
*       where guard band is (MTU) (number of cut-through ports in port group)
*
*/
GT_STATUS cpssDxChCutThroughPortGroupMaxBuffersLimitSet
(
    IN GT_U8                            devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN GT_BOOL                          buffersLimitEnable,
    IN GT_U32                           buffersLimit
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughPortGroupMaxBuffersLimitSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, buffersLimitEnable, buffersLimit));

    rc = internal_cpssDxChCutThroughPortGroupMaxBuffersLimitSet(devNum, portGroupsBmp, buffersLimitEnable, buffersLimit);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, buffersLimitEnable, buffersLimit));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCutThroughPortGroupMaxBuffersLimitGet function
* @endinternal
*
* @brief   Get max buffers limit for Cut-Through packets.
*         Packets are handled as non-Cut-Through when number of allocated buffers more then limit.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] buffersLimitEnablePtr    - (pointer to)Buffers limit enable.
*                                      - GT_TRUE  - enable.
*                                      - GT_FALSE - disable.
* @param[out] buffersLimitPtr          - (pointer to)Max buffers limit for Cut-Through traffic.
*                                      The resolution is one buffer.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on Null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 device: applicable starting from revision B1
*       buffersLimit = (buffers number per port group) – (guard band).
*       where guard band is (MTU) (number of cut-through ports in port group)
*
*/
static GT_STATUS internal_cpssDxChCutThroughPortGroupMaxBuffersLimitGet
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    OUT GT_BOOL                         *buffersLimitEnablePtr,
    OUT GT_U32                          *buffersLimitPtr
)
{
    GT_U32    regAddr;           /* register address */
    GT_U32    portGroupId = 0;   /* the port group Id */
    GT_U32    value;             /* register value */
    GT_STATUS rc;                /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_FALCON_E);

    if(!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);
        if (PRV_CPSS_DXCH_LION2_B1_AND_ABOVE_CHECK_MAC(devNum) == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE,
                LOG_ERROR_NO_MSG);
        }
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
            devNum, portGroupsBmp, portGroupId);
    }

    CPSS_NULL_PTR_CHECK_MAC(buffersLimitEnablePtr);
    CPSS_NULL_PTR_CHECK_MAC(buffersLimitPtr);

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
            singleChannelDMAConfigs.SCDMAConfig0[0];
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 12, 16,
            buffersLimitPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if(0xFFFF == *buffersLimitPtr)
        {
            *buffersLimitEnablePtr = GT_FALSE;
        }
        else
        {
            *buffersLimitEnablePtr = GT_TRUE;
        }

    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.bufMngSharedBufConfigReg;

        rc = prvCpssHwPpPortGroupReadRegister(
            devNum, portGroupId, regAddr, &value);
        if (rc != GT_OK)
        {
            return rc;
        }

        *buffersLimitEnablePtr  = BIT2BOOL_MAC((value >> 31));
        *buffersLimitPtr        = (value & 0x3FFF);
    }
    return GT_OK;
}

/**
* @internal cpssDxChCutThroughPortGroupMaxBuffersLimitGet function
* @endinternal
*
* @brief   Get max buffers limit for Cut-Through packets.
*         Packets are handled as non-Cut-Through when number of allocated buffers more then limit.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] buffersLimitEnablePtr    - (pointer to)Buffers limit enable.
*                                      - GT_TRUE  - enable.
*                                      - GT_FALSE - disable.
* @param[out] buffersLimitPtr          - (pointer to)Max buffers limit for Cut-Through traffic.
*                                      The resolution is one buffer.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on Null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 device: applicable starting from revision B1
*       buffersLimit = (buffers number per port group) – (guard band).
*       where guard band is (MTU) (number of cut-through ports in port group)
*
*/
GT_STATUS cpssDxChCutThroughPortGroupMaxBuffersLimitGet
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    OUT GT_BOOL                         *buffersLimitEnablePtr,
    OUT GT_U32                          *buffersLimitPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughPortGroupMaxBuffersLimitGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, buffersLimitEnablePtr, buffersLimitPtr));

    rc = internal_cpssDxChCutThroughPortGroupMaxBuffersLimitGet(devNum, portGroupsBmp, buffersLimitEnablePtr, buffersLimitPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, buffersLimitEnablePtr, buffersLimitPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChCutThroughBuffresUsingModeSet function
* @endinternal
*
* @brief   Set Buffers using mode.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] cutThroughEnable         – GT_TRUE – at least one port will be used in CutThrough mode.
*                                      GT_FALSE – all ports used in Store and Forward mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCutThroughBuffresUsingModeSet
(
    IN GT_U8                            devNum,
    IN GT_BOOL                          cutThroughEnable
)
{
    GT_U32    regAddr;                 /* register address           */
    GT_STATUS rc;                      /* return code                */
    GT_U32    unbalancedGroupStopMode; /* Unbalanced Group Stop Mode */
    GT_U32    groupStopLowThreshold;   /* group Stop Low Thres hold  */
    GT_U32    almostFullValue;         /* almost Full Value          */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        if (cutThroughEnable == GT_FALSE)
        {
            unbalancedGroupStopMode = 0; /*immediate_stop*/
            groupStopLowThreshold   = 0x222;
            almostFullValue         = 0;
        }
        else
        {
            unbalancedGroupStopMode = 1; /*stop_at_sop*/
            groupStopLowThreshold   = 0x3C;
            almostFullValue         = 0;
        }
        /* all rxDMA instances will be configured by one prvCpssHwPpSetRegField call */
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
            globalRxDMAConfigs.globalConfigCutThrough.CTGeneralConfiguration;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 16, 1, unbalancedGroupStopMode);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->BM.BMGlobalConfigs.BMGlobalConfig3;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 16, groupStopLowThreshold);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TXQ.bmx.almostFullDebugRegister;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 14, almostFullValue);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        if (cutThroughEnable == GT_FALSE)
        {
            groupStopLowThreshold   = 115;
        }
        else
        {
            groupStopLowThreshold   = 1000;
        }
        /* set low threshold */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->BM.BMGlobalConfigs.BMGlobalConfig3;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 16, groupStopLowThreshold);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        /* set high threshold by the same value */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->BM.BMGlobalConfigs.BMGlobalConfig3;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 16, 16, groupStopLowThreshold);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        /* configuration relevant to BOBCAT3 and ALDRIN2 only */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal internal_cpssDxChCutThroughEarlyProcessingModeSet function
* @endinternal
*
* @brief   Set Early Processing mode of Cut Through packets.
*          Set Reduced Latency Store and Forward mode or Pure Cut Through mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                      - device number
* @param[in] mode                        - early processing mode of cut through packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChCutThroughEarlyProcessingModeSet
(
    IN GT_U8                                      devNum,
    IN CPSS_DXCH_EARLY_PROCESSING_MODE_ENT        mode
)
{
    GT_U32    regAddr;    /* register address */
    GT_U32    hwValue;    /* HW value         */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
         CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    switch (mode)
    {
        case CPSS_DXCH_EARLY_PROCESSING_CUT_THROUGH_E:
            hwValue = 0;
            break;
        case CPSS_DXCH_EARLY_PROCESSING_STORE_AND_FORWARD_REDUCED_LATENCY_E:
            hwValue = 1;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_MAC(devNum).
        configs.cutThrough.CTGeneralConfig;
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, hwValue);
}

/**
* @internal cpssDxChCutThroughEarlyProcessingModeSet function
* @endinternal
*
* @brief   Set Early Processing mode of Cut Through packets.
*          Set Reduced Latency Store and Forward mode or Pure Cut Through mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                      - device number
* @param[in] mode                        - early processing mode of cut through packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChCutThroughEarlyProcessingModeSet
(
    IN GT_U8                                      devNum,
    IN CPSS_DXCH_EARLY_PROCESSING_MODE_ENT        mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughEarlyProcessingModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, devNum, mode));

    rc = internal_cpssDxChCutThroughEarlyProcessingModeSet(
        devNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCutThroughEarlyProcessingModeGet function
* @endinternal
*
* @brief   Get Early Processing mode of Cut Through packets.
*          Get Reduced Latency Store and Forward mode or Pure Cut Through mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                      - device number
* @param[out] modePtr                    - (pointer to) early processing mode of cut through packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on Null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChCutThroughEarlyProcessingModeGet
(
    IN  GT_U8                                      devNum,
    OUT CPSS_DXCH_EARLY_PROCESSING_MODE_ENT        *modePtr
)
{
    GT_STATUS rc;         /* return code      */
    GT_U32    regAddr;    /* register address */
    GT_U32    hwValue;    /* HW value         */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
         CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_MAC(devNum).
        configs.cutThrough.CTGeneralConfig;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 1, &hwValue);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    *modePtr = (hwValue == 0)
        ? CPSS_DXCH_EARLY_PROCESSING_CUT_THROUGH_E
        : CPSS_DXCH_EARLY_PROCESSING_STORE_AND_FORWARD_REDUCED_LATENCY_E;
    return GT_OK;
}

/**
* @internal cpssDxChCutThroughEarlyProcessingModeGet function
* @endinternal
*
* @brief   Get Early Processing mode of Cut Through packets.
*          Get Reduced Latency Store and Forward mode or Pure Cut Through mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                      - device number
* @param[out] modePtr                    - (pointer to) early processing mode of cut through packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on Null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChCutThroughEarlyProcessingModeGet
(
    IN  GT_U8                                      devNum,
    OUT CPSS_DXCH_EARLY_PROCESSING_MODE_ENT        *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughEarlyProcessingModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, devNum, modePtr));

    rc = internal_cpssDxChCutThroughEarlyProcessingModeGet(
        devNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCutThroughUdeCfgSet function
* @endinternal
*
* @brief   Set Cut Through configuration of User Defined Ethernet packets.
*          Related to 4 Generic ethernet types configured by
*          cpssDxChPortPipGlobalEtherTypeProtocolSet.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                      - device number
* @param[in] udeIndex                    - UDE index (APPLICABLE RANGES: 0..3)
* @param[in] udeCutThroughEnable         - GT_TRUE/GT_FALSE enable/disable Cut Through mode for UDE packets.
* @param[in] udeByteCount                - default CT Byte Count for UDE packets.
*                                          (APPLICABLE RANGES: 0..0x3FFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on out of range udeByteCount
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChCutThroughUdeCfgSet
(
    IN GT_U8       devNum,
    IN GT_U32      udeIndex,
    IN GT_BOOL     udeCutThroughEnable,
    IN GT_U32      udeByteCount
)
{
    GT_U32    regAddr;    /* register address */
    GT_U32    mask;       /* register mask */
    GT_U32    data;       /* register data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
         CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_DATA_CHECK_MAX_MAC(udeIndex, 4);
    CPSS_DATA_CHECK_MAX_MAC(udeByteCount, 0x4000);

    mask = 0;
    data = 0;

    mask |= (1 << 16);
    data |= (BOOL2BIT_MAC(udeCutThroughEnable) << 16);

    mask |= 0x3FFF;
    data |= (udeByteCount & 0x3FFF);

    regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_MAC(devNum).
        configs.cutThrough.CTGenericConfigs[udeIndex];

    return prvCpssHwPpWriteRegBitMask(devNum, regAddr, mask, data);
}

/**
* @internal cpssDxChCutThroughUdeCfgSet function
* @endinternal
*
* @brief   Set Cut Through configuration of User Defined Ethernet packets.
*          Related to 4 Generic ethernet types configured by
*          cpssDxChPortPipGlobalEtherTypeProtocolSet.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                      - device number
* @param[in] udeIndex                    - UDE index (APPLICABLE RANGES: 0..3)
* @param[in] udeCutThroughEnable         - GT_TRUE/GT_FALSE enable/disable Cut Through mode for UDE packets.
* @param[in] udeByteCount                - default CT Byte Count for UDE packets.
*                                          (APPLICABLE RANGES: 0..0x3FFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on out of range udeByteCount
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChCutThroughUdeCfgSet
(
    IN GT_U8       devNum,
    IN GT_U32      udeIndex,
    IN GT_BOOL     udeCutThroughEnable,
    IN GT_U32      udeByteCount
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughUdeCfgSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, devNum, udeIndex, udeCutThroughEnable, udeByteCount));

    rc = internal_cpssDxChCutThroughUdeCfgSet(
        devNum, udeIndex, udeCutThroughEnable, udeByteCount);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, udeIndex, udeCutThroughEnable, udeByteCount));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCutThroughUdeCfgGet function
* @endinternal
*
* @brief   Get Cut Through configuration of User Defined Ethernet packets.
*          Related to 4 Generic ethernet types configured by
*          cpssDxChPortPipGlobalEtherTypeProtocolSet.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in]  devNum                      - device number
* @param[in]  udeIndex                    - UDE index (APPLICABLE RANGES: 0..3)
* @param[out] udeCutThroughEnablePtr      - pointer to GT_TRUE/GT_FALSE
*                                           enable/disable Cut Through mode for UDE packets.
* @param[out] udeByteCountPtr             - pointer to default CT Byte Count for UDE packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCutThroughUdeCfgGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      udeIndex,
    OUT GT_BOOL     *udeCutThroughEnablePtr,
    OUT GT_U32      *udeByteCountPtr
)
{
    GT_STATUS rc;         /* return code */
    GT_U32    regAddr;    /* register address */
    GT_U32    data;       /* register data */
    GT_U32    value;      /* register field value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
         CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);



    CPSS_DATA_CHECK_MAX_MAC(udeIndex, 4);
    CPSS_NULL_PTR_CHECK_MAC(udeCutThroughEnablePtr);
    CPSS_NULL_PTR_CHECK_MAC(udeByteCountPtr);

    regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_MAC(devNum).
        configs.cutThrough.CTGenericConfigs[udeIndex];

    rc = prvCpssHwPpReadRegister(devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    value = ((data >> 16) & 1);
    *udeCutThroughEnablePtr = BIT2BOOL_MAC(value);

    value = (data & 0x3FFF);
    *udeByteCountPtr = value;

    return GT_OK;
}

/**
* @internal cpssDxChCutThroughUdeCfgGet function
* @endinternal
*
* @brief   Get Cut Through configuration of User Defined Ethernet packets.
*          Related to 4 Generic ethernet types configured by
*          cpssDxChPortPipGlobalEtherTypeProtocolSet.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in]  devNum                      - device number
* @param[in]  udeIndex                    - UDE index (APPLICABLE RANGES: 0..3)
* @param[out] udeCutThroughEnablePtr      - pointer to GT_TRUE/GT_FALSE
*                                           enable/disable Cut Through mode for UDE packets.
* @param[out] udeByteCountPtr    - pointer to default CT Byte Count for UDE packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughUdeCfgGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      udeIndex,
    OUT GT_BOOL     *udeCutThroughEnablePtr,
    OUT GT_U32      *udeByteCountPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughUdeCfgGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, devNum, udeIndex, udeCutThroughEnablePtr, udeByteCountPtr));

    rc = internal_cpssDxChCutThroughUdeCfgGet(
        devNum, udeIndex, udeCutThroughEnablePtr, udeByteCountPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, udeIndex, udeCutThroughEnablePtr, udeByteCountPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCutThroughPassengerParsingModeSet function
* @endinternal
*
* @brief   Set Cut Through passenger packet header parsing mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - device number
* @param[in] parseMode              - passenger parse mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChCutThroughPassengerParsingModeSet
(
    IN GT_U8                                              devNum,
    IN CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT   parsingMode
)
{
    GT_U32    regAddr;    /* register address */
    GT_U32    data;       /* register data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
         CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    switch (parsingMode)
    {
        case CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_ALL_DATA_E:
            data = 0;
            break;
        case CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_BYTE_COUNT_ONLY_E:
            data = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TTI.ttiCutThroughConfiguration;

    return prvCpssHwPpSetRegField(devNum, regAddr, 3, 1, data);
}

/**
* @internal cpssDxChCutThroughPassengerParsingModeSet function
* @endinternal
*
* @brief   Set Cut Through passenger packet header parsing mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - device number
* @param[in] parseMode              - passenger parse mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChCutThroughPassengerParsingModeSet
(
    IN GT_U8                                              devNum,
    IN CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT   parsingMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughPassengerParsingModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, parsingMode));

    rc = internal_cpssDxChCutThroughPassengerParsingModeSet(devNum, parsingMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, parsingMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCutThroughPassengerParsingModeGet function
* @endinternal
*
* @brief   Get Cut Through passenger packet header parsing mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in]  devNum                 - device number
* @param[out] parseModePtr           - pointer to passenger parse mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChCutThroughPassengerParsingModeGet
(
    IN  GT_U8                                              devNum,
    OUT CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT   *parsingModePtr
)
{
    GT_STATUS rc;         /* return code */
    GT_U32    regAddr;    /* register address */
    GT_U32    data;       /* register data */
    GT_U32    value;      /* register field value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
         CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(parsingModePtr);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TTI.ttiCutThroughConfiguration;

    rc = prvCpssHwPpReadRegister(devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    value = ((data >> 3) & 1);
    *parsingModePtr =
        ((value == 0)
            ? CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_ALL_DATA_E
            : CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_BYTE_COUNT_ONLY_E);
    return GT_OK;
}


/**
* @internal cpssDxChCutThroughPassengerParsingModeGet function
* @endinternal
*
* @brief   Get Cut Through passenger packet header parsing mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in]  devNum                 - device number
* @param[out] parseModePtr           - pointer to passenger parse mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChCutThroughPassengerParsingModeGet
(
    IN  GT_U8                                              devNum,
    OUT CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT   *parsingModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughPassengerParsingModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, parsingModePtr));

    rc = internal_cpssDxChCutThroughPassengerParsingModeGet(devNum, parsingModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, parsingModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChCutThroughByteCountExtractFailsCounterGet function
* @endinternal
*
* @brief   Get counter of fails extracting CT Packet Byte Count by packet header.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in]  devNum    - device number
* @param[out] countPtr  - pointer to count of packet Byte Count extracting fails.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCutThroughByteCountExtractFailsCounterGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *countPtr
)
{
    GT_STATUS rc;         /* return code */
    GT_U32    regAddr;    /* register address */
    GT_U32    data;       /* register data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
         CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(countPtr);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TTI.cutThroughByteCountExtractFailCntr;

    rc = prvCpssHwPpReadRegister(devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    *countPtr = (data & 0xFFFF);
    return GT_OK;
}

/**
* @internal cpssDxChCutThroughByteCountExtractFailsCounterGet function
* @endinternal
*
* @brief   Get counter of fails extracting CT Packet Byte Count by packet header.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in]  devNum    - device number
* @param[out] countPtr  - pointer to count of packet Byte Count extracting fails.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughByteCountExtractFailsCounterGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *countPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughByteCountExtractFailsCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, countPtr));

    rc = internal_cpssDxChCutThroughByteCountExtractFailsCounterGet(
        devNum, countPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, countPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet function
* @endinternal
*
* @brief   Set configuration Packet Header Integrity Check.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum  - device number
* @param[in] cfgPtr  - pointer to Packet Header Integrity Check.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet
(
    IN GT_U8                                                     devNum,
    IN CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC  *cfgPtr
)
{
    GT_STATUS                                 rc;         /* return code */
    GT_U32                                    regAddr;    /* register address */
    GT_U32                                    mask;       /* register mask */
    GT_U32                                    data;       /* register data */
    GT_U32                                    hwPktCmd;   /* HW packet command */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT    hwCpuCode;  /* HW CPU Code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
         CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(cfgPtr);
    CPSS_DATA_CHECK_MAX_MAC(cfgPtr->minByteCountBoundary, 0x4000);
    CPSS_DATA_CHECK_MAX_MAC(cfgPtr->maxByteCountBoundary, 0x4000);

    switch (cfgPtr->hdrIntergrityExceptionPktCmd)
    {
        case CPSS_PACKET_CMD_FORWARD_E:
            hwPktCmd = 0;
            break;
        case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
            hwPktCmd = 1;
            break;
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            hwPktCmd = 2;
            break;
        case CPSS_PACKET_CMD_DROP_HARD_E:
            hwPktCmd = 3;
            break;
        case CPSS_PACKET_CMD_DROP_SOFT_E:
            hwPktCmd = 4;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChNetIfCpuToDsaCode(
        cfgPtr->hdrIntergrityExceptionCpuCode, &hwCpuCode);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    mask = 0;
    data = 0;

    mask |= 0x3FFF;
    data |= (cfgPtr->minByteCountBoundary & 0x3FFF);

    mask |= (0x3FFF << 14);
    data |= ((cfgPtr->maxByteCountBoundary & 0x3FFF) << 14);

    mask |= (1 << 28);
    data |= (BOOL2BIT_MAC(cfgPtr->enableByteCountBoundariesCheck) << 28);

    mask |= (1 << 29);
    data |= (BOOL2BIT_MAC(cfgPtr->enableIpv4HdrCheckByChecksum) << 29);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TTI.cutThroughHdrIntegrityExceptionCfg1;

    rc = prvCpssHwPpWriteRegBitMask(devNum, regAddr, mask, data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    mask = 0;
    data = 0;

    mask |= 0xFF;
    data |= ((GT_U32)hwCpuCode & 0xFF);

    mask |= (0x7 << 8);
    data |= ((hwPktCmd & 0x7) << 8);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TTI.cutThroughHdrIntegrityExceptionCfg2;

    rc = prvCpssHwPpWriteRegBitMask(devNum, regAddr, mask, data);
    return rc;
}

/**
* @internal cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet function
* @endinternal
*
* @brief   Set configuration Packet Header Integrity Check.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum  - device number
* @param[in] cfgPtr  - pointer to Packet Header Integrity Check.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet
(
    IN GT_U8                                                     devNum,
    IN CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC  *cfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cfgPtr));

    rc = internal_cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet(
        devNum, cfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCutThroughPacketHdrIntegrityCheckCfgGet function
* @endinternal
*
* @brief   Get configuration of Packet Header Integrity Check.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in]  devNum  - device number
* @param[out] cfgPtr  - pointer to Packet Header Integrity Check.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCutThroughPacketHdrIntegrityCheckCfgGet
(
    IN  GT_U8                                                     devNum,
    OUT CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC  *cfgPtr
)
{
    GT_STATUS                                 rc;         /* return code */
    GT_U32                                    regAddr;    /* register address */
    GT_U32                                    data;       /* register data */
    GT_U32                                    value;      /* field value */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT    hwCpuCode;  /* HW CPU Code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
         CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(cfgPtr);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TTI.cutThroughHdrIntegrityExceptionCfg1;
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    value = (data & 0x3FFF);
    cfgPtr->minByteCountBoundary = value;

    value = ((data >> 14) & 0x3FFF);
    cfgPtr->maxByteCountBoundary = value;

    value = ((data >> 28) & 1);
    cfgPtr->enableByteCountBoundariesCheck = BIT2BOOL_MAC(value);

    value = ((data >> 29) & 1);
    cfgPtr->enableIpv4HdrCheckByChecksum = BIT2BOOL_MAC(value);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TTI.cutThroughHdrIntegrityExceptionCfg2;
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    value = (data & 0xFF);
    hwCpuCode = (PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT)value;
    rc = prvCpssDxChNetIfDsaToCpuCode(hwCpuCode, &(cfgPtr->hdrIntergrityExceptionCpuCode));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    value = ((data >> 8) & 0x7);
    switch (value)
    {
        case 0:
            cfgPtr->hdrIntergrityExceptionPktCmd = CPSS_PACKET_CMD_FORWARD_E;
            break;
        case 1:
            cfgPtr->hdrIntergrityExceptionPktCmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
            break;
        case 2:
            cfgPtr->hdrIntergrityExceptionPktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            break;
        case 3:
            cfgPtr->hdrIntergrityExceptionPktCmd = CPSS_PACKET_CMD_DROP_HARD_E;
            break;
        case 4:
            cfgPtr->hdrIntergrityExceptionPktCmd = CPSS_PACKET_CMD_DROP_SOFT_E;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChCutThroughPacketHdrIntegrityCheckCfgGet function
* @endinternal
*
* @brief   Get configuration of Packet Header Integrity Check.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in]  devNum  - device number
* @param[out] cfgPtr  - pointer to Packet Header Integrity Check.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughPacketHdrIntegrityCheckCfgGet
(
    IN  GT_U8                                                     devNum,
    OUT CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC  *cfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughPacketHdrIntegrityCheckCfgGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cfgPtr));

    rc = internal_cpssDxChCutThroughPacketHdrIntegrityCheckCfgGet(
        devNum, cfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCutThroughPortByteCountUpdateSet function
* @endinternal
*
* @brief   Set configuration for updating byte count per port.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                    - device number
* @param[in] portNum                   - physical port number including CPU port.
* @param[in] toSubtractOrToAdd         - GT_TRUE - to subtract, GT_FALSE to add.
* @param[in] subtractedOrAddedValue    - value to subtract or to add to packet length
*                                        (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_OUT_OF_RANGE          - on out of range subtractedOrAddedValue
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChCutThroughPortByteCountUpdateSet
(
    IN GT_U8                 devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL               toSubtractOrToAdd,
    IN GT_U32                subtractedOrAddedValue
)
{
    GT_STATUS rc;         /* return code */
    GT_U32    regAddr;    /* register address */
    GT_U32    data;       /* register data */
    GT_U32    mask;       /* register mask */
    GT_U32    portGroupId;/* the port group Id - support multi port group device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
         CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_DATA_CHECK_MAX_MAC(subtractedOrAddedValue, 8);

    mask = 0;
    data = 0;

    mask |= (1 << 19);
    data |= (BOOL2BIT_MAC(toSubtractOrToAdd) << 19);

    mask |= (0x7 << 16);
    data |= ((subtractedOrAddedValue & 0x7) << 16);

    rc = prvCpssDxChCscdMyPhysicalPortAttributesRegAddressGet(
        devNum, portNum, &regAddr, &portGroupId);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return prvCpssHwPpPortGroupWriteRegBitMask(devNum, portGroupId, regAddr, mask, data);
}

/**
* @internal cpssDxChCutThroughPortByteCountUpdateSet function
* @endinternal
*
* @brief   Set configuration for updating byte count per port.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                    - device number
* @param[in] portNum                   - physical port number including CPU port.
* @param[in] toSubtractOrToAdd         - GT_TRUE - to subtract, GT_FALSE to add.
* @param[in] subtractedOrAddedValue    - value to subtract or to add to packet length
*                                        (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_OUT_OF_RANGE          - on out of range subtractedOrAddedValue
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChCutThroughPortByteCountUpdateSet
(
    IN GT_U8                 devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL               toSubtractOrToAdd,
    IN GT_U32                subtractedOrAddedValue
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughPortByteCountUpdateSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, devNum, portNum, toSubtractOrToAdd, subtractedOrAddedValue));

    rc = internal_cpssDxChCutThroughPortByteCountUpdateSet(
        devNum, portNum, toSubtractOrToAdd, subtractedOrAddedValue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, portNum, toSubtractOrToAdd, subtractedOrAddedValue));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCutThroughPortByteCountUpdateGet function
* @endinternal
*
* @brief   Get configuration for updating byte count at per port.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in]  devNum                    - device number
* @param[in]  portNum                   - physical port number including CPU port.
* @param[out] toSubtractOrToAddPtr      - pointer to GT_TRUE - to subtract, GT_FALSE to add.
* @param[out] subtractedOrAddedValuePtr - pointer to value to subtract or to add to packet length
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCutThroughPortByteCountUpdateGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL               *toSubtractOrToAddPtr,
    OUT GT_U32                *subtractedOrAddedValuePtr
)
{
    GT_STATUS rc;         /* return code */
    GT_U32    regAddr;    /* register address */
    GT_U32    data;       /* register data */
    GT_U32    value;      /* field value */
    GT_U32    portGroupId;/* the port group Id - support multi port group device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
         CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(toSubtractOrToAddPtr);
    CPSS_NULL_PTR_CHECK_MAC(subtractedOrAddedValuePtr);

    rc = prvCpssDxChCscdMyPhysicalPortAttributesRegAddressGet(
        devNum, portNum, &regAddr, &portGroupId);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId, regAddr, &data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    value = ((data >> 19) & 1);
    *toSubtractOrToAddPtr = BIT2BOOL_MAC(value);

    value = ((data >> 16) & 0x7);
    *subtractedOrAddedValuePtr = value;

    return GT_OK;
}

/**
* @internal cpssDxChCutThroughPortByteCountUpdateGet function
* @endinternal
*
* @brief   Get configuration for updating byte count at per port.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in]  devNum                    - device number
* @param[in]  portNum                   - physical port number including CPU port.
* @param[out] toSubtractOrToAddPtr      - pointer to GT_TRUE - to subtract, GT_FALSE to add.
* @param[out] subtractedOrAddedValuePtr - pointer to value to subtract or to add to packet length
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCutThroughPortByteCountUpdateGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL               *toSubtractOrToAddPtr,
    OUT GT_U32                *subtractedOrAddedValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCutThroughPortByteCountUpdateGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, devNum, portNum, toSubtractOrToAddPtr, subtractedOrAddedValuePtr));

    rc = internal_cpssDxChCutThroughPortByteCountUpdateGet(
        devNum, portNum, toSubtractOrToAddPtr, subtractedOrAddedValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, portNum, toSubtractOrToAddPtr, subtractedOrAddedValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChCutThroughRxErrorConfigSet function
* @endinternal
*
* @brief   Set Error Configuration of RX when obtained actual packet size different from calculated.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in]  devNum                      - device number
* @param[in]  paddedPacketEofWithError    - When a CT packet is padded it‘s last
*                                           write transaction to the PB should be marked
*                                           GT_FALSE - as EOP or GT_TRUE - as EOP with Error,
* @param[in]  truncatedPacketEofWithError - When a CT packet is truncated it‘s last
*                                           write transaction to the PB should be marked
*                                           GT_FALSE - as EOP or GT_TRUE - as EOP with Error,
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvCpssDxChCutThroughRxErrorConfigSet
(
    IN  GT_U8                 devNum,
    IN  GT_BOOL               paddedPacketEofWithError,
    IN  GT_BOOL               truncatedPacketEofWithError
)
{
    GT_U32    regAddr;    /* register address */
    GT_U32    data;       /* register data */

    data =
        ((BOOL2BIT_MAC(paddedPacketEofWithError) << 1)
         | BOOL2BIT_MAC(truncatedPacketEofWithError));

    regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_MAC(devNum).
        configs.cutThrough.CTErrorConfig;
    return prvCpssHwPpSetRegField(
        devNum, regAddr, 0, 2, data);
}

/**
* @internal prvCpssDxChCutThroughPortMinCtByteCountDefaultValueSet function
* @endinternal
*
* @brief   Set minimal Cut Through packet byte count value for all RX_DMA channels.
*          Not depends on mapping physical ports to MAC/RX/TX
*
* @note   APPLICABLE DEVICES:     Falcon.
* @note   NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X  Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] minCtByteCount           - minimal Cut Through packet byte count.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_OUT_OF_RANGE          - on wrong minCtByteCount
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChCutThroughPortMinCtByteCountDefaultValueSet
(
    IN GT_U8                 devNum,
    IN GT_U32                minCtByteCount
)
{
    GT_STATUS rc;                /* return code */
    GT_U32    regAddr;           /* register address */
    GT_U32    data;              /* reg sub field data */
    GT_U32    mask;              /* Mask for selecting the written bits */
    GT_U32    globalMacNum;      /* global port number */
    GT_U32    portGroupId;       /* the port group Id - support multi port group device */
    GT_U32    localPort;         /* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
         CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
         CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    if (minCtByteCount >= BIT_14)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    data = (minCtByteCount << 2);
    mask = (0x3FFF << 2);

    for (globalMacNum = 0; (globalMacNum < PRV_CPSS_MAX_DMA_NUM_CNS); globalMacNum++)
    {
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi port groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, globalMacNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, globalMacNum);

        regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_MAC(devNum).
            configs.cutThrough.channelCTConfig[localPort];
        if (regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED) continue;

        /* Enable/Disable Cut Through forwarding */
        rc = prvCpssHwPpPortGroupWriteRegBitMask(
            devNum, portGroupId, regAddr, mask, data);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    return GT_OK;
}



