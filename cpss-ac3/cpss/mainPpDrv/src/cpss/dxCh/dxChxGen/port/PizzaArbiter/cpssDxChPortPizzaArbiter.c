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
* @file cpssDxChPortPizzaArbiter.c
*
* @brief CPSS implementation for pizza arbiter.
*
* @version   70
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE

#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/private/utils/prvCpssTimeRtUtils.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBobKResource.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBcat2Resource.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/TablePizzaArbiter/prvCpssDxChPortTablePizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>


extern GT_STATUS prvCpssDxChPortTablePizzaArbiterIfInit
(
    IN  GT_U8    devNum
);
extern GT_STATUS prvCpssDxChPortTablePizzaArbiterIfCheckSupport
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_SPEED_ENT     portSpeed
);
extern GT_STATUS prvCpssDxChPortTablePizzaArbiterIfConfigure
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_SPEED_ENT     portSpeed
);

extern GT_STATUS prvCpssDxChPortTablePizzaArbiterIfDevStateGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    OUT CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC *pizzaDeviceStatePtr
);



GT_BOOL pa_TimeTake = GT_FALSE;
GT_U32   prv_paTime_ms;
GT_U32   prv_paTime_us;

GT_STATUS prvCpssDxChPortPATimeTakeEnable
(
    GT_VOID
)
{
    pa_TimeTake = GT_TRUE;
    return GT_OK;
}

GT_STATUS prvCpssDxChPortPATimeTakeDisable
(
    GT_VOID
)
{
    pa_TimeTake = GT_FALSE;
    return GT_OK;
}


