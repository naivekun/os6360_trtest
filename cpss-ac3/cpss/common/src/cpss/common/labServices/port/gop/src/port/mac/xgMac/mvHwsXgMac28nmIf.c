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
* @file mvHwsXgMac28nmIf.c
*
* @brief XGL MAC (for bobcat2)
*
* @version   1
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

#include <cpss/common/labServices/port/gop/port/mac/xgMac/mvHwsXgMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMac28nmDb.h>
#include <cpss/common/labServices/port/gop/port/mac/xgMac/mvHwsXgMacDb.h>

#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>

#ifndef CO_CPU_RUN
static char* mvHwsMacTypeGet(void)
{
  return "XGMAC_28nm";
}
#endif

/**
* @internal hwsXgMac28nmIfInit function
* @endinternal
*
* @brief   Init XG MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsXgMac28nmIfInit(MV_HWS_MAC_FUNC_PTRS *funcPtrArray)
{
    funcPtrArray[XGMAC].macRestFunc = mvHwsXgMac28nmReset;
    funcPtrArray[XGMAC].macLinkGetFunc = mvHwsXgMac28nmLinkStatus;
    funcPtrArray[XGMAC].macModeCfgFunc = mvHwsXgMac28nmModeCfg;

#ifndef CO_CPU_RUN
    funcPtrArray[XGMAC].macTypeGetFunc = mvHwsMacTypeGet;
    funcPtrArray[XGMAC].macLbCfgFunc = mvHwsXgMac28nmLoopbackCfg;
    funcPtrArray[XGMAC].macLbStatusGetFunc = mvHwsXgMac28nmLoopbackStatusGet;
    funcPtrArray[XGMAC].macActiveStatusFunc = mvHwsXgMac28nmActiveStatusGet;
#endif
    funcPtrArray[XGMAC].macFcStateCfgFunc = mvHwsXgMac28nmFcStateCfg;
    funcPtrArray[XGMAC].macPortEnableFunc = hwsXgMac28nmPortEnable;

    return GT_OK;
}

/**
* @internal mvHwsXgMac28nmReset function
* @endinternal
*
* @brief   Set the XG MAC to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] action                   - normal or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXgMac28nmReset
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    MV_CFG_SEQ *seq;
    portMode = portMode;

    /* For BobcatA0 only: ENV_SPEED_MAC_369 - XLG MAC cannot be reset */
    if ((HWS_DEV_GOP_REV(devNum) == GOP_28NM_REV1) && (HWS_DEV_SILICON_TYPE(devNum) == BobcatA0) && (action == RESET))
    {
        return GT_OK;
    }

    if(action == FULL_RESET)
    {
        seq = &hwsXlgMac28nmSeqDb[XLGMAC_POWER_DOWN_SEQ];
    }
    else if (RESET == action)
    {
        seq = &hwsXlgMac28nmSeqDb[XLGMAC_RESET_SEQ];
    }
    else if (UNRESET == action)
    {
        seq = &hwsXlgMac28nmSeqDb[XLGMAC_UNRESET_SEQ];
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, portMacNum, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsXgMac28nmModeCfg function
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
GT_STATUS mvHwsXgMac28nmModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_CFG_SEQ *seq;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    seq = &hwsXgMacSeqDb[XGMAC_MODE_SEQ];
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portMacNumber, seq->cfgSeq, seq->cfgSeqSize));

    switch (portMode)
    {
        case _2_5GBase_QX:
        case _5GBase_DQX:
        case _5GBase_HX:
        case RXAUI:
        case _10GBase_KX2:
        case _10GBase_KX4:
            if (HWS_DEV_GOP_REV(devNum) >= GOP_28NM_REV1)
            {
                /* set RX-DMA & TX-DMA interfaces to 64bits */
                CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, portMacNum, MSM_PORT_MAC_CONTROL_REGISTER4, (1 << 9), (0xF << 9)));
            }
            break;
        case _20GBase_KX4:
            if ((HWS_DEV_GOP_REV(devNum) == GOP_28NM_REV1) || (HWS_DEV_GOP_REV(devNum) == GOP_28NM_REV2))
            {
                /* set RX-DMA & TX-DMA interfaces to 64bits */
                CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, portMacNum, MSM_PORT_MAC_CONTROL_REGISTER4, (1 << 9), (0xF << 9)));
            }
            else if (HWS_DEV_GOP_REV(devNum) >= GOP_28NM_REV3)
            {
                /* set RX-DMA & TX-DMA interfaces to 256bits */
                CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, portMacNum, MSM_PORT_MAC_CONTROL_REGISTER4, (1 << 11), (0xF << 9)));
            }
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* In Alleycat3 there is a Mux for DMA unit between regular and extended Stacking ports 25,27-29 in MSMS4 unit
       The selector (port RX interface) for changing the configuration exists
       in XLGMAC unit with MAC number 24 in External_control register:
          External_Control_0 (bit 0) - select between regular port 25 (value=0, internal MAC) and extended port 28 (value=1, external client 1)
          External_Control_1 (bit 1) - select between regular port 27 (value=0, internal MAC) and extended port 29 (value=1, external client 3) */
