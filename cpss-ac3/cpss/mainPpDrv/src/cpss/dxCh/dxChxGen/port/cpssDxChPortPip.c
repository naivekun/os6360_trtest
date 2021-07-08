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
* @file cpssDxChPortPip.c
*
* @brief CPSS DxCh Port's Pre-Ingress Prioritization (PIP) implementation.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortPip.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpssCommon/private/prvCpssMath.h>

/* check that index is in proper 0..max (as parameter) */
#define CHECK_PARAM_INDEX_MAC(_index,_firstNotValidIndex)   \
    if(_index >= _firstNotValidIndex)                       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

/* check that pip profile as parameter */
#define CHECK_PARAM_PIP_PROFILE_MAC(_devNum,_pipProfile) \
    if(_pipProfile > 3)                                  \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

/* check that pip priority as parameter */
#define CHECK_PARAM_PIP_PRIORITY_MAC(_devNum,_pipProfile) \
    if((_pipProfile > 3)) \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG); \
    else if((_pipProfile > 2) && !PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

/* check that pip priority as value */
#define CHECK_VALUE_PIP_PRIORITY_MAC(_devNum,_pipPriority) \
    if((_pipPriority > 3)) \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);  \
    else if((_pipPriority > 2) && !PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG)

/* LookUp Tables for priorities */
static const GT_U8 toHwPrioLut[]  = {3/*very high*/, 0/*high*/, 1/*med*/, 2/*low*/};
static const GT_U8 toApiPrioLut[] = {1/*high*/     , 2/*med*/ , 3/*low*/, 0/*very high*/};

/**
* @internal pip_setPerPort function
* @endinternal
*
* @brief   PIP : set 'per port' field
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (the port must have RxDma)
* @param[in] startBit                 -  of the field to set
* @param[in] numBits                  - number of bit of the field to set
* @param[in] value                    -  to set to the field
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong portNum
* @retval GT_OUT_OF_RANGE          - on value out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS   pip_setPerPort(
    IN GT_U8        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32       startBit,
    IN GT_U32       numBits,
    IN GT_U32       value
)
{
    GT_U32 regAddr;       /* register address */
    GT_U32 portRxDmaNum; /* RxDma number */
    GT_U32 portGroupId; /*the port group Id - support multi port groups device */
    GT_U32  localPort;   /* local port - support multi-port-group device */


    /* check portNum and get converted rxDma port number */
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_RXDMA_NUM_GET_MAC(devNum, portNum, portRxDmaNum);

    /* convert the 'global RxDma number' to portGroupId,local port -- supporting multi port group device */
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portRxDmaNum);
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portRxDmaNum);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
            preIngrPrioritizationConfStatus.channelPIPConfigReg[localPort];
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).singleChannelDMAPip.
            SCDMA_n_PIP_Config[localPort];
    }

    if(value >= (GT_U32)(1 << numBits))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    return  prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,
                               regAddr, startBit, numBits, value);
}

/**
* @internal pip_getPerPort function
* @endinternal
*
* @brief   PIP : get 'per port' field
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (the port must have RxDma)
* @param[in] startBit                 -  of the field to get
* @param[in] numBits                  - number of bit of the field to get
*
* @param[out] valuePtr                 - (pointer to)value to get of the field
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS   pip_getPerPort(
    IN GT_U8        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32       startBit,
    IN GT_U32       numBits,
    OUT GT_U32       *valuePtr
)
{
    GT_U32 regAddr;       /* register address */
    GT_U32 portRxDmaNum; /* RxDma number */
    GT_U32 portGroupId; /*the port group Id - support multi port groups device */
    GT_U32  localPort;   /* local port - support multi-port-group device */

    /* check portNum and get converted rxDma port number */
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_RXDMA_NUM_GET_MAC(devNum, portNum, portRxDmaNum);

    /* convert the 'global RxDma number' to portGroupId,local port -- supporting multi port group device */
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portRxDmaNum);
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portRxDmaNum);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
            preIngrPrioritizationConfStatus.channelPIPConfigReg[localPort];
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).singleChannelDMAPip.
            SCDMA_n_PIP_Config[localPort];
    }

    return  prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,
                               regAddr, startBit, numBits, valuePtr);
}

/**
* @internal pip_counterGetPerPort function
* @endinternal
*
* @brief   PIP : counter get 'per port'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (the port must have RxDma)
*
* @param[out] valuePtr                 - (pointer to)value to get of the counter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS   pip_counterGetPerPort(
    IN GT_U8        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32       *valuePtr
)
{
    GT_U32 regAddr;       /* register address */
    GT_U32 portRxDmaNum; /* RxDma number */
    GT_U32 portGroupId; /*the port group Id - support multi port groups device */
    GT_U32  localPort;   /* local port - support multi-port-group device */

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_TBD_BOOKMARK_FALCON
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    /* check portNum and get converted rxDma port number */
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_RXDMA_NUM_GET_MAC(devNum, portNum, portRxDmaNum);

    /* convert the 'global RxDma number' to portGroupId,local port -- supporting multi port group device */
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portRxDmaNum);
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portRxDmaNum);

    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).singleChannelDMAPip.
        SCDMA_n_Drop_Pkt_Counter[localPort];

    return  prvCpssHwPpPortGroupReadRegister(devNum, portGroupId,
                               regAddr, valuePtr);
}

/**
* @internal pip_etherTypeProtocolInfoGet function
* @endinternal
*
* @brief   PIP : get register address for the EtherType - 'per protocol' 'per index'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] protocol                 - the  for which we define the etherType
* @param[in] index                    - relevant to 'vlan tag' that supports 4 etherTypes (TPIDs)
*                                      relevant to 'mpls' that supports 2 etherTypes
*                                      relevant to 'user defined' that supports 4 etherTypes
*                                      ignored for other protocols.
*                                      (APPLICABLE RANGES: 0..3)
*
* @param[out] regAddrPtr               - (pointer to) register address that hold this field
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong protocol or index
*/
static GT_STATUS pip_etherTypeProtocolInfoGet
(
    IN GT_U8        devNum,
    IN CPSS_DXCH_PORT_PIP_PROTOCOL_ENT  protocol,
    IN GT_U32       index,
    OUT GT_U32      *regAddrPtr
)
{
    GT_U32 regAddr;
#ifdef GM_USED
    GT_U32 gmUsed = 1;
#else
    GT_U32 gmUsed = 0;
#endif

    switch(protocol)
    {
        case CPSS_DXCH_PORT_PIP_PROTOCOL_VLAN_TAG_E:
            /* check index*/
            CHECK_PARAM_INDEX_MAC(index,4);
            /* register VLANEtherTypeConf in Falcon:    */
            /* from CIDER_VERSION_18_02_22_E            */
            /* functionality moved to OSTPIDsConfReg    */
            /* Old OSTPIDsConfReg functionality removed */
            if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                    preIngrPrioritizationConfStatus.VLANEtherTypeConf[index];
            }
            else
            {
                /* register VLANEtherTypeConf in Falcon:    */
                /* from CIDER_VERSION_18_02_22_E            */
                /* functionality moved to OSTPIDsConfReg    */
                /* Old OSTPIDsConfReg functionality removed */
                /* Golden module yet aligned to 17_11_11_E  */
                if((PRV_CPSS_DXCH_FALCON_CIDER_VERSION_17_11_11_E  >= prvCpssDxchFalconCiderVersionGet())
                   || /* gm*/ (1 == gmUsed))
                {
                    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                        preIngrPrioritizationConfStatus.VLANEtherTypeConf[index];
                }
                else
                {
                    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                        preIngrPrioritizationConfStatus.OSTPIDsConfReg[index];
                }
            }
            break;
        case CPSS_DXCH_PORT_PIP_PROTOCOL_MPLS_E    :
            /* check index*/
            CHECK_PARAM_INDEX_MAC(index,2);
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.MPLSEtherTypeConf[index];
            break;
        case CPSS_DXCH_PORT_PIP_PROTOCOL_IPV4_E    :
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.IPv4EtherTypeConf;
            break;
        case CPSS_DXCH_PORT_PIP_PROTOCOL_IPV6_E    :
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.IPv6EtherTypeConf;
            break;
        case CPSS_DXCH_PORT_PIP_PROTOCOL_UDE_E     :
            /* check index*/
            CHECK_PARAM_INDEX_MAC(index,4);
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.genericEtherType[index];
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *regAddrPtr = regAddr;

    return GT_OK;
}

