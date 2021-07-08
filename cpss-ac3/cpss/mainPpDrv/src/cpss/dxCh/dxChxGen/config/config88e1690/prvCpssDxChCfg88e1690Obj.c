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
* @file prvCpssDxChCfg88e1690Obj.c
*
* @brief The function to manage the 88e1690 PHY-MAC object.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChCfg.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCn.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInitLedCtrl.h>

#include <msApi.h>
#include <msApiTypes.h>
#include <gtDrvSwRegs.h>
#include <msApiInternal.h>

#define UNUSED_PARAM(x) x = x

static CPSS_MACDRV_OBJ_STC *macPhy88e1690ObjPtr = NULL;

/* Translate Serdes port num / lane into SMI address */
static GT_U8 serdesSmiAdressArray[4] = {0x9, 0x12, 0x13, 0x14};
static GT_U32 serdesSmiAdressArrayLen = sizeof(serdesSmiAdressArray) / sizeof(serdesSmiAdressArray[0]);

/**
* @internal portNumCheck function
* @endinternal
*
* @brief   check the physical port number . and convert it to 'local MAC' and to
*         'driverInfoPtr'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - the physical port number
* @param[in] stage                    -  to run (PRE or POST)
*
* @param[out] doPpMacConfigPtr         - (pointer to) indicates if the switch should be set
* @param[out] localMacPortNumPtr       - (pointer to) the local MAC port number
* @param[out] driverInfoPtrPtr
*
* @retval GT_OK                    - always
*/
static GT_STATUS portNumCheck(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_U32                  *localMacPortNumPtr,
    OUT GT_CPSS_QD_DEV              **driverInfoPtrPtr
)
{
    GT_U32  portIndex;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr;

    if(stage != CPSS_MACDRV_STAGE_PRE_E)
    {
        *doPpMacConfigPtr = GT_TRUE;
        return GT_OK;
    }

    if(GT_FALSE == prvCpssDxChPortRemotePortCheck(devNum,portNum))
    {
        *doPpMacConfigPtr = GT_TRUE;
        return GT_OK;
    }

    *doPpMacConfigPtr = GT_FALSE;

    remotePhyMacInfoPtr = prvCpssDxChCfgPort88e1690RemotePhysicalPortInfoGet(devNum,portNum,&portIndex);

    if(remotePhyMacInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "The port[%d] hold no 88e1690 port's info \n",
                portNum);
    }

    /* get 'my MAC' number (1..8) */
    *localMacPortNumPtr =
        remotePhyMacInfoPtr->connectedPhyMacInfo.remotePortInfoArr[portIndex].remoteMacPortNum;
    *driverInfoPtrPtr = (GT_CPSS_QD_DEV *) remotePhyMacInfoPtr->drvInfoPtr;

    return GT_OK;
}

/**
* @internal portMacNumCheck function
* @endinternal
*
* @brief   check the cascade port MAC number . and convert it to 'driverInfoPtr'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] localMacPortNum          - the local MAC port number
* @param[in] stage                    -  to run (PRE or POST)
*
* @param[out] doPpMacConfigPtr         - (pointer to) indicates if the switch should be set
* @param[out] driverInfoPtrPtr         - driver belonging to the cascade port MAC
*
* @retval GT_OK                    - Mac number is valid and there exist correct mapping to port.
* @retval GT_BAD_PARAM             - Mac number is invalid.
* @retval GT_NOT_FOUND             - There is no mapping from MAC to port.
*/
static GT_STATUS portMacNumCheck(
    IN  GT_U8                   devNum,
    IN  GT_U32                  localMacPortNum,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_CPSS_QD_DEV              **driverInfoPtrPtr
)
{
    GT_STATUS rc;
    GT_PHYSICAL_PORT_NUM                        portNum;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr;

    PRV_CPSS_PHY_PORT_CHECK_MAC(devNum,localMacPortNum);

    if(stage != CPSS_MACDRV_STAGE_PRE_E)
    {
        *doPpMacConfigPtr = GT_TRUE;
        return GT_OK;
    }

    *doPpMacConfigPtr = GT_FALSE;

    rc = prvCpssDxChPortPhysicalPortMapReverseMappingGet(
                            devNum,
                            PRV_CPSS_DXCH_PORT_TYPE_MAC_E,
                            localMacPortNum,
                            &portNum);
    if(rc != GT_OK)
    {
        return rc;
    }
    remotePhyMacInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.remotePhyMacInfoArr[portNum];

    if(remotePhyMacInfoPtr == NULL)
    {
        *doPpMacConfigPtr = GT_TRUE;
        return GT_OK;
    }

    /* get driver object */
    *driverInfoPtrPtr = (GT_CPSS_QD_DEV *) remotePhyMacInfoPtr->drvInfoPtr;

    return GT_OK;
}

/*(1) */static GT_STATUS internal_macPhySpeedSet(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_SPEED_ENT     speed,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT CPSS_PORT_SPEED_ENT     *switchSpeedSetPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;

    GT_CPSS_PORT_FORCED_SPEED_MODE  mode;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    UNUSED_PARAM(switchSpeedSetPtr);

    switch(speed)
    {
        case CPSS_PORT_SPEED_10_E:
            mode = CPSS_PORT_FORCE_SPEED_10_MBPS;
            break;
        case CPSS_PORT_SPEED_100_E:
            mode = CPSS_PORT_FORCE_SPEED_100_MBPS;
            break;
        case CPSS_PORT_SPEED_1000_E:
            mode = CPSS_PORT_FORCE_SPEED_1000_MBPS;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvGprtSetSpeed(driverInfoPtr,localMacPortNum,mode);

    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtSetSpeed : portNum[%d]", portNum);
    }

    return  GT_OK;
}

/*(2) */static GT_STATUS internal_macPhySpeedGet(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_PORT_SPEED_ENT     *speedPtr,
    IN  CPSS_MACDRV_STAGE_ENT   stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT CPSS_PORT_SPEED_ENT     *switchSpeedSetPtr
)
{
    GT_STATUS      rc;
    GT_U32         localMacPortNum;
    GT_CPSS_QD_DEV      *driverInfoPtr;
    GT_CPSS_PORT_SPEED  speedMode;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    UNUSED_PARAM(switchSpeedSetPtr);

    rc = prvCpssDrvGprtGetSpeed(driverInfoPtr, localMacPortNum, &speedMode);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtGetForceSpeed : portNum[%d]", portNum);
    }

    switch (speedMode) {
        case CPSS_PORT_SPEED_10_MBPS:
            *speedPtr = CPSS_PORT_SPEED_10_E;
            break;
        case CPSS_PORT_SPEED_100_MBPS:
            *speedPtr = CPSS_PORT_SPEED_100_E;
            break;
        case CPSS_PORT_SPEED_1000_MBPS:
            *speedPtr = CPSS_PORT_SPEED_1000_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            break;
    }

    return  GT_OK;
}

/*(3) */static GT_STATUS internal_macPhyDuplexANSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchDuplexANSetPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    UNUSED_PARAM(switchDuplexANSetPtr);
    rc = prvCpssDrvGprtSetDuplexAutoNegEn(driverInfoPtr, localMacPortNum, state);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtSetDuplexAutoNegEn : portNum[%d]", portNum);
    }

    return GT_OK;
}

/*(4) */static GT_STATUS internal_macPhyDuplexANGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *statePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchDuplexANSetPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    UNUSED_PARAM(switchDuplexANSetPtr);

    rc = prvCpssDrvGprtGetDuplexAutoNegEn(driverInfoPtr, localMacPortNum, statePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtGetDuplexAutoNegEn : portNum[%d]", portNum);
    }

    return GT_OK;
}

/*(5) */static GT_STATUS internal_macPhyFlowCntlANSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state,
    IN  GT_BOOL                 pauseAdvertise,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *targetState,
    OUT GT_BOOL                 *targetPauseAdvertise
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    rc = prvCpssDrvGprtSetFlowCtrlAutoNegEn(driverInfoPtr, localMacPortNum, state, pauseAdvertise);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtSetFlowCtrlAutoNegEn : portNum[%d]", portNum);
    }

    *targetState          = state;
    *targetPauseAdvertise = pauseAdvertise;

    return  GT_OK;
}
/*(6) */static GT_STATUS internal_macPhyFlowCntlANGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *statePtr,
    OUT GT_BOOL                 *pauseAdvertisePtr,
    IN  CPSS_MACDRV_STAGE_ENT   stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *targetStatePtr,
    OUT GT_BOOL                 *targetPauseAdvertisePtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    rc = prvCpssDrvGprtGetFlowCtrlAutoNegEn(driverInfoPtr, localMacPortNum, statePtr, pauseAdvertisePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtGetFlowCtrlAutoNegEn : portNum[%d]", portNum);
    }

    *targetStatePtr           = *statePtr;
    *targetPauseAdvertisePtr  = *pauseAdvertisePtr;

    return  GT_OK;
}

/*(7) */static GT_STATUS internal_macPhySpeedANSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchSpeedANSetPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,
                      stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    UNUSED_PARAM(switchSpeedANSetPtr);

    rc = prvCpssDrvGprtSetSpeedAutoNegEn(driverInfoPtr, localMacPortNum, state);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtSetSpeedAutoNegEn : portNum[%d]", portNum);
    }

    return rc;
}

/*(8) */static GT_STATUS internal_macPhySpeedANGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *statePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchSpeedANSetPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    UNUSED_PARAM(switchSpeedANSetPtr);

    rc = prvCpssDrvGprtGetSpeedAutoNegEn(driverInfoPtr, localMacPortNum, statePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtGetSpeedAutoNegEn : portNum[%d]", portNum);
    }

    return GT_OK;
}

/*(9) */static GT_STATUS internal_macPhyFlowControlEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_FLOW_CONTROL_ENT  state,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT CPSS_PORT_FLOW_CONTROL_ENT  *switchFlowCntlSetPtr
)
{
    GT_STATUS        rc;
    GT_U32           localMacPortNum;
    GT_CPSS_QD_DEV       *driverInfoPtr;
    GT_CPSS_PORT_FC_MODE  fcMode;
    GT_BOOL          fcEnabled;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    UNUSED_PARAM(switchFlowCntlSetPtr);

    fcEnabled = GT_TRUE;
    switch (state) {
        case CPSS_PORT_FLOW_CONTROL_DISABLE_E:
            fcEnabled = GT_FALSE;

        case CPSS_PORT_FLOW_CONTROL_RX_TX_E:
            fcMode = CPSS_PORT_FC_TX_RX_ENABLED;
            break;

        case CPSS_PORT_FLOW_CONTROL_RX_ONLY_E:
            fcMode = CPSS_PORT_RX_ONLY;
            break;

        case CPSS_PORT_FLOW_CONTROL_TX_ONLY_E:
            fcMode = CPSS_PORT_TX_ONLY;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Failed (Bad Flow Control state)");
            break;
    }

    rc = prvCpssDrvGprtSetFlowCtrl(driverInfoPtr, localMacPortNum, fcEnabled, fcMode);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtSetFlowCtrl : portNum[%d]", portNum);
    }

    if (fcEnabled &&
        (fcMode != CPSS_PORT_RX_ONLY))
    {
        /* Set correct CCFC termination disabled at Aldrin */
        rc = cpssDxChPortCnTerminationEnableSet(devNum, portNum, GT_FALSE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortCnTerminationEnableSet: portNum[%d]", portNum);
        }
    }

    return  GT_OK;
}

/*(10)*/static GT_STATUS internal_macPhyFlowControlEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_FLOW_CONTROL_ENT      *statePtr,
    IN CPSS_MACDRV_STAGE_ENT            stage,
    OUT GT_BOOL                         *doPpMacConfigPtr,
    OUT CPSS_PORT_FLOW_CONTROL_ENT      *switchFlowCntlGetPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    GT_CPSS_PORT_FC_MODE  fcMode;
    GT_BOOL          fcEnabled;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    UNUSED_PARAM(switchFlowCntlGetPtr);

    rc = prvCpssDrvGprtGetFlowCtrl(driverInfoPtr, localMacPortNum, &fcEnabled, &fcMode);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtGetFlowCtrl : portNum[%d]", portNum);
    }

    if (fcEnabled == GT_FALSE)
    {
        *statePtr = CPSS_PORT_FLOW_CONTROL_DISABLE_E;
    }
    else
    {
        switch (fcMode)
        {
        case CPSS_PORT_FC_TX_RX_ENABLED:
            *statePtr = CPSS_PORT_FLOW_CONTROL_RX_TX_E;
            break;
        case CPSS_PORT_RX_ONLY:
            *statePtr = CPSS_PORT_FLOW_CONTROL_RX_ONLY_E;
            break;
        case CPSS_PORT_TX_ONLY:
            *statePtr = CPSS_PORT_FLOW_CONTROL_TX_ONLY_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Failed (Bad Flow Control mode)");
            break;
        }
    }

    return  GT_OK;
}

static GT_STATUS internal_macPhyCnFcTimerSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32                      index,
    IN  GT_U32                      timer,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
)
{
    GT_STATUS        rc;
    GT_U32           localMacPortNum;
    GT_CPSS_QD_DEV       *driverInfoPtr;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    if (index >= BIT_6)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Failed (Bad SPD/qFB/qDelta index)");
    }
    if (timer > 0xffff)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Failed (Bad timer value)");
    }

    /* Write command to firmware in ASCII string */
    /* Write op-code */
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, 'T');
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    /* Write portNum */
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + (GT_U8)localMacPortNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    /* Write SPD (2 MS bits of index) */
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + (GT_U8)((index >> 4) & 0x03));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    /* Write qFB (2 bits of index) */
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + (GT_U8)((index >> 2) & 0x03));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    /* Write qDelta sign (1 bits of index) */
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + (GT_U8)((index >> 1) & 0x01));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    /* Write qDelta big (1 LS bit of index) */
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + (GT_U8)((index >> 0) & 0x01));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    /* Write timer value */
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + ((timer >> 12) & 0x0f));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + ((timer >>  8) & 0x0f));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + ((timer >>  4) & 0x0f));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + ((timer >>  0) & 0x0f));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }

    return  GT_OK;
}

static GT_STATUS internal_macPhyCnFcTimerGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32                      index,
    OUT GT_U32                     *timerPtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
)
{
    GT_STATUS        rc;
    GT_U32           localMacPortNum;
    GT_CPSS_QD_DEV       *driverInfoPtr;
#ifndef ASIC_SIMULATION
    GT_U8            data;  /* Temporary character */
#endif  /* ASIC_SIMULATION */
    GT_U32           timer = 0xFFFF;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    if (index >= BIT_6)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Failed (Bad SPD/qFB/qDelta index)");
    }

#ifdef ASIC_SIMULATION

    /* Firmware is not implemented at simulation, so that prvCpssDrvImpReadComm() times out */

#else   /* ASIC_SIMULATION */

    /* Write command to firmware in ASCII string */
    /* Write op-code */
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, 'T');
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '?');
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    /* Write portNum */
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + (GT_U8)localMacPortNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    /* Write SPD (2 MS bits of index) */
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + (GT_U8)((index >> 4) & 0x03));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    /* Write qFB (2 bits of index) */
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + (GT_U8)((index >> 2) & 0x03));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    /* Write qDelta sign (1 bits of index) */
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + (GT_U8)((index >> 1) & 0x01));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }
    /* Write qDelta big (1 LS bit of index) */
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, '0' + (GT_U8)((index >> 0) & 0x01));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }

    /* Read command response from firmware in ASCII string */
    rc = prvCpssDrvImpReadComm(driverInfoPtr, &data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpReadComm");
    }
    timer  = (data << 12) & 0xf000;
    rc = prvCpssDrvImpReadComm(driverInfoPtr, &data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpReadComm");
    }
    timer |= (data <<  8) & 0x0f00;
    rc = prvCpssDrvImpReadComm(driverInfoPtr, &data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpReadComm");
    }
    timer |= (data <<  4) & 0x00f0;
    rc = prvCpssDrvImpReadComm(driverInfoPtr, &data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpReadComm");
    }
    timer |= (data <<  0) & 0x000f;

#endif  /* ASIC_SIMULATION */

    *timerPtr = timer;
    return  GT_OK;
}

