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
* @file cpssDxChTmGluePfc.c
*
* @brief Traffic Manager Glue - PFC API implementation.
*
* @version   5
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/tmGlue/private/prvCpssDxChTmGlueLog.h>

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGluePfc.h>
#include <cpss/dxCh/dxChxGen/tmGlue/private/prvCpssDxChTmGluePfc.h>
#include <cpss/extServices/cpssExtServices.h>

/**
* @internal internal_cpssDxChTmGluePfcTmTcPort2CNodeSet function
* @endinternal
*
* @brief   Sets C node value for given pfc port and traffic class.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*                                      pfcPortNum  - pfc port number. (APPLICABLE RANGES: 0..63)
* @param[in] tc                       - traffic class (APPLICABLE RANGES: 0..7).
* @param[in] cNodeValue               - C node value (APPLICABLE RANGES: 0...511).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number,
*                                       port or traffic class.
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To enable PFC response by the TM, the Timer values of the received PFC frames must be
*       redirected to the TM unit (using cpssDxChTmGluePfcResponseModeSet).
*       C-nodes must be allocated so that a C-node serves a specific (port, priority).
*       For example, to support 8 PFC priorities on a specific port, 8 C-Nodes must be associated with that port.
*       Queues that serve a specific port and a PFC priority must be associated with the respective C-node.
*       64 ingress ports (pfcPort) are available to redirect the Timer values of the received
*       PFC frames to the TM unit, this API is used along with cpssDxChTmGluePfcPortMappingSet
*       to map physical port to its pfcPort.
*       Specific CNode must have one instance in TcPortToCnode Table,
*       CNodes are switched by the API to have such behaviour.
*
*/
static GT_STATUS internal_cpssDxChTmGluePfcTmTcPort2CNodeSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   tc,
    IN GT_U32                   cNodeValue
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TM_GLUE_PFC_CNODE_INFO_STC oldCnode;
    PRV_CPSS_DXCH_TM_GLUE_PFC_CNODE_INFO_STC newCnode;

    GT_U32 value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E  | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TM_GLUE_PFC_PHY_PORT_NUM_CHECK_MAC(portNum);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);

    if (cNodeValue >= BIT_9)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }


    if (prvPfcTmGlueCnodeMappingOnce == GT_TRUE)
    {
        /* look for cNode which exist in [portNum][tc] */
        rc = prvCpssDxChTmGluePfcTmDbGetCNodeByTcPort(devNum, portNum, tc, &value);
        if (rc != GT_OK)
        {
            return rc;
        }
        newCnode.port = portNum;
        newCnode.tc = tc;
        newCnode.cNode = (GT_U16)cNodeValue;


        /* look for cNodeValue [port][tc] location */
        rc = prvCpssDxChTmGluePfcTmDbGetTcPortByCNode(devNum, cNodeValue, &oldCnode.port, &oldCnode.tc);
        if (rc != GT_OK)
        {
            return rc;
        }
        oldCnode.cNode = (GT_U16)value;


        /*   move cNode from [portNum][tc] to oldCnode [port][tc]  */
        rc =  prvCpssDxChWriteTableEntryField(devNum,
                     CPSS_DXCH_SIP5_TABLE_TM_FCU_TC_PORT_TO_CNODE_PORT_MAPPING_E,
                     oldCnode.port,
                     PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                     oldCnode.tc * 9, 9, value);

        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Set TC Port 2 CNode Entry - TC Port 2 CNode */
    rc =  prvCpssDxChWriteTableEntryField(devNum,
                 CPSS_DXCH_SIP5_TABLE_TM_FCU_TC_PORT_TO_CNODE_PORT_MAPPING_E,
                 portNum,
                 PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                 tc * 9, 9, cNodeValue);

    if (rc != GT_OK)
    {
        return rc;
    }

    /* update Port2CnodeShadow DB */
    if (prvPfcTmGlueCnodeMappingOnce == GT_TRUE)
    {
        /* update DB with switched cNodes */

        /* update Port2CnodeShadow DB */
        rc = prvCpssDxChTmGluePfcTmDbUpdateTcPortCNode(devNum, &oldCnode, &newCnode);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return rc;
}

/**
* @internal cpssDxChTmGluePfcTmTcPort2CNodeSet function
* @endinternal
*
* @brief   Sets C node value for given pfc port and traffic class.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] pfcPortNum               - pfc port number. (APPLICABLE RANGES: 0..63)
* @param[in] tc                       - traffic class (APPLICABLE RANGES: 0..7).
* @param[in] cNodeValue               - C node value (APPLICABLE RANGES: 0...511).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number,
*                                       port or traffic class.
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To enable PFC response by the TM, the Timer values of the received PFC frames must be
*       redirected to the TM unit (using cpssDxChTmGluePfcResponseModeSet).
*       C-nodes must be allocated so that a C-node serves a specific (port, priority).
*       For example, to support 8 PFC priorities on a specific port, 8 C-Nodes must be associated with that port.
*       Queues that serve a specific port and a PFC priority must be associated with the respective C-node.
*       64 ingress ports (pfcPort) are available to redirect the Timer values of the received
*       PFC frames to the TM unit, this API is used along with cpssDxChTmGluePfcPortMappingSet
*       to map physical port to its pfcPort.
*       Specific CNode must have one instance in TcPortToCnode Table,
*       CNodes are switched by the API to have such behaviour.
*
*/
GT_STATUS cpssDxChTmGluePfcTmTcPort2CNodeSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     pfcPortNum,
    IN GT_U32                   tc,
    IN GT_U32                   cNodeValue
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGluePfcTmTcPort2CNodeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pfcPortNum, tc, cNodeValue));

    rc = internal_cpssDxChTmGluePfcTmTcPort2CNodeSet(devNum, pfcPortNum, tc, cNodeValue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pfcPortNum, tc, cNodeValue));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGluePfcTmTcPort2CNodeGet function