/**
* @internal pip_pofilePriorityInfoGet function
* @endinternal
*
* @brief   PIP : get register address for the pip priority - 'per profile' 'per type' 'per fieldIndex'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] pipProfile               - index to the 'profile table'.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] type                     - 'classification' type
* @param[in] fieldIndex               - index of the priority field in the 'profile entry'
*                                      see details in description of relevant field in
*                                      CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT
*
* @param[out] regAddrPtr               - (pointer to) register address that hold this field
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong profile or type or fieldIndex
*/
static GT_STATUS pip_pofilePriorityInfoGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  pipProfile,
    IN  CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT type,
    IN  GT_U32                  fieldIndex,
    OUT GT_U32                  *regAddrPtr
)
{
    GT_U32 regAddr;

    /* check the pip profile (0..3) as parameter (not as value) */
    CHECK_PARAM_PIP_PROFILE_MAC(devNum,pipProfile);

    switch(type)
    {
        case CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_DSA_QOS_PROFILE_E:
            /* check index*/
            CHECK_PARAM_INDEX_MAC(fieldIndex,BIT_7);
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.
                qoSDSAPriorityArrayProfileSegment[pipProfile][fieldIndex/16];

            break;
        case CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_DSA_UP_E:
            /* check index*/
            CHECK_PARAM_INDEX_MAC(fieldIndex,BIT_3);
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.
                UPDSATagArrayProfile[pipProfile];
            break;
        case CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_VLAN_TAG_UP_E:
            /* check index*/
            CHECK_PARAM_INDEX_MAC(fieldIndex,BIT_3);
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.
                VLANUPPriorityArrayProfile[pipProfile];
            break;
        case CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MPLS_EXP_E:
            /* check index*/
            CHECK_PARAM_INDEX_MAC(fieldIndex,BIT_3);
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.
                MPLSEXPPriorityArrayProfile[pipProfile];
            break;
        case CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV4_TOS_E:
            /* check index*/
            CHECK_PARAM_INDEX_MAC(fieldIndex,BIT_8);
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.
                IPv4ToSPriorityArrayProfileSegment[pipProfile][fieldIndex/16];
            break;
        case CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV6_TC_E:
            /* check index*/
            CHECK_PARAM_INDEX_MAC(fieldIndex,BIT_8);
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.
                IPv6TCPriorityArrayProfileSegment[pipProfile][fieldIndex/16];
            break;
        case CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_UDE_INDEX_E:
            /* check index*/
            CHECK_PARAM_INDEX_MAC(fieldIndex,4);
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.
                etherTypePriorityArrayProfile[pipProfile];
            break;
        case CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MAC_DA_INDEX_E:
            /* check index*/
            CHECK_PARAM_INDEX_MAC(fieldIndex,4);
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.
                MACDAPriorityArrayProfile[pipProfile];
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *regAddrPtr = regAddr;

    return GT_OK;

}

/**
* @internal internal_cpssDxChPortPipGlobalEnableSet function
* @endinternal
*
* @brief   Enable/disable the PIP engine. (Global to the device)
*         if enabled packets are dropped according to their parsed priority if FIFOs
*         before the control pipe are above a configurable threshold
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE  -   PIP
*                                      GT_FALSE - disable PIP
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
)
{
    GT_U32 regAddr;       /* register address */

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    /* check sip5_10 support */
    PRV_CPSS_APPLICABLE_SIP_CHECK_MAC(devNum,CPSS_GEN_SIP_5_10_E);

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).PIPEnable;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
            preIngrPrioritizationConfStatus.preingressPrioritizationEnable;
    }

    /*<Pre-ingress prioritization En>*/
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, BOOL2BIT_MAC(enable));
}

/**
* @internal cpssDxChPortPipGlobalEnableSet function
* @endinternal
*
* @brief   Enable/disable the PIP engine. (Global to the device)
*         if enabled packets are dropped according to their parsed priority if FIFOs
*         before the control pipe are above a configurable threshold
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE  -   PIP
*                                      GT_FALSE - disable PIP
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChPortPipGlobalEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipGlobalEnableGet function
* @endinternal
*
* @brief   Get the state Enable/disable of the PIP engine. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - enable  PIP
*                                      GT_FALSE - disable PIP
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalEnableGet
(
    IN  GT_U8        devNum,
    OUT GT_BOOL      *enablePtr
)
{
    GT_U32 value;         /* register value */
    GT_U32 regAddr;       /* register address */
    GT_STATUS rc;         /* function return value */

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    /* check sip5_10 support */
    PRV_CPSS_APPLICABLE_SIP_CHECK_MAC(devNum,CPSS_GEN_SIP_5_10_E);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).PIPEnable;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
            preIngrPrioritizationConfStatus.preingressPrioritizationEnable;
    }

    /*<Pre-ingress prioritization En>*/
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 1, &value);

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChPortPipGlobalEnableGet function
* @endinternal
*
* @brief   Get the state Enable/disable of the PIP engine. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - enable  PIP
*                                      GT_FALSE - disable PIP
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalEnableGet
(
    IN  GT_U8        devNum,
    OUT GT_BOOL      *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChPortPipGlobalEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipGlobalEtherTypeProtocolSet function
* @endinternal
*
* @brief   Set the PIP etherTypes for the protocol. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] protocol                 - the  for which we define the etherType
* @param[in] index                    - relevant to 'vlan tag' that supports 4 etherTypes (TPIDs)
*                                      relevant to 'mpls' that supports 2 etherTypes
*                                      relevant to 'user defined' that supports 4 etherTypes
*                                      ignored for other protocols.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] etherType                -  to recognize the protocols packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or protocol or index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalEtherTypeProtocolSet
(
    IN GT_U8        devNum,
    IN CPSS_DXCH_PORT_PIP_PROTOCOL_ENT  protocol,
    IN GT_U32       index,
    IN GT_U16       etherType
)
{
    GT_U32 regAddr;       /* register address */
    GT_STATUS rc;         /* function return value */

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    /* check sip5_10 support */
    PRV_CPSS_APPLICABLE_SIP_CHECK_MAC(devNum,CPSS_GEN_SIP_5_10_E);

    rc = pip_etherTypeProtocolInfoGet(devNum,protocol,index,&regAddr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*<etherType>*/
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 16, etherType);
}