static GT_STATUS internal_macPhyFcModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_DXCH_PORT_FC_MODE_ENT  cpssFcMode,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
)
{
    GT_STATUS        rc;
    GT_U32           localMacPortNum;
    GT_CPSS_QD_DEV       *driverInfoPtr;
    GT_CPSS_PORT_FC_MODE  fcMode;
    GT_BOOL          fcEnabled;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    rc = prvCpssDrvGprtGetFlowCtrl(driverInfoPtr, localMacPortNum, &fcEnabled, &fcMode);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtGetFlowCtrl : portNum[%d]", portNum);
    }

    switch (cpssFcMode) {
        case CPSS_DXCH_PORT_FC_MODE_802_3X_E:
            if (fcMode == CPSS_PORT_PFC_ENABLED)
            {
                fcMode = CPSS_PORT_FC_TX_RX_ENABLED;
            }
            break;

        case CPSS_DXCH_PORT_FC_MODE_PFC_E:
            fcEnabled = GT_TRUE;
            fcMode = CPSS_PORT_PFC_ENABLED;
            break;

        case CPSS_DXCH_PORT_FC_MODE_LL_FC_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "Failed (Bad Flow Control state)");
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Failed (Bad Flow Control state)");
            break;
    }

    rc = prvCpssDrvGprtSetFlowCtrl(driverInfoPtr, localMacPortNum, fcEnabled, fcMode);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtSetFlowCtrl : portNum[%d]", portNum);
    }

    return  GT_OK;
}

static GT_STATUS internal_macPhyFcModeGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_DXCH_PORT_FC_MODE_ENT *fcModePtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
)
{
    GT_STATUS        rc;
    GT_U32           localMacPortNum;
    GT_CPSS_QD_DEV       *driverInfoPtr;
    GT_CPSS_PORT_FC_MODE  fcMode;
    GT_BOOL          fcEnabled;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    rc = prvCpssDrvGprtGetFlowCtrl(driverInfoPtr, localMacPortNum, &fcEnabled, &fcMode);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtGetFlowCtrl : portNum[%d]", portNum);
    }

    if (fcMode == CPSS_PORT_PFC_ENABLED)
    {
        *fcModePtr = CPSS_DXCH_PORT_FC_MODE_PFC_E;
    }
    else
    {
        *fcModePtr = CPSS_DXCH_PORT_FC_MODE_802_3X_E;
    }

    return  GT_OK;
}

static GT_STATUS internal_macPhyFcParamsSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32                      xOffThreshold,
    IN  GT_U32                      xOnThreshold,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
)
{
    GT_STATUS        rc;
    GT_U32           localMacPortNum;
    GT_CPSS_QD_DEV       *driverInfoPtr;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    CPSS_PARAM_CHECK_MAX_MAC(xOffThreshold/2,BIT_8);
    CPSS_PARAM_CHECK_MAX_MAC(xOnThreshold/2,xOffThreshold/2);

    rc = prvCpssDrvGprtSetQueueCtrl(driverInfoPtr,
                            localMacPortNum,
                            GT_CPSS_QUEUE_CFG_FC_THRESHOLD_BEGIN,
                            (GT_U8)(xOffThreshold / 2));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtSetQueueCtrl: portNum[%d]", portNum);
    }

    rc = prvCpssDrvGprtSetQueueCtrl(driverInfoPtr,
                            localMacPortNum,
                            GT_CPSS_QUEUE_CFG_FC_THRESHOLD_END,
                            (GT_U8)(xOnThreshold / 2));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtSetQueueCtrl: portNum[%d]", portNum);
    }

    return  GT_OK;
}

static GT_STATUS internal_macPhyFcParamsGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_U32                      *xOffThresholdPtr,
    OUT GT_U32                      *xOnThresholdPtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
)
{
    GT_STATUS        rc;
    GT_U32           localMacPortNum;
    GT_CPSS_QD_DEV       *driverInfoPtr;
    GT_U8            data;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    rc = prvCpssDrvGprtGetQueueCtrl(driverInfoPtr,
                            localMacPortNum,
                            GT_CPSS_QUEUE_CFG_FC_THRESHOLD_BEGIN,
                            &data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtGetQueueCtrl: portNum[%d]", portNum);
    }
    *xOffThresholdPtr = data * 2;

    rc = prvCpssDrvGprtGetQueueCtrl(driverInfoPtr,
                            localMacPortNum,
                            GT_CPSS_QUEUE_CFG_FC_THRESHOLD_END,
                            &data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtGetQueueCtrl: portNum[%d]", portNum);
    }

    *xOnThresholdPtr = data * 2;

    return  GT_OK;
}

/*(11)*/static GT_STATUS internal_macPhyPeriodicFlowCntlSet
(
    IN  GT_U8                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT    state,
    IN  CPSS_MACDRV_STAGE_ENT                       stage,
    OUT GT_BOOL                                    *doPpMacConfigPtr,
    OUT GT_BOOL                                    *switchPeriodicFlowCntlSetPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    if ((state != CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E) &&
        (state != CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_XOFF_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    UNUSED_PARAM(switchPeriodicFlowCntlSetPtr);

    rc = prvCpssDrvGprtSetPauseLimitOut(driverInfoPtr, localMacPortNum,
                              (state == CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_XOFF_E ? 1 : 0));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtSetPauseLimitOut : portNum[%d]", portNum);
    }

    return  GT_OK;
}

/*(12)*/static GT_STATUS internal_macPhyPeriodicFlowCntlGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                     *statePtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *switchPereodicFlowCntlSetPtr
)
{
    GT_STATUS rc;
    GT_U32    localMacPortNum;
    GT_CPSS_QD_DEV *driverInfoPtr;
    GT_U8     limitOut;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    UNUSED_PARAM(switchPereodicFlowCntlSetPtr);

    rc = prvCpssDrvGprtGetPauseLimitOut(driverInfoPtr, localMacPortNum, &limitOut);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtGetPauseLimitOut: portNum[%d]", portNum);
    }
    *statePtr = (limitOut == 1) ? GT_TRUE : GT_FALSE;

    return  GT_OK;
}

/*(13)*/static GT_STATUS internal_macPhyBackPressureSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_BOOL                     state,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *switchBPSetPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    GT_CPSS_PORT_FC_MODE  fcMode;
    GT_BOOL          fcEnabled;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    UNUSED_PARAM(switchBPSetPtr);

    /* read FCMode to stay it the same. */
    rc = prvCpssDrvGprtGetFlowCtrl(driverInfoPtr, localMacPortNum, &fcEnabled, &fcMode);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtGetFlowCtrl : portNum[%d]", portNum);
    }

    /*  Set FCValue = state */
    if (state != fcEnabled)
    {
        rc = prvCpssDrvGprtSetFlowCtrl(driverInfoPtr, localMacPortNum, state, fcMode);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtSetFlowCtrl : portNum[%d]", portNum);
        }
    }

    return  GT_OK;
}

/*(14)*/static GT_STATUS internal_macPhyBackPressureGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                     *statePtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *switchBPSetPtr
)
{
    GT_STATUS        rc;
    GT_U32           localMacPortNum;
    GT_CPSS_QD_DEV       *driverInfoPtr;
    GT_CPSS_PORT_FC_MODE  fcMode;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    UNUSED_PARAM(switchBPSetPtr);

    rc = prvCpssDrvGprtGetFlowCtrl(driverInfoPtr, localMacPortNum, statePtr, &fcMode);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtGetFlowCtrl : portNum[%d]", portNum);
    }

    return  GT_OK;
}

/*(15)*/static GT_STATUS internal_macPhyPortLinkGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                    *isLinkUpPtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *switchLinkStatusGetPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    rc = prvCpssDrvGprtGetLinkState(driverInfoPtr,localMacPortNum,isLinkUpPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtGetLinkState : portNum[%d]", portNum);
    }

    *switchLinkStatusGetPtr = *isLinkUpPtr;

    return rc;
}

/*(16)*/static GT_STATUS internal_macPhyDuplexSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_DUPLEX_ENT        dMode,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT CPSS_PORT_DUPLEX_ENT        *switchDuplexSetPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    GT_BOOL isFullDuplex;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    UNUSED_PARAM(switchDuplexSetPtr);

    isFullDuplex = (dMode == CPSS_PORT_FULL_DUPLEX_E) ? GT_TRUE : GT_FALSE;
    rc = prvCpssDrvGprtSetDuplex(driverInfoPtr,localMacPortNum,isFullDuplex);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtSetDuplex : portNum[%d]", portNum);
    }

    return  GT_OK;
}

/*(17)*/static GT_STATUS internal_macPhyDuplexGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT  CPSS_PORT_DUPLEX_ENT       *dModePtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT CPSS_PORT_DUPLEX_ENT        *switchDuplexSetPtr
)
{
    GT_STATUS rc;
    GT_U32      localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    GT_BOOL     isFullDuplex;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    UNUSED_PARAM(switchDuplexSetPtr);

    rc = prvCpssDrvGprtGetDuplex(driverInfoPtr,localMacPortNum, &isFullDuplex);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtGetDuplex : portNum[%d]", portNum);
    }

    *dModePtr = isFullDuplex ? CPSS_PORT_FULL_DUPLEX_E : CPSS_PORT_HALF_DUPLEX_E;

    return  GT_OK;
}

/**
* @internal internal_macPhyPortEnableSet function
* @endinternal
*
* @brief   Set Port enable state.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] stage                    -  for callback run (PRE or POST)
* @param[in] enable                   - port  state
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_macPhyPortEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_BOOL                     enable,
    IN  CPSS_MACDRV_STAGE_ENT       stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *switchPortEnableSetPtr
)
{
    GT_STATUS           rc;
    GT_U32              localMacPortNum;
    GT_CPSS_QD_DEV          *driverInfoPtr;
    GT_CPSS_PORT_STP_STATE   state;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    UNUSED_PARAM(switchPortEnableSetPtr);

    state = (enable == GT_FALSE) ? GT_CPSS_PORT_DISABLE : GT_CPSS_PORT_FORWARDING;
    rc = prvCpssDrvGstpSetPortState(driverInfoPtr,localMacPortNum,state);
    return  rc;
}

/**
* @internal internal_macPhyPortEnableGet function
* @endinternal
*
* @brief   Get Port enable state.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] stage                    -  for callback run (PRE or POST)
*
* @param[out] enablePtr                - port enable state pointer
*                                      switchPortEnableGetPtr        - port status pointer to value for switch MAC
* @param[out] doPpMacConfigPtr         - pointer to the parameter defines if the switch MAC
*                                      will be configurated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_macPhyPortEnableGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                    *enablePtr,
    IN  CPSS_MACDRV_STAGE_ENT       stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_BOOL                     *targetEnableSetPtr
)
{
    GT_STATUS           rc;
    GT_U32              localMacPortNum;
    GT_CPSS_QD_DEV          *driverInfoPtr;
    GT_CPSS_PORT_STP_STATE   state;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    rc = prvCpssDrvGstpGetPortState(driverInfoPtr,localMacPortNum,&state);
    if(rc!= GT_OK )
    {
        return rc;
    }

    *targetEnableSetPtr = *enablePtr = (state == GT_CPSS_PORT_DISABLE) ? GT_FALSE : GT_TRUE;

    return  GT_OK;
}

/*(20)*/static GT_STATUS internal_macPhyExcessiveCollisionDropSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchExcColDropSetPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    UNUSED_PARAM(switchExcColDropSetPtr);

    rc = prvCpssDrvGsysSetDiscardExcessive(driverInfoPtr, enable);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGsysSetDiscardExcessive : portNum[%d]", portNum);
    }

    return  GT_OK;
}

/*(21)*/static GT_STATUS internal_macPhyExcessiveCollisionDropGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchExcColDropSetPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    UNUSED_PARAM(switchExcColDropSetPtr);

    rc = prvCpssDrvGsysGetDiscardExcessive(driverInfoPtr, enablePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGsysGetDiscardExcessive : portNum[%d]", portNum);
    }

    return  GT_OK;
}

/*(22)*/static GT_STATUS internal_macPhyPaddingEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchPaddingSetPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }
    UNUSED_PARAM(switchPaddingSetPtr);

    /* padding is always enabled */
    return (enable == GT_TRUE) ? GT_OK : GT_NOT_SUPPORTED;
}

/*(23)*/static GT_STATUS internal_macPhyPaddingEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchPaddingSetPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }
    UNUSED_PARAM(switchPaddingSetPtr);

    /* padding is always enabled */
    *enablePtr = GT_TRUE;

    return  GT_OK;
}

/*(24)*/static GT_STATUS internal_macPhyPreambleLengthSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   length,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT CPSS_PORT_DIRECTION_ENT *targetDirectionPtr,
    OUT GT_U32                  *switchPreambleLengthPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    UNUSED_PARAM(targetDirectionPtr);
    UNUSED_PARAM(switchPreambleLengthPtr);

    /* make the behaviour similar to xCat3 Giga port. */
    if (length != 4 && length != 8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (direction != CPSS_PORT_DIRECTION_TX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* preamble is always 8B (including Start Symbol and SFD) in 88E1690 */
    return  (length == 8) ? GT_OK : GT_NOT_SUPPORTED;
}
/*(25)*/static GT_STATUS internal_macPhyPreambleLengthGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  CPSS_PORT_DIRECTION_ENT  direction,
    OUT GT_U32                   *lengthPtr,
    IN  CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT CPSS_PORT_DIRECTION_ENT *targetDirectionPtr,
    OUT GT_U32                  *switchPreambleLengthPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    UNUSED_PARAM(switchPreambleLengthPtr);
    UNUSED_PARAM(targetDirectionPtr);

    /* make the behaviour similar to xCat3 Giga port. */
    if (direction != CPSS_PORT_DIRECTION_TX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* preamble is always 8B (including Start Symbol and SFD) in 88E1690 */
    *lengthPtr = 8;

    return  GT_OK;
}

/*(26)*/static GT_STATUS internal_macPhyCRCCheckSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    OUT GT_BOOL                *switchCRCCheckSetPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    UNUSED_PARAM(switchCRCCheckSetPtr);

    rc = prvCpssDrvGprtSetAllowedBad(driverInfoPtr, localMacPortNum, !enable);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtSetAllowedBad : portNum[%d]", portNum);
    }

    return  GT_OK;
}

static GT_STATUS internal_macPhyCRCCheckGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    OUT GT_BOOL                *switchCRCCheckSetPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_BOOL      allowedBad;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    UNUSED_PARAM(switchCRCCheckSetPtr);

    rc = prvCpssDrvGprtGetAllowedBad(driverInfoPtr, localMacPortNum, &allowedBad);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtGetAllowedBad : portNum[%d]", portNum);
    }

    *enablePtr = !allowedBad;
    return  GT_OK;
}

/**
* @internal internal_macPhyMRUSet function
* @endinternal
*
* @brief   set MRU size.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] mruSize                  - MRU size
* @param[in] stage                    -  for callback run (PRE or POST)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_macPhyMRUSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  mruSize,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    OUT GT_U32                 *switchMRUSetPtr
)
{
    GT_STATUS       rc;
    GT_U32          localMacPortNum;
    GT_CPSS_QD_DEV      *driverInfoPtr;
    GT_CPSS_JUMBO_MODE   mode;

    UNUSED_PARAM(switchMRUSetPtr);

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    switch(mruSize)
    {
        case 10240: mode = GT_CPSS_JUMBO_MODE_10240; break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvGprtSetJumboMode(driverInfoPtr, localMacPortNum, mode);
    return  rc;
}

/**
* @internal internal_macPhyMRUGet function
* @endinternal
*
* @brief   get MRU size.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] stage                    -  for callback run (PRE or POST)
*
* @param[out] mruSizePtr               - MRU size value pointer
* @param[out] switchMRUGetPtr          - switch MAC MRU size pointer
*                                      to value for switch MAC
* @param[out] doPpMacConfigPtr         - pointer to the parameter defines if the switch MAC
*                                      will be configurated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_macPhyMRUGet
(
    IN   GT_U8                   devNum,
    IN   GT_PHYSICAL_PORT_NUM    portNum,
    OUT  GT_U32                 *mruSizePtr,
    IN   CPSS_MACDRV_STAGE_ENT   stage,
    OUT  GT_BOOL                *doPpMacConfigPtr,
    OUT  GT_U32                 *switchMRUGetPtr
)
{
    GT_STATUS       rc;
    GT_U32          localMacPortNum;
    GT_CPSS_QD_DEV      *driverInfoPtr;
    GT_CPSS_JUMBO_MODE   mode;

    UNUSED_PARAM(switchMRUGetPtr);

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    rc = prvCpssDrvGprtGetJumboMode(driverInfoPtr, localMacPortNum, &mode);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(mode)
    {
        case GT_CPSS_JUMBO_MODE_1522:  *mruSizePtr =  1522; break;
        case GT_CPSS_JUMBO_MODE_2048:  *mruSizePtr =  2048; break;
        case GT_CPSS_JUMBO_MODE_10240: *mruSizePtr = 10240; break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return  GT_OK;
}

/**
* @internal internal_macPhyPortPowerDownSet function
* @endinternal
*
* @brief   Set Port Power state up/down.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] powerDown                - GT_FALSE: power up/GT_TRUE: power down
* @param[in] stage                    -  for callback run (PRE or POST)
*
* @param[out] doPpMacConfigPtr         - pointer to the parameter defines if the switch MAC
*                                      will be configured
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_macPhyPortPowerDownSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_BOOL                     powerDown,
    IN  CPSS_MACDRV_STAGE_ENT       stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
)
{
    GT_STATUS           rc;
    GT_U32              localMacPortNum;
    GT_CPSS_QD_DEV          *driverInfoPtr;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    rc = prvCpssDrvGphyPortPowerDown(driverInfoPtr,localMacPortNum,powerDown);
    return  rc;
}


/*
 *  internal_macPhyPortAttributesGet
 *
 * Description: defines link status / speed / duplex get callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   CPSS_PORT_ATTRIBUTES_STC  *portAttributesPtr
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS: (30) Port Link Status / speed / duplex Get callback function
 *
 */