* @endinternal
*
* @brief   Gets C node for given traffic class and pfc port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] pfcPortNum               - pfc port number. (APPLICABLE RANGES: 0..63).
* @param[in] tc                       - traffic class (APPLICABLE RANGES: 0..7).
*
* @param[out] cNodeValuePtr            - (pointer to) C node value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number, C node,
*                                       traffic class or port.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChTmGluePfcTmTcPort2CNodeGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     pfcPortNum,
    IN GT_U32                   tc,
    OUT GT_U32                 *cNodeValuePtr
)
{
    GT_U32  value;                  /* HW field value */
    GT_STATUS rc;                   /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E  | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TM_GLUE_PFC_PHY_PORT_NUM_CHECK_MAC(pfcPortNum);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);
    CPSS_NULL_PTR_CHECK_MAC(cNodeValuePtr);


    /* Get TC Port 2 CNode Entry - TC Port 2 CNode */
    rc = prvCpssDxChReadTableEntryField(devNum,
                 CPSS_DXCH_SIP5_TABLE_TM_FCU_TC_PORT_TO_CNODE_PORT_MAPPING_E,
                 pfcPortNum,
                 PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                 tc * 9, 9, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    *cNodeValuePtr = value;

    return GT_OK;
}

/**
* @internal cpssDxChTmGluePfcTmTcPort2CNodeGet function
* @endinternal
*
* @brief   Gets C node for given traffic class and pfc port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] pfcPortNum               - pfc port number. (APPLICABLE RANGES: 0..63).
* @param[in] tc                       - traffic class (APPLICABLE RANGES: 0..7).
*
* @param[out] cNodeValuePtr            - (pointer to) C node value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number, C node,
*                                       traffic class or port.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChTmGluePfcTmTcPort2CNodeGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     pfcPortNum,
    IN GT_U32                   tc,
    OUT GT_U32                 *cNodeValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGluePfcTmTcPort2CNodeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pfcPortNum, tc, cNodeValuePtr));

    rc = internal_cpssDxChTmGluePfcTmTcPort2CNodeGet(devNum, pfcPortNum, tc, cNodeValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pfcPortNum, tc, cNodeValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGluePfcResponseModeSet function
* @endinternal
*
* @brief   Enable/Disable PFC (Priority Flow Control) support with Traffic Manager (TM).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number, relevant for Caelum only. (APPLICABLE RANGES: 0..255).
* @param[in] responseMode             - PFC response mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number or PFC response mode.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Determine whether Timer values of the received PFC frames are redirected to be responded
*       by the TM unit.(used along with cpssDxChTmGluePfcTmTcPort2CNodeSet)
*       for Bobcat2 configuration is applied at Device level (portNum is ignored).
*       for Caelum configuration is applied at portNum only. (used along with cpssDxChTmGluePfcPortMappingSet)
*
*/
static GT_STATUS internal_cpssDxChTmGluePfcResponseModeSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT responseMode
)
{
    GT_STATUS rc;     /* return code */
    GT_U32 regAddr;   /* registre address */
    GT_U32 data;      /* HW data */
    GT_U32 pfcData;   /* HW data */
    GT_U32 bitIndex;  /* bit index */
    GT_U32 mask;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E  | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    switch(responseMode)
    {
        case CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TM_E:
            /* TM FCU ingress must be enabled if PFC is working using TM */
            data = 0x1;
            pfcData = 0x1;
            break;
        case CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TXQ_E:
            data = 0x4;
            pfcData = 0x0;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)) /* for bobk/caelum its per port */
    {
        PRV_CPSS_DXCH_ENHANCED_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        /* Set Port PFC Response Mode */
        /* the manipulations of 4 ports in entry are managed by the 'tables engine' */
        rc =  prvCpssDxChWriteTableEntryField(devNum,
                     CPSS_DXCH_SIP5_TABLE_TM_FCU_PORT_TO_PHYSICAL_PORT_MAPPING_E,
                     portNum,
                     PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                     7,
                     1,
                     pfcData);

        /* for caelum fcu global ingress enable is configured with
           cpssDxChTmGlueFlowControlEnableSet
        */
    }
    else
    {
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            bitIndex = 12;
        }
        else
        {
            bitIndex = 11;
        }

        regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCTriggerGlobalConfig;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, bitIndex, 1, pfcData);
        if(GT_OK != rc)
        {
            return rc;
        }

        mask = 0x5;

        /* fcu global ingress enable*/
        regAddr = PRV_DXCH_REG1_UNIT_TM_FCU_MAC(devNum).TMFCUGlobalConfigs;
        rc = prvCpssHwPpPortGroupWriteRegBitMask(devNum,
                                                 CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                 regAddr, mask, data);
    }

    return rc;
}