GT_STATUS prvCpssDxChPortPATimeDiffGet
(
    OUT GT_U32 *prv_paTime_msPtr,
    OUT GT_U32 *prv_paTime_usPtr
)
{
    if (pa_TimeTake == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    *prv_paTime_msPtr = prv_paTime_ms;
    *prv_paTime_usPtr = prv_paTime_us;
    return GT_OK;
}

/**
* @internal prvCpssDxChPortPizzaArbiterIfInit function
* @endinternal
*
* @brief   Pizza arbiter initialization in all Units where it's present
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note I forced to place this function here, because it needs number of port
*       group where CPU port is connected and there is just no more suitable
*       place.
*
*/
GT_STATUS prvCpssDxChPortPizzaArbiterIfInit
(
    IN  GT_U8                   devNum
)
{
    GT_STATUS   rc;                 /* return code */
    PRV_CPSS_GEN_PP_CONFIG_STC* devPtr; /* pointer to device to be processed*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_XCAT2_E | CPSS_FALCON_E);


    devPtr = PRV_CPSS_PP_MAC(devNum);

    if (devPtr->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E )
    {
        if (systemRecoveryInfo.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E)
        {
            rc = prvCpssDxChPortTablePizzaArbiterIfInit(devNum);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        return GT_OK;
    }
    else if (PRV_CPSS_SIP_5_CHECK_MAC(devNum)) /* &&      CPSS_PP_FAMILY_DXCH_BOBCAT2_E, */
    {
        rc = prvCpssDxChPortDynamicPizzaArbiterIfInit(devNum);
        if (rc != GT_OK)
        {
            return rc;
        }
        return GT_OK;
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChPortPizzaArbiterIfCheckSupport function
* @endinternal
*
* @brief   Check whether Pizza Arbiter can be configured to new port's i/f mode and speed
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portSpeed                - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortPizzaArbiterIfCheckSupport
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_SPEED_ENT     portSpeed
)
{
    GT_STATUS   rc;                 /* return code */
    PRV_CPSS_GEN_PP_CONFIG_STC* devPtr; /* pointer to device to be processed*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_XCAT2_E | CPSS_FALCON_E);


    devPtr = PRV_CPSS_PP_MAC(devNum);

    if (devPtr->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) /* &&      CPSS_PP_FAMILY_DXCH_BOBCAT2_E, */
    {
        rc = prvCpssDxChPortTablePizzaArbiterIfCheckSupport(devNum,portNum, portSpeed);
        if (rc != GT_OK)
        {
            return rc;
        }
        return GT_OK;
    }
    if (devPtr->devFamily >= CPSS_PP_FAMILY_DXCH_BOBCAT2_E) /* &&      CPSS_PP_FAMILY_DXCH_BOBCAT2_E, */
    {
        GT_U32 portSpeedInMBit;

        rc = prvCpssDxChPortDynamicPizzaArbiterSpeedConv(devNum,portNum,portSpeed,/*OUT*/&portSpeedInMBit);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChPortDynamicPizzaArbiterIfCheckSupport(devNum,portNum, portSpeedInMBit);
        if (rc != GT_OK)
        {
            return rc;
        }
        return GT_OK;
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);;
}



/**
* @internal prvCpssDxChPortPizzaArbiterIfConfigure function
* @endinternal
*
* @brief   Configure Pizza Arbiter according to new port's i/f mode and speed
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portSpeed                - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortPizzaArbiterIfConfigure
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_SPEED_ENT     portSpeed
)
{
    GT_STATUS   rc;                 /* return code */
    PRV_CPSS_GEN_PP_CONFIG_STC* devPtr; /* pointer to device to be processed*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_XCAT2_E | CPSS_FALCON_E);

    devPtr = PRV_CPSS_PP_MAC(devNum);

    if (devPtr->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) /* &&      CPSS_PP_FAMILY_DXCH_BOBCAT2_E, */
    {
        rc = prvCpssDxChPortTablePizzaArbiterIfConfigure(devNum,portNum, portSpeed);
        if (rc != GT_OK)
        {
            return rc;
        }
        return GT_OK;
    }
    if (devPtr->devFamily >= CPSS_PP_FAMILY_DXCH_BOBCAT2_E) /* &&      CPSS_PP_FAMILY_DXCH_BOBCAT2_E, */
    {
        GT_U32 portSpeedInMBit;
        GT_TIME timeStart = {0,0};

        if (pa_TimeTake == GT_TRUE)
        {
            rc = prvCpssOsTimeRTns(&timeStart);
            if (rc != GT_OK)
            {
                return rc;
            }
        }


        rc = prvCpssDxChPortDynamicPizzaArbiterSpeedConv(devNum,portNum,portSpeed,/*OUT*/&portSpeedInMBit);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChPortDynamicPizzaArbiterIfConfigure(devNum,portNum, portSpeedInMBit);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (pa_TimeTake)
        {
            prvCpssOsTimeRTDiff(timeStart, /*OUT*/&prv_paTime_ms, &prv_paTime_us);
        }

        return GT_OK;
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);;
}

/**
* @internal prvCpssDxChPortPizzaArbiterIfCCFCClientConfigure function
* @endinternal
*
* @brief   Configure Pizza Arbiter for CCFC port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] macNum                   - mac Num
*                                      txq       - txq number
* @param[in] portSpeed                - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortPizzaArbiterIfCCFCClientConfigure
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  macNum,
    IN  GT_U32                  txqNum,
    IN  CPSS_PORT_SPEED_ENT     portSpeed
)
{
    GT_STATUS   rc;                 /* return code */
    GT_U32 portSpeedInMBit;
    GT_TIME timeStart = {0,0};


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_FALCON_E);

    if (pa_TimeTake == GT_TRUE)
    {
        rc = prvCpssOsTimeRTns(&timeStart);
        if (rc != GT_OK)
        {
            return rc;
        }
    }


    rc = prvCpssDxChPortDynamicPizzaArbiterSpeedConv(devNum,GT_NA,portSpeed,/*OUT*/&portSpeedInMBit);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortDynamicPizzaArbiterIfCCFCClientConfigure(devNum,macNum,txqNum,portSpeedInMBit);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (pa_TimeTake)
    {
        prvCpssOsTimeRTDiff(timeStart, /*OUT*/&prv_paTime_ms, &prv_paTime_us);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortPizzaArbiterAllCCFCClientsConfigure function
* @endinternal
*
* @brief   Configure Pizza Arbiter for all CCFC ports - i.e. extended cascade ports
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortPizzaArbiterAllCCFCClientsConfigure
(
    IN  GT_U8                   devNum
)
{
    GT_STATUS                       rc;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_FALCON_E);

    for (portNum = 0; portNum < PRV_CPSS_MAX_MAC_PORTS_NUM_CNS; portNum++)
    {
        /* Get Port Mapping DataBase */
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
        if (portMapShadowPtr->valid == GT_FALSE )
        {
            continue;
        }
        if (portMapShadowPtr->portMap.isExtendedCascadePort == GT_FALSE)
        {
            continue;
        }

        remotePhyMacInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.remotePhyMacInfoArr[portNum];
        if(remotePhyMacInfoPtr == NULL)
        {
            continue;
        }

        if (remotePhyMacInfoPtr->connectedPhyMacInfo.internalFcTxqPort == CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS)
        {
            continue;
        }

        rc = prvCpssDxChPortPizzaArbiterIfCCFCClientConfigure(devNum,
                        portMapShadowPtr->portMap.macNum,
                        remotePhyMacInfoPtr->connectedPhyMacInfo.internalFcTxqPort,
                        CPSS_PORT_SPEED_1000_E);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}



/**
* @internal prvCpssDxChPortPizzaArbiterIfDelete function
* @endinternal
*
* @brief   Delete Pizza Arbiter
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortPizzaArbiterIfDelete
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum
)
{
    GT_STATUS   rc;                 /* return code */
    PRV_CPSS_GEN_PP_CONFIG_STC* devPtr; /* pointer to device to be processed*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_XCAT2_E | CPSS_FALCON_E);

    devPtr = PRV_CPSS_PP_MAC(devNum);

    if (devPtr->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        rc = prvCpssDxChPortTablePizzaArbiterIfDelete(devNum, portNum);
    }
    else if (devPtr->devFamily >= CPSS_PP_FAMILY_DXCH_BOBCAT2_E) /* &&      CPSS_PP_FAMILY_DXCH_BOBCAT2_E, */
    {
        rc = prvCpssDxChPortDynamicPizzaArbiterIfConfigure(devNum,portNum, 0);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }

    return rc;
}



