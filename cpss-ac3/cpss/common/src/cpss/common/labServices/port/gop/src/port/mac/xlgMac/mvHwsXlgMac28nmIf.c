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
*******************************************************************************
* mvHwsXglMac28nmIf.c
*
* DESCRIPTION:
*       XGL MAC interface (for bobcat2)
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMac28nmDb.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMac28nmIf.h>

#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>

#ifndef CO_CPU_RUN
static char* mvHwsMacTypeGet(void)
{
  return "XLGMAC_28nm";
}
#endif

/**
* @internal hwsXlgMac28nmIfInit function
* @endinternal
*
* @brief   Init XLG MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsXlgMac28nmIfInit(GT_U8 devNum, MV_HWS_MAC_FUNC_PTRS *funcPtrArray)
{
    CHECK_STATUS(hwsXlgMac28nmSeqInit(devNum));

    funcPtrArray[XLGMAC].macRestFunc = mvHwsXlgMac28nmReset;
    funcPtrArray[XLGMAC].macLinkGetFunc = mvHwsXlgMac28nmLinkStatus;
    funcPtrArray[XLGMAC].macModeCfgFunc = mvHwsXlgMac28nmModeCfg;
#ifndef CO_CPU_RUN
    funcPtrArray[XLGMAC].macTypeGetFunc = mvHwsMacTypeGet;
    funcPtrArray[XLGMAC].macLbCfgFunc = mvHwsXlgMac28nmLoopbackCfg;
    funcPtrArray[XLGMAC].macLbStatusGetFunc = mvHwsXlgMac28nmLoopbackStatusGet;
    funcPtrArray[XLGMAC].macActiveStatusFunc = mvHwsXlgMac28nmActiveStatusGet;
    funcPtrArray[XLGMAC].macClearChannelCfgFunc = NULL;
#endif
    funcPtrArray[XLGMAC].macFcStateCfgFunc = mvHwsXlgMac28nmFcStateCfg;
    funcPtrArray[XLGMAC].macPortEnableFunc = hwsXlgMac28IfPortEnable;

    return GT_OK;
}

/**
* @internal mvHwsXlgMac28nmReset function
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
GT_STATUS mvHwsXlgMac28nmReset
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    MV_CFG_SEQ  *seq;
    GT_U32      mlgMode=0;

    portMode = portMode;
#if ( (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3)) && defined(CHX_FAMILY) )
    if(hwsFalconAsBobcat3Check() && (HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) &&
        (portMacNum == 64))
    {
        /* this port is NOT CG port in Falcon device */
    }
    else
#endif /* (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3)) && defined(CHX_FAMILY) */
    if (  (((portMacNum % 4) == 0) &&
            (((HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) && (portMacNum != 72)) ||
             ((HWS_DEV_SILICON_TYPE(devNum) == Pipe) &&(portMacNum == 12)) ||
             ((HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) && (portMacNum <= 20)))))
    {
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_UNIT, portMacNum, CG_CONTROL_2, &mlgMode, (1 << 6)));
    }

    /* For BobcatA0 only: ENV_SPEED_MAC_369 - XLG MAC cannot be reset */
    if ((HWS_DEV_GOP_REV(devNum) == GOP_28NM_REV1) && (HWS_DEV_SILICON_TYPE(devNum) == BobcatA0) && (action == RESET))
    {
        return GT_OK;
    }

    if(FULL_RESET == action)
    {
        /* 100G_MLG */
        if (((mlgMode >> 6) == 0x1) && ((portMacNum % 4) == 0))
        {
            seq = &hwsXlgMac28nmSeqDb[XLGMAC_POWER_DOWN_4_Lane_MLG_SEQ];
        }
        else
        {
            seq = &hwsXlgMac28nmSeqDb[XLGMAC_POWER_DOWN_SEQ];
        }
    }
    else if(RESET == action)
    {
        /* 100G_MLG */
        if (((mlgMode >> 6) == 0x1) && ((portMacNum % 4) == 0))
        {
            seq = &hwsXlgMac28nmSeqDb[XLGMAC_RESET_4_Lane_MLG_SEQ];
        }
        else
        {
            seq = &hwsXlgMac28nmSeqDb[XLGMAC_RESET_SEQ];
        }
    }
    else if (UNRESET == action)
    {
        /* 100G_MLG */
        if (((mlgMode >> 6) == 0x1) && ((portMacNum % 4) == 0))
        {
            seq = &hwsXlgMac28nmSeqDb[XLGMAC_UNRESET_4_Lane_MLG_SEQ];
        }
        else
        {
            seq = &hwsXlgMac28nmSeqDb[XLGMAC_UNRESET_SEQ];
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, portMacNum, seq->cfgSeq, seq->cfgSeqSize));

    if ((HWS_DEV_GOP_REV(devNum) >= GOP_28NM_REV3) && (FULL_RESET == action))
    {
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, portMacNum, MSM_PORT_METAL_FIX, 0, (0xF << 9)));
    }

    return GT_OK;
}

