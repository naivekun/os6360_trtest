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
* @file prvCpssDxChPortDynamicPAPortSpeedDB.c
*
* @brief bobcat2 and higher dynamic (algorithmic) pizza arbiter Port Speed DB
*
* @version   4
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPAPortSpeedDB.h>




GT_STATUS prvCpssDxChPortDynamicPAPortSpeedDBInit
(
    IN    GT_U8    devNum
)
{
    GT_STATUS rc;
    GT_PHYSICAL_PORT_NUM i;
    PRV_CPSS_DXCH_PORT_PA_PORT_DB_STC *portDBPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_FALCON_E);


    portDBPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paPortDB;

    portDBPtr->numInitPorts = 0;
    for (i = 0 ; i < PRV_CPSS_DXCH_PORT_PA_DB_MAX_PORTS_NUM_CNS; i++)
    {
        portDBPtr->prv_portDB[i].physicalPortNum = i;
        portDBPtr->prv_portDB[i].isInitilized = GT_FALSE;
        portDBPtr->prv_portDB[i].portSpeedInMBit        = 0;
        portDBPtr->prv_portDB[i].ilknChannelSpeedInMBit = 0;
    }
    for (i = 0 ;
          i < (sizeof(portDBPtr->txFifoSpeedFactorArr)/sizeof(portDBPtr->txFifoSpeedFactorArr[0]));
          i++)
    {
        portDBPtr->txFifoSpeedFactorArr[i] = 100; /* 100 percent */
    }

    rc = prvCpssDxChPortDynamicPAIlknSpeedResolutionSet(devNum,40,0);/* default resolution, 0 bw, the IF is still not initialized */
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

GT_STATUS prvCpssDxChPortDynamicPAPortSpeedDBSet
(
    IN    GT_U8                       devNum,
    IN    GT_PHYSICAL_PORT_NUM        portNum,
    IN    GT_U32                      speedInMBit
)
{
    PRV_CPSS_DXCH_PORT_PA_PORT_DB_STC *portDBPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_FALCON_E);


    portDBPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paPortDB;
    if (portNum >= PRV_CPSS_DXCH_PORT_PA_DB_MAX_PORTS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    if (speedInMBit > 0)
    {
        portDBPtr->prv_portDB[portNum].isInitilized = GT_TRUE;
        portDBPtr->prv_portDB[portNum].portSpeedInMBit  = speedInMBit;
        portDBPtr->numInitPorts ++;
    }
    else
    {
        portDBPtr->prv_portDB[portNum].isInitilized = GT_FALSE;
        portDBPtr->prv_portDB[portNum].portSpeedInMBit  = 0;
        portDBPtr->numInitPorts --;
    }

    return GT_OK;
}

GT_STATUS prvCpssDxChPortDynamicPAPortSpeedDBGet
(
    IN    GT_U8                      devNum,
    IN    GT_PHYSICAL_PORT_NUM       portNum,
    OUT   GT_BOOL                   *isInitilizedPtr,
    OUT   GT_U32                    *speedInMBitPtr
)
{
    PRV_CPSS_DXCH_PORT_PA_PORT_DB_STC *portDBPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(isInitilizedPtr);
    CPSS_NULL_PTR_CHECK_MAC(speedInMBitPtr);

    portDBPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paPortDB;

    if (portNum >= PRV_CPSS_DXCH_PORT_PA_DB_MAX_PORTS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *isInitilizedPtr = portDBPtr->prv_portDB[portNum].isInitilized;;
    *speedInMBitPtr  = portDBPtr->prv_portDB[portNum].portSpeedInMBit;
    return GT_OK;
}

/**
* @internal prvCpssDxChPortDynamicPAPortTxFifoSpeedFactorDBSet function
* @endinternal
*
* @brief   Set TX_FIFO Pizza Arbiter Channel Speed factor.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*                                       mapped to TX_FIFO Pizza Arbiter Channel
* @param[in] speedFactor              - Speed Factor in percent
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortDynamicPAPortTxFifoSpeedFactorDBSet
(
    IN    GT_U8                      devNum,
    IN    GT_PHYSICAL_PORT_NUM       portNum,
    IN    GT_U32                     speedFactor
)
{
    GT_STATUS                         rc;                                                                  \
    PRV_CPSS_DXCH_PORT_PA_PORT_DB_STC *portDBPtr;
    GT_U32                            portMacNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(
        devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
        | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
        | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_FALCON_E);

    if (speedFactor > 0xFFFF)
    {
        /* DB uses 16 bits only for this value */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    portDBPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paPortDB;

    rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(
        devNum, portNum, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &portMacNum);
    if (GT_OK != rc)
    {
        return rc;
    }

    portDBPtr->txFifoSpeedFactorArr[portMacNum] = (GT_U16)speedFactor;
    return GT_OK;
}