static GT_STATUS internal_macPhyPortAttributesGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT CPSS_PORT_ATTRIBUTES_STC    *portAttributesPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    GT_BOOL         state;      /* Temporary link state */
    GT_CPSS_PORT_SPEED   speed;      /* Temporary link speed */
    GT_BOOL         duplex;     /* Temporary link duplex mode */

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    rc = prvCpssDrvGprtGetPortAttributes(driverInfoPtr,localMacPortNum,
                                &state,
                                &speed,
                                &duplex);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGprtGetLinkState : portNum[%d]", portNum);
    }

    portAttributesPtr->portLinkUp = state;
    switch (speed)
    {
        case CPSS_PORT_SPEED_10_MBPS:
            portAttributesPtr->portSpeed = CPSS_PORT_SPEED_10_E;
            break;
        case CPSS_PORT_SPEED_100_MBPS:
            portAttributesPtr->portSpeed = CPSS_PORT_SPEED_100_E;
            break;
        case CPSS_PORT_SPEED_1000_MBPS:
            portAttributesPtr->portSpeed = CPSS_PORT_SPEED_1000_E;
            break;
        default:
            portAttributesPtr->portSpeed = CPSS_PORT_SPEED_NA_E;
            break;
    }
    portAttributesPtr->portDuplexity = duplex ?
                                        CPSS_PORT_FULL_DUPLEX_E :
                                        CPSS_PORT_HALF_DUPLEX_E;

    return rc;
}

/*
 *  internal_macPhyCscdPortTypeSet
 *
 * Description: Cascade Type set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *   CPSS_PORT_DIRECTION_ENT      portDirection,
 *   CPSS_CSCD_PORT_TYPE_ENT      port cascade Type - currently only Network type
 *
 * OUTPUTS:
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS:
 *        None.
 *
 */
GT_STATUS internal_macPhyCscdPortTypeSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    IN CPSS_PORT_DIRECTION_ENT      portDirection,
    IN CPSS_CSCD_PORT_TYPE_ENT      portType
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;

    UNUSED_PARAM(portDirection);
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    if (portType != CPSS_CSCD_PORT_NETWORK_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}


/*
 *  internal_macPhyCscdPortTypeGet
 *
 * Description: Cascade Type get callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *   CPSS_PORT_DIRECTION_ENT      portDirection,
 *
 * OUTPUTS:
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *   CPSS_CSCD_PORT_TYPE_ENT      *portTypePtr - currently only Network type supported
 *
 *  COMMENTS:
 *        None.
 *
 */
GT_STATUS internal_macPhyCscdPortTypeGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    IN CPSS_PORT_DIRECTION_ENT      portDirection,
    OUT CPSS_CSCD_PORT_TYPE_ENT     *portTypePtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;

    UNUSED_PARAM(portDirection);
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    *portTypePtr = CPSS_CSCD_PORT_NETWORK_E;

    return rc;
}

/*
 *  internal_macPhyMacSaLsbSet
 *
 * Description: MAC SA LSB set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *   GT_U8                  macSaLsb - The least significant byte of the MAC SA
 *
 * OUTPUTS:
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS:
 *        None.
 *
 */
GT_STATUS internal_macPhyMacSaLsbSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    IN GT_U8                        macSaLsb
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;

    UNUSED_PARAM(macSaLsb);
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
}

/*
 *  internal_macPhyMacSaLsbGet
 *
 * Description: MAC SA LSB get callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *   GT_U8                  *macSaLsbPtr - The least significant byte of the MAC SA
 *
 *  COMMENTS:
 *        None.
 *
 */
GT_STATUS internal_macPhyMacSaLsbGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT GT_U8                       *macSaLsbPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;

    UNUSED_PARAM(macSaLsbPtr);
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
}

/*
 *  internal_macPhyMacSaBaseSet
 *
 * Description: MAC SA BASE set callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U32                  portMacNum  - port MAC number (Note: not the remote
 *                                          port but the MAC number through which a
 *                                          group of remote ports are connected)
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *   GT_ETHERADDR                  macPtr - The MAC SA
 *
 * OUTPUTS:
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *
 *  COMMENTS:
 *        None.
 *
 */
static GT_STATUS internal_macPhyMacSaBaseSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portMacNum,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    IN  GT_ETHERADDR                *macPtr
)
{
    GT_STATUS rc;
    GT_CPSS_QD_DEV   *driverInfoPtr;

    rc = portMacNumCheck(devNum,portMacNum,stage,doPpMacConfigPtr,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    rc = prvCpssDrvGsysSetDuplexPauseMac(driverInfoPtr,macPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvGsysSetDuplexPauseMac : portMacNum[%d]", portMacNum);
    }

    return rc;
}


/*
 *  internal_macPhyPortInterfaceModeGet
 *
 * Description: port interface mode get callback for MACPHY object
 *
 * INPUTS:
 *   GT_U8                   devNum    - device number
 *   GT_U8                   portNum   - port number
 *   CPSS_MACDRV_STAGE_ENT    stage    - stage for callback run (PRE or POST)
 *
 * OUTPUTS:
 *   GT_BOOL         *doPpMacConfigPtr              - pointer to the parameter defines if the switch MAC
 *                                                    will be configured
 *   CPSS_PORT_INTERFACE_MODE_ENT                  *ifModePtr - Port interface mode
 *
 *  COMMENTS:
 *        None.
 *
 */
GT_STATUS internal_macPhyPortInterfaceModeGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    *ifModePtr = CPSS_PORT_INTERFACE_MODE_SGMII_E;
    return GT_OK;
}

/**
* @internal internal_macTypeGet function
* @endinternal
*
* @brief   Get port MAC type.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] stage                    -  for callback run (PRE or POST)
*
* @param[out] doPpMacConfigPtr         - (pointer to)the parameter defines if the switch MAC will be configured
* @param[out] portMacTypePtr           - (pointer to) port MAC type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_macTypeGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_PORT_MAC_TYPE_ENT  *portMacTypePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    *portMacTypePtr = CPSS_PORT_MAC_TYPE_GE_E;

    return  GT_OK;
}


/* STUB function to be 'place holder' */
static GT_STATUS internal_macStub
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr,
    IN const char*    funcNamePtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    if(funcNamePtr)
    {
        CPSS_LOG_INFORMATION_MAC("[%s] was not implemented yet for remote physical port [%d]\n",
            funcNamePtr,portNum);
    }

    return  GT_OK;
}

/**
* @internal internal_portLoopbackSet function
* @endinternal
*
* @brief   Set the internal Loopback state in the packet processor MAC port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - If GT_TRUE,  loopback
*                                      If GT_FALSE, disable loopback
* @param[in] stage                    -  for callback run (PRE or POST)
*
* @param[out] doPpMacConfigPtr         - (pointer to)the parameter defines if the switch MAC will be configured
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_portLoopbackSet(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable,
    IN  CPSS_MACDRV_STAGE_ENT   stage,
    OUT GT_BOOL                *doPpMacConfigPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    return prvCpssDrvGphySetPortLoopback(driverInfoPtr,localMacPortNum,enable);
}

/**
* @internal internal_portLoopbackGet function
* @endinternal
*
* @brief   Get Internal Loopback
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] stage                    -  for callback run (PRE or POST)
*
* @param[out] doPpMacConfigPtr         - (pointer to)the parameter defines if the switch MAC will be configured
* @param[out] enablePtr                - Pointer to the Loopback state.
*                                      If GT_TRUE, loopback is enabled
*                                      If GT_FALSE, loopback is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not relevant for the CPU port.
*
*/
static GT_STATUS internal_portLoopbackGet(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr,
    IN  CPSS_MACDRV_STAGE_ENT   stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    return prvCpssDrvGphyGetPortLoopback(driverInfoPtr,localMacPortNum,enablePtr);
}

/**
* @internal internal_portForceLinkDownSet function
* @endinternal
*
* @brief   Enable/disable Force Link Down on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] doPpMacConfigPtr         - (pointer to)the parameter defines if the switch MAC will be configured
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS internal_portForceLinkDownSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state,
    IN  CPSS_MACDRV_STAGE_ENT   stage,
    OUT GT_BOOL                 *doPpMacConfigPtr

)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    return prvCpssDrvGprtSetPortForceLinkDown(driverInfoPtr,localMacPortNum,state);
}

/**
* @internal internal_portForceLinkDownGet function
* @endinternal
*
* @brief   Get Force Link Down state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] doPpMacConfigPtr         - (pointer to)the parameter defines if the switch MAC will be configured
* @param[out] statePtr                 - (pointer to) force link down state.
*                                      GT_TRUE for force link down, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_VALUE             - on bad register value
*/
GT_STATUS internal_portForceLinkDownGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *statePtr,
    IN  CPSS_MACDRV_STAGE_ENT   stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    return prvCpssDrvGprtGetPortForceLinkDown(driverInfoPtr,localMacPortNum,statePtr);
}

/**
* @internal internal_portForceLinkPassEnableSet function
* @endinternal
*
* @brief   Enable/disable Force Link Pass on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] doPpMacConfigPtr         - (pointer to)the parameter defines if the switch MAC will be configured
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS internal_portForceLinkPassEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state,
    IN  CPSS_MACDRV_STAGE_ENT   stage,
    OUT GT_BOOL                 *doPpMacConfigPtr

)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    return prvCpssDrvGprtSetPortForceLinkPassEnable(driverInfoPtr,localMacPortNum,state);
}

/**
* @internal internal_portForceLinkPassEnableGet function
* @endinternal
*
* @brief   Get Force Link Pass state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] doPpMacConfigPtr         - (pointer to)the parameter defines if the switch MAC will be configured
* @param[out] statePtr                 - (pointer to) force link pass state.
*                                      GT_TRUE for force link down, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_VALUE             - on bad register value
*/
GT_STATUS internal_portForceLinkPassEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *statePtr,
    IN  CPSS_MACDRV_STAGE_ENT   stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    return prvCpssDrvGprtGetPortForceLinkPassEnable(driverInfoPtr,localMacPortNum,statePtr);
}

