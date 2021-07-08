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
* @file mvHwsGeMac28nmIf.c
*
* @brief GE MAC interface
*
* @version   6
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacDb.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMacDb.h>

#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>

#ifndef CO_CPU_RUN
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacIf.h>

GT_STATUS mvHwsGeNetMac28nmReset
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
);

GT_STATUS mvHwsQsgmiiMac28nmReset
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
);

static char* mvHwsMacTypeGet(void)
{
  return "GEMAC_X_28nm";
}
#endif

/**
* @internal hwsGeMac28nmIfInit function
* @endinternal
*
* @brief   Init GE MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsGeMac28nmIfInit(GT_U8 devNum, MV_HWS_MAC_FUNC_PTRS *funcPtrArray)
{
    CHECK_STATUS(hwsGeMacSeqInit(devNum));

    funcPtrArray[GEMAC_X].macRestFunc = mvHwsGeMac28nmReset;
    funcPtrArray[GEMAC_X].macModeCfgFunc = mvHwsGeMac28nmModeCfg;
    funcPtrArray[GEMAC_X].macLinkGetFunc = mvHwsGeMac28nmLinkStatus;
    funcPtrArray[GEMAC_X].macFcStateCfgFunc = mvHwsGeNetMac28nmFcStateCfg;

    funcPtrArray[GEMAC_NET_X].macRestFunc = mvHwsGeNetMac28nmReset;      /* Network port implementation */
    funcPtrArray[GEMAC_NET_X].macModeCfgFunc = mvHwsGeNetMac28nmModeCfg; /* Network port implementation */
    funcPtrArray[GEMAC_NET_X].macLinkGetFunc = mvHwsGeMac28nmLinkStatus;
    funcPtrArray[GEMAC_NET_X].macFcStateCfgFunc = mvHwsGeNetMac28nmFcStateCfg;

    funcPtrArray[QSGMII_MAC].macRestFunc = mvHwsQsgmiiMac28nmReset;
    funcPtrArray[QSGMII_MAC].macModeCfgFunc = mvHwsQsgmiiMac28nmModeCfg;
    funcPtrArray[QSGMII_MAC].macLinkGetFunc = mvHwsGeMac28nmLinkStatus;
    funcPtrArray[QSGMII_MAC].macFcStateCfgFunc = mvHwsGeNetMac28nmFcStateCfg;

#ifndef CO_CPU_RUN
    funcPtrArray[GEMAC_X].macLbCfgFunc = mvHwsGeMacLoopbackCfg;
    funcPtrArray[GEMAC_X].macLbStatusGetFunc = mvHwsGeMac28nmLoopbackStatusGet;
    funcPtrArray[GEMAC_X].macActiveStatusFunc = mvHwsGeMac28nmActiveStatusGet;
    funcPtrArray[GEMAC_X].macTypeGetFunc = mvHwsMacTypeGet;

    funcPtrArray[GEMAC_NET_X].macLbCfgFunc = mvHwsGeMacLoopbackCfg;
    funcPtrArray[GEMAC_NET_X].macLbStatusGetFunc = mvHwsGeMac28nmLoopbackStatusGet;
    funcPtrArray[GEMAC_NET_X].macActiveStatusFunc = mvHwsGeMac28nmActiveStatusGet;
    funcPtrArray[GEMAC_NET_X].macTypeGetFunc = mvHwsMacTypeGet;

    funcPtrArray[QSGMII_MAC].macLbCfgFunc = mvHwsGeMacLoopbackCfg;
    funcPtrArray[QSGMII_MAC].macLbStatusGetFunc = mvHwsGeMac28nmLoopbackStatusGet;
    funcPtrArray[QSGMII_MAC].macActiveStatusFunc = mvHwsQsgmiiMac28nmActiveStatusGet;
    funcPtrArray[QSGMII_MAC].macTypeGetFunc = mvHwsMacTypeGet;
#endif

    funcPtrArray[GEMAC_X].macPortEnableFunc = hwsGeMac28nmIfPortEnable;

    return GT_OK;
}