/**
* @internal prvCpssDxChPortDynamicPAPortTxFifoSpeedFactorDBGet function
* @endinternal
*
* @brief   Get TX_FIFO Pizza Arbiter Channel Speed factor.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*                                       mapped to TX_FIFO Pizza Arbiter Channel
* @param[out] speedFactor             - (pointer to)Speed Factor in percent
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortDynamicPAPortTxFifoSpeedFactorDBGet
(
    IN    GT_U8                      devNum,
    IN    GT_PHYSICAL_PORT_NUM       portNum,
    OUT   GT_U32                     *speedFactorPtr
)
{
    GT_STATUS                         rc;                                                                  \
    PRV_CPSS_DXCH_PORT_PA_PORT_DB_STC *portDBPtr;
    GT_U32                            portMacNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(
        devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
        | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
        | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(speedFactorPtr);

    portDBPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paPortDB;

    rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(
        devNum, portNum, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &portMacNum);
    if (GT_OK != rc)
    {
        return rc;
    }

    *speedFactorPtr = portDBPtr->txFifoSpeedFactorArr[portMacNum];
    return GT_OK;
}

/**
* @internal prvCpssDxChPortDynamicPAPortTxFifoAdjustedSpeedDBGet function
* @endinternal
*
* @brief   Calculate TX_FIFO Pizza Arbiter Channel Speed by Port Speed and Channel Speed Factor.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*                                       mapped to TX_FIFO Pizza Arbiter Channel
* @param[out] isInitilizedPtr         - (pointer to)GT_TRUE - initialized, GT_FALSE - not initialized.
* @param[out] speedInMBitPtr          - (pointer to)Channel Speed in MBit/Second
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortDynamicPAPortTxFifoAdjustedSpeedDBGet
(
    IN    GT_U8                      devNum,
    IN    GT_PHYSICAL_PORT_NUM       portNum,
    OUT   GT_BOOL                    *isInitilizedPtr,
    OUT   GT_U32                     *speedInMBitPtr
)
{
    GT_STATUS                         rc;                                                                  \
    PRV_CPSS_DXCH_PORT_PA_PORT_DB_STC *portDBPtr;
    GT_U32                            portMacNum;
    GT_U32                            speedFactor;
    GT_U32                            speedInMBit;
    GT_U32                            resolution;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(
        devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
        | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
        | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(isInitilizedPtr);
    CPSS_NULL_PTR_CHECK_MAC(speedInMBitPtr);
    if (portNum >= PRV_CPSS_DXCH_PORT_PA_DB_MAX_PORTS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    portDBPtr  = &PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paPortDB;
    resolution = PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paMinSliceResolutionMbps;

    *isInitilizedPtr = portDBPtr->prv_portDB[portNum].isInitilized;
    if (*isInitilizedPtr == GT_FALSE)
    {
        *speedInMBitPtr = 0;
        return GT_OK;
    }

    speedInMBit  = portDBPtr->prv_portDB[portNum].portSpeedInMBit;

    rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(
        devNum, portNum, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &portMacNum);
    if (GT_OK != rc)
    {
        /* port without MAC - return original speed */
        *speedInMBitPtr  = speedInMBit;
        return GT_OK;
    }

    speedFactor  = portDBPtr->txFifoSpeedFactorArr[portMacNum];

    /* apply factor (in percent) to speed */
    speedInMBit = ((speedInMBit * speedFactor) / 100);

    /* adjust speed by resolution */
    if (resolution != 0)
    {
        speedInMBit += ((resolution + 1) / 2);
        speedInMBit -= (speedInMBit % resolution);
    }
    *speedInMBitPtr  = speedInMBit;
    return GT_OK;
}

GT_STATUS prvCpssDxChPortDynamicPAIlknChannelSpeedDBSet
(
    IN    GT_U8                       devNum,
    IN    GT_PHYSICAL_PORT_NUM        portNum,
    IN    GT_U32                      speedInMBit
)
{
    PRV_CPSS_DXCH_PORT_PA_PORT_DB_STC *portDBPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_FALCON_E);


    portDBPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paPortDB;
    if (portNum >= PRV_CPSS_DXCH_PORT_PA_DB_MAX_PORTS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    if (speedInMBit > 0)
    {
        portDBPtr->prv_portDB[portNum].isInitilized           = GT_TRUE;
        portDBPtr->prv_portDB[portNum].ilknChannelSpeedInMBit = speedInMBit;
        portDBPtr->numInitPorts ++;
    }
    else
    {
        portDBPtr->prv_portDB[portNum].isInitilized           = GT_FALSE;
        portDBPtr->prv_portDB[portNum].ilknChannelSpeedInMBit = 0;
        portDBPtr->numInitPorts --;
    }

    return GT_OK;
}