/**
* @internal mvHwsXlgMac28nmModeCfg function
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
GT_STATUS mvHwsXlgMac28nmModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_CFG_SEQ      *seq;
    MV_HWS_XLG_MAC_SUB_SEQ seqId;
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(curPortParams.numOfActLanes)
    {
        case 1:
            /* from Bobcat3 and above in Serdes speed 25_78125G (width mode == 40BIT) */
            if (curPortParams.serdes10BitStatus == _40BIT_ON)
            {
                if (curPortParams.portFecMode != RS_FEC)
                {
                    seqId = XLGMAC_MODE_1_Lane_25G_SEQ;
                }
                else
                {
                    seqId = XLGMAC_MODE_1_Lane_25G_RS_FEC_SEQ;
                }

            }
            else
            {
                seqId = XLGMAC_MODE_1_Lane_SEQ;
            }
            break;
        case 2:
            /* from Bobcat3 and above in Serdes speed 25_78125G (width mode == 40BIT) */
            if (curPortParams.serdes10BitStatus == _40BIT_ON)
            {
                if (curPortParams.portFecMode != RS_FEC)
                {
                    seqId = XLGMAC_MODE_2_Lane_50G_SEQ;
                }
                else
                {
                    seqId = XLGMAC_MODE_2_Lane_50G_RS_FEC_SEQ;
                }
            }
            else
            {
                seqId = XLGMAC_MODE_2_Lane_SEQ;
            }
            break;
        case 4:
            if (portMode == _100GBase_MLG)
            {
                seqId = XLGMAC_MODE_4_Lane_MLG_SEQ;
            }
            else
            {
                seqId = XLGMAC_MODE_4_Lane_SEQ;
            }
            break;
        default:
            return GT_BAD_PARAM;
    }

    seq = &hwsXlgMac28nmSeqDb[seqId];
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portMacNumber, seq->cfgSeq, seq->cfgSeqSize));

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
#endif /*  CM3 */

    /* for Pipe TxDMA interface should work in 256bits. */
    if (HWS_DEV_SILICON_TYPE(devNum) == Pipe)
    {
        if ((seqId == XLGMAC_MODE_1_Lane_25G_SEQ) || (seqId == XLGMAC_MODE_1_Lane_25G_RS_FEC_SEQ))
        {
            /* for XLG port: set Rx/Tx DMA interface width to 256 bits */
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, portMacNum, MSM_PORT_MAC_CONTROL_REGISTER4, 0x810, 0x1F10));
        }
    }

    if (HWS_DEV_GOP_REV(devNum) >= GOP_28NM_REV3)
    {
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, portMacNum, MSM_PORT_METAL_FIX, (1 << 9), (0xF << 9)));
    }

    return GT_OK;
}

/**
* @internal mvHwsXlgMac28nmLoopbackCfg function
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
GT_STATUS mvHwsXlgMac28nmLoopbackCfg
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
      return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, macNum, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsXlgMac28nmFcStateCfg function
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
GT_STATUS mvHwsXlgMac28nmFcStateCfg
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
* @internal mvHwsXlgMac28nmLoopbackStatusGet function
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
GT_STATUS mvHwsXlgMac28nmLoopbackStatusGet
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

/**
* @internal mvHwsXlgMac28nmActiveStatusGet function
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
GT_STATUS mvHwsXlgMac28nmActiveStatusGet
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
    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, XLGMAC_UNIT, macNum, MSM_PORT_MAC_CONTROL_REGISTER0, &data, 0));
    if (!((data >> 1) & 1))
    {
        /* unit is under RESET */
        return GT_OK;
    }

    /* get number of active lanes */
    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, XLGMAC_UNIT, macNum, MSM_PORT_MAC_CONTROL_REGISTER3, &data, 0));
    switch ((data >> 13) & 7)
    {
    case 1:
        *numOfLanes = 1;
        break;
    case 2:
        *numOfLanes = 2;
        break;
    case 3:
        *numOfLanes = 4;
        break;
    case 7:
    case 0:
    default:
        *numOfLanes = 0;
        break;
    }

    /* Check if 40G or 50G defined */
    if(HWS_DEV_SILICON_TYPE(devNum) == Bobcat3 || HWS_DEV_SILICON_TYPE(devNum) == Aldrin2)
    {
        if(*numOfLanes == 4)
        {
            /* read {cg_goto_sd} field */
            CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_UNIT, (macNum & HWS_2_LSB_MASK_CNS), CG_CONTROL_2, &data, 0xF << 13));
            if(data != 0)
            {
                *numOfLanes = 2;
            }
        }
    }

    return GT_OK;
}
#endif

/**
* @internal mvHwsXlgMac28nmLinkStatus function
* @endinternal
*
* @brief   Get MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXlgMac28nmLinkStatus
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

/**
* @internal hwsXlgMac28IfPortEnable function
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
GT_STATUS hwsXlgMac28IfPortEnable
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

