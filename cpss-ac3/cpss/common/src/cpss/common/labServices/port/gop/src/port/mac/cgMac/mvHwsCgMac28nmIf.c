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
* @file mvHwsCgMac28nmIf.c
*
* @brief CG MAC Bobcat3 interface
*
* @version   1
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mac/cgMac/mvHwsCgMac28nmDb.h>
#include <cpss/common/labServices/port/gop/port/mac/cgMac/mvHwsCgMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mac/cgMac/mvHwsCgMacIf.h>/*needed for mvHwsCgMacLoopbackStatusGet() */
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/common/os/mvSemaphore.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>

extern GT_STATUS mvHwsCgPcs28nmActiveLanesNumGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      pcsNum,
    GT_U32      *activeLanes
);

static char* mvHwsMacTypeGet(void)
{
  return "CGMAC_28nm";
}

/**
* @internal hwsCgMac28nmIfInit function
* @endinternal
*
* @brief   Init XLG MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsCgMac28nmIfInit(GT_U8 devNum, MV_HWS_MAC_FUNC_PTRS *funcPtrArray)
{

    CHECK_STATUS(hwsCgMac28nmSeqInit(devNum));

    funcPtrArray[CGMAC].macRestFunc    = mvHwsCgMac28nmReset;
    funcPtrArray[CGMAC].macModeCfgFunc = mvHwsCgMac28nmModeCfg;
    funcPtrArray[CGMAC].macLinkGetFunc = mvHwsCgMac28nmLinkStatus;
    funcPtrArray[CGMAC].macLbCfgFunc   = mvHwsCgMac28nmLoopbackSet;

    funcPtrArray[CGMAC].macLbStatusGetFunc = mvHwsCgMac28nmLoopbackStatusGet;

    funcPtrArray[CGMAC].macActiveStatusFunc = mvHwsCgMac28nmActiveStatusGet;

    funcPtrArray[CGMAC].macTypeGetFunc = mvHwsMacTypeGet;

    funcPtrArray[CGMAC].macFcStateCfgFunc = mvHwsCgMac28nmFcStateCfg;

    funcPtrArray[CGMAC].macPortEnableFunc = mvHwsCgMac28nmIfPortEnable;

    return GT_OK;
}

/**
* @internal mvHwsCgMac28nmModeCfg function
* @endinternal
*
* @brief   Set the CG MAC in GOP.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgMac28nmModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_CFG_SEQ *seq;
    GT_STATUS  st;
    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (curPortParams.numOfActLanes)
    {
        case 1: /* 25G */
            seq = &hwsCgMac28nmSeqDb[CGMAC_MODE_1_LANE_SEQ];
            break;
        case 2: /* 50G */
            seq = &hwsCgMac28nmSeqDb[CGMAC_MODE_2_LANE_SEQ];
            break;
        case 4: /* 100G/102G */
            seq = &hwsCgMac28nmSeqDb[CGMAC_MODE_4_LANE_SEQ];
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    mvHwsCgMac28nmAccessLock(devNum, portMacNum);
    st = mvCfgSeqExec(devNum, portGroup, curPortParams.portMacNumber, seq->cfgSeq, seq->cfgSeqSize);
    mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
    CHECK_STATUS_EXT(st, LOG_ARG_MAC_IDX_MAC(portMacNum), LOG_ARG_STRING_MAC("calling mvCfgSeqExec"))

    return GT_OK;
}