/* Counters retrieving functions */
static GT_STATUS internal_macPhyCounterGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *internalInfoPtr;
    PRV_CPSS_DXCH_CFG_REMOTE_MAC_INFO_STC *prvRemoteMacInfoPtr;
    GT_U64                      *mibShadowPtr;

    GT_CPSS_STATS_COUNTERS  counter;                 /* Stat counter to read */
    GT_BOOL            isDoubleCounter;         /* Whether counter is 64-bits */
    GT_U32             counterValueHiTmp = 0;   /* Temporary result of Hi counter read */
    GT_U64             counterValueTmp;         /* Temporary result of counter read */

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    internalInfoPtr = driverInfoPtr->appData;
    prvRemoteMacInfoPtr = &internalInfoPtr->prvRemoteMacInfoArr[localMacPortNum];
    counterValueTmp.l[0] = 0;
    counterValueTmp.l[1] = 0;
    isDoubleCounter = GT_FALSE;
    switch(cntrName)
    {
        case CPSS_GOOD_OCTETS_RCV_E:
            counter = CPSS_STATS_InGoodOctetsLo;
            isDoubleCounter = GT_TRUE;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.goodOctetsRcv;
            break;
        case CPSS_BAD_OCTETS_RCV_E:
            counter = CPSS_STATS_InBadOctets;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.badOctetsRcv;
            break;
        case CPSS_MAC_TRANSMIT_ERR_E:
            counter = CPSS_STATS_OutFCSErr;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.macTransmitErr;
            break;
        case CPSS_BRDC_PKTS_RCV_E:
            counter = CPSS_STATS_InBroadcasts;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.brdcPktsRcv;
            break;
        case CPSS_MC_PKTS_RCV_E:
            counter = CPSS_STATS_InMulticasts;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.mcPktsRcv;
            break;
        case CPSS_PKTS_64_OCTETS_E:
            counter = CPSS_STATS_64Octets;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.pkts64Octets;
            break;
        case CPSS_PKTS_65TO127_OCTETS_E:
            counter = CPSS_STATS_127Octets;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.pkts65to127Octets;
            break;
        case CPSS_PKTS_128TO255_OCTETS_E:
            counter = CPSS_STATS_255Octets;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.pkts128to255Octets;
            break;
        case CPSS_PKTS_256TO511_OCTETS_E:
            counter = CPSS_STATS_511Octets;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.pkts256to511Octets;
            break;
        case CPSS_PKTS_512TO1023_OCTETS_E:
            counter = CPSS_STATS_1023Octets;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.pkts512to1023Octets;
            break;
        case CPSS_PKTS_1024TOMAX_OCTETS_E:
            counter = CPSS_STATS_MaxOctets;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.pkts1024tomaxOoctets;
            break;
        case CPSS_GOOD_OCTETS_SENT_E:
            counter = CPSS_STATS_OutOctetsLo;
            isDoubleCounter = GT_TRUE;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.goodOctetsSent;
            break;
        case CPSS_EXCESSIVE_COLLISIONS_E:
            counter = CPSS_STATS_Excessive;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.excessiveCollisions;
            break;
        case CPSS_MC_PKTS_SENT_E:
            counter = CPSS_STATS_OutMulticasts;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.mcPktsSent;
            break;
        case CPSS_BRDC_PKTS_SENT_E:
            counter = CPSS_STATS_OutBroadcasts;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.brdcPktsSent;
            break;
        case CPSS_FC_SENT_E:
            counter = CPSS_STATS_OutPause;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.fcSent;
            break;
        case CPSS_GOOD_FC_RCV_E:
            counter = CPSS_STATS_InPause;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.goodFcRcv;
            break;
        case CPSS_DROP_EVENTS_E:
            counter = CPSS_STATS_InDiscards;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.dropEvents;
            break;
        case CPSS_UNDERSIZE_PKTS_E:
            counter = CPSS_STATS_InUndersize;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.undersizePkts;
            break;
        case CPSS_FRAGMENTS_PKTS_E:
            counter = CPSS_STATS_InFragments;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.fragmentsPkts;
            break;
        case CPSS_OVERSIZE_PKTS_E:
            counter = CPSS_STATS_InOversize;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.oversizePkts;
            break;
        case CPSS_JABBER_PKTS_E:
            counter = CPSS_STATS_InJabber;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.jabberPkts;
            break;
        case CPSS_MAC_RCV_ERROR_E:
            counter = CPSS_STATS_InRxErr;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.macRcvError;
            break;
        case CPSS_BAD_CRC_E:
            counter = CPSS_STATS_InFCSErr;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.badCrc;
            break;
        case CPSS_COLLISIONS_E:
            counter = CPSS_STATS_Collisions;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.collisions;
            break;
        case CPSS_LATE_COLLISIONS_E:
            counter = CPSS_STATS_Late;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.lateCollisions;
            break;
        case CPSS_GOOD_UC_PKTS_RCV_E:
            counter = CPSS_STATS_InUnicasts;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.ucPktsRcv;
            break;
        case CPSS_GOOD_UC_PKTS_SENT_E:
            counter = CPSS_STATS_OutUnicasts;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.ucPktsSent;
            break;
        case CPSS_MULTIPLE_PKTS_SENT_E:
            counter = CPSS_STATS_Multiple;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.multiplePktsSent;
            break;
        case CPSS_DEFERRED_PKTS_SENT_E:
            counter = CPSS_STATS_Deferred;
            mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow.deferredPktsSent;
            break;

        case CPSS_GOOD_PKTS_RCV_E:
        case CPSS_BAD_PKTS_RCV_E:
        case CPSS_GOOD_PKTS_SENT_E:
        case CPSS_UNRECOG_MAC_CNTR_RCV_E:
        case CPSS_PKTS_1024TO1518_OCTETS_E:
        case CPSS_PKTS_1519TOMAX_OCTETS_E:
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (isDoubleCounter)
    {
        /* Read High part of 64-bit counter - 1st. time */
        rc = prvCpssDrvGstatsGetPortCounter(driverInfoPtr,localMacPortNum,counter+1, &counterValueHiTmp);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* Read Low (or only) part of counter */
    rc = prvCpssDrvGstatsGetPortCounter(driverInfoPtr,localMacPortNum,counter, &counterValueTmp.l[0]);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (isDoubleCounter)
    {
        /* Read High part of 64-bit counter - 2nd. time */
        rc = prvCpssDrvGstatsGetPortCounter(driverInfoPtr,localMacPortNum,counter+1, &counterValueTmp.l[1]);
        if(rc != GT_OK)
        {
            return rc;
        }
        if (counterValueTmp.l[1] != counterValueHiTmp)
        {
            /* High part of 64-bit counter has changed - read Low part again */
            rc = prvCpssDrvGstatsGetPortCounter(driverInfoPtr,localMacPortNum,counter, &counterValueTmp.l[0]);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /* Adjust counter value according to Clear on Read emulation */
    *cntrValuePtr = prvCpssMathSub64(counterValueTmp,*mibShadowPtr);

    /* Save shadow counter value according to Clear on Read emulation*/
    if (prvRemoteMacInfoPtr->clearOnReadEnable)
    {
        *mibShadowPtr = counterValueTmp;
    }

    return  GT_OK;
}

static GT_STATUS internal_macPhyCountersOnPortGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArrayPtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *internalInfoPtr;
    PRV_CPSS_DXCH_CFG_REMOTE_MAC_INFO_STC *prvRemoteMacInfoPtr;
    CPSS_PORT_MAC_COUNTER_SET_STC                   *mibShadowPtr;

    GT_CPSS_STATS_COUNTER_SET    statsCounterSet;    /* Temporary result of counter read */

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    internalInfoPtr = driverInfoPtr->appData;
    prvRemoteMacInfoPtr = &internalInfoPtr->prvRemoteMacInfoArr[localMacPortNum];
    mibShadowPtr = &prvRemoteMacInfoPtr->mibShadow;
    cpssOsMemSet (portMacCounterSetArrayPtr, 0, sizeof(*portMacCounterSetArrayPtr));
    cpssOsMemSet (&statsCounterSet, 0, sizeof(statsCounterSet));

    /* Read all counters for one port */
    rc = prvCpssDrvGstatsGetPortAllCounters(driverInfoPtr,localMacPortNum, &statsCounterSet);
    if(rc != GT_OK)
    {
        return rc;
    }
    portMacCounterSetArrayPtr->goodOctetsRcv.l[0] = statsCounterSet.InGoodOctetsLo;
    portMacCounterSetArrayPtr->goodOctetsRcv.l[1] = statsCounterSet.InGoodOctetsHi;
    portMacCounterSetArrayPtr->badOctetsRcv.l[0] = statsCounterSet.InBadOctets;
    portMacCounterSetArrayPtr->macTransmitErr.l[0] = statsCounterSet.OutFCSErr;
    portMacCounterSetArrayPtr->brdcPktsRcv.l[0] = statsCounterSet.InBroadcasts;
    portMacCounterSetArrayPtr->mcPktsRcv.l[0] = statsCounterSet.InMulticasts;
    portMacCounterSetArrayPtr->pkts64Octets.l[0] = statsCounterSet.Octets64;
    portMacCounterSetArrayPtr->pkts65to127Octets.l[0] = statsCounterSet.Octets127;
    portMacCounterSetArrayPtr->pkts128to255Octets.l[0] = statsCounterSet.Octets255;
    portMacCounterSetArrayPtr->pkts256to511Octets.l[0] = statsCounterSet.Octets511;
    portMacCounterSetArrayPtr->pkts512to1023Octets.l[0] = statsCounterSet.Octets1023;
    portMacCounterSetArrayPtr->pkts1024tomaxOoctets.l[0] = statsCounterSet.OctetsMax;
    portMacCounterSetArrayPtr->goodOctetsSent.l[0] = statsCounterSet.OutOctetsLo;
    portMacCounterSetArrayPtr->goodOctetsSent.l[1] = statsCounterSet.OutOctetsHi;
    portMacCounterSetArrayPtr->excessiveCollisions.l[0] = statsCounterSet.Excessive;
    portMacCounterSetArrayPtr->mcPktsSent.l[0] = statsCounterSet.OutMulticasts;
    portMacCounterSetArrayPtr->brdcPktsSent.l[0] = statsCounterSet.OutBroadcasts;
    portMacCounterSetArrayPtr->fcSent.l[0] = statsCounterSet.OutPause;
    portMacCounterSetArrayPtr->goodFcRcv.l[0] = statsCounterSet.InPause;
    portMacCounterSetArrayPtr->dropEvents.l[0] = statsCounterSet.InDiscards;
    portMacCounterSetArrayPtr->undersizePkts.l[0] = statsCounterSet.InUndersize;
    portMacCounterSetArrayPtr->fragmentsPkts.l[0] = statsCounterSet.InFragments;
    portMacCounterSetArrayPtr->oversizePkts.l[0] = statsCounterSet.InOversize;
    portMacCounterSetArrayPtr->jabberPkts.l[0] = statsCounterSet.InJabber;
    portMacCounterSetArrayPtr->macRcvError.l[0] = statsCounterSet.InRxErr;
    portMacCounterSetArrayPtr->badCrc.l[0] = statsCounterSet.InFCSErr;
    portMacCounterSetArrayPtr->collisions.l[0] = statsCounterSet.Collisions;
    portMacCounterSetArrayPtr->lateCollisions.l[0] = statsCounterSet.Late;
    portMacCounterSetArrayPtr->ucPktsRcv.l[0] = statsCounterSet.InUnicasts;
    portMacCounterSetArrayPtr->ucPktsSent.l[0] = statsCounterSet.OutUnicasts;
    portMacCounterSetArrayPtr->multiplePktsSent.l[0] = statsCounterSet.Multiple;
    portMacCounterSetArrayPtr->deferredPktsSent.l[0] = statsCounterSet.Deferred;

    /* Adjust counter value according to Clear on Read emulation */
    portMacCounterSetArrayPtr->goodOctetsRcv = prvCpssMathSub64(
                        portMacCounterSetArrayPtr->goodOctetsRcv,
                        mibShadowPtr->goodOctetsRcv);
    portMacCounterSetArrayPtr->badOctetsRcv.l[0] -= mibShadowPtr->badOctetsRcv.l[0];
    portMacCounterSetArrayPtr->macTransmitErr.l[0] -= mibShadowPtr->macTransmitErr.l[0];
    portMacCounterSetArrayPtr->brdcPktsRcv.l[0] -= mibShadowPtr->brdcPktsRcv.l[0];
    portMacCounterSetArrayPtr->mcPktsRcv.l[0] -= mibShadowPtr->mcPktsRcv.l[0];
    portMacCounterSetArrayPtr->pkts64Octets.l[0] -= mibShadowPtr->pkts64Octets.l[0];
    portMacCounterSetArrayPtr->pkts65to127Octets.l[0] -= mibShadowPtr->pkts65to127Octets.l[0];
    portMacCounterSetArrayPtr->pkts128to255Octets.l[0] -= mibShadowPtr->pkts128to255Octets.l[0];
    portMacCounterSetArrayPtr->pkts256to511Octets.l[0] -= mibShadowPtr->pkts256to511Octets.l[0];
    portMacCounterSetArrayPtr->pkts512to1023Octets.l[0] -= mibShadowPtr->pkts512to1023Octets.l[0];
    portMacCounterSetArrayPtr->pkts1024tomaxOoctets.l[0] -= mibShadowPtr->pkts1024tomaxOoctets.l[0];
    portMacCounterSetArrayPtr->goodOctetsSent = prvCpssMathSub64(
                        portMacCounterSetArrayPtr->goodOctetsSent,
                        mibShadowPtr->goodOctetsSent);
    portMacCounterSetArrayPtr->excessiveCollisions.l[0] -= mibShadowPtr->excessiveCollisions.l[0];
    portMacCounterSetArrayPtr->mcPktsSent.l[0] -= mibShadowPtr->mcPktsSent.l[0];
    portMacCounterSetArrayPtr->brdcPktsSent.l[0] -= mibShadowPtr->brdcPktsSent.l[0];
    portMacCounterSetArrayPtr->fcSent.l[0] -= mibShadowPtr->fcSent.l[0];
    portMacCounterSetArrayPtr->goodFcRcv.l[0] -= mibShadowPtr->goodFcRcv.l[0];
    portMacCounterSetArrayPtr->dropEvents.l[0] -= mibShadowPtr->dropEvents.l[0];
    portMacCounterSetArrayPtr->undersizePkts.l[0] -= mibShadowPtr->undersizePkts.l[0];
    portMacCounterSetArrayPtr->fragmentsPkts.l[0] -= mibShadowPtr->fragmentsPkts.l[0];
    portMacCounterSetArrayPtr->oversizePkts.l[0] -= mibShadowPtr->oversizePkts.l[0];
    portMacCounterSetArrayPtr->jabberPkts.l[0] -= mibShadowPtr->jabberPkts.l[0];
    portMacCounterSetArrayPtr->macRcvError.l[0] -= mibShadowPtr->macRcvError.l[0];
    portMacCounterSetArrayPtr->badCrc.l[0] -= mibShadowPtr->badCrc.l[0];
    portMacCounterSetArrayPtr->collisions.l[0] -= mibShadowPtr->collisions.l[0];
    portMacCounterSetArrayPtr->lateCollisions.l[0] -= mibShadowPtr->lateCollisions.l[0];
    portMacCounterSetArrayPtr->ucPktsRcv.l[0] -= mibShadowPtr->ucPktsRcv.l[0];
    portMacCounterSetArrayPtr->ucPktsSent.l[0] -= mibShadowPtr->ucPktsSent.l[0];
    portMacCounterSetArrayPtr->multiplePktsSent.l[0] -= mibShadowPtr->multiplePktsSent.l[0];
    portMacCounterSetArrayPtr->deferredPktsSent.l[0] -= mibShadowPtr->deferredPktsSent.l[0];

    /* Save shadow counter value according to Clear on Read emulation*/
    if (prvRemoteMacInfoPtr->clearOnReadEnable)
    {
        mibShadowPtr->goodOctetsRcv.l[0] = statsCounterSet.InGoodOctetsLo;
        mibShadowPtr->goodOctetsRcv.l[1] = statsCounterSet.InGoodOctetsHi;
        mibShadowPtr->badOctetsRcv.l[0] = statsCounterSet.InBadOctets;
        mibShadowPtr->macTransmitErr.l[0] = statsCounterSet.OutFCSErr;
        mibShadowPtr->brdcPktsRcv.l[0] = statsCounterSet.InBroadcasts;
        mibShadowPtr->mcPktsRcv.l[0] = statsCounterSet.InMulticasts;
        mibShadowPtr->pkts64Octets.l[0] = statsCounterSet.Octets64;
        mibShadowPtr->pkts65to127Octets.l[0] = statsCounterSet.Octets127;
        mibShadowPtr->pkts128to255Octets.l[0] = statsCounterSet.Octets255;
        mibShadowPtr->pkts256to511Octets.l[0] = statsCounterSet.Octets511;
        mibShadowPtr->pkts512to1023Octets.l[0] = statsCounterSet.Octets1023;
        mibShadowPtr->pkts1024tomaxOoctets.l[0] = statsCounterSet.OctetsMax;
        mibShadowPtr->goodOctetsSent.l[0] = statsCounterSet.OutOctetsLo;
        mibShadowPtr->goodOctetsSent.l[1] = statsCounterSet.OutOctetsHi;
        mibShadowPtr->excessiveCollisions.l[0] = statsCounterSet.Excessive;
        mibShadowPtr->mcPktsSent.l[0] = statsCounterSet.OutMulticasts;
        mibShadowPtr->brdcPktsSent.l[0] = statsCounterSet.OutBroadcasts;
        mibShadowPtr->unrecogMacCntrRcv.l[0] = 0;
        mibShadowPtr->fcSent.l[0] = statsCounterSet.OutPause;
        mibShadowPtr->goodFcRcv.l[0] = statsCounterSet.InPause;
        mibShadowPtr->dropEvents.l[0] = statsCounterSet.InDiscards;
        mibShadowPtr->undersizePkts.l[0] = statsCounterSet.InUndersize;
        mibShadowPtr->fragmentsPkts.l[0] = statsCounterSet.InFragments;
        mibShadowPtr->oversizePkts.l[0] = statsCounterSet.InOversize;
        mibShadowPtr->jabberPkts.l[0] = statsCounterSet.InJabber;
        mibShadowPtr->macRcvError.l[0] = statsCounterSet.InRxErr;
        mibShadowPtr->badCrc.l[0] = statsCounterSet.InFCSErr;
        mibShadowPtr->collisions.l[0] = statsCounterSet.Collisions;
        mibShadowPtr->lateCollisions.l[0] = statsCounterSet.Late;
        mibShadowPtr->badFcRcv.l[0] = 0;
        mibShadowPtr->ucPktsRcv.l[0] = statsCounterSet.InUnicasts;
        mibShadowPtr->ucPktsSent.l[0] = statsCounterSet.OutUnicasts;
        mibShadowPtr->multiplePktsSent.l[0] = statsCounterSet.Multiple;
        mibShadowPtr->deferredPktsSent.l[0] = statsCounterSet.Deferred;
        mibShadowPtr->pkts1024to1518Octets.l[0] = 0;
        mibShadowPtr->pkts1519toMaxOctets.l[0] = 0;
    }

    return  GT_OK;
}

static GT_STATUS internal_macPhyCounterCaptureGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
)
{
    GT_STATUS   rc;
    GT_U32      localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *internalInfoPtr;
    PRV_CPSS_DXCH_CFG_REMOTE_MAC_INFO_STC *prvRemoteMacInfoPtr;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    internalInfoPtr = driverInfoPtr->appData;
    if (internalInfoPtr->connectedPhyMacInfo.useMibCounterReadByPacket == GT_FALSE)
    {
        /* Use regular SMI reads */
        return internal_macPhyCounterGet(
                        devNum,
                        portNum,
                        cntrName,
                        cntrValuePtr,
                        stage,
                        doPpMacConfigPtr);
    }

    /* Use MIB counter read by packet */
    prvRemoteMacInfoPtr = &internalInfoPtr->prvRemoteMacInfoArr[localMacPortNum];
    if (prvRemoteMacInfoPtr->captureMibShadowValid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Triggered MIB counters read is not ready.");
    }

    /* Return captured counter */
    switch(cntrName)
    {
        case CPSS_GOOD_OCTETS_RCV_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.goodOctetsRcv;
            break;
        case CPSS_BAD_OCTETS_RCV_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.badOctetsRcv;
            break;
        case CPSS_MAC_TRANSMIT_ERR_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.macTransmitErr;
            break;
        case CPSS_BRDC_PKTS_RCV_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.brdcPktsRcv;
            break;
        case CPSS_MC_PKTS_RCV_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.mcPktsRcv;
            break;
        case CPSS_PKTS_64_OCTETS_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.pkts64Octets;
            break;
        case CPSS_PKTS_65TO127_OCTETS_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.pkts65to127Octets;
            break;
        case CPSS_PKTS_128TO255_OCTETS_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.pkts128to255Octets;
            break;
        case CPSS_PKTS_256TO511_OCTETS_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.pkts256to511Octets;
            break;
        case CPSS_PKTS_512TO1023_OCTETS_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.pkts512to1023Octets;
            break;
        case CPSS_PKTS_1024TOMAX_OCTETS_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.pkts1024tomaxOoctets;
            break;
        case CPSS_GOOD_OCTETS_SENT_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.goodOctetsSent;
            break;
        case CPSS_EXCESSIVE_COLLISIONS_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.excessiveCollisions;
            break;
        case CPSS_MC_PKTS_SENT_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.mcPktsSent;
            break;
        case CPSS_BRDC_PKTS_SENT_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.brdcPktsSent;
            break;
        case CPSS_FC_SENT_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.fcSent;
            break;
        case CPSS_GOOD_FC_RCV_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.goodFcRcv;
            break;
        case CPSS_DROP_EVENTS_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.dropEvents;
            break;
        case CPSS_UNDERSIZE_PKTS_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.undersizePkts;
            break;
        case CPSS_FRAGMENTS_PKTS_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.fragmentsPkts;
            break;
        case CPSS_OVERSIZE_PKTS_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.oversizePkts;
            break;
        case CPSS_JABBER_PKTS_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.jabberPkts;
            break;
        case CPSS_MAC_RCV_ERROR_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.macRcvError;
            break;
        case CPSS_BAD_CRC_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.badCrc;
            break;
        case CPSS_COLLISIONS_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.collisions;
            break;
        case CPSS_LATE_COLLISIONS_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.lateCollisions;
            break;
        case CPSS_GOOD_UC_PKTS_RCV_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.ucPktsRcv;
            break;
        case CPSS_GOOD_UC_PKTS_SENT_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.ucPktsSent;
            break;
        case CPSS_MULTIPLE_PKTS_SENT_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.multiplePktsSent;
            break;
        case CPSS_DEFERRED_PKTS_SENT_E:
            *cntrValuePtr = prvRemoteMacInfoPtr->captureMibShadow.deferredPktsSent;
            break;

        case CPSS_GOOD_PKTS_RCV_E:
        case CPSS_BAD_PKTS_RCV_E:
        case CPSS_GOOD_PKTS_SENT_E:
        case CPSS_UNRECOG_MAC_CNTR_RCV_E:
        case CPSS_PKTS_1024TO1518_OCTETS_E:
        case CPSS_PKTS_1519TOMAX_OCTETS_E:
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

static GT_STATUS internal_macPhyCountersCaptureOnPortGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArrayPtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
)
{
    GT_STATUS   rc;
    GT_U32      localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *internalInfoPtr;
    PRV_CPSS_DXCH_CFG_REMOTE_MAC_INFO_STC *prvRemoteMacInfoPtr;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    internalInfoPtr = driverInfoPtr->appData;
    if (internalInfoPtr->connectedPhyMacInfo.useMibCounterReadByPacket == GT_FALSE)
    {
        /* Use regular SMI reads */
        return internal_macPhyCountersOnPortGet(
                        devNum,
                        portNum,
                        portMacCounterSetArrayPtr,
                        stage,
                        doPpMacConfigPtr);
    }

    /* Use MIB counter read by packet */
    prvRemoteMacInfoPtr = &internalInfoPtr->prvRemoteMacInfoArr[localMacPortNum];
    if (prvRemoteMacInfoPtr->captureMibShadowValid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Triggered MIB counters read is not ready.");
    }

    /* Return captured counter set */
    *portMacCounterSetArrayPtr = prvRemoteMacInfoPtr->captureMibShadow;

    return GT_OK;
}

static GT_STATUS internal_macPhyCountersCaptureTriggerSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *internalInfoPtr;
    PRV_CPSS_DXCH_CFG_REMOTE_MAC_INFO_STC *prvRemoteMacInfoPtr;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    internalInfoPtr = driverInfoPtr->appData;
    if (internalInfoPtr->connectedPhyMacInfo.useMibCounterReadByPacket == GT_FALSE)
    {
        /* Use regular SMI reads -No need to trigger*/
        return GT_OK;
    }

    /* Use MIB counter read by packet - Trigger read by firmware */
    prvRemoteMacInfoPtr = &internalInfoPtr->prvRemoteMacInfoArr[localMacPortNum];
    prvRemoteMacInfoPtr->captureMibShadowValid = GT_FALSE;

    /* Write command to firmware in ASCII string */
    /* Write op-code + port number */
    rc = prvCpssDrvImpWriteComm(driverInfoPtr, (GT_U8) ('0' + localMacPortNum));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvImpWriteComm");
    }

    return GT_OK;
}