/**
* @internal cpssDxChPortPipGlobalEtherTypeProtocolSet function
* @endinternal
*
* @brief   Set the PIP etherTypes for the protocol. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] protocol                 - the  for which we define the etherType
* @param[in] index                    - relevant to 'vlan tag' that supports 4 etherTypes (TPIDs)
*                                      relevant to 'mpls' that supports 2 etherTypes
*                                      relevant to 'user defined' that supports 4 etherTypes
*                                      ignored for other protocols.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] etherType                -  to recognize the protocols packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or protocol or index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalEtherTypeProtocolSet
(
    IN GT_U8        devNum,
    IN CPSS_DXCH_PORT_PIP_PROTOCOL_ENT  protocol,
    IN GT_U32       index,
    IN GT_U16       etherType
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalEtherTypeProtocolSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, protocol, index, etherType));

    rc = internal_cpssDxChPortPipGlobalEtherTypeProtocolSet(devNum, protocol, index, etherType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, protocol, index, etherType));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipGlobalEtherTypeProtocolGet function
* @endinternal
*
* @brief   Get the PIP etherTypes for the protocol. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] protocol                 - the  for which we define the etherType
* @param[in] index                    - relevant to 'mpls' that supports 2 etherTypes
*                                      relevant to 'user defined' that supports 4 etherTypes
*                                      ignored for other protocols.
*                                      (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or protocol or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalEtherTypeProtocolGet
(
    IN GT_U8        devNum,
    IN CPSS_DXCH_PORT_PIP_PROTOCOL_ENT  protocol,
    IN GT_U32       index,
    OUT GT_U16      *etherTypePtr
)
{
    GT_U32 value;         /* register value */
    GT_U32 regAddr;       /* register address */
    GT_STATUS rc;         /* function return value */

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    /* check sip5_10 support */
    PRV_CPSS_APPLICABLE_SIP_CHECK_MAC(devNum,CPSS_GEN_SIP_5_10_E);
    CPSS_NULL_PTR_CHECK_MAC(etherTypePtr);

    rc = pip_etherTypeProtocolInfoGet(devNum,protocol,index,&regAddr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*<etherType>*/
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 16, &value);

    *etherTypePtr = (GT_U16)value;

    return rc;
}

/**
* @internal cpssDxChPortPipGlobalEtherTypeProtocolGet function
* @endinternal
*
* @brief   Get the PIP etherTypes for the protocol. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] protocol                 - the  for which we define the etherType
* @param[in] index                    - relevant to 'mpls' that supports 2 etherTypes
*                                      relevant to 'user defined' that supports 4 etherTypes
*                                      ignored for other protocols.
*                                      (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or protocol or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalEtherTypeProtocolGet
(
    IN GT_U8        devNum,
    IN CPSS_DXCH_PORT_PIP_PROTOCOL_ENT  protocol,
    IN GT_U32       index,
    OUT GT_U16      *etherTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalEtherTypeProtocolGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, protocol, index, etherTypePtr));

    rc = internal_cpssDxChPortPipGlobalEtherTypeProtocolGet(devNum, protocol, index, etherTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, protocol, index, etherTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipTrustEnableSet function
* @endinternal
*
* @brief   Set port as trusted/not trusted PIP port.
*         A 'trusted' port will set priority level for ingress packets according to
*         packet's fields.
*         Otherwise, for the 'not trusted' port will use the port’s default priority
*         level.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE  - set port as 'trusted'
*                                      GT_FALSE - set port as 'not trusted'
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipTrustEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    /* check sip5_10 support */
    PRV_CPSS_APPLICABLE_SIP_CHECK_MAC(devNum,CPSS_GEN_SIP_5_10_E);

    /* <Port <n> Trusted> */
    return  pip_setPerPort(devNum, portNum , 0, 1, BOOL2BIT_MAC(enable));
}

/**
* @internal cpssDxChPortPipTrustEnableSet function
* @endinternal
*
* @brief   Set port as trusted/not trusted PIP port.
*         A 'trusted' port will set priority level for ingress packets according to
*         packet's fields.
*         Otherwise, for the 'not trusted' port will use the port’s default priority
*         level.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE  - set port as 'trusted'
*                                      GT_FALSE - set port as 'not trusted'
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipTrustEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipTrustEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortPipTrustEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipTrustEnableGet function
* @endinternal
*
* @brief   Get port's trusted/not trusted .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                -  (pointer to)
*                                      GT_TRUE  - set port as 'trusted'
*                                      GT_FALSE - set port as 'not trusted'
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipTrustEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;         /* function return value */
    GT_U32 value;         /* register value */

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    /* check sip5_10 support */
    PRV_CPSS_APPLICABLE_SIP_CHECK_MAC(devNum,CPSS_GEN_SIP_5_10_E);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* <Port <n> Trusted> */
    rc =  pip_getPerPort(devNum, portNum , 0, 1, &value);

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChPortPipTrustEnableGet function
* @endinternal
*
* @brief   Get port's trusted/not trusted .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                -  (pointer to)
*                                      GT_TRUE  - set port as 'trusted'
*                                      GT_FALSE - set port as 'not trusted'
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipTrustEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipTrustEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortPipTrustEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipProfileSet function
* @endinternal
*
* @brief   Set port's PIP profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] pipProfile               - the profile of the port.
*                                      (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipProfileSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  pipProfile
)
{
    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    /* check sip5_10 support */
    PRV_CPSS_APPLICABLE_SIP_CHECK_MAC(devNum,CPSS_GEN_SIP_5_10_E);

    /* <Port <n> Pip Profile> */
    return  pip_setPerPort(devNum, portNum , 3, 2, pipProfile);
}

/**
* @internal cpssDxChPortPipProfileSet function
* @endinternal
*
* @brief   Set port's PIP profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] pipProfile               - the profile of the port.
*                                      (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipProfileSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  pipProfile
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipProfileSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, pipProfile));

    rc = internal_cpssDxChPortPipProfileSet(devNum, portNum, pipProfile);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, pipProfile));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipProfileGet function
* @endinternal
*
* @brief   Get port's PIP profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] pipProfilePtr            - (pointer to) the profile of the port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipProfileGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *pipProfilePtr
)
{
    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    /* check sip5_10 support */
    PRV_CPSS_APPLICABLE_SIP_CHECK_MAC(devNum,CPSS_GEN_SIP_5_10_E);
    CPSS_NULL_PTR_CHECK_MAC(pipProfilePtr);

    /* <Port <n> Pip Profile> */
    return pip_getPerPort(devNum, portNum , 3, 2, pipProfilePtr);
}

/**
* @internal cpssDxChPortPipProfileGet function
* @endinternal
*
* @brief   Get port's PIP profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] pipProfilePtr            - (pointer to) the profile of the port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipProfileGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *pipProfilePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipProfileGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, pipProfilePtr));

    rc = internal_cpssDxChPortPipProfileGet(devNum, portNum, pipProfilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, pipProfilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipVidClassificationEnableSet function
* @endinternal
*
* @brief   Set port PIP enable/disable VID classification .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE   -  VID classification
*                                      GT_FALSE  - disable VID classification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipVidClassificationEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    /* check sip5_10 support */
    PRV_CPSS_APPLICABLE_SIP_CHECK_MAC(devNum,CPSS_GEN_SIP_5_10_E);

    /* <Port <n> VID Enable> */
    return  pip_setPerPort(devNum, portNum , 5, 1, BOOL2BIT_MAC(enable));
}

/**
* @internal cpssDxChPortPipVidClassificationEnableSet function
* @endinternal
*
* @brief   Set port PIP enable/disable VID classification .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE   -  VID classification
*                                      GT_FALSE  - disable VID classification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipVidClassificationEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipVidClassificationEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortPipVidClassificationEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipVidClassificationEnableGet function
* @endinternal
*
* @brief   Get port PIP enable/disable VID classification .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE   - enable VID classification
*                                      GT_FALSE  - disable VID classification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipVidClassificationEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_U32 value;         /* register value */
    GT_STATUS rc;         /* function return value */

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    /* check sip5_10 support */
    PRV_CPSS_APPLICABLE_SIP_CHECK_MAC(devNum,CPSS_GEN_SIP_5_10_E);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* <Port <n> VID Enable> */
    rc = pip_getPerPort(devNum, portNum , 5, 1, &value);

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChPortPipVidClassificationEnableGet function
* @endinternal
*
* @brief   Get port PIP enable/disable VID classification .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE   - enable VID classification
*                                      GT_FALSE  - disable VID classification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipVidClassificationEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipVidClassificationEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortPipVidClassificationEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipPrioritySet function
* @endinternal
*
* @brief   Set priority per Port for 'classification' type.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] type                     - 'classification' type
* @param[in] priority                 - the  value to set.
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..2).
*                                      - (0-high,1-mid,2-low)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..3).
*                                      - (0-very high,1-high,2-mid,3-low)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum or type
* @retval GT_OUT_OF_RANGE          - on out of range priority
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipPrioritySet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT type,
    IN  GT_U32                  priority
)
{
    GT_U32       startBit;

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    /* check sip5_10 support */
    PRV_CPSS_APPLICABLE_SIP_CHECK_MAC(devNum,CPSS_GEN_SIP_5_10_E);
    /* check the pip priority (0..3) as value (not as parameter) */
    CHECK_VALUE_PIP_PRIORITY_MAC(devNum,priority);

    if(type == CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ETHERTYPE_AND_VID_E)
    {
        /* <Ingress Port <n> Pip Priority> */
        startBit = 6;
    }
    else
    if (type == CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_DEFAULT_E)
    {
        /* <Port <n> Default PIP Priority>*/
        startBit = 1;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        /* Translate priority */
        priority = toHwPrioLut[priority];
    }
    return  pip_setPerPort(devNum, portNum , startBit, 2, priority);
}