/**
* @internal mvHwsGeMac28nmReset function
* @endinternal
*
* @brief   Set the GE MAC to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] action                   - normal or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGeMac28nmReset
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    MV_CFG_SEQ *seq;
    portMode = portMode;

    if ((RESET == action) || (FULL_RESET == action))
    {
        seq = &hwsGeMacSeqDb[GEMAC_RESET_SEQ];
    }
    else if (UNRESET == action)
    {
        seq = &hwsGeMacSeqDb[GEMAC_UNRESET_SEQ];
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, portMacNum, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsGeMac28nmModeCfg function
* @endinternal
*
* @brief   Set the internal mux's to the required MAC in the GOP.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGeMac28nmModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_CFG_SEQ *seq;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    seq = &hwsGeMacSeqDb[GEMAC_X_MODE_SEQ];
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portMacNumber, seq->cfgSeq, seq->cfgSeqSize));

    /* In Alleycat3 there is a MUX in DMA unit (connected to Serdes #10) for selecting between two modes in XLGMAC unit with MAC #29.
       The selector (port RX interface) exists in External_control register and the configuration can be changed
       between these two modes:
       - Switch CPU port #31 (as Ethernet port in MII mode): External_Control_1 value=0, port RX interface is connected to
            the internal MAC #31 of CPU port.
       - SDMA (PEX mode): External_Control_1 bit value=1, port RX interface is connected to the external client 3 */
    if (HWS_DEV_SILICON_TYPE(devNum) == Alleycat3A0)
    {
        if (curPortParams.portMacNumber == 31)
        {
            /* For Switch CPU port #31 mode: set External_Control_1 bit to work with internal MAC #31 of CPU port */
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, 29, EXTERNAL_CONTROL, 0, 0x2));
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsGeMac28nmLinkStatus function
* @endinternal
*
* @brief   Get MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGeMac28nmLinkStatus
(
    GT_U8         devNum,
    GT_UOPT       portGroup,
    GT_UOPT       macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL       *linkStatus
)
{
    GT_UREG_DATA data;

    portMode = portMode;
    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, GEMAC_UNIT, macNum, GIG_Port_Status_Reg, &data, 0));

    *linkStatus = data & 1;

    return GT_OK;
}

/**
* @internal mvHwsGeNetMac28nmReset function
* @endinternal
*
* @brief   Set the GE MAC to reset or exit from reset (network port).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] action                   - normal or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGeNetMac28nmReset
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    MV_CFG_SEQ *seq;
    portMode = portMode;

    if ((RESET == action) || (FULL_RESET == action))
    {
        seq = &hwsGeMacSeqDb[GEMAC_RESET_SEQ];
    }
    else if (UNRESET == action)
    {
        seq = &hwsGeMacSeqDb[GEMAC_UNRESET_SEQ];
    }
    else
    {
        hwsOsPrintf("Error: undefined reset action resetMode = %d \n", action);
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, portMacNum, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsGeNetMac28nmModeCfg function
* @endinternal
*
* @brief   Set the internal mux's to the required MAC in the GOP.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGeNetMac28nmModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_CFG_SEQ *seq;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    seq = &hwsGeMacSeqDb[GEMAC_NET_X_MODE_SEQ];
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portMacNumber, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

#ifndef CO_CPU_RUN
/**
* @internal mvHwsGeMac28nmLoopbackStatusGet function
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
GT_STATUS mvHwsGeMac28nmLoopbackStatusGet
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

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, GEMAC_UNIT, macNum, GIG_PORT_MAC_CONTROL_REGISTER1, &data, 0));

    *lbType = ((data >> 5) & 1) ? TX_2_RX_LB : DISABLE_LB;

    return GT_OK;
}

/**
* @internal mvHwsGeMac28nmActiveStatusGet function
* @endinternal
*
* @brief   Return number of MAC active lanes or 0, if current MAC isn't active.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
*
* @param[out] numOfLanes               - number of lanes agregated in PCS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGeMac28nmActiveStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  *numOfLanes
)
{
    GT_U32 data;

    portMode = portMode;
    if (numOfLanes == NULL)
    {
        return GT_BAD_PARAM;
    }

    *numOfLanes = 0;
    /* port MAC type bit 1 */
    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, GEMAC_UNIT, macNum, GIG_PORT_MAC_CONTROL_REGISTER0, &data, 0));
    if (((data >> 1) & 1) != 1)
    {
        /* 0x1 = 1000Base-X_mode */
        return GT_OK;
    }

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, GEMAC_UNIT, macNum, GIG_PORT_MAC_CONTROL_REGISTER2, &data, 0));
    if ((data >> 6) & 1)
    {
        /* unit is under RESET */
        return GT_OK;
    }
    *numOfLanes = 1;

    return GT_OK;
}