static GT_STATUS internal_macPhyCountersCaptureTriggerGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                     *captureIsDonePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *internalInfoPtr;
    PRV_CPSS_DXCH_CFG_REMOTE_MAC_INFO_STC *prvRemoteMacInfoPtr;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    internalInfoPtr = driverInfoPtr->appData;
    if (internalInfoPtr->connectedPhyMacInfo.useMibCounterReadByPacket == GT_FALSE)
    {
        /* Use regular SMI reads -No need to trigger*/
        *captureIsDonePtr = GT_TRUE;
        return GT_OK;
    }

    /* Use MIB counter read by packet - Trigger read by firmware */
    prvRemoteMacInfoPtr = &internalInfoPtr->prvRemoteMacInfoArr[localMacPortNum];
    *captureIsDonePtr = prvRemoteMacInfoPtr->captureMibShadowValid;

    return GT_OK;
}

static GT_STATUS internal_macPhyCountersEnable
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN GT_BOOL                      enable,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchEnableSetPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;

    UNUSED_PARAM(switchEnableSetPtr);
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    /* MIB counters are always enabled at this device */
    if (enable)
    {
        return GT_OK;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
}

static GT_STATUS internal_macPhyCountersEnableGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                     *enablePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    /* MIB counters are always enabled at this device */
    *enablePtr = GT_TRUE;

    return  GT_OK;
}

static GT_STATUS internal_macPhyCountersClearOnReadSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN GT_BOOL                      enable,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchEnableSetPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *internalInfoPtr;
    PRV_CPSS_DXCH_CFG_REMOTE_MAC_INFO_STC *prvRemoteMacInfoPtr;

    UNUSED_PARAM(switchEnableSetPtr);
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    internalInfoPtr = driverInfoPtr->appData;
    prvRemoteMacInfoPtr = &internalInfoPtr->prvRemoteMacInfoArr[localMacPortNum];

    prvRemoteMacInfoPtr->clearOnReadEnable = enable;

    return  GT_OK;
}

static GT_STATUS internal_macPhyCountersClearOnReadGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                     *enablePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *internalInfoPtr;
    PRV_CPSS_DXCH_CFG_REMOTE_MAC_INFO_STC *prvRemoteMacInfoPtr;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    internalInfoPtr = driverInfoPtr->appData;
    prvRemoteMacInfoPtr = &internalInfoPtr->prvRemoteMacInfoArr[localMacPortNum];

    *enablePtr = prvRemoteMacInfoPtr->clearOnReadEnable;

    return  GT_OK;
}

static GT_STATUS internal_macPhyCountersRxHistogramEnable
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN GT_BOOL                      enable,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchEnableSetPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;

    GT_CPSS_HISTOGRAM_MODE    gtHistogramMode;    /* Temporary Histogram mode */

    UNUSED_PARAM(switchEnableSetPtr);
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    rc = prvCpssDrvGstatsGetHistogramMode(driverInfoPtr,&gtHistogramMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (enable)
    {
        gtHistogramMode |= GT_CPSS_COUNT_RX_ONLY;
    }
    else
    {
        gtHistogramMode &= ~GT_CPSS_COUNT_RX_ONLY;
    }

    if(gtHistogramMode == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "88e1690 Histogram can not disabled for Rx and for Tx : portNum[%d] \n", portNum);
    }

    rc = prvCpssDrvGstatsSetHistogramMode(driverInfoPtr,gtHistogramMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    return  GT_OK;
}

static GT_STATUS internal_macPhyCountersTxHistogramEnable
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN GT_BOOL                      enable,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr,
    OUT GT_BOOL                 *switchEnableSetPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;

    GT_CPSS_HISTOGRAM_MODE    gtHistogramMode;    /* Temporary Histogram mode */

    UNUSED_PARAM(switchEnableSetPtr);
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    rc = prvCpssDrvGstatsGetHistogramMode(driverInfoPtr,&gtHistogramMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (enable)
    {
        gtHistogramMode |= GT_CPSS_COUNT_TX_ONLY;
    }
    else
    {
        gtHistogramMode &= ~GT_CPSS_COUNT_TX_ONLY;
    }

    if(gtHistogramMode == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "88e1690 Histogram can not disabled for Rx and for Tx : portNum[%d] \n", portNum);
    }

    rc = prvCpssDrvGstatsSetHistogramMode(driverInfoPtr,gtHistogramMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    return  GT_OK;
}