/**
* @internal cpssDxChPortPipPrioritySet function
* @endinternal
*
* @brief   Set priority per Port for 'classification' type.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] type                     - 'classification' type
* @param[in] priority                 - the  value to set.
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..2).
*                                      - (0-high,1-mid,2-low)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..3).
*                                      - (0-very high,1-high,2-mid,3-low)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum or type
* @retval GT_OUT_OF_RANGE          - on out of range priority
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipPrioritySet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT type,
    IN  GT_U32                  priority
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipPrioritySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, type, priority));

    rc = internal_cpssDxChPortPipPrioritySet(devNum, portNum, type, priority);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, type, priority));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipPriorityGet function
* @endinternal
*
* @brief   Get priority per Port for 'classification' type.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] type                     - 'classification' type
*
* @param[out] priorityPtr              - (pointer to)the priority value to set.
*                                      For Bobcat3:     For Bobcat2, Caelum, Aldrin, AC3X:
*                                      0 - Very High    0 - High
*                                      1 - High         1 - Medium
*                                      2 - Medium       2 - Low
*                                      3 - Low
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum or type
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipPriorityGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT type,
    OUT GT_U32                  *priorityPtr
)
{
    GT_U32       startBit;
    GT_STATUS    rc;

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    /* check sip5_10 support */
    PRV_CPSS_APPLICABLE_SIP_CHECK_MAC(devNum,CPSS_GEN_SIP_5_10_E);
    CPSS_NULL_PTR_CHECK_MAC(priorityPtr);

    if(type == CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ETHERTYPE_AND_VID_E)
    {
        /* <Ingress Port <n> Pip Priority> */
        startBit = 6;
    }
    else
    if (type == CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_DEFAULT_E)
    {
        /* <Port <n> Default PIP Priority>*/
        startBit = 1;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = pip_getPerPort(devNum, portNum , startBit, 2, priorityPtr);

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        /* Translate priority */
        *priorityPtr = toApiPrioLut[*priorityPtr];
    }
    return  rc;
}

/**
* @internal cpssDxChPortPipPriorityGet function
* @endinternal
*
* @brief   Get priority per Port for 'classification' type.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] type                     - 'classification' type
*
* @param[out] priorityPtr              - (pointer to)the priority value to set.
*                                      For Bobcat3:     For Bobcat2, Caelum, Aldrin, AC3X:
*                                      0 - Very High    0 - High
*                                      1 - High         1 - Medium
*                                      2 - Medium       2 - Low
*                                      3 - Low
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum or type
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipPriorityGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT type,
    OUT GT_U32                  *priorityPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipPriorityGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, type, priorityPtr));

    rc = internal_cpssDxChPortPipPriorityGet(devNum, portNum, type, priorityPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, type, priorityPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipGlobalProfilePrioritySet function
* @endinternal
*
* @brief   Set priority per PIP profile Per 'classification' type.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] pipProfile               - index to the 'profile table'.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] type                     - 'classification' type
* @param[in] fieldIndex               - index of the priority field in the 'profile entry'
*                                      see details in description of relevant field in
*                                      CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT
* @param[in] priority                 - the  value to set.
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..2).
*                                      - (0-high,1-mid,2-low)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..3).
*                                      - (0-very high,1-high,2-mid,3-low)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or type or pipProfile or
*                                       fieldIndex
* @retval GT_OUT_OF_RANGE          - on out of range priority
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalProfilePrioritySet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  pipProfile,
    IN  CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT type,
    IN  GT_U32                  fieldIndex,
    IN  GT_U32                  priority
)
{
    GT_STATUS rc;         /* function return value */
    GT_U32 regAddr;       /* register address */
    GT_U32 numBits = 2;   /* number of bits to set */
    GT_U32 hwValue = priority;/* value to write to HW */
    GT_U32 hwIndex = fieldIndex;/* index to write to HW */

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    /* check sip5_10 support */
    PRV_CPSS_APPLICABLE_SIP_CHECK_MAC(devNum,CPSS_GEN_SIP_5_10_E);
    /* check the pip priority (0..3) as value (not as parameter) */
    CHECK_VALUE_PIP_PRIORITY_MAC(devNum,priority);

    /* Translate priority */
    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        priority = toHwPrioLut[priority];
        hwValue = priority;
    }

    if ((type == CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV6_TC_E) &&
        GT_FALSE != PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
            PRV_CPSS_DXCH_BOBCAT2_RXDMA_PIP_IPV6_WRONG_TC_BITS_WA_E))
    {
        if (fieldIndex >= BIT_6)
        {
            /* only 6 bits of the TC actually influence the ipv6 packet */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        /* the actual field is (TC_from_packet <<2) + 2 bits from 'flow label' */
        hwIndex = fieldIndex << 2;
        /* we need to duplicate the value to 4 consecutive places */
        hwValue = (priority << 6) | (priority << 4) | (priority << 2) | priority;
        numBits = 8;/* 2 bits time the 4 consecutive places */
    }

    rc = pip_pofilePriorityInfoGet(devNum,pipProfile,type,hwIndex,&regAddr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* each field is 2 bits , (up to) 16 fields in the register */
    return prvCpssHwPpSetRegField(devNum, regAddr, 2 * (hwIndex%16), numBits, hwValue);
}

/**
* @internal cpssDxChPortPipGlobalProfilePrioritySet function
* @endinternal
*
* @brief   Set priority per PIP profile Per 'classification' type.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] pipProfile               - index to the 'profile table'.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] type                     - 'classification' type
* @param[in] fieldIndex               - index of the priority field in the 'profile entry'
*                                      see details in description of relevant field in
*                                      CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT
* @param[in] priority                 - the  value to set.
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..2).
*                                      - (0-high,1-mid,2-low)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..3).
*                                      - (0-very high,1-high,2-mid,3-low)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or type or pipProfile or
*                                       fieldIndex
* @retval GT_OUT_OF_RANGE          - on out of range priority
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalProfilePrioritySet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  pipProfile,
    IN  CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT type,
    IN  GT_U32                  fieldIndex,
    IN  GT_U32                  priority
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalProfilePrioritySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pipProfile, type, fieldIndex, priority));

    rc = internal_cpssDxChPortPipGlobalProfilePrioritySet(devNum, pipProfile, type, fieldIndex, priority);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pipProfile, type, fieldIndex, priority));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipGlobalProfilePriorityGet function
* @endinternal
*
* @brief   Get priority per PIP profile Per 'classification' type. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] pipProfile               - index to the 'profile table'.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] type                     - 'classification' type
* @param[in] fieldIndex               - index of the priority field in the 'profile entry'
*                                      see details in description of relevant field in
*                                      CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT
*
* @param[out] priorityPtr              - (pointer to)the priority value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or type or pipProfile or fieldIndex
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalProfilePriorityGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  pipProfile,
    IN  CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT type,
    IN  GT_U32                  fieldIndex,
    OUT GT_U32                  *priorityPtr
)
{
    GT_STATUS rc;         /* function return value */
    GT_U32 regAddr;       /* register address */
    GT_U32 hwIndex = fieldIndex;/* index to write to HW */

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    /* check sip5_10 support */
    PRV_CPSS_APPLICABLE_SIP_CHECK_MAC(devNum,CPSS_GEN_SIP_5_10_E);
    CPSS_NULL_PTR_CHECK_MAC(priorityPtr);

    if ((type == CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV6_TC_E) &&
        GT_FALSE != PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
            PRV_CPSS_DXCH_BOBCAT2_RXDMA_PIP_IPV6_WRONG_TC_BITS_WA_E))
    {
        if (fieldIndex >= BIT_6)
        {
            /* only 6 bits of the TC actually influence the ipv6 packet */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        /* the actual field is (TC_from_packet <<2) + 2 bits from 'flow label' */
        hwIndex = fieldIndex << 2;
        /* the 'set' function did : duplicate the value to 4 places consecutive places.
           this 'get' function we return the 'first' one.
        */
    }

    rc = pip_pofilePriorityInfoGet(devNum,pipProfile,type,hwIndex,&regAddr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* each field is 2 bits , (up to) 16 fields in the register */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 2 * (hwIndex%16), 2, priorityPtr);

    /* Translate priority */
    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        *priorityPtr = toApiPrioLut[*priorityPtr];
    }

    return rc;
}

