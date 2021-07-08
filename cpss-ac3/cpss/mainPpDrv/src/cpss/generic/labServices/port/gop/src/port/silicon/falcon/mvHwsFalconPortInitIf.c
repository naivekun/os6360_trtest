/*******************************************************************************
*            Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsFalconPortInitIf.c
*
* @brief
*
* @version   55
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <silicon/falcon/mvHwsFalconPortIf.h>
#include <silicon/falcon/mvHwsPortCalIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsD2dIf.h>
#include <cpss/common/labServices/port/gop/port/mac/d2dMac/mvHwsD2dMacIf.h>

GT_BOOL hwsPpHwTraceFlag = GT_FALSE;

GT_VOID debugHwsPpHwTraceEnableSet(GT_BOOL enable)
{
    hwsPpHwTraceFlag = enable;
    return;
}

#if (!defined MV_HWS_REDUCED_BUILD)

GT_STATUS mvHwsFalconPortInit
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_BOOL serdesInit = GT_TRUE;
    GT_U32  d2dIdx, d2dNumRaven, d2dNumEagle, channel;
    MV_HWS_REF_CLOCK_SUP_VAL  refClock;
    MV_HWS_REF_CLOCK_SOURCE   refClockSource;
    if (NULL == portInitInParamPtr)
    {
        return GT_BAD_PTR;
    }
#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL)
    {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("****** port %d create mode %d ******\n", phyPortNum, portMode);
    }
#endif
    refClock = portInitInParamPtr->refClock;
    refClockSource = portInitInParamPtr->refClockSource;
    /* validation */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    /* Init D2D */
    d2dIdx = PRV_HWS_D2D_CONVERT_PORT_TO_D2D_IDX_MAC(phyPortNum);
    d2dNumRaven = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 1);
    d2dNumEagle  = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 0);
    channel = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(phyPortNum);

    CHECK_STATUS(mvHwsD2dConfigChannel(devNum, d2dNumRaven, channel, curPortParams.serdesSpeed, curPortParams.numOfActLanes));
    CHECK_STATUS(mvHwsD2dConfigChannel(devNum, d2dNumEagle, channel, curPortParams.serdesSpeed, curPortParams.numOfActLanes));

    CHECK_STATUS(mvHwsMtipExtClockEnable(devNum, phyPortNum, portMode, GT_TRUE));
    CHECK_STATUS(mvHwsMtipExtMacResetRelease(devNum, phyPortNum, portMode, GT_TRUE));

    CHECK_STATUS(mvHwsMpfConfigChannel(devNum, phyPortNum, portMode, GT_TRUE));
    CHECK_STATUS(mvHwsMpfSetPchMode(devNum, phyPortNum, portMode, GT_TRUE));
    CHECK_STATUS(hwsD2dMacChannelEnable(devNum, d2dNumRaven, channel, GT_TRUE));
    CHECK_STATUS(hwsD2dMacChannelEnable(devNum, d2dNumEagle, channel, GT_TRUE));

    /*CHECK_STATUS(mvHwsMtipExtReset(devNum, phyPortNum, portMode, GT_TRUE));*/
    CHECK_STATUS(mvHwsMtipExtSetLaneWidth(devNum, phyPortNum, portMode, curPortParams.serdes10BitStatus));
    CHECK_STATUS(mvHwsMtipExtFecTypeSet(devNum, phyPortNum, portMode, curPortParams.portFecMode));
    CHECK_STATUS(mvHwsMtipExtSetChannelMode(devNum, phyPortNum, portMode, GT_TRUE));
    /*CHECK_STATUS(mvHwsPortLoopbackValidate((GT_U32)curPortParams.portPcsType, lbPort, &serdesInit));*/

    if (serdesInit == GT_TRUE)
    {
        hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasLanesNum = curPortParams.numOfActLanes;
        hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasMacNum  = curPortParams.portMacNumber;
        /* rebuild active lanes list according to current configuration (redundancy) */
        CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));
        /* power up the serdes */
        CHECK_STATUS(mvHwsPortSerdesPowerUp(devNum, portGroup, phyPortNum, portMode, refClock, refClockSource, &(curLanesList[0])));
    }

    /* Configure MAC/PCS */
    CHECK_STATUS(mvHwsPortModeCfg(devNum, portGroup, phyPortNum, portMode));

    /* Un-Reset the port */
    CHECK_STATUS(mvHwsPortStartCfg(devNum, portGroup, phyPortNum, portMode));

    /* Un-Reset mtip */
    CHECK_STATUS(mvHwsMtipExtReset(devNum, phyPortNum, portMode, GT_FALSE));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL)
    {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("**********************\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsFalconPortReset function
* @endinternal
*
* @brief   Clears the port mode and release all its resources according to selected.
*         Does not verify that the selected mode/port number is valid at the core
*         level and actual terminated mode.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] action                   - Power down or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsFalconPortReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_ACTION      action
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32  d2dIdx, d2dNumRaven, d2dNumEagle, channel;
    /*GT_U32 i;*/

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("****** port %d delete mode %d ******\n", phyPortNum, portMode);
    }
#endif
    /* validation */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
       return GT_BAD_PTR;
    }
     /* Init D2D */
    d2dIdx = PRV_HWS_D2D_CONVERT_PORT_TO_D2D_IDX_MAC(phyPortNum);
    d2dNumRaven = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 1);
    d2dNumEagle  = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 0);
    channel = PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(phyPortNum);

    CHECK_STATUS(mvHwsMtipExtReset(devNum, phyPortNum, portMode, GT_TRUE));

    CHECK_STATUS(mvHwsPortStopCfg(devNum, portGroup, phyPortNum, portMode, action, &(curLanesList[0]), FULL_RESET, FULL_RESET));

    CHECK_STATUS(mvHwsMtipExtMacResetRelease(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsMtipExtClockEnable(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsMtipExtSetLaneWidth(devNum, phyPortNum, portMode, _10BIT_OFF));
    CHECK_STATUS(mvHwsMtipExtFecTypeSet(devNum, phyPortNum, portMode, FEC_NA));
    CHECK_STATUS(mvHwsMtipExtSetChannelMode(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsD2dDisableChannel(devNum, d2dNumRaven, channel));
    CHECK_STATUS(mvHwsD2dDisableChannel(devNum, d2dNumEagle, channel));
#if 0 /*used only for galtis*/
    if(!cpssDeviceRunCheck_onEmulator())
    {
        /* PORT_RESET on each related serdes */
        for (i = 0; (PORT_RESET == action) && (i < curPortParams.numOfActLanes); i++) {
            CHECK_STATUS(mvHwsSerdesReset(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]),
                                          HWS_DEV_SERDES_TYPE(devNum), GT_TRUE, GT_TRUE, GT_TRUE));
        }
    }
#endif

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("**********************\n");
    }
#endif
    return GT_OK;
}

#endif