static GT_STATUS internal_macPhyCountersHistogramEnableGet
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    OUT GT_BOOL  * enablePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;

    GT_CPSS_HISTOGRAM_MODE    gtHistogramMode;    /* Temporary Histogram mode */

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    rc = prvCpssDrvGstatsGetHistogramMode(driverInfoPtr,&gtHistogramMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch (direction)
    {
        case CPSS_PORT_DIRECTION_RX_E:
            *enablePtr = ((gtHistogramMode == GT_CPSS_COUNT_RX_ONLY) || (gtHistogramMode == GT_CPSS_COUNT_RX_TX));
            break;
        case CPSS_PORT_DIRECTION_TX_E:
            *enablePtr = ((gtHistogramMode == GT_CPSS_COUNT_TX_ONLY) || (gtHistogramMode == GT_CPSS_COUNT_RX_TX));
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return  GT_OK;
}

static GT_STATUS internal_macPhyCounterOnPhySideGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        cascadePortNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
)
{
    GT_STATUS        rc;
    GT_U32             localMacPortNum = 9;    /* Local MAC port num of Phy side cascade port */
    GT_CPSS_QD_DEV       *driverInfoPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr;
    GT_CPSS_STATS_COUNTERS  counter;                 /* Stat counter to read */
    GT_BOOL            isDoubleCounter;         /* Whether counter is 64-bits */
    GT_U32             counterValueHiTmp = 0;   /* Temporary result of Hi counter read */


    UNUSED_PARAM(stage);

    *doPpMacConfigPtr = GT_FALSE;

    remotePhyMacInfoPtr = prvCpssDxChCfgPort88e1690CascadePortToInfoGet(devNum,cascadePortNum);
    if (remotePhyMacInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Failed (Bad Serdes Port)");
    }
    driverInfoPtr = (GT_CPSS_QD_DEV *) remotePhyMacInfoPtr->drvInfoPtr;

    cntrValuePtr->l[0] = 0;
    cntrValuePtr->l[1] = 0;
    isDoubleCounter = GT_FALSE;
    switch(cntrName)
    {
        case CPSS_GOOD_OCTETS_RCV_E:
            counter = CPSS_STATS_InGoodOctetsLo;
            isDoubleCounter = GT_TRUE;
            break;
        case CPSS_BAD_OCTETS_RCV_E:
            counter = CPSS_STATS_InBadOctets;
            break;
        case CPSS_MAC_TRANSMIT_ERR_E:
            counter = CPSS_STATS_OutFCSErr;
            break;
        case CPSS_BRDC_PKTS_RCV_E:
            counter = CPSS_STATS_InBroadcasts;
            break;
        case CPSS_MC_PKTS_RCV_E:
            counter = CPSS_STATS_InMulticasts;
            break;
        case CPSS_PKTS_64_OCTETS_E:
            counter = CPSS_STATS_64Octets;
            break;
        case CPSS_PKTS_65TO127_OCTETS_E:
            counter = CPSS_STATS_127Octets;
            break;
        case CPSS_PKTS_128TO255_OCTETS_E:
            counter = CPSS_STATS_255Octets;
            break;
        case CPSS_PKTS_256TO511_OCTETS_E:
            counter = CPSS_STATS_511Octets;
            break;
        case CPSS_PKTS_512TO1023_OCTETS_E:
            counter = CPSS_STATS_1023Octets;
            break;
        case CPSS_PKTS_1024TOMAX_OCTETS_E:
            counter = CPSS_STATS_MaxOctets;
            break;
        case CPSS_GOOD_OCTETS_SENT_E:
            counter = CPSS_STATS_OutOctetsLo;
            isDoubleCounter = GT_TRUE;
            break;
        case CPSS_EXCESSIVE_COLLISIONS_E:
            counter = CPSS_STATS_Excessive;
            break;
        case CPSS_MC_PKTS_SENT_E:
            counter = CPSS_STATS_OutMulticasts;
            break;
        case CPSS_BRDC_PKTS_SENT_E:
            counter = CPSS_STATS_OutBroadcasts;
            break;
        case CPSS_FC_SENT_E:
            counter = CPSS_STATS_OutPause;
            break;
        case CPSS_GOOD_FC_RCV_E:
            counter = CPSS_STATS_InPause;
            break;
        case CPSS_DROP_EVENTS_E:
            counter = CPSS_STATS_InDiscards;
            break;
        case CPSS_UNDERSIZE_PKTS_E:
            counter = CPSS_STATS_InUndersize;
            break;
        case CPSS_FRAGMENTS_PKTS_E:
            counter = CPSS_STATS_InFragments;
            break;
        case CPSS_OVERSIZE_PKTS_E:
            counter = CPSS_STATS_InOversize;
            break;
        case CPSS_JABBER_PKTS_E:
            counter = CPSS_STATS_InJabber;
            break;
        case CPSS_MAC_RCV_ERROR_E:
            counter = CPSS_STATS_InRxErr;
            break;
        case CPSS_BAD_CRC_E:
            counter = CPSS_STATS_InFCSErr;
            break;
        case CPSS_COLLISIONS_E:
            counter = CPSS_STATS_Collisions;
            break;
        case CPSS_LATE_COLLISIONS_E:
            counter = CPSS_STATS_Late;
            break;
        case CPSS_GOOD_UC_PKTS_RCV_E:
            counter = CPSS_STATS_InUnicasts;
            break;
        case CPSS_GOOD_UC_PKTS_SENT_E:
            counter = CPSS_STATS_OutUnicasts;
            break;
        case CPSS_MULTIPLE_PKTS_SENT_E:
            counter = CPSS_STATS_Multiple;
            break;
        case CPSS_DEFERRED_PKTS_SENT_E:
            counter = CPSS_STATS_Deferred;
            break;

        case CPSS_GOOD_PKTS_RCV_E:
        case CPSS_BAD_PKTS_RCV_E:
        case CPSS_GOOD_PKTS_SENT_E:
        case CPSS_UNRECOG_MAC_CNTR_RCV_E:
        case CPSS_PKTS_1024TO1518_OCTETS_E:
        case CPSS_PKTS_1519TOMAX_OCTETS_E:
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (isDoubleCounter)
    {
        /* Read High part of 64-bit counter - 1st. time */
        rc = prvCpssDrvGstatsGetPortCounter(driverInfoPtr,localMacPortNum,counter+1, &counterValueHiTmp);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* Read Low (or only) part of counter */
    rc = prvCpssDrvGstatsGetPortCounter(driverInfoPtr,localMacPortNum,counter, &cntrValuePtr->l[0]);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (isDoubleCounter)
    {
        /* Read High part of 64-bit counter - 2nd. time */
        rc = prvCpssDrvGstatsGetPortCounter(driverInfoPtr,localMacPortNum,counter+1, &cntrValuePtr->l[1]);
        if(rc != GT_OK)
        {
            return rc;
        }
        if (cntrValuePtr->l[1] != counterValueHiTmp)
        {
            /* High part of 64-bit counter has changed - read Low part again */
            rc = prvCpssDrvGstatsGetPortCounter(driverInfoPtr,localMacPortNum,counter, &cntrValuePtr->l[0]);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return  GT_OK;
}

static GT_STATUS internal_macPhyCountersOnPhySidePortGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        cascadePortNum,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArrayPtr,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
)
{
    GT_STATUS rc;
    GT_U32      localMacPortNum = 9;    /* Local MAC port num of Phy side cascade port */
    GT_CPSS_QD_DEV   *driverInfoPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr;
    GT_CPSS_STATS_COUNTER_SET    statsCounterSet;    /* Temporary result of counter read */

    UNUSED_PARAM(stage);

    *doPpMacConfigPtr = GT_FALSE;

    remotePhyMacInfoPtr = prvCpssDxChCfgPort88e1690CascadePortToInfoGet(devNum,cascadePortNum);
    if (remotePhyMacInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Failed (Bad Serdes Port)");
    }
    driverInfoPtr = (GT_CPSS_QD_DEV *) remotePhyMacInfoPtr->drvInfoPtr;

    cpssOsMemSet (portMacCounterSetArrayPtr, 0, sizeof(*portMacCounterSetArrayPtr));
    cpssOsMemSet (&statsCounterSet, 0, sizeof(statsCounterSet));

    /* Read all counters for one port */
    rc = prvCpssDrvGstatsGetPortAllCounters(driverInfoPtr,localMacPortNum, &statsCounterSet);
    if(rc != GT_OK)
    {
        return rc;
    }
    portMacCounterSetArrayPtr->goodOctetsRcv.l[0] = statsCounterSet.InGoodOctetsLo;
    portMacCounterSetArrayPtr->goodOctetsRcv.l[1] = statsCounterSet.InGoodOctetsHi;
    portMacCounterSetArrayPtr->badOctetsRcv.l[0] = statsCounterSet.InBadOctets;
    portMacCounterSetArrayPtr->macTransmitErr.l[0] = statsCounterSet.OutFCSErr;
    portMacCounterSetArrayPtr->brdcPktsRcv.l[0] = statsCounterSet.InBroadcasts;
    portMacCounterSetArrayPtr->mcPktsRcv.l[0] = statsCounterSet.InMulticasts;
    portMacCounterSetArrayPtr->pkts64Octets.l[0] = statsCounterSet.Octets64;
    portMacCounterSetArrayPtr->pkts65to127Octets.l[0] = statsCounterSet.Octets127;
    portMacCounterSetArrayPtr->pkts128to255Octets.l[0] = statsCounterSet.Octets255;
    portMacCounterSetArrayPtr->pkts256to511Octets.l[0] = statsCounterSet.Octets511;
    portMacCounterSetArrayPtr->pkts512to1023Octets.l[0] = statsCounterSet.Octets1023;
    portMacCounterSetArrayPtr->pkts1024tomaxOoctets.l[0] = statsCounterSet.OctetsMax;
    portMacCounterSetArrayPtr->goodOctetsSent.l[0] = statsCounterSet.OutOctetsLo;
    portMacCounterSetArrayPtr->goodOctetsSent.l[1] = statsCounterSet.OutOctetsHi;
    portMacCounterSetArrayPtr->excessiveCollisions.l[0] = statsCounterSet.Excessive;
    portMacCounterSetArrayPtr->mcPktsSent.l[0] = statsCounterSet.OutMulticasts;
    portMacCounterSetArrayPtr->brdcPktsSent.l[0] = statsCounterSet.OutBroadcasts;
    portMacCounterSetArrayPtr->fcSent.l[0] = statsCounterSet.OutPause;
    portMacCounterSetArrayPtr->goodFcRcv.l[0] = statsCounterSet.InPause;
    portMacCounterSetArrayPtr->dropEvents.l[0] = statsCounterSet.InDiscards;
    portMacCounterSetArrayPtr->undersizePkts.l[0] = statsCounterSet.InUndersize;
    portMacCounterSetArrayPtr->fragmentsPkts.l[0] = statsCounterSet.InFragments;
    portMacCounterSetArrayPtr->oversizePkts.l[0] = statsCounterSet.InOversize;
    portMacCounterSetArrayPtr->jabberPkts.l[0] = statsCounterSet.InJabber;
    portMacCounterSetArrayPtr->macRcvError.l[0] = statsCounterSet.InRxErr;
    portMacCounterSetArrayPtr->badCrc.l[0] = statsCounterSet.InFCSErr;
    portMacCounterSetArrayPtr->collisions.l[0] = statsCounterSet.Collisions;
    portMacCounterSetArrayPtr->lateCollisions.l[0] = statsCounterSet.Late;
    portMacCounterSetArrayPtr->ucPktsRcv.l[0] = statsCounterSet.InUnicasts;
    portMacCounterSetArrayPtr->ucPktsSent.l[0] = statsCounterSet.OutUnicasts;
    portMacCounterSetArrayPtr->multiplePktsSent.l[0] = statsCounterSet.Multiple;
    portMacCounterSetArrayPtr->deferredPktsSent.l[0] = statsCounterSet.Deferred;

    return  GT_OK;
}

/**
* @internal internal_macPhyHwSmiRegisterSet function
* @endinternal
*
* @brief   Write value to specified SMI Register on a specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] phyReg                   - SMI register
* @param[in] data                     -  to write.
* @param[in] stage                    - port callback function stage
*
* @param[out] doPpMacConfigPtr         - (pointer to) port MAC configuaration condition.
*                                      GT_TRUE - continue with port MAC configuration
*                                      GT_FALSE - bypass port MAC configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_macPhyHwSmiRegisterSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   phyReg,
    IN  GT_U16                  data,
    IN  CPSS_MACDRV_STAGE_ENT   stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
)
{
    GT_CPSS_QD_DEV *dev;
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_U8 hwPort;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&dev);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PHY(localMacPortNum);
    if (hwPort == GT_CPSS_INVALID_PHY)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Failed (Bad PHY Port)");
    }

    if (phyReg >= PRV_CPSS_BIT(5))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Failed (Bad SMI register address)");
    }

    return prvCpssDrvHwSetSMIPhyReg(dev, hwPort, phyReg, data);
}
/**
* @internal internal_macPhyHwSmiRegisterGet function
* @endinternal
*
* @brief   Read specified SMI Register on a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] phyReg                   - SMI register
* @param[in] stage                    - port callback function stage
*
* @param[out] dataPtr                  - (pointer to) the read data.
* @param[out] doPpMacConfigPtr         - (pointer to) port MAC configuaration condition.
*                                      GT_TRUE - continue with port MAC configuration
*                                      GT_FALSE - bypass port MAC configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_macPhyHwSmiRegisterGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   phyReg,
    OUT GT_U16                  *dataPtr,
    IN  CPSS_MACDRV_STAGE_ENT   stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
)
{
    GT_CPSS_QD_DEV *dev;
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_U8 hwPort;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&dev);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PHY(localMacPortNum);
    if (hwPort == GT_CPSS_INVALID_PHY)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Failed (Bad PHY Port)");
    }

    if (phyReg >= PRV_CPSS_BIT(5))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Failed (Bad SMI register address)");
    }

    return prvCpssDrvHwGetSMIPhyReg(dev, hwPort, phyReg, dataPtr);
}

/**
* @internal internal_macPhyHw10GSmiRegisterSet function
* @endinternal
*
* @brief   Write value to specified 10G SMI Register on a specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - Serdes lane number
* @param[in] phyReg                   - SMI register
* @param[in] phyDev                   - SMI device
* @param[in] data                     -  to write.
* @param[in] stage                    - port callback function stage
*
* @param[out] doPpMacConfigPtr         - (pointer to) port MAC configuaration condition.
*                                      GT_TRUE - continue with port MAC configuration
*                                      GT_FALSE - bypass port MAC configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_macPhyHw10GSmiRegisterSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   laneNum,
    IN  GT_U16                  phyReg,
    IN  GT_U8                   phyDev,
    IN  GT_U16                  data,
    IN  CPSS_MACDRV_STAGE_ENT   stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
)
{
    GT_CPSS_QD_DEV *dev;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr;

    UNUSED_PARAM(stage);

    *doPpMacConfigPtr = GT_FALSE;
    if (laneNum >= serdesSmiAdressArrayLen)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Failed (Bad Serdes Lane)");
    }

    remotePhyMacInfoPtr = prvCpssDxChCfgPort88e1690CascadePortToInfoGet(devNum,portNum);
    if (remotePhyMacInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Failed (Bad Serdes Port)");
    }
    dev = (GT_CPSS_QD_DEV *) remotePhyMacInfoPtr->drvInfoPtr;

    return prvCpssDrvHwSetSMIC45PhyReg(dev,
                phyDev,
                serdesSmiAdressArray[laneNum],
                phyReg,
                data);
}

/**
* @internal internal_macPhyHw10GSmiRegisterGet function
* @endinternal
*
* @brief   Read specified 10G SMI Register on a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - Serdes lane number
* @param[in] phyReg                   - SMI register
* @param[in] phyDev                   - SMI device
* @param[in] stage                    - port callback function stage
*
* @param[out] dataPtr                  - (pointer to) the read data.
* @param[out] doPpMacConfigPtr         - (pointer to) port MAC configuaration condition.
*                                      GT_TRUE - continue with port MAC configuration
*                                      GT_FALSE - bypass port MAC configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_macPhyHw10GSmiRegisterGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   laneNum,
    IN  GT_U16                  phyReg,
    IN  GT_U8                   phyDev,
    OUT GT_U16                  *dataPtr,
    IN  CPSS_MACDRV_STAGE_ENT   stage,
    OUT GT_BOOL                 *doPpMacConfigPtr
)
{
    GT_CPSS_QD_DEV *dev;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr;

    UNUSED_PARAM(stage);

    *doPpMacConfigPtr = GT_FALSE;
    if (laneNum >= serdesSmiAdressArrayLen)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Failed (Bad Serdes Lane)");
    }

    remotePhyMacInfoPtr = prvCpssDxChCfgPort88e1690CascadePortToInfoGet(devNum,portNum);
    if (remotePhyMacInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Failed (Bad Serdes Port)");
    }
    dev = (GT_CPSS_QD_DEV *) remotePhyMacInfoPtr->drvInfoPtr;

    return prvCpssDrvHwGetSMIC45PhyReg(dev,
                phyDev,
                serdesSmiAdressArray[laneNum],
                phyReg,
                dataPtr);
}

/**
* @internal internal_macDrvMacPortTpidProfileSet function
* @endinternal
*
* @brief   Function sets TPID to remote port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - remote port number.
* @param[in] ethMode                  - TAG0/TAG1 selector
* @param[in] profile                  - TPID profile. (APPLICABLE RANGE: 0..7)
* @param[in] stage                    - port callback function stage
*
* @param[out] doPpMacConfigPtr         - (pointer to) port MAC configuaration condition.
*                                      GT_TRUE - continue with port MAC configuration
*                                      GT_FALSE - bypass port MAC configuration
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - profile > 7
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note relevant to 'etherType_0' only ! (no limit on 'etherType_1')
*       88e1690 port : cpssDxChBrgVlanPortIngressTpidProfileSet : profile.
*       1. cpss get bmp of the profile.
*       a. bmp of '0' - treated as TPID 0x8100
*       2. if the bmp hold pointer to to '0x8100'
*       a. set the 88e1690 port as : <Frame Mode> = Normal Network (value 0)
*       else -- the bmp is NOT '0x8100' --
*       use 'first TPID' in bmp
*       a. set the 88e1690 port as : <PortEType> = etherType0 (register "Port E Type")
*       b. set the 88e1690 port as : <Frame Mode> = Provider    (value 2)
*       3. set the profile in the Aldrin (for this port)
*       cpssDxChBrgVlanIngressTpidProfileSet   : no 88e1690 change.
*       cpssDxChBrgVlanPortIngressTpidProfileSet : no 88e1690 change.
*
*/
static GT_STATUS internal_macDrvMacPortTpidProfileSet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN  GT_U32               profile,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr
)
{
    GT_CPSS_QD_DEV   *driverInfoPtr;
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_U32  tpidBmp,entryIndex;
    GT_U16  etherType = 0;
    const GT_U16 vlanTag8100 = 0x8100;
    GT_BOOL use_vlanTag8100 = GT_FALSE;
    GT_U16  used_etherType = 0;
    GT_BOOL set_used_etherType = GT_FALSE;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    *doPpMacConfigPtr = GT_TRUE;
    if(ethMode != CPSS_VLAN_ETHERTYPE0_E)
    {
        return GT_OK;
    }

    rc = cpssDxChBrgVlanIngressTpidProfileGet(devNum,profile,ethMode,&tpidBmp);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(tpidBmp == 0)
    {
        /* allow empty BMP ... meaning that no TPID should recognize packets as 'tagged' */
        /* Put the 88E1690 port in 'Network' mode - by setting the flag below */
        use_vlanTag8100 = GT_TRUE;

        /* Put the 88E1690 port in 'untagged' mode in this case */
        rc = prvCpssDrvGvlnSetPortVlanDot1qMode(driverInfoPtr,localMacPortNum,
                GT_CPSS_DISABLE);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGvlnSetPortVlanDot1qMode: failed");
        }
    }
    else
    {
        for(entryIndex = 0 ; entryIndex < 8 ; entryIndex++)
        {
            if(0 == (tpidBmp & (1<<entryIndex)))
            {
                continue;
            }

            rc = cpssDxChBrgVlanTpidEntryGet(devNum,
                CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E,
                entryIndex,
                &etherType);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(set_used_etherType == GT_FALSE)
            {
                set_used_etherType = GT_TRUE;
                used_etherType = etherType;/* use first one (if 0x8100 not in the list) */
            }

            if(etherType == vlanTag8100)
            {
                /* force to use 0x8100 ... if in the list */
                use_vlanTag8100 = GT_TRUE;
                break;
            }
        }

        /* Put the 88E1690 port back in '802.1q fallback' mode in this case */
        rc = prvCpssDrvGvlnSetPortVlanDot1qMode(driverInfoPtr,localMacPortNum,
                use_vlanTag8100 == GT_TRUE ?
                                GT_CPSS_FALLBACK :
                                GT_CPSS_DISABLE);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGvlnSetPortVlanDot1qMode: failed");
        }
    }

    if(use_vlanTag8100 == GT_FALSE && set_used_etherType == GT_TRUE)
    {
        /* we need to set the TPID to the port */
        rc = prvCpssDrvGprtSetPortEType(driverInfoPtr,localMacPortNum,used_etherType);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGprtSetPortEType : failed");
        }
    }

    rc = prvCpssDrvGprtSetFrameMode(driverInfoPtr,localMacPortNum,
            use_vlanTag8100 == GT_TRUE ?
                GT_CPSS_FRAME_MODE_NORMAL :
                GT_CPSS_FRAME_MODE_PROVIDER);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGprtSetFrameMode : failed");
    }

    return GT_OK;
}

/**
* @internal internal_portInterfaceSpeedSupportGet function
* @endinternal
*
* @brief   Check if given pair ifMode and speed supported by given port on
*         given device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
* @param[in] stage                    - port callback function stage
*
* @param[out] supportedPtr             ? GT_TRUE ? (ifMode; speed) supported
*                                      GT_FALSE ? (ifMode; speed) not supported
* @param[out] doPpMacConfigPtr         - (pointer to) port MAC configuaration condition.
*                                      GT_TRUE - continue with port MAC configuration
*                                      GT_FALSE - bypass port MAC configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if one of input parameters wrong
* @retval GT_BAD_PTR               - if supportedPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_portInterfaceSpeedSupportGet(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    OUT GT_BOOL                         *supportedPtr,
    IN  CPSS_MACDRV_STAGE_ENT           stage,
    OUT GT_BOOL                         *doPpMacConfigPtr
)
{
    GT_STATUS rc;
    GT_U32  localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    switch(ifMode)
    {
        case CPSS_PORT_INTERFACE_MODE_SGMII_E:
        case CPSS_PORT_INTERFACE_MODE_QSGMII_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_10_E:
                case CPSS_PORT_SPEED_100_E:
                case CPSS_PORT_SPEED_1000_E:
                    *supportedPtr = GT_TRUE;
                break;
            default:
                *supportedPtr = GT_FALSE;
                break;
            }
        default:
            *supportedPtr = GT_FALSE;
            break;
    }

    return GT_OK;
}

/**
* @internal internal_macDrvMacPortMacStatusGet function
* @endinternal
*
* @brief   Reads bits that indicate different problems on specified port.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] stage                    -  for callback run (PRE or POST)
*
* @param[out] portMacStatusPtr         - info about port MAC
* @param[out] doPpMacConfigPtr         - pointer to the parameter defines if the switch MAC
*                                      will be configurated
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS internal_macDrvMacPortMacStatusGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PORT_MAC_STATUS_STC    *portMacStatusPtr,
    IN  CPSS_MACDRV_STAGE_ENT       stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
)
{
    GT_STATUS   rc;
    GT_U32      localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    GT_CPSS_PORT_MAC_STATUS  portStatus;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"internal_macDrvMacPortMacStatusGet : failed");
    }

    rc = prvCpssDrvGprtGetPortMacStatus(driverInfoPtr, localMacPortNum, &portStatus);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"gprtGetPortStatus : failed");
    }

    /* Copy fields to MAC info structure */
    portMacStatusPtr->isPortRxPause = portStatus.isPortRxPause;
    portMacStatusPtr->isPortTxPause = portStatus.isPortTxPause;
    portMacStatusPtr->isPortBackPres = portStatus.isPortBackPres;
    portMacStatusPtr->isPortBufFull = GT_FALSE;
    portMacStatusPtr->isPortSyncFail = GT_FALSE;
    portMacStatusPtr->isPortHiErrorRate = GT_FALSE;
    portMacStatusPtr->isPortAnDone = GT_FALSE;
    portMacStatusPtr->isPortFatalError = GT_FALSE;

    return rc;
}


/**
* @internal internal_macDrvMacPortDefaultUPSet function
* @endinternal
*
* @brief   Set default user priority (VPT) for untagged packet to a given port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - packet processor device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] defaultUserPrio          - default user priority (VPT) (APPLICABLE RANGES: 0..7).
* @param[in] stage                    -  for callback run (PRE or POST)
*
* @param[out] doPpMacConfigPtr         - (pointer to)the parameter defines if the switch MAC will be configured
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_macDrvMacPortDefaultUPSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_U8        defaultUserPrio,
    IN CPSS_MACDRV_STAGE_ENT    stage,
    OUT GT_BOOL                *doPpMacConfigPtr
)
{
    GT_STATUS   rc;
    GT_U32      localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    rc = prvCpssDrvGqosSetDefFPri(driverInfoPtr,localMacPortNum,defaultUserPrio);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGqosSetDefFPri : failed");
    }

    /*
        NOTE : TRICKY :
        we allow the DX device to also set it's remote port with defaultUserPrio ,
        because it will allow us to use the DX value for the 'Get' function , and
        not need to add a 'Get' Callback from this PHY device
    */
    (*doPpMacConfigPtr) = GT_TRUE;

    return GT_OK;
}