/**
* @internal cpssDxChPortPipGlobalProfilePriorityGet function
* @endinternal
*
* @brief   Get priority per PIP profile Per 'classification' type. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] pipProfile               - index to the 'profile table'.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] type                     - 'classification' type
* @param[in] fieldIndex               - index of the priority field in the 'profile entry'
*                                      see details in description of relevant field in
*                                      CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT
*
* @param[out] priorityPtr              - (pointer to)the priority value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or type or pipProfile or fieldIndex
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalProfilePriorityGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  pipProfile,
    IN  CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT type,
    IN  GT_U32                  fieldIndex,
    OUT GT_U32                  *priorityPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalProfilePriorityGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pipProfile, type, fieldIndex, priorityPtr));

    rc = internal_cpssDxChPortPipGlobalProfilePriorityGet(devNum, pipProfile, type, fieldIndex, priorityPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pipProfile, type, fieldIndex, priorityPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipGlobalVidClassificationSet function
* @endinternal
*
* @brief   Set PIP vid classification value per index. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    - one of 4 indexes.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] vid                      - the  to match
*                                      (APPLICABLE RANGES: 0..(4k-1))
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_OUT_OF_RANGE          - on out of range of vid
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalVidClassificationSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    IN  GT_U16                  vid
)
{
    GT_U32 regAddr;       /* register address */

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    /* check sip5_10 support */
    PRV_CPSS_APPLICABLE_SIP_CHECK_MAC(devNum,CPSS_GEN_SIP_5_10_E);

    CHECK_PARAM_INDEX_MAC(index,4);

    if(vid >= BIT_12)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
        preIngrPrioritizationConfStatus.VIDConf[index];

    /* <VId Conf <n>> */
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 12, vid);
}

/**
* @internal cpssDxChPortPipGlobalVidClassificationSet function
* @endinternal
*
* @brief   Set PIP vid classification value per index. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    - one of 4 indexes.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] vid                      - the  to match
*                                      (APPLICABLE RANGES: 0..(4k-1))
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_OUT_OF_RANGE          - on out of range of vid
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalVidClassificationSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    IN  GT_U16                  vid
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalVidClassificationSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, vid));

    rc = internal_cpssDxChPortPipGlobalVidClassificationSet(devNum, index, vid);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, vid));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipGlobalVidClassificationGet function
* @endinternal
*
* @brief   Get PIP vid classification value per index. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    - one of 4 indexes.
*                                      (APPLICABLE RANGES: 0..3)
*
* @param[out] vidPtr                   - (pointer to)the vid to match
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalVidClassificationGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    OUT GT_U16                  *vidPtr
)
{
    GT_U32 regAddr;       /* register address */
    GT_U32 value;         /* register value */
    GT_STATUS rc;         /* function return value */

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    /* check sip5_10 support */
    PRV_CPSS_APPLICABLE_SIP_CHECK_MAC(devNum,CPSS_GEN_SIP_5_10_E);

    CHECK_PARAM_INDEX_MAC(index,4);
    CPSS_NULL_PTR_CHECK_MAC(vidPtr);

    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
        preIngrPrioritizationConfStatus.VIDConf[index];

    /* <VId Conf <n>> */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 12, &value);

    *vidPtr = (GT_U16)value;

    return rc;
}

/**
* @internal cpssDxChPortPipGlobalVidClassificationGet function
* @endinternal
*
* @brief   Get PIP vid classification value per index. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    - one of 4 indexes.
*                                      (APPLICABLE RANGES: 0..3)
*
* @param[out] vidPtr                   - (pointer to)the vid to match
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalVidClassificationGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    OUT GT_U16                  *vidPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalVidClassificationGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, vidPtr));

    rc = internal_cpssDxChPortPipGlobalVidClassificationGet(devNum, index, vidPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, vidPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortPipGlobalMacDaClassificationEntrySet function
* @endinternal
*
* @brief   Set PIP Mac Da classification entry. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    -  to the 'mac DA classification table'.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] entryPtr                 - (pointer to) the entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of value in entry
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalMacDaClassificationEntrySet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    IN  CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC *entryPtr
)
{
    GT_U32 regAddr;       /* register address */
    GT_STATUS rc;         /* function return value */
    GT_U32  lsWord , msWord;

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    /* check sip5_10 support */
    PRV_CPSS_APPLICABLE_SIP_CHECK_MAC(devNum,CPSS_GEN_SIP_5_10_E);

    CHECK_PARAM_INDEX_MAC(index,4);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    /*******************/
    /* set the pattern */
    /*******************/

    lsWord =
        entryPtr->macAddrValue.arEther[2] << 24 |
        entryPtr->macAddrValue.arEther[3] << 16 |
        entryPtr->macAddrValue.arEther[4] <<  8 |
        entryPtr->macAddrValue.arEther[5] <<  0 ;
    msWord =
        entryPtr->macAddrValue.arEther[0] <<  8 |
        entryPtr->macAddrValue.arEther[1] <<  0 ;

    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
        preIngrPrioritizationConfStatus.MACDA4LsbBytesConf[index];

    /* <MAC DA 4 Lsb Bytes Conf <n>> */
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, lsWord);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
        preIngrPrioritizationConfStatus.MACDA2MsbBytesConf[index];

    /* <MAC DA 2 Msb Bytes Conf <n>> */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 16 , msWord);
    if(rc != GT_OK)
    {
        return rc;
    }

    /****************/
    /* set the mask */
    /****************/
    lsWord =
        entryPtr->macAddrMask.arEther[2] << 24 |
        entryPtr->macAddrMask.arEther[3] << 16 |
        entryPtr->macAddrMask.arEther[4] <<  8 |
        entryPtr->macAddrMask.arEther[5] <<  0 ;
    msWord =
        entryPtr->macAddrMask.arEther[0] <<  8 |
        entryPtr->macAddrMask.arEther[1] <<  0 ;

    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
        preIngrPrioritizationConfStatus.MACDAMask4LsbConf[index];

    /* <MAC DA Mask 4 Lsb Conf <n>> */
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, lsWord);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
        preIngrPrioritizationConfStatus.MACDAMask2MsbConf[index];

    /* <MAC DA Mask 2 Msb Conf <n>> */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 16 , msWord);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortPipGlobalMacDaClassificationEntrySet function