/*******************************************************************************
* prvCpssDxChPortPizzaArbiterIfTxQClientConfigure
*
* DESCRIPTION:
*       Configure Pizza Arbiter for new special TXQ client (remote physical ports)
*
* APPLICABLE DEVICES:
*        Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum   - device number
*       portNum  - physical port number (not applicable CCFC client)
*       portSpeedMbps - port speed
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - wrong devNum, portNum
*       GT_FAIL         - on error
*       GT_NOT_SUPPORTED - not supported
*
* COMMENTS:
*
*******************************************************************************/
#if 0
GT_STATUS prvCpssDxChPortPizzaArbiterIfTxQClientConfigure
(
    IN  GT_U8                                     devNum,
    IN  PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_TYPE_ENT clientType,
    IN  GT_PHYSICAL_PORT_NUM                      portNum,
    IN  GT_U32                                    txq,
    IN  GT_U32                                    portSpeedMbps
)
{
    GT_STATUS   rc;                     /* return code */
    PRV_CPSS_GEN_PP_CONFIG_STC* devPtr; /* pionter to device to be processed*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_FALCON_E);

    devPtr = PRV_CPSS_PP_MAC(devNum);

    if (devPtr->devFamily >= CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
    {
        rc = prvCpssDxChPortDynamicPizzaArbiterIfTxQClientConfigure(devNum,portNum, 0);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }

    return rc;

}


/**
* @internal prvCpssDxChPortPizzaArbiterIfTxQClientDelete function
* @endinternal
*
* @brief   delete special TXQ client from Pizza Arbiter(remote physical ports)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] txq                      - clients' txq
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortPizzaArbiterIfTxQClientDelete
(
    IN  GT_U8                                     devNum,
    IN  GT_U32                                    txq
)
{
    GT_STATUS   rc;                     /* return code */
    PRV_CPSS_GEN_PP_CONFIG_STC* devPtr; /* pionter to device to be processed*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_FALCON_E);

    devPtr = PRV_CPSS_PP_MAC(devNum);

    if (devPtr->devFamily >= CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
    {
        rc = prvCpssDxChPortDynamicPizzaArbiterIfTxQClientDelete(devNum,txq);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }

    return rc;
}

#endif

/**
* @internal internal_cpssDxChPortPizzaArbiterDevStateInit function
* @endinternal
*
* @brief   Init the structure CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] pizzaDeviceStatePtr      - pointer to structure describing the state of each units.
* @param[in,out] pizzaDeviceStatePtr      - pointer to structure describing the state of each units.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on pizzaDeviceStatePtr is NULL
*/
static GT_STATUS internal_cpssDxChPortPizzaArbiterDevStateInit
(
    INOUT CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC *pizzaDeviceStatePtr
)
{
    if (NULL == pizzaDeviceStatePtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(pizzaDeviceStatePtr,0,sizeof(*pizzaDeviceStatePtr));
    return GT_OK;
}

/**
* @internal cpssDxChPortPizzaArbiterDevStateInit function
* @endinternal
*
* @brief   Init the structure CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] pizzaDeviceStatePtr      - pointer to structure describing the state of each units.
* @param[in,out] pizzaDeviceStatePtr      - pointer to structure describing the state of each units.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on pizzaDeviceStatePtr is NULL
*/
GT_STATUS cpssDxChPortPizzaArbiterDevStateInit
(
    INOUT CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC *pizzaDeviceStatePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPizzaArbiterDevStateInit);
     /*Must use zero level protection , do not remove this*/
    CPSS_ZERO_LEVEL_API_LOCK_MAC;
    CPSS_LOG_API_ENTER_MAC((funcId, pizzaDeviceStatePtr));

    rc = internal_cpssDxChPortPizzaArbiterDevStateInit(pizzaDeviceStatePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, pizzaDeviceStatePtr));
    CPSS_ZERO_LEVEL_API_UNLOCK_MAC;

    return rc;
}