static GT_STATUS internal_macPhyVctLengthOffsetSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_32                       vctLengthOffset,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
)
{
    GT_STATUS        rc;
    GT_U32           localMacPortNum;
    GT_CPSS_QD_DEV       *driverInfoPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *internalInfoPtr;
    PRV_CPSS_DXCH_CFG_REMOTE_MAC_INFO_STC *prvRemoteMacInfoPtr;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    internalInfoPtr = driverInfoPtr->appData;
    prvRemoteMacInfoPtr = &internalInfoPtr->prvRemoteMacInfoArr[localMacPortNum];

    prvRemoteMacInfoPtr->vctLengthOffset = vctLengthOffset;

    return  GT_OK;
}

static GT_STATUS internal_macPhyVctLengthOffsetGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_32                       *vctLengthOffsetPtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
)
{
    GT_STATUS        rc;
    GT_U32           localMacPortNum;
    GT_CPSS_QD_DEV       *driverInfoPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *internalInfoPtr;
    PRV_CPSS_DXCH_CFG_REMOTE_MAC_INFO_STC *prvRemoteMacInfoPtr;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    internalInfoPtr = driverInfoPtr->appData;
    prvRemoteMacInfoPtr = &internalInfoPtr->prvRemoteMacInfoArr[localMacPortNum];

    *vctLengthOffsetPtr = prvRemoteMacInfoPtr->vctLengthOffset;

    return  GT_OK;
}

static GT_STATUS internal_macPhyEeeConfigSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_MACDRV_EEE_CONFIG_STC  *eeeConfigPtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
)
{
    GT_STATUS        rc;
    GT_U32           localMacPortNum;
    GT_CPSS_QD_DEV       *driverInfoPtr;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    rc = prvCpssDrvGprtSetPortEEE(driverInfoPtr,
                localMacPortNum,
                eeeConfigPtr->enable,
                (eeeConfigPtr->mode != CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_DISABLED_E),
                eeeConfigPtr->twLimit,
                eeeConfigPtr->liLimit,
                eeeConfigPtr->tsLimit);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGprtSetPortEEE: failed");
    }

    return  GT_OK;
}

static GT_STATUS internal_macPhyEeeConfigGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_MACDRV_EEE_CONFIG_STC  *eeeConfigPtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
)
{
    GT_STATUS        rc;
    GT_U32           localMacPortNum;
    GT_CPSS_QD_DEV       *driverInfoPtr;
    GT_BOOL          manualTmp;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    rc = prvCpssDrvGprtGetPortEEE(driverInfoPtr,
                localMacPortNum,
                &eeeConfigPtr->enable,
                &manualTmp,
                &eeeConfigPtr->twLimit,
                &eeeConfigPtr->liLimit,
                &eeeConfigPtr->tsLimit);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGprtGetPortEEE: failed");
    }
    eeeConfigPtr->mode = ((manualTmp && eeeConfigPtr->enable) ?
                            CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_FORCED_E :
                            CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_DISABLED_E);

    return  GT_OK;
}

static GT_STATUS internal_macPhyEeeStatusGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                     *rxStatusPtr,
    OUT GT_BOOL                     *txStatusPtr,
    IN CPSS_MACDRV_STAGE_ENT        stage,
    OUT GT_BOOL                     *doPpMacConfigPtr
)
{
    GT_STATUS        rc;
    GT_U32           localMacPortNum;
    GT_CPSS_QD_DEV   *driverInfoPtr;
    GT_U16           data;

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    /* Read status from PHY XMDIO address space: Device 3, Register 1 */
    data = (0 << 14) | (3 << 0);
    rc = prvCpssDrvHwWritePagedPhyReg(driverInfoPtr, (GT_U8)localMacPortNum, 0, PRV_CPSS_QD_PHY_XMDIO_ACCESS_CONTROL_REG, data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, "Write PHY register HW error: page = 0, port = %d", localMacPortNum);
    }

    data = 1;
    rc = prvCpssDrvHwWritePagedPhyReg(driverInfoPtr, (GT_U8)localMacPortNum, 0, PRV_CPSS_QD_PHY_XMDIO_ACCESS_ADDRESS_DATA_REG, data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, "Write PHY register HW error: page = 0, port = %d", localMacPortNum);
    }

    data = (1 << 14) | (3 << 0);
    rc = prvCpssDrvHwWritePagedPhyReg(driverInfoPtr, (GT_U8)localMacPortNum, 0, PRV_CPSS_QD_PHY_XMDIO_ACCESS_CONTROL_REG, data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, "Write PHY register HW error: page = 0, port = %d", localMacPortNum);
    }

    rc = prvCpssDrvHwReadPagedPhyReg(driverInfoPtr, (GT_U8)localMacPortNum, 0, PRV_CPSS_QD_PHY_XMDIO_ACCESS_ADDRESS_DATA_REG, &data);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, "Read PHY register HW error: page = 0, port = %d", localMacPortNum);
    }

    *rxStatusPtr = (data >> 8) & 0x01;
    *txStatusPtr = (data >> 9) & 0x01;

    return rc;
}

static GT_STATUS internal_macPhyLedGlobalConfigSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                cascadePortNum,
    IN  CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC  *ledGlobalConfigPtr,
    IN CPSS_MACDRV_STAGE_ENT                stage,
    OUT GT_BOOL                            *doPpMacConfigPtr
)
{
    GT_STATUS        rc;
    GT_CPSS_QD_DEV       *driverInfoPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr;
    GT_U32           value;         /* Temporary value */

    UNUSED_PARAM(stage);

    *doPpMacConfigPtr = GT_FALSE;

    remotePhyMacInfoPtr = prvCpssDxChCfgPort88e1690CascadePortToInfoGet(devNum,cascadePortNum);
    if (remotePhyMacInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Failed (Bad Serdes Port)");
    }
    driverInfoPtr = (GT_CPSS_QD_DEV *) remotePhyMacInfoPtr->drvInfoPtr;

    /* Check parameters */
    CPSS_PARAM_CHECK_MAX_MAC(ledGlobalConfigPtr->directMode, BIT_1);
    CPSS_PARAM_CHECK_MAX_MAC(ledGlobalConfigPtr->skipColumns, BIT_4);
    CPSS_PARAM_CHECK_MAX_MAC(ledGlobalConfigPtr->specialLed1Bitmap, BIT_9);
    CPSS_PARAM_CHECK_MAX_MAC(ledGlobalConfigPtr->specialLed2Bitmap, BIT_9);
    CPSS_PARAM_CHECK_MAX_MAC(ledGlobalConfigPtr->specialLed3Bitmap, BIT_9);

    /* Write parameters to HW */
    value = ((ledGlobalConfigPtr->directMode & 0x01) << 4) |
            ((ledGlobalConfigPtr->skipColumns & 0x0F) << 0);
    rc = prvCpssDrvGprtSetLED(driverInfoPtr,
                    0,
                    GT_CPSS_LED_CFG_GLOBAL_CONTROL,
                    value);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGprtSetLED: failed");
    }

    value = ledGlobalConfigPtr->specialLed1Bitmap;
    rc = prvCpssDrvGprtSetLED(driverInfoPtr,
                    0,
                    GT_CPSS_LED_CFG_SPECIAL_LED1,
                    value);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGprtSetLED: failed");
    }

    value = ledGlobalConfigPtr->specialLed2Bitmap;
    rc = prvCpssDrvGprtSetLED(driverInfoPtr,
                    0,
                    GT_CPSS_LED_CFG_SPECIAL_LED2,
                    value);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGprtSetLED: failed");
    }

    value = ledGlobalConfigPtr->specialLed3Bitmap;
    rc = prvCpssDrvGprtSetLED(driverInfoPtr,
                    0,
                    GT_CPSS_LED_CFG_SPECIAL_LED3,
                    value);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGprtSetLED: failed");
    }

    return rc;
}

static GT_STATUS internal_macPhyLedGlobalConfigGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                cascadePortNum,
    OUT CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC  *ledGlobalConfigPtr,
    IN CPSS_MACDRV_STAGE_ENT                stage,
    OUT GT_BOOL                            *doPpMacConfigPtr
)
{
    GT_STATUS        rc;
    GT_CPSS_QD_DEV       *driverInfoPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr;
    GT_U32           value;         /* Temporary value */

    UNUSED_PARAM(stage);

    *doPpMacConfigPtr = GT_FALSE;
    cpssOsMemSet (ledGlobalConfigPtr, 0, sizeof(*ledGlobalConfigPtr));

    remotePhyMacInfoPtr = prvCpssDxChCfgPort88e1690CascadePortToInfoGet(devNum,cascadePortNum);
    if (remotePhyMacInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Failed (Bad Serdes Port)");
    }
    driverInfoPtr = (GT_CPSS_QD_DEV *) remotePhyMacInfoPtr->drvInfoPtr;

    rc = prvCpssDrvGprtGetLED(driverInfoPtr,
                    0,
                    GT_CPSS_LED_CFG_GLOBAL_CONTROL,
                    &value);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGprtGetLED: failed");
    }
    ledGlobalConfigPtr->directMode = ((value >> 4) & 0x01);
    ledGlobalConfigPtr->skipColumns = ((value >> 0) & 0x0F);

    rc = prvCpssDrvGprtGetLED(driverInfoPtr,
                    0,
                    GT_CPSS_LED_CFG_SPECIAL_LED1,
                    &value);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGprtGetLED: failed");
    }
    ledGlobalConfigPtr->specialLed1Bitmap = value;

    rc = prvCpssDrvGprtGetLED(driverInfoPtr,
                    0,
                    GT_CPSS_LED_CFG_SPECIAL_LED2,
                    &value);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGprtGetLED: failed");
    }
    ledGlobalConfigPtr->specialLed2Bitmap = value;

    rc = prvCpssDrvGprtGetLED(driverInfoPtr,
                    0,
                    GT_CPSS_LED_CFG_SPECIAL_LED3,
                    &value);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGprtGetLED: failed");
    }
    ledGlobalConfigPtr->specialLed3Bitmap = value;

    return rc;
}

static GT_STATUS internal_macPhyLedPerPortConfigSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_LED_PHY_PORT_CONF_STC    *ledPerPortConfigPtr,
    IN CPSS_MACDRV_STAGE_ENT                stage,
    OUT GT_BOOL                            *doPpMacConfigPtr
)
{
    GT_STATUS        rc;
    GT_U32           localMacPortNum;
    GT_CPSS_QD_DEV       *driverInfoPtr;
    GT_U32           value;         /* Temporary value */

    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    /* Check parameters */
    CPSS_PARAM_CHECK_MAX_MAC(ledPerPortConfigPtr->pulseStretch, 5);
    CPSS_PARAM_CHECK_MAX_MAC(ledPerPortConfigPtr->blinkRate, 6);

    /* Write parameters to HW */
    value = ledPerPortConfigPtr->led0Select;
    rc = prvCpssDrvGprtSetLED(driverInfoPtr,
                    localMacPortNum,
                    GT_CPSS_LED_CFG_LED0,
                    value);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGprtSetLED: failed");
    }

    value = ledPerPortConfigPtr->led1Select;
    rc = prvCpssDrvGprtSetLED(driverInfoPtr,
                    localMacPortNum,
                    GT_CPSS_LED_CFG_LED1,
                    value);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGprtSetLED: failed");
    }

    value = ledPerPortConfigPtr->pulseStretch;
    rc = prvCpssDrvGprtSetLED(driverInfoPtr,
                    localMacPortNum,
                    GT_CPSS_LED_CFG_PULSE_STRETCH,
                    value);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGprtSetLED: failed");
    }

    value = ledPerPortConfigPtr->blinkRate;
    rc = prvCpssDrvGprtSetLED(driverInfoPtr,
                    localMacPortNum,
                    GT_CPSS_LED_CFG_BLINK_RATE,
                    value);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGprtSetLED: failed");
    }

    return rc;
}

static GT_STATUS internal_macPhyLedPerPortConfigGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT CPSS_DXCH_LED_PHY_PORT_CONF_STC    *ledPerPortConfigPtr,
    IN CPSS_MACDRV_STAGE_ENT                stage,
    OUT GT_BOOL                            *doPpMacConfigPtr
)
{
    GT_STATUS        rc;
    GT_U32           localMacPortNum;
    GT_CPSS_QD_DEV       *driverInfoPtr;
    GT_U32           value;         /* Temporary value */

    cpssOsMemSet (ledPerPortConfigPtr, 0, sizeof(*ledPerPortConfigPtr));
    rc = portNumCheck(devNum,portNum,stage,doPpMacConfigPtr,&localMacPortNum,&driverInfoPtr);
    if(rc != GT_OK || (*doPpMacConfigPtr) == GT_TRUE)
    {
        return rc;
    }

    rc = prvCpssDrvGprtGetLED(driverInfoPtr,
                    localMacPortNum,
                    GT_CPSS_LED_CFG_LED0,
                    &value);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGprtGetLED: failed");
    }
    ledPerPortConfigPtr->led0Select = (CPSS_DXCH_LED_PHY_SELECT_ENT) value;

    rc = prvCpssDrvGprtGetLED(driverInfoPtr,
                    localMacPortNum,
                    GT_CPSS_LED_CFG_LED1,
                    &value);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGprtGetLED: failed");
    }
    ledPerPortConfigPtr->led1Select = (CPSS_DXCH_LED_PHY_SELECT_ENT) value;

    rc = prvCpssDrvGprtGetLED(driverInfoPtr,
                    localMacPortNum,
                    GT_CPSS_LED_CFG_PULSE_STRETCH,
                    &value);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGprtGetLED: failed");
    }
    ledPerPortConfigPtr->pulseStretch = value;

    rc = prvCpssDrvGprtGetLED(driverInfoPtr,
                    localMacPortNum,
                    GT_CPSS_LED_CFG_BLINK_RATE,
                    &value);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGprtGetLED: failed");
    }
    ledPerPortConfigPtr->blinkRate = value;

    return rc;
}

static GT_STATUS internal_macPhyGpioConfigSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                cascadePortNum,
    IN  GT_U32                              directionBitmap,
    IN CPSS_MACDRV_STAGE_ENT                stage,
    OUT GT_BOOL                            *doPpMacConfigPtr
)
{
    GT_STATUS        rc;
    GT_CPSS_QD_DEV       *driverInfoPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr;

    UNUSED_PARAM(stage);

    *doPpMacConfigPtr = GT_FALSE;

    remotePhyMacInfoPtr = prvCpssDxChCfgPort88e1690CascadePortToInfoGet(devNum,cascadePortNum);
    if (remotePhyMacInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Failed (Bad Serdes Port)");
    }
    driverInfoPtr = (GT_CPSS_QD_DEV *) remotePhyMacInfoPtr->drvInfoPtr;

    /* Check parameters */
    CPSS_PARAM_CHECK_MAX_MAC(directionBitmap, BIT_16);

    /* Write parameters to HW */
    rc = prvCpssDrvGsysSetGpioDirection(driverInfoPtr, directionBitmap);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGsysSetGpioDirection: failed");
    }

    return rc;
}

static GT_STATUS internal_macPhyGpioConfigGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                cascadePortNum,
    OUT GT_U32                             *modeBitmapPtr,
    OUT GT_U32                             *directionBitmapPtr,
    IN CPSS_MACDRV_STAGE_ENT                stage,
    OUT GT_BOOL                            *doPpMacConfigPtr
)
{
    GT_STATUS        rc;
    GT_CPSS_QD_DEV       *driverInfoPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr;

    UNUSED_PARAM(stage);

    *doPpMacConfigPtr = GT_FALSE;

    remotePhyMacInfoPtr = prvCpssDxChCfgPort88e1690CascadePortToInfoGet(devNum,cascadePortNum);
    if (remotePhyMacInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Failed (Bad Serdes Port)");
    }
    driverInfoPtr = (GT_CPSS_QD_DEV *) remotePhyMacInfoPtr->drvInfoPtr;

    /* Check parameters */
    CPSS_NULL_PTR_CHECK_MAC(modeBitmapPtr);
    CPSS_NULL_PTR_CHECK_MAC(directionBitmapPtr);

    /* Get parameters from HW */
    rc = prvCpssDrvGsysGetGpioConfigMod(driverInfoPtr,modeBitmapPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGsysGetGpioConfigMod: failed");
    }
    rc = prvCpssDrvGsysGetGpioDirection(driverInfoPtr,directionBitmapPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGsysGetGpioDirection: failed");
    }

    return rc;
}