* @endinternal
*
* @brief   Set PIP Mac Da classification entry. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    -  to the 'mac DA classification table'.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] entryPtr                 - (pointer to) the entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of value in entry
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalMacDaClassificationEntrySet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    IN  CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalMacDaClassificationEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, entryPtr));

    rc = internal_cpssDxChPortPipGlobalMacDaClassificationEntrySet(devNum, index, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortPipGlobalMacDaClassificationEntryGet function
* @endinternal
*
* @brief   Get PIP Mac Da classification entry. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    -  to the 'mac DA classification table'.
*                                      (APPLICABLE RANGES: 0..3)
*
* @param[out] entryPtr                 - (pointer to) the entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalMacDaClassificationEntryGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    OUT CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC *entryPtr
)
{
    GT_U32 regAddr;       /* register address */
    GT_STATUS rc;         /* function return value */
    GT_U32  lsWord , msWord;

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    /* check sip5_10 support */
    PRV_CPSS_APPLICABLE_SIP_CHECK_MAC(devNum,CPSS_GEN_SIP_5_10_E);

    CHECK_PARAM_INDEX_MAC(index,4);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    /*******************/
    /* get the pattern */
    /*******************/
    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
        preIngrPrioritizationConfStatus.MACDA4LsbBytesConf[index];

    /* <MAC DA 4 Lsb Bytes Conf <n>> */
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &lsWord);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
        preIngrPrioritizationConfStatus.MACDA2MsbBytesConf[index];

    /* <MAC DA 2 Msb Bytes Conf <n>> */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 16 , &msWord);
    if(rc != GT_OK)
    {
        return rc;
    }

    entryPtr->macAddrValue.arEther[0] = (GT_U8)    (msWord >>  8);
    entryPtr->macAddrValue.arEther[1] = (GT_U8)    (msWord >>  0);

    entryPtr->macAddrValue.arEther[2] = (GT_U8)    (lsWord >> 24);
    entryPtr->macAddrValue.arEther[3] = (GT_U8)    (lsWord >> 16);
    entryPtr->macAddrValue.arEther[4] = (GT_U8)    (lsWord >>  8);
    entryPtr->macAddrValue.arEther[5] = (GT_U8)    (lsWord >>  0);

    /****************/
    /* get the mask */
    /****************/
    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
        preIngrPrioritizationConfStatus.MACDAMask4LsbConf[index];

    /* <MAC DA Mask 4 Lsb Conf <n>> */
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &lsWord);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
        preIngrPrioritizationConfStatus.MACDAMask2MsbConf[index];

    /* <MAC DA Mask 2 Msb Conf <n>> */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 16 , &msWord);
    if(rc != GT_OK)
    {
        return rc;
    }

    entryPtr->macAddrMask.arEther[0] = (GT_U8)    (msWord >>  8);
    entryPtr->macAddrMask.arEther[1] = (GT_U8)    (msWord >>  0);

    entryPtr->macAddrMask.arEther[2] = (GT_U8)    (lsWord >> 24);
    entryPtr->macAddrMask.arEther[3] = (GT_U8)    (lsWord >> 16);
    entryPtr->macAddrMask.arEther[4] = (GT_U8)    (lsWord >>  8);
    entryPtr->macAddrMask.arEther[5] = (GT_U8)    (lsWord >>  0);

    return GT_OK;
}

/**
* @internal cpssDxChPortPipGlobalMacDaClassificationEntryGet function
* @endinternal
*
* @brief   Get PIP Mac Da classification entry. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    -  to the 'mac DA classification table'.
*                                      (APPLICABLE RANGES: 0..3)
*
* @param[out] entryPtr                 - (pointer to) the entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalMacDaClassificationEntryGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    OUT CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalMacDaClassificationEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, entryPtr));

    rc = internal_cpssDxChPortPipGlobalMacDaClassificationEntryGet(devNum, index, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal pipGlobalBurstFifoThresholdDpMaskCreateCheck function
* @endinternal
*
* @brief   Creates bit mask for available datapathes with non-zero port count and
*         checks if dataPathBitmap passed to
*         cpssDxChPortPipGlobalMacDaClassificationEntryG(S)et uses only them.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*
* @param[out] maskPtr                  - created mask
* @param[out] dpSizePtr                - mask size, may be NULL
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
*/
static GT_STATUS pipGlobalBurstFifoThresholdDpMaskCreateCheck
(
    IN  GT_U8                devNum,
    IN  GT_DATA_PATH_BMP     dataPathBmp,
    OUT GT_U32               *maskPtr,
    OUT GT_U32               *dpSizePtr
)
{
    GT_U32      lMask = 0; /* Local mask */
    GT_U32      ii;
    GT_U32      lDpSize;

    if(dataPathBmp == 0) CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        lDpSize = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes;
        lMask = BIT_MASK_0_31_MAC(lDpSize);
    }
    else
    {
        lMask = 0;
        lDpSize = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp;
        for(ii = 0; ii < lDpSize; ii++)
        {
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.
                    info[ii].dataPathNumOfPorts)
            {
                lMask |= (1 << ii);
            }
        }
    }
    if(dataPathBmp & ~(lMask))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "The bmp[0x%8.8x] hold"
            " bits out of the valid range[0x%8.8x] \n", dataPathBmp, lMask);
    }
    else
    {
        *maskPtr = lMask;
        if(NULL != dpSizePtr)
        {
            *dpSizePtr = lDpSize;
        }
        return GT_OK;
    }
}

/**
* @internal internal_cpssDxChPortPipGlobalBurstFifoThresholdsSet function
* @endinternal
*
* @brief   Set Burst FIFO Threshold for specific PIP priority.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      (APPLICABLE DEVICES Caelum; Aldrin; AC3X; Bobcat3)
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
*                                      3. For multi-pipe device:
*                                      bitmap must be set with at least one bit representing
*                                      pipe(s).  If a bit of non valid pipe is set then
*                                      function returns GT_BAD_PARAM. Value
*                                      CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
* @param[in] priority                 - the  to set the threshold.
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..2).
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..3).
* @param[in] threshold                - the  value. (number of packets)
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..0x3FF)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_OUT_OF_RANGE          - on out of range threshold
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalBurstFifoThresholdsSet
(
    IN GT_U8            devNum,
    IN GT_DATA_PATH_BMP dataPathBmp,
    IN GT_U32           priority,
    IN GT_U32           threshold
)
{
    GT_U32 regAddr;       /* register address */
    GT_STATUS rc;         /* return code */
    GT_U32 dpIdx;         /* Datapath index */
    GT_U32 thrOff;        /* offset of threshold in the register*/
    GT_U32 mask;          /* mask of available datapathes */
    GT_U32 fieldSize;     /* field size is different for SIP 5.20 */
    GT_U32 isDataPathBmpApplicable; /* Is dataPathBmp applicable flag */

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
        CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E |
        CPSS_LION2_E);

    /* check sip5_10 support */
    PRV_CPSS_APPLICABLE_SIP_CHECK_MAC(devNum,CPSS_GEN_SIP_5_10_E);

    /* check the pip priority (0..3) as parameter (not as value) */
    CHECK_PARAM_PIP_PRIORITY_MAC(devNum, priority);

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        if(priority < 2)
        {
            thrOff = (priority == 0) ? 0 : 16;
        }
        else
        {
            thrOff = (priority == 2) ? 0 : 16;
        }
        fieldSize = 16;

        /* the FIFOs are in IA units.
           Number of IA units equal to number of processing pipes. */
        isDataPathBmpApplicable = (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes > 1) ? GT_TRUE : GT_FALSE;
    }
    else /* (priority < 4) */
    {
        thrOff = priority * 10;
        fieldSize = 10;

        /* the FIFOs are in RXDMA units. */
        isDataPathBmpApplicable = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.supportMultiDataPath;
    }

    /* check threshold */
    if(threshold >= (GT_U32)(1 << fieldSize))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(isDataPathBmpApplicable
       && (dataPathBmp != CPSS_DATA_PATH_UNAWARE_MODE_CNS))
    {

        rc = pipGlobalBurstFifoThresholdDpMaskCreateCheck(devNum,
            dataPathBmp, &mask, NULL);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Only selected datapathes */
        dpIdx = 0;
        rc = GT_OK; /* If nothing to change -- OK by default */
        for(; mask; mask >>= 1)
        {
            if(mask & dataPathBmp & 0x01)
            {
                if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                {
                    if(priority < 2)
                    {
                        regAddr =
                            PRV_DXCH_REG1_UNIT_IA_INDEX_MAC(devNum, dpIdx).
                                PIPPrioThresholds0;
                    }
                    else /* (priority < 4) */
                    {
                        regAddr =
                            PRV_DXCH_REG1_UNIT_IA_INDEX_MAC(devNum, dpIdx).
                                PIPPrioThresholds1;
                    }
                }
                else
                {
                    regAddr = PRV_DXCH_REG1_UNIT_RXDMA_INDEX_MAC(devNum, dpIdx).
                        globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                        PIPPrioThresholds;
                }
                rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, thrOff,
                    fieldSize, threshold);
                if(rc != GT_OK)
                {
                    break;
                }
            }
            dataPathBmp >>= 1;
            dpIdx++;
        }
    }
    else
    {
        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            /* Globally configure multi-pipe device */
            if(priority < 2)
            {
                regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).PIPPrioThresholds0;
            }
            else /* (priority < 4) */
            {
                regAddr = PRV_DXCH_REG1_UNIT_IA_MAC(devNum).PIPPrioThresholds1;
            }

            rc = prvCpssHwPpSetRegField(devNum, regAddr, thrOff, fieldSize,
                threshold);
        }
        else
        {
            /* Not the case with multidatapath device or datapath unaware */
            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
                preIngrPrioritizationConfStatus.PIPPrioThresholds;

            rc = prvCpssHwPpSetRegField(devNum, regAddr, thrOff, fieldSize,
                threshold);
        }
    }
    return rc;
}