/**
* @internal internal_cpssDxChPortPizzaArbiterDevStateGet function
* @endinternal
*
* @brief   Lion 2,3 Pizza Arbiter State which includes state of all
*         -
*         - state of slices
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupId              - number of port group
*
* @param[out] pizzaDeviceStatePtr      - pointer to structure describing the state of each units.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPizzaArbiterDevStateGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    OUT CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC *pizzaDeviceStatePtr
)
{
    GT_STATUS   rc;                 /* return code */
    PRV_CPSS_GEN_PP_CONFIG_STC* devPtr; /* pionter to device to be processed*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_XCAT2_E | CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(pizzaDeviceStatePtr);

    rc = cpssDxChPortPizzaArbiterDevStateInit(pizzaDeviceStatePtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    devPtr = PRV_CPSS_PP_MAC(devNum);

    if (devPtr->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) /* &&      CPSS_PP_FAMILY_DXCH_BOBCAT2_E, */
    {
        rc = prvCpssDxChPortTablePizzaArbiterIfDevStateGet(devNum,portGroupId, pizzaDeviceStatePtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        return GT_OK;
    }
    if (devPtr->devFamily >= CPSS_PP_FAMILY_DXCH_BOBCAT2_E) /* &&      CPSS_PP_FAMILY_DXCH_BOBCAT2_E, */
    {
        rc = prvCpssDxChPortDynamicPizzaArbiterIfDevStateGet(devNum,portGroupId, pizzaDeviceStatePtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        return GT_OK;
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);;
}

/**
* @internal cpssDxChPortPizzaArbiterDevStateGet function
* @endinternal
*
* @brief   Lion 2,3 Pizza Arbiter State which includes state of all
*         -
*         - state of slices
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupId              - number of port group
*
* @param[out] pizzaDeviceStatePtr      - pointer to structure describing the state of each units.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPizzaArbiterDevStateGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    OUT CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC *pizzaDeviceStatePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPizzaArbiterDevStateGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, pizzaDeviceStatePtr));

    rc = internal_cpssDxChPortPizzaArbiterDevStateGet(devNum, portGroupId, pizzaDeviceStatePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupId, pizzaDeviceStatePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPizzaArbiterIfConfigSet function
* @endinternal
*
* @brief   Configure Pizza Arbiter according to new port's i/f mode and speed
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portSpeed                - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NO_RESOURCE           - no bandwidth to supply channel speed
* @retval GT_NOT_SUPPORTED         - not supported
*/
static GT_STATUS internal_cpssDxChPortPizzaArbiterIfConfigSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_SPEED_ENT     portSpeed
)
{
    GT_STATUS rc;
    CPSS_DXCH_DETAILED_PORT_MAP_STC       *portMapShadowPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E| CPSS_XCAT2_E | CPSS_FALCON_E);

    /* Get Port Mapping DataBase */
    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
    if (portMapShadowPtr->valid == GT_FALSE )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((portSpeed < CPSS_PORT_SPEED_10_E) || (portSpeed >= CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortPizzaArbiterIfConfigure(devNum, portNum, portSpeed);

    return rc;
}

/**
* @internal cpssDxChPortPizzaArbiterIfConfigSet function
* @endinternal
*
* @brief   Configure Pizza Arbiter according to new port's i/f mode and speed
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portSpeed                - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NO_RESOURCE           - no bandwidth to supply channel speed
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS cpssDxChPortPizzaArbiterIfConfigSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_SPEED_ENT     portSpeed
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPizzaArbiterIfConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portSpeed));

    rc = internal_cpssDxChPortPizzaArbiterIfConfigSet(devNum, portNum, portSpeed);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portSpeed));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortResourcesConfigSet function
* @endinternal
*
* @brief   Caelum port resource configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port interface mode
* @param[in] speed                    - port interface speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device, ifMode, speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortResourcesConfigSet
(
    IN GT_U8                           devNum,
    IN GT_PHYSICAL_PORT_NUM            portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS            rc;
    CPSS_DXCH_DETAILED_PORT_MAP_STC       *portMapShadowPtr;
    GT_U32               regAddr;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                            | CPSS_LION_E| CPSS_LION2_E | CPSS_XCAT2_E);

    if (CPSS_PP_FAMILY_DXCH_XCAT3_E != PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
            /* Get Port Mapping DataBase */
                rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
                if(rc != GT_OK)
                {
                        return rc;
                }
                if (portMapShadowPtr->valid == GT_FALSE )
                {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
        }
    if((ifMode >= CPSS_PORT_INTERFACE_MODE_NA_E) || (ifMode < CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if((speed < CPSS_PORT_SPEED_10_E) || (speed >= CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum) || PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum) ||
       PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum) || PRV_CPSS_DXCH_BOBCAT3_CHECK_MAC(devNum) ||
       PRV_CPSS_DXCH_ALDRIN2_CHECK_MAC(devNum) || PRV_CPSS_DXCH_FALCON_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChCaelumPortResourcesConfig(devNum, portNum, ifMode, speed);
    }
    else if (CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);
        /* Set FCA interface width */
        rc = prvCpssDxChPortXcat3FcaBusWidthSet(devNum, portNum, CPSS_PORT_SPEED_1000_E);
        if (rc != GT_OK)
        {
                       return rc;
        }

        /* Set Count External FC En enable */
        if(PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum,portNum) == PRV_CPSS_GE_PORT_GE_ONLY_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portNum].macRegsPerType[PRV_CPSS_PORT_GE_E].macCtrl4;
            rc =  prvCpssHwPpSetRegField(devNum, regAddr, 8, 1, 1);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else
    {
        rc = prvCpssDxChBcat2PortResourcesConfig(devNum, portNum, ifMode, speed, GT_TRUE);
    }

    return rc;
}

/**
* @internal cpssDxChPortResourcesConfigSet function
* @endinternal
*
* @brief   Caelum port resource configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port interface mode
* @param[in] speed                    - port interface speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device, ifMode, speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortResourcesConfigSet
(
    IN GT_U8                           devNum,
    IN GT_PHYSICAL_PORT_NUM            portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortResourcesConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ifMode, speed));

    rc = internal_cpssDxChPortResourcesConfigSet(devNum, portNum, ifMode, speed);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ifMode, speed));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}