static GT_STATUS internal_macPhyGpioDataRead
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                cascadePortNum,
    OUT GT_U32                             *dataBitmapPtr,
    IN CPSS_MACDRV_STAGE_ENT                stage,
    OUT GT_BOOL                            *doPpMacConfigPtr
)
{
    GT_STATUS        rc;
    GT_CPSS_QD_DEV       *driverInfoPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr;

    UNUSED_PARAM(stage);

    *doPpMacConfigPtr = GT_FALSE;

    remotePhyMacInfoPtr = prvCpssDxChCfgPort88e1690CascadePortToInfoGet(devNum,cascadePortNum);
    if (remotePhyMacInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Failed (Bad Serdes Port)");
    }
    driverInfoPtr = (GT_CPSS_QD_DEV *) remotePhyMacInfoPtr->drvInfoPtr;

    /* Check parameters */
    CPSS_NULL_PTR_CHECK_MAC(dataBitmapPtr);

    /* Get data from HW */
    rc = prvCpssDrvGsysGetGpioData(driverInfoPtr,dataBitmapPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGsysGetGpioData: failed");
    }

    return rc;
}

static GT_STATUS internal_macPhyGpioDataWrite
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                cascadePortNum,
    IN  GT_U32                              dataBitmap,
    IN  GT_U32                              dataBitmapMask,
    IN CPSS_MACDRV_STAGE_ENT                stage,
    OUT GT_BOOL                            *doPpMacConfigPtr
)
{
    GT_STATUS        rc;
    GT_CPSS_QD_DEV       *driverInfoPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr;
    GT_U32           value;         /* Temporary value */

    UNUSED_PARAM(stage);

    *doPpMacConfigPtr = GT_FALSE;

    remotePhyMacInfoPtr = prvCpssDxChCfgPort88e1690CascadePortToInfoGet(devNum,cascadePortNum);
    if (remotePhyMacInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Failed (Bad Serdes Port)");
    }
    driverInfoPtr = (GT_CPSS_QD_DEV *) remotePhyMacInfoPtr->drvInfoPtr;

    /* Check parameters */
    CPSS_PARAM_CHECK_MAX_MAC(dataBitmap, BIT_16);
    CPSS_PARAM_CHECK_MAX_MAC(dataBitmapMask, BIT_16);

    /* Write parameters to HW */
    rc = prvCpssDrvGsysGetGpioData(driverInfoPtr,&value);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGsysGetGpioData: failed");
    }
    value &= ~dataBitmapMask;
    value |= (dataBitmap & dataBitmapMask);
    rc = prvCpssDrvGsysSetGpioData(driverInfoPtr,value);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDrvGsysSetGpioData: failed");
    }

    return rc;
}

/**
* @internal prvCpssDxChCfgPort88e1690CountersObjInit function
* @endinternal
*
* @brief   Initialize 88e1690 object - Counter access part.
*
* @note   APPLICABLE DEVICES:      ALL.
* @note   NOT APPLICABLE DEVICES:  None.
*
*                                       void
*
* @note To be called from object creation function.
*
*/
static void prvCpssDxChCfgPort88e1690CountersObjInit(CPSS_MACDRV_COUNTERS_OBJ_STC* macPhy88e1690CountersObjPtr)
{
    /* object function init */

    macPhy88e1690CountersObjPtr->macDrvMacPortMacCounterGet = internal_macPhyCounterGet;
    macPhy88e1690CountersObjPtr->macDrvMacPortMacCountersOnPortGet = internal_macPhyCountersOnPortGet;
    macPhy88e1690CountersObjPtr->macDrvMacPortMacCounterCaptureGet = internal_macPhyCounterCaptureGet;
    macPhy88e1690CountersObjPtr->macDrvMacPortMacCountersCaptureOnPortGet = internal_macPhyCountersCaptureOnPortGet;
    macPhy88e1690CountersObjPtr->macDrvMacPortMacCountersCaptureTriggerSet = internal_macPhyCountersCaptureTriggerSet;
    macPhy88e1690CountersObjPtr->macDrvMacPortMacCountersCaptureTriggerGet = internal_macPhyCountersCaptureTriggerGet;
    macPhy88e1690CountersObjPtr->macDrvMacPortMacCountersEnable = internal_macPhyCountersEnable;
    macPhy88e1690CountersObjPtr->macDrvMacPortMacCountersEnableGet = internal_macPhyCountersEnableGet;
    macPhy88e1690CountersObjPtr->macDrvMacPortMacCountersClearOnReadSet = internal_macPhyCountersClearOnReadSet;
    macPhy88e1690CountersObjPtr->macDrvMacPortMacCountersClearOnReadGet = internal_macPhyCountersClearOnReadGet;
    macPhy88e1690CountersObjPtr->macDrvMacPortMacCountersRxHistogramEnable = internal_macPhyCountersRxHistogramEnable;
    macPhy88e1690CountersObjPtr->macDrvMacPortMacCountersTxHistogramEnable = internal_macPhyCountersTxHistogramEnable;
    macPhy88e1690CountersObjPtr->macDrvMacPortMacCountersHistogramEnableGet = internal_macPhyCountersHistogramEnableGet;
    macPhy88e1690CountersObjPtr->macDrvMacPortMacOversizedPacketsCounterModeSet = NULL;
    macPhy88e1690CountersObjPtr->macDrvMacPortMacOversizedPacketsCounterModeGet = NULL;
    macPhy88e1690CountersObjPtr->macDrvMacPortMacCounterOnPhySideGet = internal_macPhyCounterOnPhySideGet;
    macPhy88e1690CountersObjPtr->macDrvMacPortMacCountersOnPhySidePortGet = internal_macPhyCountersOnPhySidePortGet;

}

/*******************************************************************************
* prvCpssDxChCfgPort88e1690ObjInit
*
* DESCRIPTION:
*       create 88e1690 object. (singleton)
*
* APPLICABLE DEVICES:
*        ALL.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       void
*
* OUTPUTS:
*
* RETURNS:
*       pointer to the '88e1690' object.
*       NULL - on GT_OUT_OF_CPU_MEM error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
CPSS_MACDRV_OBJ_STC* prvCpssDxChCfgPort88e1690ObjInit(void)
{
    if(macPhy88e1690ObjPtr)
    {
        /* already initialized */
        return macPhy88e1690ObjPtr;
    }

    /* memory allocation for phy object */
    macPhy88e1690ObjPtr = cpssOsMalloc(sizeof(CPSS_MACDRV_OBJ_STC));
    if(macPhy88e1690ObjPtr == NULL)
    {
        return NULL;
    }

    /* NULL set for allocated memory */
    cpssOsMemSet(macPhy88e1690ObjPtr,0,sizeof(CPSS_MACDRV_OBJ_STC));

    /* object function init */

    macPhy88e1690ObjPtr->macDrvMacSpeedSetFunc = internal_macPhySpeedSet;       /* 1 */
    macPhy88e1690ObjPtr->macDrvMacSpeedGetFunc = internal_macPhySpeedGet;       /* 2 */
    macPhy88e1690ObjPtr->macDrvMacDuplexANSetFunc = internal_macPhyDuplexANSet;    /* 3 */
    macPhy88e1690ObjPtr->macDrvMacDuplexANGetFunc = internal_macPhyDuplexANGet;    /* 4 */
    macPhy88e1690ObjPtr->macDrvMacFlowCntlANSetFunc = internal_macPhyFlowCntlANSet;  /* 5 */
    macPhy88e1690ObjPtr->macDrvMacFlowCntlANGetFunc = internal_macPhyFlowCntlANGet;  /* 6 */
    macPhy88e1690ObjPtr->macDrvMacSpeedANSetFunc = internal_macPhySpeedANSet;     /* 7 */
    macPhy88e1690ObjPtr->macDrvMacSpeedANGetFunc = internal_macPhySpeedANGet;     /* 8 */
    macPhy88e1690ObjPtr->macDrvMacFlowCntlSetFunc = internal_macPhyFlowControlEnableSet; /* 9 */
    macPhy88e1690ObjPtr->macDrvMacFlowCntlGetFunc = internal_macPhyFlowControlEnableGet; /* 10 */
    macPhy88e1690ObjPtr->macDrvMacPeriodFlowCntlSetFunc = internal_macPhyPeriodicFlowCntlSet;    /* 11 */
    macPhy88e1690ObjPtr->macDrvMacPeriodFlowCntlGetFunc = internal_macPhyPeriodicFlowCntlGet;    /* 12 */
    macPhy88e1690ObjPtr->macDrvMacBackPrSetFunc = internal_macPhyBackPressureSet;     /* 13 */
    macPhy88e1690ObjPtr->macDrvMacBackPrGetFunc = internal_macPhyBackPressureGet;     /* 14 */
    macPhy88e1690ObjPtr->macDrvMacPortlinkGetFunc = internal_macPhyPortLinkGet;     /* 15 */
    macPhy88e1690ObjPtr->macDrvMacDuplexSetFunc = internal_macPhyDuplexSet;     /* 16 */
    macPhy88e1690ObjPtr->macDrvMacDuplexGetFunc = internal_macPhyDuplexGet;     /* 17 */
    macPhy88e1690ObjPtr->macDrvMacPortEnableSetFunc = internal_macPhyPortEnableSet;     /* 18 */
    macPhy88e1690ObjPtr->macDrvMacPortEnableGetFunc = internal_macPhyPortEnableGet;     /* 19 */
    macPhy88e1690ObjPtr->macDrvMacExcessiveCollisionDropSetFunc = internal_macPhyExcessiveCollisionDropSet;  /* 20 */
    macPhy88e1690ObjPtr->macDrvMacExcessiveCollisionDropGetFunc = internal_macPhyExcessiveCollisionDropGet;  /* 21 */
    macPhy88e1690ObjPtr->macDrvMacPaddingEnableSetFunc = internal_macPhyPaddingEnableSet;     /* 22 */
    macPhy88e1690ObjPtr->macDrvMacPaddingEnableGetFunc = internal_macPhyPaddingEnableGet;     /* 23 */
    macPhy88e1690ObjPtr->macDrvMacPreambleLengthSetFunc = internal_macPhyPreambleLengthSet;     /* 24 */
    macPhy88e1690ObjPtr->macDrvMacPreambleLengthGetFunc = internal_macPhyPreambleLengthGet;     /* 25 */
    macPhy88e1690ObjPtr->macDrvMacCRCCheckSetFunc = internal_macPhyCRCCheckSet;        /* 26 */
    macPhy88e1690ObjPtr->macDrvMacCRCCheckGetFunc = internal_macPhyCRCCheckGet;        /* 27 */
    macPhy88e1690ObjPtr->macDrvMacMRUSetFunc = internal_macPhyMRUSet;        /* 28 */
    macPhy88e1690ObjPtr->macDrvMacMRUGetFunc = internal_macPhyMRUGet;        /* 29 */
    macPhy88e1690ObjPtr->macDrvMacPortPowerDownSetFunc = internal_macPhyPortPowerDownSet;

    prvCpssDxChCfgPort88e1690CountersObjInit (&macPhy88e1690ObjPtr->macCounters);

    macPhy88e1690ObjPtr->macDrvMacTypeGetFunc = internal_macTypeGet;
    macPhy88e1690ObjPtr->macDrvMacPortLoopbackSetFunc = internal_portLoopbackSet;
    macPhy88e1690ObjPtr->macDrvMacPortLoopbackGetFunc = internal_portLoopbackGet;
    macPhy88e1690ObjPtr->macDrvMacPortForceLinkDownSetFunc = internal_portForceLinkDownSet;
    macPhy88e1690ObjPtr->macDrvMacPortForceLinkDownGetFunc = internal_portForceLinkDownGet;
    macPhy88e1690ObjPtr->macDrvMacPortForceLinkPassEnableSetFunc = internal_portForceLinkPassEnableSet;
    macPhy88e1690ObjPtr->macDrvMacPortForceLinkPassEnableGetFunc = internal_portForceLinkPassEnableGet;
    macPhy88e1690ObjPtr->macDrvMacHwSmiRegisterSetFunc = internal_macPhyHwSmiRegisterSet;
    macPhy88e1690ObjPtr->macDrvMacHwSmiRegisterGetFunc = internal_macPhyHwSmiRegisterGet;
    macPhy88e1690ObjPtr->macDrvMacHw10GSmiRegisterSetFunc = internal_macPhyHw10GSmiRegisterSet;
    macPhy88e1690ObjPtr->macDrvMacHw10GSmiRegisterGetFunc = internal_macPhyHw10GSmiRegisterGet;
    macPhy88e1690ObjPtr->macDrvMacPortAttributesGetFunc = internal_macPhyPortAttributesGet;
    macPhy88e1690ObjPtr->macDrvMacCscdPortTypeSetFunc = internal_macPhyCscdPortTypeSet;
    macPhy88e1690ObjPtr->macDrvMacPortInterfaceSpeedSupportGetFunc = internal_portInterfaceSpeedSupportGet;
    macPhy88e1690ObjPtr->macDrvMacCscdPortTypeGetFunc = internal_macPhyCscdPortTypeGet;
    macPhy88e1690ObjPtr->macDrvMacSaLsbSetFunc = internal_macPhyMacSaLsbSet;
    macPhy88e1690ObjPtr->macDrvMacSaLsbGetFunc = internal_macPhyMacSaLsbGet;
    macPhy88e1690ObjPtr->macDrvMacSaBaseSetFunc = internal_macPhyMacSaBaseSet;
    macPhy88e1690ObjPtr->macDrvMacPortInterfaceModeGetFunc = internal_macPhyPortInterfaceModeGet;
    macPhy88e1690ObjPtr->macDrvMacPortTpidProfileSetFunc = internal_macDrvMacPortTpidProfileSet;
    macPhy88e1690ObjPtr->macDrvMacPortMacStatusGetFunc = internal_macDrvMacPortMacStatusGet;
    macPhy88e1690ObjPtr->macDrvMacPortDefaultUPSetFunc = internal_macDrvMacPortDefaultUPSet;

    macPhy88e1690ObjPtr->macDrvMacCnFcTimerSetFunc = internal_macPhyCnFcTimerSet;
    macPhy88e1690ObjPtr->macDrvMacCnFcTimerGetFunc = internal_macPhyCnFcTimerGet;
    macPhy88e1690ObjPtr->macDrvMacFcModeSetFunc = internal_macPhyFcModeSet;
    macPhy88e1690ObjPtr->macDrvMacFcModeGetFunc = internal_macPhyFcModeGet;
    macPhy88e1690ObjPtr->macDrvMacFcParamsSetFunc = internal_macPhyFcParamsSet;
    macPhy88e1690ObjPtr->macDrvMacFcParamsGetFunc = internal_macPhyFcParamsGet;

    macPhy88e1690ObjPtr->macDrvMacVctLengthOffsetSetFunc = internal_macPhyVctLengthOffsetSet;
    macPhy88e1690ObjPtr->macDrvMacVctLengthOffsetGetFunc = internal_macPhyVctLengthOffsetGet;

    macPhy88e1690ObjPtr->macDrvMacEeeConfigSetFunc = internal_macPhyEeeConfigSet;
    macPhy88e1690ObjPtr->macDrvMacEeeConfigGetFunc = internal_macPhyEeeConfigGet;
    macPhy88e1690ObjPtr->macDrvMacEeeStatusGetFunc = internal_macPhyEeeStatusGet;

    macPhy88e1690ObjPtr->macDrvMacLedGlobalConfigSetFunc = internal_macPhyLedGlobalConfigSet;
    macPhy88e1690ObjPtr->macDrvMacLedGlobalConfigGetFunc = internal_macPhyLedGlobalConfigGet;
    macPhy88e1690ObjPtr->macDrvMacLedPerPortConfigSetFunc = internal_macPhyLedPerPortConfigSet;
    macPhy88e1690ObjPtr->macDrvMacLedPerPortConfigGetFunc = internal_macPhyLedPerPortConfigGet;

    macPhy88e1690ObjPtr->macDrvMacGpioConfigSetFunc = internal_macPhyGpioConfigSet;
    macPhy88e1690ObjPtr->macDrvMacGpioConfigGetFunc = internal_macPhyGpioConfigGet;
    macPhy88e1690ObjPtr->macDrvMacGpioDataReadFunc = internal_macPhyGpioDataRead;
    macPhy88e1690ObjPtr->macDrvMacGpioDataWriteFunc = internal_macPhyGpioDataWrite;

    macPhy88e1690ObjPtr->objStubFunc = internal_macStub;

    return macPhy88e1690ObjPtr;
}


/**
* @internal prvCpssDxChCfgPort88e1690ObjDestroy function
* @endinternal
*
* @brief   destroy 88e1690 object. (singleton)
*
* @note   APPLICABLE DEVICES:      ALL.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - always
*/
GT_STATUS prvCpssDxChCfgPort88e1690ObjDestroy(void)
{
    FREE_PTR_MAC(macPhy88e1690ObjPtr);

    return GT_OK;
}