/**
* @internal cpssDxChPortPipGlobalBurstFifoThresholdsSet function
* @endinternal
*
* @brief   Set Burst FIFO Threshold for specific PIP priority.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      (APPLICABLE DEVICES Caelum; Aldrin; AC3X; Bobcat3)
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
*                                      3. For multi-pipe device:
*                                      bitmap must be set with at least one bit representing
*                                      pipe(s).  If a bit of non valid pipe is set then
*                                      function returns GT_BAD_PARAM. Value
*                                      CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
* @param[in] priority                 - the  to set the threshold.
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..2).
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..3).
* @param[in] threshold                - the  value. (number of packets)
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..0x3FF)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_OUT_OF_RANGE          - on out of range threshold
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalBurstFifoThresholdsSet
(
    IN GT_U8            devNum,
    IN GT_DATA_PATH_BMP dataPathBmp,
    IN GT_U32           priority,
    IN GT_U32           threshold
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId,
        cpssDxChPortPipGlobalBurstFifoThresholdsSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, dataPathBmp, priority, threshold));

    rc = internal_cpssDxChPortPipGlobalBurstFifoThresholdsSet(devNum,
        dataPathBmp, priority, threshold);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, dataPathBmp,
        priority, threshold));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipGlobalBurstFifoThresholdsGet function
* @endinternal
*
* @brief   Get Burst FIFO Threshold for specific PIP priority.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      (APPLICABLE DEVICES Caelum; Aldrin; AC3X; Bobcat3)
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
*                                      3. For multi-pipe device:
*                                      bitmap must be set with at least one bit representing
*                                      pipe(s). If a bit of non valid data path is set then
*                                      function returns GT_BAD_PARAM. Value
*                                      PSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
*                                      4. read only from first data path of the bitmap.
* @param[in] priority                 - the  to set the threshold.
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..2).
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..3).
*
* @param[out] thresholdPtr             - (pointer to)the threshold value. (number of packets)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalBurstFifoThresholdsGet
(
    IN GT_U8            devNum,
    IN GT_DATA_PATH_BMP dataPathBmp,
    IN GT_U32           priority,
    OUT GT_U32         *thresholdPtr
)

{
    GT_U32 regAddr;       /* register address */
    GT_U32 dmaIdx;
    GT_U32 mask;          /* mask of available datapathes */
    GT_U32 dpSize;
    GT_U32 rc;
    GT_U32 thrOff;
    GT_U32 isDataPathBmpApplicable; /* Is dataPathBmp applicable flag */

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    /* check sip5_10 support */
    PRV_CPSS_APPLICABLE_SIP_CHECK_MAC(devNum,CPSS_GEN_SIP_5_10_E);

    /* check the pip priority (0..3) as parameter (not as value) */
    CHECK_PARAM_PIP_PRIORITY_MAC(devNum,priority);

    CPSS_NULL_PTR_CHECK_MAC(thresholdPtr);

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        /* the FIFOs are in IA units.
           Number of IA units equal to number of processing pipes. */
        isDataPathBmpApplicable = (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes > 1) ? GT_TRUE : GT_FALSE;
    }
    else
    {
        /* the FIFOs are in RXDMA units. */
        isDataPathBmpApplicable = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.supportMultiDataPath;
    }

    dmaIdx = 0;
    if(isDataPathBmpApplicable
       && (dataPathBmp != CPSS_DATA_PATH_UNAWARE_MODE_CNS))
    {
        rc = pipGlobalBurstFifoThresholdDpMaskCreateCheck(devNum,
            dataPathBmp, &mask, &dpSize);
        if (rc != GT_OK)
        {
            return rc;
        }
        while(dmaIdx < dpSize)
        {
            if(mask & dataPathBmp & 0x1)
            {
                break;
            }
            dataPathBmp >>= 1;
            mask >>= 1;
            dmaIdx++;
        }
        if(dmaIdx == dpSize)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        if(priority < 2)
        {
            thrOff = (priority == 0) ? 0 : 16;
            regAddr = PRV_DXCH_REG1_UNIT_IA_INDEX_MAC(devNum, dmaIdx).
                PIPPrioThresholds0;
        }
        else
        {
            thrOff = (priority == 2) ? 0 : 16;
            regAddr = PRV_DXCH_REG1_UNIT_IA_INDEX_MAC(devNum, dmaIdx).
                PIPPrioThresholds1;
        }
        /* 16 bits per priority <PIP <Low/Mid/High/VeryHigh> Prio Threshold> */
        rc = prvCpssHwPpGetRegField(devNum, regAddr, thrOff, 16, thresholdPtr);
    }
    else /* (priority < 4) */
    {
        thrOff = priority * 10;
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_INDEX_MAC(devNum, dmaIdx).
            globalRxDMAConfigs.preIngrPrioritizationConfStatus.
            PIPPrioThresholds;
        /* 10 bits per priority <PIP <Low/Mid/High/VeryHigh> Prio Threshold> */
        rc = prvCpssHwPpGetRegField(devNum, regAddr, thrOff, 10, thresholdPtr);
    }
    return rc;
}