#ifndef CM3
    if (HWS_DEV_SILICON_TYPE(devNum) == Alleycat3A0)
    {
        if (curPortParams.portMacNumber == 28)
        {
            /* for extended Stacking port 28: set External_Control_0 bit to work with external client 1 */
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, 24, EXTERNAL_CONTROL, 1, 0x1));
        }
        else if(curPortParams.portMacNumber == 25)
        {
            /* for regular Stacking port 25: set External_Control_0 bit to work with internal MAC 24 */
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, 24, EXTERNAL_CONTROL, 0, 0x1));
        }

        if (curPortParams.portMacNumber == 29)
        {
            /* for extended Stacking port 29: set External_Control_1 bit to work with external client 3 */
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, 24, EXTERNAL_CONTROL, (1 << 1), (1 << 1)));
        }
        else if(curPortParams.portMacNumber == 27)
        {
            /* for regular Stacking port 27: set External_Control_1 bit to work with internal MAC 24 */
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, 24, EXTERNAL_CONTROL, 0, (1 << 1)));
        }
    }
#endif /* CM3 */

    return GT_OK;
}

/**
* @internal mvHwsXgMac28nmLoopbackCfg function
* @endinternal
*
* @brief   Configure MAC loopback.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] lbType                   - supported loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXgMac28nmLoopbackCfg
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
)
{
    MV_CFG_SEQ *seq;

    portMode = portMode;

    switch (lbType)
    {
    case DISABLE_LB:
      seq = &hwsXlgMac28nmSeqDb[XLGMAC_LPBK_NORMAL_SEQ];
      break;
    case RX_2_TX_LB:
      seq = &hwsXlgMac28nmSeqDb[XLGMAC_LPBK_RX2TX_SEQ];
      break;
    case TX_2_RX_LB:
      seq = &hwsXlgMac28nmSeqDb[XLGMAC_LPBK_TX2RX_SEQ];
      break;
    default:
      return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, macNum, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsXgMac28nmFcStateCfg function
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
GT_STATUS mvHwsXgMac28nmFcStateCfg
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
            seq = &hwsXlgMac28nmSeqDb[XLGMAC_FC_DISABLE_SEQ];
            break;
        case MV_HWS_PORT_FLOW_CONTROL_RX_TX_E:
            seq = &hwsXlgMac28nmSeqDb[XLGMAC_FC_BOTH_SEQ];
            break;
        case MV_HWS_PORT_FLOW_CONTROL_RX_ONLY_E:
            seq = &hwsXlgMac28nmSeqDb[XLGMAC_FC_RX_ONLY_SEQ];
            break;
        case MV_HWS_PORT_FLOW_CONTROL_TX_ONLY_E:
            seq = &hwsXlgMac28nmSeqDb[XLGMAC_FC_TX_ONLY_SEQ];
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, macNum, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

#ifndef CO_CPU_RUN
/**
* @internal mvHwsXgMac28nmLoopbackStatusGet function
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
GT_STATUS mvHwsXgMac28nmLoopbackStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
)
{
    GT_UREG_DATA data;

    portMode = portMode;
    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, XLGMAC_UNIT, macNum, MSM_PORT_MAC_CONTROL_REGISTER1, &data, 0));

    switch ((data >> 13) & 3)
    {
    case 0:
        *lbType = DISABLE_LB;
      break;
    case 1:
        *lbType = TX_2_RX_LB;
      break;
    case 2:
        *lbType = RX_2_TX_LB;
      break;
    default:
      return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}
#endif

/**
* @internal mvHwsXgMac28nmLinkStatus function
* @endinternal
*
* @brief   Get MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXgMac28nmLinkStatus
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *linkStatus
)
{
    GT_UREG_DATA data;

    portMode = portMode;
    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, XLGMAC_UNIT, macNum, MSM_PORT_STATUS, &data, 0));

    *linkStatus = data & 1;
    return GT_OK;
}

#ifndef CO_CPU_RUN
/**
* @internal mvHwsXgMac28nmActiveStatusGet function
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
GT_STATUS mvHwsXgMac28nmActiveStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_UOPT                 *numOfLanes
)
{
    GT_UREG_DATA data;

    portMode = portMode;
    if (numOfLanes == NULL)
    {
        return GT_BAD_PARAM;
    }

    *numOfLanes = 0;
    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, XLGMAC_UNIT, macNum, MSM_PORT_MAC_CONTROL_REGISTER0, &data, 0));
    if (!((data >> 1) & 1))
    {
        /* unit is under RESET */
        return GT_OK;
    }
    /* check if XPCS used */
    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, XLGMAC_UNIT, macNum, MSM_PORT_MAC_CONTROL_REGISTER4, &data, 0));
    if (((data >> 8) & 1) == 0)
    {
        /* XPCS doesn't used */
        return GT_OK;
    }

    *numOfLanes = 1;

    return GT_OK;
}
#endif

/**
* @internal hwsXgMac28nmPortEnable function
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
GT_STATUS hwsXgMac28nmPortEnable
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 enable
)
{
    portMode = portMode;
    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, macNum,
            MSM_PORT_MAC_CONTROL_REGISTER0, (enable==GT_TRUE) ? 1 : 0, 0x1));

    return GT_OK;
}