/**
* @internal mvHwsCgMac28nmReset function
* @endinternal
*
* @brief   Set CG MAC RESET/UNRESET or FULL_RESET action.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] portMode                 - port mode
* @param[in] action                   - RESET/UNRESET or FULL_RESET
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgMac28nmReset
(
    GT_U8           devNum,
    GT_U32          portGroup,
    GT_U32          portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET    action
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_CFG_SEQ  *seq=NULL;
    GT_STATUS   st;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (FULL_RESET == action)
    {
        switch (curPortParams.numOfActLanes)
        {
            case 1: /* 25G */
                seq = &hwsCgMac28nmSeqDb[CGMAC_POWER_DOWN_1_LANE_SEQ];
                break;
            case 2: /* 50G */
                seq = &hwsCgMac28nmSeqDb[CGMAC_POWER_DOWN_2_LANE_SEQ];
                break;
            case 4: /* 100G/102G */
                seq = &hwsCgMac28nmSeqDb[CGMAC_POWER_DOWN_4_LANE_SEQ];
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else if (RESET == action)
    {
        switch (curPortParams.numOfActLanes)
        {
            case 1: /* 25G */
                seq = &hwsCgMac28nmSeqDb[CGMAC_RESET_SEQ];
                break;
            case 2: /* 50G */
                seq = &hwsCgMac28nmSeqDb[CGMAC_RESET_SEQ];
                break;
            case 4: /* 100G */
                seq = &hwsCgMac28nmSeqDb[CGMAC_RESET_SEQ];
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else if (UNRESET == action)
    {
        switch (curPortParams.numOfActLanes)
        {
            case 1: /* 25G */
                seq = &hwsCgMac28nmSeqDb[CGMAC_UNRESET_SEQ];
                break;
            case 2: /* 50G */
                seq = &hwsCgMac28nmSeqDb[CGMAC_UNRESET_SEQ];
                break;
            case 4: /* 100G */
                seq = &hwsCgMac28nmSeqDb[CGMAC_UNRESET_SEQ];
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    mvHwsCgMac28nmAccessLock(devNum, portMacNum);
    st = mvCfgSeqExec(devNum, portGroup, portMacNum, seq->cfgSeq, seq->cfgSeqSize);
    mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
    CHECK_STATUS_EXT(st, LOG_ARG_MAC_IDX_MAC(portMacNum), LOG_ARG_STRING_MAC("calling mvCfgSeqExec"))
    return GT_OK;
}

/**
* @internal mvHwsCgMac28nmActiveStatusGet function
* @endinternal
*
* @brief   Return number of MAC active lanes or 0, if current MAC isn't active.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      pcsNum    - physical MAC number
*
* @param[out] numOfLanes               - number of lanes agregated in MAC
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgMac28nmActiveStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_UOPT                 *numOfLanes
)
{
    GT_U32 data;

    portMode = portMode;
    if (numOfLanes == NULL)
    {
      return GT_BAD_PARAM;
    }

    *numOfLanes = 0;

    if(macNum % 4 != 0)
    {
        return GT_OK;
    }

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_UNIT, macNum, CG_RESETS, &data, 0));
    /* check unreset bit status */
    if ((data & (0x5 << 26))!= (0x5 << 26))
    {
        /* unit is under RESET */
        return GT_OK;
    }

    /* get number of active lanes */
    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_UNIT, macNum, CG_CONTROL_2, &data, 0xFF << 9));
    if (data == 0x1E000)
    {
        *numOfLanes = 4;
    }
    else
    {
        *numOfLanes = 0;
    }
    return GT_OK;
}


/**
* @internal mvHwsCgMac28nmLinkStatus function
* @endinternal
*
* @brief   Get CG MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgMac28nmLinkStatus
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL     *linkStatus
)
{
    GT_STATUS rc;
    GT_U32  data;
    GT_U32  activeLanes;

    portMode = portMode;
    /* get the number of active lanes on CG PCS: 1=25G, 2=50G, 4=100G */
    rc = mvHwsCgPcs28nmActiveLanesNumGet(devNum, portGroup, macNum, &activeLanes);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (activeLanes)
    {
        case 1: /* 25G */
        case 2: /* 50G */
            CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, XLGMAC_UNIT, macNum, MSM_PORT_STATUS, &data, 0));
            *linkStatus = data & 1;
            break;
        case 4: /* 100G */
            CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_UNIT, macNum, CG_IP_STATUS, &data, (1 << 29)));
            *linkStatus = (data >> 29) & 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal mvHwsCgMac28nmLoopbackStatusGet function
* @endinternal
*
* @brief   Retrive MAC loopback status.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
*
* @param[out] lbType                   - supported loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgMac28nmLoopbackStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
)
{
    GT_U32 data;

    portMode = portMode;

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_UNIT, macNum, CG_COMMAND_CONFIG, &data, 0));

    *lbType = DISABLE_LB;
    if ((data >> 10) & 1)
    {
        /* loopback enabled (only Tx2Rx supported) */
        *lbType = TX_2_RX_LB;
    }
    return GT_OK;
}