/**
* @internal cpssDxChPortPipGlobalBurstFifoThresholdsGet function
* @endinternal
*
* @brief   Get Burst FIFO Threshold for specific PIP priority.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      (APPLICABLE DEVICES Caelum; Aldrin; AC3X; Bobcat3)
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
*                                      3. For multi-pipe device:
*                                      bitmap must be set with at least one bit representing
*                                      pipe(s). If a bit of non valid data path is set then
*                                      function returns GT_BAD_PARAM. Value
*                                      PSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
*                                      4. read only from first data path of the bitmap.
* @param[in] priority                 - the  to set the threshold.
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..2).
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..3).
*
* @param[out] thresholdPtr             - (pointer to)the threshold value. (number of packets)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalBurstFifoThresholdsGet
(
    IN GT_U8            devNum,
    IN GT_DATA_PATH_BMP dataPathBmp,
    IN GT_U32           priority,
    OUT GT_U32         *thresholdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId,
        cpssDxChPortPipGlobalBurstFifoThresholdsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, dataPathBmp, priority,
        thresholdPtr));

    rc = internal_cpssDxChPortPipGlobalBurstFifoThresholdsGet(devNum,
        dataPathBmp, priority, thresholdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, dataPathBmp,
        priority, thresholdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal pipGlobalDropCounterGet function
* @endinternal
*
* @brief   Get PIP drop counter of specific PIP priority. (Global to the device)
*         NOTE: the counters are 'clear on read'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] priority                 - the  to set the threshold.
*                                      (APPLICABLE RANGES: 0..2)
*
* @param[out] counterPtr               - (pointer to) the counter value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS pipGlobalDropCounterGet
(
    IN GT_U8        devNum,
    IN GT_U32       priority,
    OUT GT_U64      *counterPtr
)
{
    GT_STATUS rc;         /* function return value */
    GT_U32 regAddrMsb,regAddrLsb;       /* register address */
    GT_U64 tmpCounter1,tmpCounter2;

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        if(priority == 3)/* low */
        {
            regAddrLsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPLowPriorityDropGlobalCntrLSb;
            regAddrMsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPLowPriorityDropGlobalCntrMsb;
        }
        else
        if(priority == 2)/* med */
        {
            regAddrLsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPMedPriorityDropGlobalCntrLsb;
            regAddrMsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPMedPriorityDropGlobalCntrMsb;
        }
        else
        if(priority == 1)/* high */
        {
            regAddrLsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPHighPriorityDropGlobalCntrLsb;
            regAddrMsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPHighPriorityDropGlobalCntrMsb;
        }
        else /* very high */
        {
            regAddrLsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPVeryHighPriorityDropGlobalCntrLsb;
            regAddrMsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPVeryHighPriorityDropGlobalCntrMsb;

        }
    }
    else
    {
        if(priority == 2)/* low */
        {
            regAddrLsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPLowPriorityDropGlobalCntrLSb;
            regAddrMsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPLowPriorityDropGlobalCntrMsb;
        }
        else
        if(priority == 1)/* med */
        {
            regAddrLsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPMedPriorityDropGlobalCntrLsb;
            regAddrMsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPMedPriorityDropGlobalCntrMsb;
        }
        else /* high */
        {
            regAddrLsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPHighPriorityDropGlobalCntrLsb;
            regAddrMsb = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                globalRxDMAConfigs.preIngrPrioritizationConfStatus.
                PIPHighPriorityDropGlobalCntrMsb;
        }
    }

    tmpCounter2.l[0] = 0;
    /* first read MSWord , than read LSWord */
    /* because read of LSWord clears the counter */
    rc =  prvCpssPortGroupsCounterSummary(devNum, regAddrMsb,0,32,
                        &tmpCounter2.l[1],/* get counter as 32 bits */
                        NULL);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssPortGroupsCounterSummary(devNum, regAddrLsb ,0,32,
                        NULL,
                        &tmpCounter1);/* get counter as 64 bits , because can be
                                         aggregation from several RXDMA units */
    if(rc != GT_OK)
    {
        return rc;
    }

    /* summary the counters */
    (*counterPtr)= prvCpssMathAdd64(tmpCounter1,tmpCounter2);

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortPipGlobalDropCounterGet function
* @endinternal
*
* @brief   Get all PIP drop counters (Global to the device)
*         NOTE: the counters are 'clear on read'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Falcon.
*
* @param[in] devNum                   - physical device number
*
* @param[out] countersArr[4]           - (pointer to) the counters of
*                                      - 3 priorities (0-high, 1-mid, 2-low) for Bobcat2,
*                                      Caelum, Aldrin, AC3X
*                                      - 4 priorities (0-very high, 1-high, 2-mid, 3-low) for
*                                      Bobcat3
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipGlobalDropCounterGet
(
    IN GT_U8        devNum,
    OUT GT_U64      countersArr[4]
)
{
    GT_STATUS rc;         /* function return value */
    GT_U32  regAddr;
    GT_U32  priority;
    GT_U32  maxPriority;

    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_FALCON_E);

    /* check sip5_10 support */
    PRV_CPSS_APPLICABLE_SIP_CHECK_MAC(devNum,CPSS_GEN_SIP_5_10_E);

    CPSS_NULL_PTR_CHECK_MAC(countersArr);

    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        /* This trigger needs to be activated before reading the
           PIP Priority Drop Global Counter (this is a trigger for all priority counters).
           This is a self cleared bit.
        */
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).globalRxDMAConfigs.
            preIngrPrioritizationConfStatus.PIPDropGlobalCounterTrigger;

        /* check that all port groups are ready */
        rc = prvCpssPortGroupBusyWait(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                    regAddr,0,/*bit0 - <Load All Global Drop Counters */
                    GT_TRUE);/* only to check the bit --> no 'busy wait' */
        if (rc != GT_OK)
        {
            return rc;
        }

        /* trigger the bit : <Load All Global Drop Counters > */
        rc = prvCpssHwPpSetRegField(devNum, regAddr,0 , 1, 1);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* wait that all port groups are ready */
        rc = prvCpssPortGroupBusyWait(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                    regAddr,0,/*bit0 - <Load All Global Drop Counters */
                    GT_FALSE);/* 'busy wait' on the bit*/
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        maxPriority = 4;
    }
    else
    {
        countersArr[3].l[0] = 0;
        countersArr[3].l[1] = 0;
        maxPriority = 3;
    }

    for(priority = 0 ; priority < maxPriority ; priority++)
    {
        rc = pipGlobalDropCounterGet(devNum,priority,&countersArr[priority]);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;

}
/**
* @internal cpssDxChPortPipGlobalDropCounterGet function
* @endinternal
*
* @brief   Get all PIP drop counters (Global to the device)
*         NOTE: the counters are 'clear on read'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Falcon.
*
* @param[in] devNum                   - physical device number
*
* @param[out] countersArr[4]           - (pointer to) the counters of
*                                      - 3 priorities (0-high, 1-mid, 2-low) for Bobcat2,
*                                      Caelum, Aldrin, AC3X
*                                      - 4 priorities (0-very high, 1-high, 2-mid, 3-low) for
*                                      Bobcat3
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalDropCounterGet
(
    IN GT_U8        devNum,
    OUT GT_U64      countersArr[4]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipGlobalDropCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, countersArr));

    rc = internal_cpssDxChPortPipGlobalDropCounterGet(devNum, countersArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, countersArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPipDropCounterGet function
* @endinternal
*
* @brief   Get PIP drop counter of specific port.
*         NOTE: the counters are 'clear on read'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] counterPtr               - (pointer to) the counter value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPipDropCounterGet
(
    IN GT_U8        devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U64      *counterPtr
)
{
    /* check that the device exists and supports the PIP feature */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
        CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    /* check sip5_10 support */
    PRV_CPSS_APPLICABLE_SIP_CHECK_MAC(devNum,CPSS_GEN_SIP_5_10_E);
    CPSS_NULL_PTR_CHECK_MAC(counterPtr);

    /* the counter supports only 32 bits */
    counterPtr->l[1] = 0;

    return pip_counterGetPerPort(devNum,portNum,&counterPtr->l[0]);
}

/**
* @internal cpssDxChPortPipDropCounterGet function
* @endinternal
*
* @brief   Get PIP drop counter of specific port.
*         NOTE: the counters are 'clear on read'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] counterPtr               - (pointer to) the counter value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipDropCounterGet
(
    IN GT_U8        devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U64      *counterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPipDropCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, counterPtr));

    rc = internal_cpssDxChPortPipDropCounterGet(devNum, portNum, counterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, counterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