/**
* @internal cpssDxChTmGluePfcResponseModeSet function
* @endinternal
*
* @brief   Enable/Disable PFC (Priority Flow Control) support with Traffic Manager (TM).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number, relevant for Caelum only. (APPLICABLE RANGES: 0..255).
* @param[in] responseMode             - PFC response mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number or PFC response mode.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Determine whether Timer values of the received PFC frames are redirected to be responded
*       by the TM unit.(used along with cpssDxChTmGluePfcTmTcPort2CNodeSet)
*       for Bobcat2 configuration is applied at Device level (portNum is ignored).
*       for Caelum configuration is applied at portNum only. (used along with cpssDxChTmGluePfcPortMappingSet)
*
*/
GT_STATUS cpssDxChTmGluePfcResponseModeSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT responseMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGluePfcResponseModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, responseMode));

    rc = internal_cpssDxChTmGluePfcResponseModeSet(devNum, portNum, responseMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, responseMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGluePfcResponseModeGet function
* @endinternal
*
* @brief   Get PFC (Priority Flow Control) for TM support status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number, relevant for Caelum only. (APPLICABLE RANGES: 0..255).
*
* @param[out] responseModePtr          - (pointert to) PFC response mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number number.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_STATE             - on wrong response mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Get whether Timer values of the received PFC frames are redirected to be
*       responded by the TM unit.
*       for Bobcat2 configuration is retrived by Device level (portNum is ignored).
*       for Caelum configuration is retrieved by portNum.
*
*/
static GT_STATUS internal_cpssDxChTmGluePfcResponseModeGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT *responseModePtr
)
{
    GT_STATUS rc;      /* return code */
    GT_U32    regAddr; /* register address */
    GT_U32    data;    /* HW data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E  | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(responseModePtr);


    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)) /* for bobk pfc response is per port */
    {
        PRV_CPSS_DXCH_ENHANCED_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        /* Get Port PFC Response Mode */
        /* the manipulations of 4 ports in entry are managed by the 'tables engine' */
        rc =  prvCpssDxChReadTableEntryField(devNum,
                     CPSS_DXCH_SIP5_TABLE_TM_FCU_PORT_TO_PHYSICAL_PORT_MAPPING_E,
                     portNum,
                     PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                     7,
                     1,
                     &data);

        if(rc != GT_OK)
        {
            return rc;
        }

        if(data == 0x1)
        {
            *responseModePtr = CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TM_E;
        }
        else
        {
            *responseModePtr = CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TXQ_E;
        }
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_TM_FCU_MAC(devNum).TMFCUGlobalConfigs;
        rc = prvCpssHwPpPortGroupReadRegBitMask(devNum,
                                                CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                regAddr,
                                                0x5,
                                                &data);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(data == 0x1 || data == 0x3) /* TM FCU Ingress enable or TM FCU Egress which might be enable */
        {
            *responseModePtr = CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TM_E;
        }
        else if(data == 0x4 || data == 0x6) /* TM FCU Ingress disable or TM FCU Egress which might be enable */
        {
            *responseModePtr = CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TXQ_E;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChTmGluePfcResponseModeGet function
* @endinternal
*
* @brief   Get PFC (Priority Flow Control) for TM support status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number, relevant for Caelum only. (APPLICABLE RANGES: 0..255).
*
* @param[out] responseModePtr          - (pointert to) PFC response mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number number.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_STATE             - on wrong response mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Get whether Timer values of the received PFC frames are redirected to be
*       responded by the TM unit.
*       for Bobcat2 configuration is retrived by Device level (portNum is ignored).
*       for Caelum configuration is retrieved by portNum.
*
*/
GT_STATUS cpssDxChTmGluePfcResponseModeGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT *responseModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGluePfcResponseModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, responseModePtr));

    rc = internal_cpssDxChTmGluePfcResponseModeGet(devNum, portNum, responseModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, responseModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGluePfcPortMappingSet function
* @endinternal
*
* @brief   Map physical port to pfc port, used to map physical ports 0..255
*         to pfc ports 0..63.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number. (APPLICABLE RANGES: 0..255).
* @param[in] pfcPortNum               - ingress pfc port number. (APPLICABLE RANGES: 0..63).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number, or port.
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 64 ingress ports (pfcPort) are available to redirect the Timer values of the received
*       PFC frames to the TM unit, this API is used to map physical port to its pfcPort.
*
*/
static GT_STATUS internal_cpssDxChTmGluePfcPortMappingSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_PHYSICAL_PORT_NUM     pfcPortNum
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E  | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    if(!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);
    PRV_CPSS_DXCH_TM_GLUE_PFC_PHY_PORT_NUM_CHECK_MAC(pfcPortNum);

    /* Set Physical Port to PFC Port mapping Entry */
    rc =  prvCpssDxChWriteTableEntryField(devNum,
                 CPSS_DXCH_SIP5_TABLE_TM_FCU_PORT_TO_PHYSICAL_PORT_MAPPING_E,
                 portNum,
                 PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                 0,
                 6,
                 pfcPortNum);

        return rc;
}

/**
* @internal cpssDxChTmGluePfcPortMappingSet function
* @endinternal
*
* @brief   Map physical port to pfc port, used to map physical ports 0..255
*         to pfc ports 0..63.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number. (APPLICABLE RANGES: 0..255).
* @param[in] pfcPortNum               - ingress pfc port number. (APPLICABLE RANGES: 0..63).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number, or port.
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 64 ingress ports (pfcPort) are available to redirect the Timer values of the received
*       PFC frames to the TM unit, this API is used to map physical port to its pfcPort.
*
*/
GT_STATUS cpssDxChTmGluePfcPortMappingSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_PHYSICAL_PORT_NUM     pfcPortNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGluePfcPortMappingSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, pfcPortNum));

    rc = internal_cpssDxChTmGluePfcPortMappingSet(devNum, portNum, pfcPortNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, pfcPortNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTmGluePfcPortMappingGet function
* @endinternal
*
* @brief   Get physical port to pfc port mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number. (APPLICABLE RANGES: 0..255).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number, or port.
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChTmGluePfcPortMappingGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_PHYSICAL_PORT_NUM    *pfcPortNumPtr
)
{
    GT_STATUS rc;                   /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E  | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    if(!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChTmGluePfcPortMappingGet(devNum, portNum, pfcPortNumPtr);

    return rc;
}

/**
* @internal cpssDxChTmGluePfcPortMappingGet function
* @endinternal
*
* @brief   Get physical port to pfc port mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number. (APPLICABLE RANGES: 0..255).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number, or port.
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChTmGluePfcPortMappingGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_PHYSICAL_PORT_NUM    *pfcPortNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTmGluePfcPortMappingGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, pfcPortNumPtr));

    rc = internal_cpssDxChTmGluePfcPortMappingGet(devNum, portNum, pfcPortNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, pfcPortNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