/**
* @internal mvHwsCgMac28nmLoopbackSet function
* @endinternal
*
* @brief   Set MAC loopback .
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
*
* @param[out] lbType                   - supported loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgMac28nmLoopbackSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
)
{
    GT_U32 data;

    portMode = portMode;
    switch (lbType)
    {
        case DISABLE_LB:
            data = 0;
            break;
        case TX_2_RX_LB:
            data = 1;
            break;
        default:
            return GT_FAIL;
    }

    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, CG_UNIT, macNum, CG_COMMAND_CONFIG, data<<10, 1<<10));

    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, CG_UNIT, macNum, CG_CONTROL_0, data<<27, 1<<27));

    return GT_OK;
}

/**
* @internal mvHwsCgMac28nmIfPortEnable function
* @endinternal
*
* @brief   Perform port enable on the a port MAC.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgMac28nmIfPortEnable
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 enable
)
{
    portMode = portMode;
    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, CG_UNIT, macNum,
            CG_CONTROL_0, (enable==GT_TRUE) ? (1<<20) : 0, (1<<20)));

    return GT_OK;
}

/**
* @internal mvHwsCgMac28nmFcStateCfg function
* @endinternal
*
* @brief   Configure Flow Control state.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] fcState                  - flow control state
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgMac28nmFcStateCfg
(
    GT_U8                           devNum,
    GT_UOPT                         portGroup,
    GT_UOPT                         macNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_FLOW_CONTROL_ENT    fcState
)
{
    MV_CFG_SEQ *seq;

    portMode = portMode;
    switch (fcState)
    {
        case MV_HWS_PORT_FLOW_CONTROL_DISABLE_E:
            seq = &hwsCgMac28nmSeqDb[CGMAC_FC_DISABLE_SEQ];
            break;
        case MV_HWS_PORT_FLOW_CONTROL_RX_TX_E:
            seq = &hwsCgMac28nmSeqDb[CGMAC_FC_BOTH_SEQ];
            break;
        case MV_HWS_PORT_FLOW_CONTROL_RX_ONLY_E:
            seq = &hwsCgMac28nmSeqDb[CGMAC_FC_RX_ONLY_SEQ];
            break;
        case MV_HWS_PORT_FLOW_CONTROL_TX_ONLY_E:
            seq = &hwsCgMac28nmSeqDb[CGMAC_FC_TX_ONLY_SEQ];
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, macNum, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsCgMac28nmAccessLock function
* @endinternal
*
* @brief   Protection Definition
*         =====================
*         Multi-Processor Environment
*         This case is protected by HW Semaphore
*         HW Semaphore is defined based in MSYS / CM3 resources
*         In case customer does not use MSYS / CM3 resources,
*         the customer will need to implement its own HW Semaphore
*         This protection is relevant ONLY in case Service CPU Firmware is loaded to CM3
* @param[in] devNum                   - device id
* @param[in] macNum                   - mac number
*                                       None
*/
void mvHwsCgMac28nmAccessLock
(
    GT_U8   devNum,
    GT_UOPT macNum
)
{

    /*
    ** HW Semaphore Protection Section
    ** ===============================
    */
#ifdef MV_HWS_REDUCED_BUILD_EXT_CM3
    /* When running on any Service CPU, HW semaphore always used */
    mvSemaLock(devNum, MV_SEMA_CG + (macNum>>2));
#else
    /* When running on Host CPU, HW semaphore always used only when a service CPU is present */
    if ( mvHwsServiceCpuEnableGet(devNum) ) {
        mvSemaLock(devNum, MV_SEMA_CG + (macNum>>2));
    }
#endif

}

/**
* @internal mvHwsCgMac28nmAccessUnlock function
* @endinternal
*
* @brief   release cg sem
*
* @param[in] devNum                   - device id
* @param[in] macNum                   - mac number
*                                       None
*/
void mvHwsCgMac28nmAccessUnlock
(
    GT_U8   devNum,
    GT_UOPT macNum
)
{

    /*
    ** HW Semaphore Protection Section
    ** ===============================
    */
#ifdef MV_HWS_REDUCED_BUILD_EXT_CM3
    /* When running on any Service CPU, HW semaphore always used */
    mvSemaUnlock(devNum, MV_SEMA_CG + (macNum>>2));
#else
    /* When running on Host CPU, HW semaphore always used only when a service CPU is present */
    if ( mvHwsServiceCpuEnableGet(devNum) ) {
        mvSemaUnlock(devNum, MV_SEMA_CG + (macNum>>2));
    }
#endif

}

/**
* @internal mvHwsCgMac28nmAccessGet function
* @endinternal
*
* @brief   Check if CG is in reset.
*         cg lock should be taken before the call to this function
* @param[in] devNum                   - device id
* @param[in] portGroup                - port group id
* @param[in] macNum                   - mac number
*                                       None
*/
GT_BOOL mvHwsCgMac28nmAccessGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 macNum
)
{
    GT_U32    reset, resetMask, cntrl0;
    GT_STATUS st;
    /* check reset register */
    st = genUnitRegisterGet(devNum, portGroup, CG_UNIT, macNum, CG_RESETS, &reset, 0);
    if (st != GT_OK)
    {
        return GT_FALSE;
    }
    if ((HWS_DEV_SILICON_TYPE(devNum) == Pipe || HWS_DEV_SILICON_TYPE(devNum) == Aldrin2 ))
    { /* in pipe there is also bit 30 for reset cg*/
        resetMask = 0x54000000;
    }
    else
    {
        resetMask = 0x14000000;
    }
    st = genUnitRegisterGet(devNum, portGroup, CG_UNIT, macNum, CG_CONTROL_0, &cntrl0, 0);
    if ((st != GT_OK) || ((reset & resetMask) != resetMask) || ((cntrl0 & (0x1 << 22)) != (0x1 << 22))){
        /* unit is under RESET */
        return GT_FALSE;
    }
    else
    {
        /* the caller should unlock the sem when operation is finished*/
        return GT_TRUE;
    }

}