GT_STATUS prvCpssDxChPortDynamicPAIlknChannelSpeedDBGet
(
    IN    GT_U8                      devNum,
    IN    GT_PHYSICAL_PORT_NUM       portNum,
    OUT   GT_BOOL                   *isInitilizedPtr,
    OUT   GT_U32                    *speedInMBitPtr
)
{
    PRV_CPSS_DXCH_PORT_PA_PORT_DB_STC *portDBPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(isInitilizedPtr);
    CPSS_NULL_PTR_CHECK_MAC(speedInMBitPtr);

    portDBPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paPortDB;

    if (portNum >= PRV_CPSS_DXCH_PORT_PA_DB_MAX_PORTS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *isInitilizedPtr = portDBPtr->prv_portDB[portNum].isInitilized;;
    *speedInMBitPtr  = portDBPtr->prv_portDB[portNum].ilknChannelSpeedInMBit;
    return GT_OK;
}


GT_STATUS prvCpssDxChPortDynamicPAIlknSpeedResolutionSet
(
    IN    GT_U8                       devNum,
    IN    GT_U32                      speedResulutionMBps,
    IN    GT_U32                      ilknIFnaxBWMbps
)
{
    PRV_CPSS_DXCH_PORT_PA_PORT_DB_STC *portDBPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);


    portDBPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paPortDB;
    portDBPtr->ilknSpeedResolutionMbps = speedResulutionMBps; /* default resolution */
    portDBPtr->ilknIFnaxBWMbps         = ilknIFnaxBWMbps;

    return GT_OK;
}


GT_STATUS prvCpssDxChPortDynamicPAIlknSpeedResolutionGet
(
    IN    GT_U8                       devNum,
    OUT   GT_U32                     *speedResulutionMBpsPtr,
    OUT   GT_U32                     *ilknIFnaxBWMbpsPtr
)
{
    PRV_CPSS_DXCH_PORT_PA_PORT_DB_STC *portDBPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(speedResulutionMBpsPtr);
    CPSS_NULL_PTR_CHECK_MAC(ilknIFnaxBWMbpsPtr);

    portDBPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paPortDB;
    *speedResulutionMBpsPtr = portDBPtr->ilknSpeedResolutionMbps;
    *ilknIFnaxBWMbpsPtr     = portDBPtr->ilknIFnaxBWMbps;
    return GT_OK;
}

/*---------------------------------------------------------------------*
 *   PA client BW database for clinets having fixed BW (provisioning)  *
 *      operations:                                                    *
 *          Init                                                       *
 *          Add/Update                                                 *
 *          Remove                                                     *
 *  example (on base of TM):                                           *
 *       TxQ    64    25000                                            *
 *       TxDMA  73    25000                                            *
 *       TxFIFO 73    25000                                            *
 *---------------------------------------------------------------------*/

GT_STATUS prvCpssDxChPortDynamicPAClientBWListInit
(
    IN GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PA_CLIENT_BW_LIST_STC *listPtr;
    GT_U32 i;

    listPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paPortDB.paClientBWList;


    listPtr->len = 0;
    listPtr->size = sizeof(listPtr->arr)/sizeof(listPtr->arr[0]);
    for (i = 0 ; i < listPtr->size; i++)
    {
        listPtr->arr[i].unitType = CPSS_DXCH_PA_UNIT_UNDEFINED_E;
        listPtr->arr[i].clinetId = BAD_CLIENT_ID;
        listPtr->arr[i].bwMbps   = 0;
    }
    return GT_OK;
}



GT_STATUS prvCpssDxChPortDynamicPAClientBWListFind
(
    IN   GT_U8 devNum,
    IN   CPSS_DXCH_PA_UNIT_ENT unitType,
    IN   GT_U32                clinetId,
    OUT  PRV_CPSS_DXCH_PA_SINGLE_CLIENT_BW_STC **clientPtrPtr
)
{
    GT_U32 i;
    PRV_CPSS_DXCH_PA_CLIENT_BW_LIST_STC *listPtr;


    listPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paPortDB.paClientBWList;


    CPSS_NULL_PTR_CHECK_MAC(clientPtrPtr);
    for (i = 0 ; i < listPtr->len; i++)
    {
        if (listPtr->arr[i].unitType == unitType && listPtr->arr[i].clinetId == clinetId)
        {
            *clientPtrPtr = &listPtr->arr[i];
            return GT_OK;
        }
    }
    *clientPtrPtr = (PRV_CPSS_DXCH_PA_SINGLE_CLIENT_BW_STC *)NULL;
    return GT_OK;
}