/**
* @internal mvHwsQsgmiiMac28nmActiveStatusGet function
* @endinternal
*
* @brief   Return number of MAC active lanes or 0, if current MAC isn't active.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
*
* @param[out] numOfLanes               - number of lanes agregated in MAC
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsQsgmiiMac28nmActiveStatusGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32      *numOfLanes
)
{
    GT_U32 data;

    portMode = portMode;
    if (numOfLanes == NULL)
    {
        return GT_BAD_PARAM;
    }

    *numOfLanes = 0;

    /* check if MAC unit is under RESET */
    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, GEMAC_UNIT, macNum, GIG_PORT_MAC_CONTROL_REGISTER2, &data, 0));
    if ((data >> 6) & 1)
    {
        return GT_OK;
    }

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, GEMAC_UNIT, macNum, GIG_PORT_MAC_CONTROL_REGISTER4, &data, 0));
    /* if bit #7==0, port type is QSGMII --> number of active lanes is 4 */
    if (((data >> 7) & 1) == 0)
    {
        *numOfLanes = 4;
    }

    return GT_OK;
}

/**
* @internal mvHwsQsgmiiMac28nmReset function
* @endinternal
*
* @brief   Set the QSGMII MAC to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] action                   - normal or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsQsgmiiMac28nmReset
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    GT_U8 macId;
    GT_U8 numOfMac = 4;
    GT_UREG_DATA data;

    portMode = portMode;

    if ((RESET == action) || (FULL_RESET == action))
    {
        data = (1 << 7);
    }
    else if (UNRESET == action)
    {
        data = 0;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    /* for QSGMII mode the setting should start from the first macNum in the GOP */
    portMacNum = portMacNum - portMacNum % 4;

    for (macId = (GT_U8)portMacNum; macId < (((GT_U8)portMacNum) + numOfMac); macId++)
    {
        /* set MUX to SGMII */
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, macId, EXTERNAL_CONTROL, data, (1 << 7)));

        /* reset MAC */
        CHECK_STATUS(mvHwsGeNetMac28nmReset(devNum, portGroup, macId, portMode, action));
    }

    return GT_OK;
}

/**
* @internal mvHwsQsgmiiMac28nmModeCfg function
* @endinternal
*
* @brief   Set the internal mux's to the required MAC in the GOP.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsQsgmiiMac28nmModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_UOPT macId;
    GT_UOPT numOfMac = 4;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* for AC3 and below no need to do anything */
    if (HWS_DEV_GOP_REV(devNum) < GOP_28NM_REV2)
    {
        return GT_OK;
    }

    /* for BC2 and above*/
    for(macId = curPortParams.portMacNumber; macId < (curPortParams.portMacNumber + numOfMac); macId++)
    {
        /* enable Inband_Auto_Neg */
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, macId, GIG_Port_Auto_Negotiation_Configuration, (1<<2), (1 << 2)));

        /* for Bobk/BC2 XLGMAC is not supported for port number <= 47 */
        if ((((HWS_DEV_SILICON_TYPE(devNum) == BobK) || (HWS_DEV_GOP_REV(devNum) == GOP_28NM_REV2)) && (curPortParams.portMacNumber > 47))
        || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin) || (HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2))
        {
            /*Change mac_mode_dma_1g to 0x1 (GigMAC mode)*/
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, macId, MSM_PORT_MAC_CONTROL_REGISTER4, (1 << 12), (1 << 12)));
            /* Change mac_mode_select to 0 (GEMAC) */
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, macId, MSM_PORT_MAC_CONTROL_REGISTER3, 0, (7 << 13)));
        }
    }

    return GT_OK;
}

#endif


/**
* @internal mvHwsGeNetMac28nmFcStateCfg function
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
GT_STATUS mvHwsGeNetMac28nmFcStateCfg
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
            seq = &hwsGeMacSeqDb[GEMAC_FC_DISABLE_SEQ];
            break;
        case MV_HWS_PORT_FLOW_CONTROL_RX_TX_E:
            seq = &hwsGeMacSeqDb[GEMAC_FC_BOTH_SEQ];
            break;
        case MV_HWS_PORT_FLOW_CONTROL_RX_ONLY_E:
            seq = &hwsGeMacSeqDb[GEMAC_FC_RX_ONLY_SEQ];
            break;
        case MV_HWS_PORT_FLOW_CONTROL_TX_ONLY_E:
            seq = &hwsGeMacSeqDb[GEMAC_FC_TX_ONLY_SEQ];
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, macNum, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal hwsGeMac28nmIfPortEnable function
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
GT_STATUS hwsGeMac28nmIfPortEnable
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 enable
)
{
    portMode = portMode;
    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, macNum,
            GIG_PORT_MAC_CONTROL_REGISTER0, (enable==GT_TRUE) ? 1 : 0, 0x1));

    return GT_OK;
}