GT_STATUS prvCpssDxChPortDynamicPAClientBWListUpdate
(
    IN   GT_U8 devNum,
    IN   CPSS_DXCH_PA_UNIT_ENT unitType,
    IN   GT_U32                clientId,
    IN   GT_U32                bwMbps
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PA_SINGLE_CLIENT_BW_STC *clientPtr;
    PRV_CPSS_DXCH_PA_CLIENT_BW_LIST_STC *listPtr;


    listPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paPortDB.paClientBWList;


    if (bwMbps == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG); /* call remove */
    }
    rc = prvCpssDxChPortDynamicPAClientBWListFind(devNum,unitType,clientId,/*OUT*/&clientPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (clientPtr != NULL)  /* found , already exists ! just update */
    {
        clientPtr->bwMbps = bwMbps;
        return GT_OK;
    }
    /* not found, append */
    if (listPtr->len == listPtr->size)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG); /* no place !*/
    }
    listPtr->arr[listPtr->len].unitType = unitType;
    listPtr->arr[listPtr->len].clinetId = clientId;
    listPtr->arr[listPtr->len].bwMbps   = bwMbps;
    listPtr->len++;
    return GT_OK;
}


GT_STATUS prvCpssDxChPortDynamicPAClientBWListDelete
(
    IN   GT_U8                 devNum,
    IN   CPSS_DXCH_PA_UNIT_ENT unitType,
    IN   GT_U32                clientId
)
{
    GT_STATUS rc;
    GT_U32 i;
    PRV_CPSS_DXCH_PA_SINGLE_CLIENT_BW_STC *clientPtr;
    GT_U32 idx;
    PRV_CPSS_DXCH_PA_CLIENT_BW_LIST_STC *listPtr;


    listPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paPortDB.paClientBWList;


    rc = prvCpssDxChPortDynamicPAClientBWListFind(devNum,unitType,clientId,/*OUT*/&clientPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (clientPtr == NULL)  /* not found  */
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    idx = (GT_U32)(clientPtr - &listPtr->arr[0]);
    for (i = idx; i<listPtr->len-1; i++)
    {
        listPtr->arr[i] = listPtr->arr[i+1];
    }
    listPtr->len--;
    return GT_OK;
}


GT_STATUS prvCpssDxChPortDynamicPAClientBWGet
(
    IN   GT_U8                 devNum,
    IN   CPSS_DXCH_PA_UNIT_ENT unitType,
    IN   GT_U32                clientId,
    OUT  GT_U32               *bwMbpsPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PA_SINGLE_CLIENT_BW_STC *clientPtr;

    CPSS_NULL_PTR_CHECK_MAC(bwMbpsPtr)
    rc = prvCpssDxChPortDynamicPAClientBWListFind(devNum,unitType,clientId,/*OUT*/&clientPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (clientPtr == NULL)  /* not found  */
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    *bwMbpsPtr = clientPtr->bwMbps;
    return GT_OK;
}


GT_U32 prvCpssDxChPortDynamicPAClientBWListLenGet
(
    IN   GT_U8                 devNum
)
{
    PRV_CPSS_DXCH_PA_CLIENT_BW_LIST_STC *listPtr;

    listPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paPortDB.paClientBWList;


    return listPtr->len;
}

PRV_CPSS_DXCH_PA_SINGLE_CLIENT_BW_STC * prvCpssDxChPortDynamicPAClientBWListEntryByIdxGet
(
    IN   GT_U8                 devNum,
    IN   GT_U32                idx
)
{
    PRV_CPSS_DXCH_PA_CLIENT_BW_LIST_STC *listPtr;

    listPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paPortDB.paClientBWList;

    if (idx >= listPtr->len)
    {
        return (PRV_CPSS_DXCH_PA_SINGLE_CLIENT_BW_STC*)NULL;
    }

    return &listPtr->arr[idx];
}

/*----------------------------------------------------------------------------*
 *   PA special TXQ client group (used for extender-cascade-ports clients)    *
 *          Init (groupId)                                                    *
 *          Add                                                               *
 *          Remove                                                            *
 *  example :                                                                 *
 *       type    phys Txq  BWMbps                                             *
 *      -------                                                               *
 *       ccfc    xxx  70   1000                                               *
 *       remote  101  51   1000                                               *
 *       remote  102  51   1000                                               *
 *----------------------------------------------------------------------------*/
#define BAD_VALUE (GT_U32)(~0)
GT_STATUS prvDynamicPATxQClientGroupClientClear
(
    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_STC * ptr
)
{
    CPSS_NULL_PTR_CHECK_MAC(ptr);

    ptr->clientType = PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_UNDEFINED_E;
    ptr->physicalPortNum = BAD_VALUE;
    ptr->speedMbps       = 0;
    ptr->txqPort         = BAD_VALUE;
    return GT_OK;
}


GT_STATUS prvDynamicPATxQClientGroupInit
(
    IN   PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC *groupPtr,
    IN   GT_U32 groupId
)
{
    GT_U32 i;
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(groupPtr);

    groupPtr->groupId = groupId;
    groupPtr->groupSpeedMbps = 0;
    groupPtr->tmp_groupTxqClient = BAD_VALUE;
    groupPtr->isExtendedCascadeExist = GT_FALSE; /* still there is no extended-cascade port */
    groupPtr->clientListSize = sizeof(groupPtr->clientIdList)/sizeof(groupPtr->clientIdList[0]);
    groupPtr->clientListLen = 0;
    for (i = 0 ; i < groupPtr->clientListSize; i++)
    {
        rc = prvDynamicPATxQClientGroupClientClear(&groupPtr->clientIdList[i]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

GT_STATUS prvDynamicPATxQClientGroupClientFind
(
    IN   PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC *groupPtr,
    IN   GT_U32                                      txqPort,
    OUT  GT_U32                                     *clientIdxPtr,
    OUT  PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_STC      **clientPtrPtr
)
{
    GT_U32 i;

    CPSS_NULL_PTR_CHECK_MAC(groupPtr);
    CPSS_NULL_PTR_CHECK_MAC(clientIdxPtr);
    CPSS_NULL_PTR_CHECK_MAC(clientPtrPtr);


    for (i = 0 ; i < groupPtr->clientListLen; i++)
    {
        if (groupPtr->clientIdList[i].clientType != PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_UNDEFINED_E)
        {
            if (groupPtr->clientIdList[i].txqPort == txqPort)
            {
                *clientIdxPtr = i;
                *clientPtrPtr = &groupPtr->clientIdList[i];
                return GT_OK;
            }
        }
    }
    *clientIdxPtr = BAD_VALUE;
    *clientPtrPtr = (PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_STC *)NULL;
    return GT_OK;
}


GT_STATUS prvDynamicPATxQClientGroupClientAdd
(
    IN   PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC *groupPtr,
    IN   PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_TYPE_ENT   clientType,
    IN   GT_PHYSICAL_PORT_NUM                        physicalPortNum,
    IN   GT_U32                                      txqPort,
    IN   GT_U32                                      speedMbps
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_STC *clientPtr;
    GT_U32                                clientIdx;

    CPSS_NULL_PTR_CHECK_MAC(groupPtr);

    rc = prvDynamicPATxQClientGroupClientFind(groupPtr,txqPort,/*OUT*/&clientIdx,&clientPtr);
    if (GT_OK != rc)
    {
        return rc;
    }
    if (clientPtr != NULL)
    {
        /* verify data */
        if (clientPtr->clientType != clientType)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        /* client exists , but changed the speed */
        if (clientPtr->speedMbps != speedMbps)
        {
            groupPtr->groupSpeedMbps += speedMbps;
            groupPtr->groupSpeedMbps -= clientPtr->speedMbps;
            clientPtr->speedMbps = speedMbps;
        }
        return GT_OK;
    }
    if (groupPtr->clientListLen >= groupPtr->clientListSize)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }
    /* add new client */
    groupPtr->clientIdList[groupPtr->clientListLen].clientType      = clientType;
    groupPtr->clientIdList[groupPtr->clientListLen].physicalPortNum = physicalPortNum;
    groupPtr->clientIdList[groupPtr->clientListLen].txqPort         = txqPort;
    groupPtr->clientIdList[groupPtr->clientListLen].speedMbps       = speedMbps;
    groupPtr->clientListLen++;

    groupPtr->groupSpeedMbps += speedMbps;
    return GT_OK;
}


GT_STATUS prvDynamicPATxQClientGroupClientDelete
(
    IN   PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC *groupPtr,
    IN   GT_U32                                      txqPort
)
{
    GT_U32 idx;
    GT_STATUS rc;
    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_STC *clientPtr;
    GT_U32                                clientIdx;

    CPSS_NULL_PTR_CHECK_MAC(groupPtr);

    rc = prvDynamicPATxQClientGroupClientFind(groupPtr,txqPort,/*OUT*/&clientIdx,&clientPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (clientPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    /* store aside client speed */
    groupPtr->groupSpeedMbps -= clientPtr->speedMbps;
    /* move all client after the client to be deleted forward */
    for (idx = clientIdx; idx < groupPtr->clientListLen - 1; idx++)
    {
        groupPtr->clientIdList[idx] = groupPtr->clientIdList[groupPtr->clientListLen];
    }
    rc = prvDynamicPATxQClientGroupClientClear(&groupPtr->clientIdList[idx]);
    if (rc != GT_OK)
    {
        return rc;
    }

    groupPtr->clientListLen--;
    return GT_OK;
}


/*-----------------------------------------------------------*/
/* List of groups                                            */
/*-----------------------------------------------------------*/
GT_STATUS prvDynamicPATxQClientGroupListInit
(
    IN   PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_DB_STC *groupListPtr
)
{
    GT_U32 i;
    GT_STATUS rc;

    groupListPtr->listSize = sizeof(groupListPtr->groupIdPlaceList)/sizeof(groupListPtr->groupIdPlaceList[0]);
    groupListPtr->listLen = 0;
    for (i = 0 ; i < groupListPtr->listSize; i++)
    {
        groupListPtr->groupIdPlaceList[i].groupId = BAD_VALUE;
        groupListPtr->groupIdPlaceList[i].place   = BAD_VALUE;
        rc = prvDynamicPATxQClientGroupInit(&groupListPtr->groupDB[i],BAD_VALUE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

GT_STATUS prvDynamicPATxQClientGroupListPlaceFind
(
    IN   PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_DB_STC *groupListPtr,
    IN   GT_U32 groupId,
    OUT  GT_BOOL *isFoundPtr,
    OUT  GT_U32  *placeInListPtr,
    OUT  GT_U32  *placeInDbPtr
)
{
    GT_U32 i;
    GT_U32 place;

    CPSS_NULL_PTR_CHECK_MAC(groupListPtr);
    CPSS_NULL_PTR_CHECK_MAC(isFoundPtr);
    CPSS_NULL_PTR_CHECK_MAC(placeInListPtr);
    CPSS_NULL_PTR_CHECK_MAC(placeInDbPtr);

    *isFoundPtr     = GT_FALSE;
    *placeInListPtr = BAD_VALUE;
    *placeInDbPtr   = BAD_VALUE;

    for (i = 0 ; i < groupListPtr->listLen; i++)
    {
        if (groupListPtr->groupIdPlaceList[i].groupId == groupId)
        {
            place = groupListPtr->groupIdPlaceList[i].place;
            /* vallidate DB that found */
            if (groupListPtr->groupDB[place].groupId != groupId)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            *isFoundPtr = GT_TRUE;
            *placeInListPtr = i;
            *placeInDbPtr   = place;
            return GT_OK;
        }
    }
    /* not found */
    return GT_OK;
}

GT_STATUS prvDynamicPATxQClientGroupListGet
(
    IN   PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_DB_STC *groupListPtr,
    IN   GT_U32 groupId,
    OUT  PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC **groupPtrPtr
)
{
    GT_STATUS rc;
    GT_BOOL isFound;
    GT_U32  placeInList;
    GT_U32  placeInDb;

    CPSS_NULL_PTR_CHECK_MAC(groupListPtr);
    CPSS_NULL_PTR_CHECK_MAC(groupPtrPtr);

    *groupPtrPtr = (PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC *)NULL;
    /* find at groupId x place  list */
    rc = prvDynamicPATxQClientGroupListPlaceFind(groupListPtr,groupId,/*OUT*/&isFound,&placeInList,&placeInDb);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (isFound == GT_TRUE)
    {
       *groupPtrPtr = &groupListPtr->groupDB[placeInDb];
    }
    return GT_OK;
}

GT_STATUS prvDynamicPATxQClientGroupListByIdxGet
(
    IN   PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_DB_STC *groupListPtr,
    IN   GT_U32 idx,
    OUT  PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC **groupPtrPtr
)
{
    GT_U32  placeInDb;

    CPSS_NULL_PTR_CHECK_MAC(groupListPtr);
    CPSS_NULL_PTR_CHECK_MAC(groupPtrPtr);

    *groupPtrPtr = (PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC *)NULL;
    /* find at groupId x place  list */
    if (idx >= groupListPtr->listLen)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    placeInDb = groupListPtr->groupIdPlaceList[idx].place;
    *groupPtrPtr = &groupListPtr->groupDB[placeInDb];
    return GT_OK;
}


GT_STATUS prvDynamicPATxQClientGroupListAdd
(
    IN   PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_DB_STC *groupListPtr,
    IN   GT_U32 groupId
)
{
    GT_STATUS rc;
    GT_U32 i;
    GT_U32 place;
    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC *groupPtr;


    CPSS_NULL_PTR_CHECK_MAC(groupListPtr);

    rc = prvDynamicPATxQClientGroupListGet(groupListPtr,groupId,/*OUT*/&groupPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (groupPtr != (PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC*)NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG); /* alredy exist :  error ???*/
    }
    if (groupListPtr->listLen >= groupListPtr->listSize)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }
    /* find empty place in groupDB */
    for (i = 0 ; i < groupListPtr->listSize; i++)
    {
        if (groupListPtr->groupIdPlaceList[i].groupId == BAD_VALUE) /* found */
        {
            place = i;
            /* append to list of groupId x place */
            groupListPtr->groupIdPlaceList[groupListPtr->listLen].groupId = groupId;
            groupListPtr->groupIdPlaceList[groupListPtr->listLen].place   = i;
            groupListPtr->listLen++;
            /* init group */
            rc = prvDynamicPATxQClientGroupInit(&groupListPtr->groupDB[place],groupId);
            if (rc != GT_OK)
            {
                return rc;
            }
            return GT_OK;
        }
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);  /* something wrong : there shall be empty place */
}


GT_STATUS prvDynamicPATxQClientGroupListDelete
(
    IN   PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_DB_STC *groupListPtr,
    IN   GT_U32 groupId
)
{
    GT_STATUS rc;
    GT_U32 i;
    GT_BOOL isFound;
    GT_U32  placeInList;
    GT_U32  placeInDb;
    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC *groupPtr;


    CPSS_NULL_PTR_CHECK_MAC(groupListPtr);

    /* find at groupId x place  list */
    rc = prvDynamicPATxQClientGroupListPlaceFind(groupListPtr,groupId,/*OUT*/&isFound,&placeInList,&placeInDb);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (isFound == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    groupPtr = &groupListPtr->groupDB[placeInDb];
    rc = prvDynamicPATxQClientGroupInit(groupPtr,BAD_VALUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* move all above found on one place */
    for (i = placeInList ; i < groupListPtr->listLen - 1; i++)
    {
        groupListPtr->groupIdPlaceList[i] = groupListPtr->groupIdPlaceList[i+1];
    }
    groupListPtr->groupIdPlaceList[groupListPtr->listLen-1].groupId = BAD_VALUE;
    groupListPtr->groupIdPlaceList[groupListPtr->listLen-1].place   = BAD_VALUE;
    groupListPtr->listLen--;
    return GT_OK;  /* something wrong : there shall be empty place */
}


/*------------------------*
 * dev level interface    *
 *------------------------*/
PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_DB_STC * prvCpssDxChPortDynamicPATxQClientGroupListFromCentralDBGet
(
    IN   GT_U8  devNum
)
{
    return &(PRV_CPSS_DXCH_PP_MAC(devNum)->paData.paPortDB.paSpecialTxQClientGroupDB);
}



GT_STATUS prvCpssDxChPortDynamicPATxQClientGroupListInit
(
    IN   GT_U8  devNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_DB_STC *listPtr;

    listPtr = prvCpssDxChPortDynamicPATxQClientGroupListFromCentralDBGet(devNum);
    rc = prvDynamicPATxQClientGroupListInit(listPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;

}


GT_STATUS prvCpssDxChPortDynamicPATxQClientGroupListGet
(
    IN   GT_U8  devNum,
    IN   GT_U32 groupId,
    OUT  PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC **groupPtrPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_DB_STC *listPtr;

    listPtr = prvCpssDxChPortDynamicPATxQClientGroupListFromCentralDBGet(devNum);
    rc = prvDynamicPATxQClientGroupListGet(listPtr,groupId,/*OUT*/groupPtrPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

GT_STATUS prvCpssDxChPortDynamicPATxQClientGroupListAdd
(
    IN   GT_U8  devNum,
    IN   GT_U32 groupId
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_DB_STC *listPtr;

    listPtr = prvCpssDxChPortDynamicPATxQClientGroupListFromCentralDBGet(devNum);

    rc = prvDynamicPATxQClientGroupListAdd(listPtr,groupId);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}


GT_STATUS prvCpssDxChPortDynamicPATxQClientGroupListDelete
(
    IN   GT_U8  devNum,
    IN   GT_U32 groupId
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_DB_STC *listPtr;

    listPtr = prvCpssDxChPortDynamicPATxQClientGroupListFromCentralDBGet(devNum);
    rc = prvDynamicPATxQClientGroupListDelete(listPtr,groupId);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

GT_STATUS prvCpssDxChPortDynamicPATxQClientGroupListLenGet
(
    IN   GT_U8   devNum,
    OUT  GT_U32  *lenPtr
)
{
    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_DB_STC *listPtr;

    CPSS_NULL_PTR_CHECK_MAC(lenPtr);
    listPtr = prvCpssDxChPortDynamicPATxQClientGroupListFromCentralDBGet(devNum);
    *lenPtr = listPtr->listLen;
    return GT_OK;
}

GT_STATUS prvCpssDxChPortDynamicPATxQClientGroupListByIdxGet
(
    IN   GT_U8  devNum,
    IN   GT_U32 idx,
    OUT  PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC **groupPtrPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_DB_STC *listPtr;

    listPtr = prvCpssDxChPortDynamicPATxQClientGroupListFromCentralDBGet(devNum);
    rc = prvDynamicPATxQClientGroupListByIdxGet(listPtr,idx,/*OUT*/groupPtrPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}


GT_STATUS prvCpssDxChPortDynamicPATxQClientGroupListClientAdd
(
    IN   GT_U8                                       devNum,
    IN   GT_U32                                      groupId,
    IN   PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_TYPE_ENT   clientType,
    IN   GT_PHYSICAL_PORT_NUM                        physicalPortNum,
    IN   GT_U32                                      txqPort,
    IN   GT_U32                                      speedMbps
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC * groupPtr;
    rc = prvCpssDxChPortDynamicPATxQClientGroupListGet(devNum,groupId,/*OUT*/&groupPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (groupPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    rc = prvDynamicPATxQClientGroupClientAdd(groupPtr,clientType,physicalPortNum,txqPort,speedMbps);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

GT_STATUS prvCpssDxChPortDynamicPATxQClientGroupListClientDelete
(
    IN   GT_U8                                       devNum,
    IN   GT_U32                                      groupId,
    IN   GT_U32                                      txqPort
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC * groupPtr;
    rc = prvCpssDxChPortDynamicPATxQClientGroupListGet(devNum,groupId,/*OUT*/&groupPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (groupPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    rc = prvDynamicPATxQClientGroupClientDelete(groupPtr,txqPort);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}


GT_STATUS prvCpssDxChPortDynamicPATxQClientGroupListCascadePortStatusSet
(
    IN   GT_U8                                       devNum,
    IN   GT_U32                                      groupId,
    IN   GT_BOOL                                     status   /* GT_TRUE -- exists , GT_FALSE -- no */
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC * groupPtr;
    rc = prvCpssDxChPortDynamicPATxQClientGroupListGet(devNum,groupId,/*OUT*/&groupPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (groupPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    groupPtr->isExtendedCascadeExist = status;
    return GT_OK;
}


/**
* @internal prvCpssDxChRemotePortSpeedGet function
* @endinternal
*
* @brief   Get speed of remote port from Pizza Arbiter TXQ special clients
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin, Bobcat3; Aldrin2; Falcon;
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
*
* @param[out] speedMbpsPtr             - (pointer to) port speed in Mbps
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong port number or not Remote Port
*/
GT_STATUS prvCpssDxChRemotePortSpeedGet
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32 *speedMbpsPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_STC *groupPtr;
    PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_STC       *clientPtr;
    GT_U32                                      clientIdx;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(speedMbpsPtr);

    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum,portNum,/*OUT*/&portMapPtr);
    if (rc != GT_OK)
    {
        return rc;
    }


    if (portMapPtr->valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (portMapPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* get the group if it does not exists */
    rc = prvCpssDxChPortDynamicPATxQClientGroupListGet(devNum,portMapPtr->portMap.macNum,/*OUT*/&groupPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (groupPtr == NULL) /* group does not exists ( mac of remote port was used as group id  */
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    rc = prvDynamicPATxQClientGroupClientFind(groupPtr,portMapPtr->portMap.txqNum,/*OUT*/&clientIdx,&clientPtr);
    if (GT_OK != rc)
    {
        return rc;
    }
    if (clientPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    *speedMbpsPtr = clientPtr->speedMbps;
    return GT_OK;
}


