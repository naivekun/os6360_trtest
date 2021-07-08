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
* mvHwsMMPcsIf_v3.c
*
* DESCRIPTION:
*       MMPCS V3.0 interface API for puma3B0
*
* FILE REVISION NUMBER:
*       $Revision: 17 $
*
*******************************************************************************/

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcs28nmIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcs28nmDb.h>

#ifdef RUN_PCS_DEBUG
#include <gtOs/gtOsIo.h>
#define DEBUG_PCS(s)              hwsOsPrintf s
#else
#define DEBUG_PCS(s)
#endif

#ifndef CO_CPU_RUN
static char* mvHwsPcsTypeGetFunc(void)
{
    return "MMPCS_28nm";
}
#endif

static GT_STATUS mvHwsMMPcs28nmPcsNumGet
(
    GT_U8       devNum,
    GT_U32      firstPcsNum,
    GT_U8       pcsIndex,
    GT_UOPT     *activePcsNum
);


#ifdef GM_USED
    #define GM_WA_BAD_READ_PCS_REG \
        data = 0;/* the GM returns 0xbadad ... modify to 0 */
#else
    #define GM_WA_BAD_READ_PCS_REG /*empty*/
#endif /*GM_USED*/



/**
* @internal mvHwsMMPcs28nmIfInit function
* @endinternal
*
* @brief   Init MMPCS configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmIfInit(MV_HWS_PCS_FUNC_PTRS *funcPtrArray)
{

#ifndef CO_CPU_RUN
    funcPtrArray[MMPCS].pcsTestGenFunc = mvHwsMMPcs28nmTestGenCfg;
    funcPtrArray[MMPCS].pcsTestGenStatusFunc = mvHwsMMPcs28nmTestGenStatus;
    funcPtrArray[MMPCS].pcsTypeGetFunc = mvHwsPcsTypeGetFunc;
    funcPtrArray[MMPCS].pcsFecCfgFunc = mvHwsMMPcs28nmFecConfig;
    funcPtrArray[MMPCS].pcsFecCfgGetFunc = mvHwsMMPcs28nmFecConfigGet;
    funcPtrArray[MMPCS].pcsLbCfgFunc = mvHwsMMPcs28nmLoopBack;
    funcPtrArray[MMPCS].pcsLbCfgGetFunc = mvHwsMMPcs28nmLoopBackGet;
    funcPtrArray[MMPCS].pcsActiveStatusGetFunc = mvHwsMMPcs28nmActiveStatusGet;
    funcPtrArray[MMPCS].pcsCheckGearBoxFunc = mvHwsMMPcs28nmCheckGearBox;
#endif

    funcPtrArray[MMPCS].pcsAlignLockGetFunc = mvHwsMMPcs28nmAlignLockGet;
    funcPtrArray[MMPCS].pcsRxResetFunc = mvHwsMMPcs28nmRxReset;
    funcPtrArray[MMPCS].pcsExtPllCfgFunc = mvMmPcs28nmExtPllCfg;
    funcPtrArray[MMPCS].pcsResetFunc = mvHwsMMPcs28nmReset;
    funcPtrArray[MMPCS].pcsModeCfgFunc = mvHwsMMPcs28nmMode;
    funcPtrArray[MMPCS].pcsSignalDetectMaskEn = mvHwsMMPcs28nmSignalDetectMaskSet;

    funcPtrArray[MMPCS].pcsSendFaultSetFunc = mvHwsMMPcs28nmSendFaultSet;

    return GT_OK;
}

/**
* @internal mvHwsMMPcs28nmRxReset function
* @endinternal
*
* @brief   Set the selected RX PCS type and number to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] action                   - reset / unreset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmRxReset
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    MV_HWS_RESET            action
)
{
    GT_UREG_DATA data;
    GT_U8   i;
    GT_U8   countVal;
    GT_UOPT activePcsNum;

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MMPCS_UNIT, pcsNum, PCS40G_COMMON_CONTROL, &data, 0));
    GM_WA_BAD_READ_PCS_REG
    countVal = (data >> 8) & 3;
    countVal = (countVal == 0) ? 1 : countVal * 2;

    if (action == RESET)
    {
        /* reset PCS Rx only */
        /* on first lane reset bits 5,4,3 */
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, MMPCS_UNIT, pcsNum, PCS_RESET_REG, 0, 0x38));
        for (i = 1; i < countVal; i++)
        {
            /* Return the current active PCS number */
            CHECK_STATUS(mvHwsMMPcs28nmPcsNumGet(devNum, pcsNum, i, &activePcsNum));

            /* on other lanes reset bits 4,3 only */
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, MMPCS_UNIT, activePcsNum, PCS_RESET_REG, 0, 0x18));
        }
    }
    else
    {
        /* un reset PCS Rx only */
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, MMPCS_UNIT, pcsNum, PCS_RESET_REG, (1 << 5), (1 << 5)));
        for (i = 0; i < countVal; i++)
        {
            /* Return the current active PCS number */
            CHECK_STATUS(mvHwsMMPcs28nmPcsNumGet(devNum, pcsNum, i, &activePcsNum));

            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, MMPCS_UNIT, activePcsNum, PCS_RESET_REG, (1 << 4), (1 << 4)));
        }
        for (i = 0; i < countVal; i++)
        {
            /* Return the current active PCS number */
            CHECK_STATUS(mvHwsMMPcs28nmPcsNumGet(devNum, pcsNum, i, &activePcsNum));

            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, MMPCS_UNIT, activePcsNum, PCS_RESET_REG, (1 << 3), (1 << 3)));
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsMMPcs28nmSendFaultSet function
* @endinternal
*
* @brief   Configure the PCS to start or stop sending fault signals to partner.
*         on single lane, the result will be local-fault on the sender and remote-fault on the receiver,
*         on multi-lane there will be local-fault on both sides, and there won't be align lock
*         at either side.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] portMode                 - portMode
* @param[in] send                     - send or stop sending
*       faults
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmSendFaultSet
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                pcsNum,
    MV_HWS_PORT_STANDARD  portMode,
    GT_BOOL               send
)
{

    MV_CFG_SEQ   *seq;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, pcsNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (send==GT_TRUE)
    {
            /*  start sending fault signals - single lane - single write operation*/
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, MMPCS_UNIT, pcsNum, TEST_PATTERN_CONFIGURATION, 0x6, 0));
            /*  start sending fault signals - multi lane - full lanes sequence */
            seq = &hwsMmPsc28nmSeqDb[MMPCS_START_SEND_FAULT_MULTI_LANE_SEQ];
            CHECK_STATUS_EXT(mvCfgSeqExec(devNum, portGroup, pcsNum, seq->cfgSeq, seq->cfgSeqSize), LOG_ARG_PCS_IDX_MAC(pcsNum));
    }
    else
    {
        /*  stop sending fault signals - single lane - single write operation*/
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, MMPCS_UNIT, pcsNum, TEST_PATTERN_CONFIGURATION, 0x0, 0));
        if ((curPortParams.portFecMode == RS_FEC) && (curPortParams.numOfActLanes == 1))
        {
                /* stop sending faults signals - multi lanes - and return the lane markers to RS-FEC configuration */
                seq =&hwsMmPsc28nmSeqDb[MMPCS_STOP_SEND_FAULT_MULTI_LANE_RS_FEC_SEQ];
        }
        else
        {
            /*  stop sending fault signals - multi lane - full lanes sequence */
            seq = &hwsMmPsc28nmSeqDb[MMPCS_STOP_SEND_FAULT_MULTI_LANE_SEQ];
        }
        CHECK_STATUS_EXT(mvCfgSeqExec(devNum, portGroup, pcsNum, seq->cfgSeq, seq->cfgSeqSize), LOG_ARG_PCS_IDX_MAC(pcsNum));
    }

    return GT_OK;
}

/**
* @internal mvHwsMMPcs28nmReset function
* @endinternal
*
* @brief   Set the selected PCS type and number to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] action                   - reset / unreset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmReset
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    GT_UREG_DATA countVal;
    MV_CFG_SEQ   *seq;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_SERDES_BUS_WIDTH_ENT   serdes10BitStatus;
    GT_U32 pcsNum;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsNum = curPortParams.portPcsNumber;
    serdes10BitStatus = curPortParams.serdes10BitStatus;
    countVal = curPortParams.numOfActLanes;

    seq = NULL;
    if (FULL_RESET == action)
    {
       switch (countVal)
       {
           case 1:
                /* from Bobcat3 and above in Serdes speed 25_78125G (width mode == 40BIT) */
                if ((serdes10BitStatus == _40BIT_ON) && (HWS_DEV_SILICON_TYPE(devNum) >= Bobcat3))
                {
                    seq = &hwsMmPsc28nmSeqDb[MMPCS_POWER_DOWN_1_LANE_25G_SEQ];
                }
                else
                {
                    seq = &hwsMmPsc28nmSeqDb[MMPCS_RESET_1_LANE_SEQ];
                }
                break;
           case 2:
               if ((HWS_DEV_SILICON_TYPE(devNum) != Alleycat3A0) || (pcsNum != 29))
               {
                   /* from Bobcat3 and above in Serdes speed 25_78125G (width mode == 40BIT) */
                   if ((serdes10BitStatus == _40BIT_ON) && (HWS_DEV_SILICON_TYPE(devNum) >= Bobcat3))
                   {
                       /* in 50G PCS mode: two Serdes lanes are mapped to 4 PCS lanes */
                       seq = &hwsMmPsc28nmSeqDb[MMPCS_POWER_DOWN_4_LANE_50G_SEQ];
                   }
                   else
                   {
                       seq = &hwsMmPsc28nmSeqDb[MMPCS_POWER_DOWN_2_LANE_SEQ];
                   }
               }
               else
               {
                   seq = &hwsMmPsc28nmSeqDb[MMPCS_FABRIC_POWER_DOWN_2_LANE_SEQ];
               }
               break;
           case 4:
               /* from Bobcat3 and above in Serdes speed 25_78125G (width mode == 40BIT) */
               if ((serdes10BitStatus == _40BIT_ON) && (HWS_DEV_SILICON_TYPE(devNum) >= Bobcat3))
               {
                   /* in 50G PCS mode: two Serdes lanes are mapped to 4 PCS lanes */
                   seq = &hwsMmPsc28nmSeqDb[MMPCS_POWER_DOWN_4_LANE_50G_SEQ];
               }
               else
               {
                   seq = &hwsMmPsc28nmSeqDb[MMPCS_POWER_DOWN_4_LANE_SEQ];
               }
               break;
           default:
               HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("wrong full-reset countVal"), LOG_ARG_GEN_PARAM_MAC(countVal));
       }
   }/* FULL_RESET action */
   else if (RESET == action)
   {
        switch(countVal)
        {
           case 1:
               /* from Bobcat3 and above in Serdes speed 25_78125G (width mode == 40BIT) */
               if ((serdes10BitStatus == _40BIT_ON) && (HWS_DEV_SILICON_TYPE(devNum) >= Bobcat3))
               {
                   seq = &hwsMmPsc28nmSeqDb[MMPCS_RESET_1_LANE_25G_SEQ];
               }
               else
               {
                   seq = &hwsMmPsc28nmSeqDb[MMPCS_RESET_1_LANE_SEQ];
               }
               break;
           case 2:
               if ((HWS_DEV_SILICON_TYPE(devNum) != Alleycat3A0) || (pcsNum != 29))
               {
                   /* from Bobcat3 and above in Serdes speed 25_78125G (width mode == 40BIT) */
                   if ((serdes10BitStatus == _40BIT_ON) && (HWS_DEV_SILICON_TYPE(devNum) >= Bobcat3))
                   {
                       /* in 50G PCS mode: two Serdes lanes are mapped to 4 PCS lanes */
                       seq = &hwsMmPsc28nmSeqDb[MMPCS_RESET_4_LANE_50G_SEQ];
                   }
                   else
                   {
                       seq = &hwsMmPsc28nmSeqDb[MMPCS_RESET_2_LANE_SEQ];
                   }
               }
               else
               {
                   seq = &hwsMmPsc28nmSeqDb[MMPCS_FABRIC_RESET_2_LANE_SEQ];
               }
               break;
            case 4:
                seq = &hwsMmPsc28nmSeqDb[MMPCS_RESET_4_LANE_SEQ];
                break;
            default:
                HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("wrong reset countVal"), LOG_ARG_GEN_PARAM_MAC(countVal));
        }
    } /* RESET action */
    else if (UNRESET == action)
    {
        switch(countVal)
        {
            case 1:
                if ((serdes10BitStatus == _40BIT_ON) && (HWS_DEV_SILICON_TYPE(devNum) >= Bobcat3))
                {
                    seq = &hwsMmPsc28nmSeqDb[MMPCS_UNRESET_1_LANE_25G_SEQ];
                    if (portMode == _26_7GBase_KR)
                    {
                        seq = &hwsMmPsc28nmSeqDb[MMPCS_UNRESET_1_LANE_26_7G_SEQ];
                    }
                }
                else if (HWS_DEV_GOP_REV(devNum) < GOP_28NM_REV3)
                {
                    seq = &hwsMmPsc28nmSeqDb[MMPCS_UNRESET_1_LANE_SEQ];
                }
                else
                {
                    seq = &hwsMmPsc28nmSeqDb[MMPCS_GOP_REV3_UNRESET_1_LANE_SEQ];
                }
                break;
            case 2:
                if((HWS_DEV_SILICON_TYPE(devNum) != Alleycat3A0) || (pcsNum != 29))
                {
                    /* from Bobcat3 and above in Serdes speed 25_78125G (width mode == 40BIT) */
                    if ((serdes10BitStatus == _40BIT_ON) && (HWS_DEV_SILICON_TYPE(devNum) >= Bobcat3))
                    {   /* in 50G PCS mode: two Serdes lanes are mapped to 4 PCS lanes */
                        seq = &hwsMmPsc28nmSeqDb[MMPCS_GOP_REV3_UNRESET_4_LANE_SEQ];
                    }
                    else if (HWS_DEV_GOP_REV(devNum) < GOP_28NM_REV3)
                    {
                        seq = &hwsMmPsc28nmSeqDb[MMPCS_UNRESET_2_LANE_SEQ];
                    }
                    else
                    {
                        seq = &hwsMmPsc28nmSeqDb[MMPCS_GOP_REV3_UNRESET_2_LANE_SEQ];
                    }
                }
                else
                {
                    seq = &hwsMmPsc28nmSeqDb[MMPCS_FABRIC_UNRESET_2_LANE_SEQ];
                }
                break;
            case 4:
                if (HWS_DEV_GOP_REV(devNum) < GOP_28NM_REV3)
                {
                    seq = &hwsMmPsc28nmSeqDb[MMPCS_UNRESET_4_LANE_SEQ];
                }
                else
                {
                    seq = &hwsMmPsc28nmSeqDb[MMPCS_GOP_REV3_UNRESET_4_LANE_SEQ];
                }
                break;
            default:
                HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("wrong unreset countVal "), LOG_ARG_GEN_PARAM_MAC(countVal));
        }
    }/* UNRESET action */
    else
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("undefined reset action resetMode "), LOG_ARG_GEN_PARAM_MAC(action));
    }

    if (NULL == seq)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ARG_STRING_MAC("MPCS28nm reset sequence not initialized "),
                                        LOG_ARG_GEN_PARAM_MAC(action), LOG_ARG_GEN_PARAM_MAC(countVal));
    }

    CHECK_STATUS_EXT(mvCfgSeqExec(devNum, portGroup, pcsNum, seq->cfgSeq, seq->cfgSeqSize), LOG_ARG_PCS_IDX_MAC(pcsNum));

    return GT_OK;
}


/**
* @internal mvHwsMMPcs28nmMode function
* @endinternal
*
* @brief   Set the internal mux's to the required PCS in the PI.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmMode
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32                  numOfLanes;
    MV_CFG_SEQ *seq;
    GT_UOPT activePcsNum;
    GT_UOPT i;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    numOfLanes = (curPortParams.serdesMediaType == RXAUI_MEDIA) ? 2 * curPortParams.numOfActLanes : curPortParams.numOfActLanes;

    /* clear register used to prevent WA function execution */
    if (HWS_DEV_SILICON_TYPE(devNum) != Alleycat3A0)
    {
        /* EXTERNAL_CONTROL - Select recovered clock 0 per pair of cores.
           In Alleycat3 device this setting is not relevant due to Mux in DMA unit between regular and extended Stacking ports */
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, curPortParams.portPcsNumber, EXTERNAL_CONTROL, 0, 3));

        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, MMPCS_UNIT, curPortParams.portPcsNumber, FEC_DEC_DMA_WR_DATA, 0, 0));
    }

    /* reduce the probability of the link loss to about once in ~1000 times */
    for (i = 0; i < numOfLanes; i++)
    {
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, MMPCS_UNIT, curPortParams.portPcsNumber, CHANNEL0_RX_GB_FIFO_CONFIGURATION + i*4, 0x18, 0));
    }

    switch (numOfLanes)
    {
        case 1:
            /* from Bobcat3 and above in Serdes speed 25_78125G (width mode == 40BIT) */
            if (curPortParams.serdes10BitStatus == _40BIT_ON)
            {
                seq = &hwsMmPsc28nmSeqDb[MMPCS_MODE_1_LANE_25G_SEQ];
            }
            else
            {
                seq = &hwsMmPsc28nmSeqDb[MMPCS_MODE_1_LANE_SEQ];
            }
            break;
        case 2:
            if ((HWS_DEV_SILICON_TYPE(devNum) != Alleycat3A0) || (curPortParams.portPcsNumber != 29))
            {
                /* from Bobcat3 and above in Serdes speed 25_78125G (width mode == 40BIT) */
                if (curPortParams.serdes10BitStatus == _40BIT_ON)
                {
                    /* in 50G PCS mode: two Serdes lanes are mapped to 4 PCS lanes */
                    seq = &hwsMmPsc28nmSeqDb[MMPCS_MODE_4_LANE_50G_SEQ];
                }
                else
                {
                    seq = &hwsMmPsc28nmSeqDb[MMPCS_MODE_2_LANE_SEQ];
                }
            }
            else
            {
                seq = &hwsMmPsc28nmSeqDb[MMPCS_FABRIC_MODE_2_LANE_SEQ];
            }
            break;
        case 4:
            seq = &hwsMmPsc28nmSeqDb[MMPCS_MODE_4_LANE_SEQ];
            break;
        default:
            return GT_NOT_SUPPORTED;
    }
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portPcsNumber, seq->cfgSeq, seq->cfgSeqSize));

    /* Configure FEC disable/enable on PCS */
    CHECK_STATUS(mvHwsMMPcs28nmFecConfig(devNum, portGroup, curPortParams.portPcsNumber, curPortParams.portFecMode));

    for (i = 0; i < numOfLanes; i++)
    {
        /* Return the current active PCS number */
        CHECK_STATUS(mvHwsMMPcs28nmPcsNumGet(devNum, curPortParams.portPcsNumber, (GT_U8)i, &activePcsNum));

        /* always set bit 7 to 0; */
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, MMPCS_UNIT, activePcsNum, PCS40G_COMMON_CONTROL, 0, (1 << 7)));
    }

    return GT_OK;
}

GT_STATUS mvMmPcs28nmExtPllCfg
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum
)
{

    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, MMPCS_UNIT, pcsNum, PCS40G_COMMON_CONTROL, (1 << 14), (1 << 14)));

    return GT_OK;
}

/**
* @internal mvHwsMMPcs28nmSignalDetectMaskSet function
* @endinternal
*
* @brief   Set all related PCS with Signal Detect Mask value (1/0).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] maskEn                   - if true, enable signal detect mask bit
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmSignalDetectMaskSet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    GT_BOOL                 maskEn
)
{
    GT_UREG_DATA data;
    GT_U8        countVal, i;
    GT_UOPT      activePcsNum;

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MMPCS_UNIT, pcsNum, PCS40G_COMMON_CONTROL, &data, 0));
    GM_WA_BAD_READ_PCS_REG
    countVal = (data >> 8) & 3;
    countVal = (countVal == 0) ? 1 : countVal * 2;

    data = (maskEn == GT_TRUE) ? (1 << 14) : 0;
    for (i = 0; i < countVal; i++)
    {
        /* Return the current active PCS number */
        CHECK_STATUS(mvHwsMMPcs28nmPcsNumGet(devNum, pcsNum, i, &activePcsNum));

        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, MMPCS_UNIT, activePcsNum, CHANNEL_CONFIGURATION, data, (1 << 14)));
    }

    return GT_OK;
}

/**
* @internal mvHwsMMPcs28nmAlignLockGet function
* @endinternal
*
* @brief   Read align lock status of given MPCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
*
* @param[out] lock                     - true or false.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmAlignLockGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    GT_BOOL                 *lock
)
{
    GT_UREG_DATA data;

    /* Read if PCS is aligned and all lanes are locked.*/
    genUnitRegisterGet(devNum, portGroup, MMPCS_UNIT, pcsNum, PCS40G_COMMON_STATUS, &data, 0x1);
    GM_WA_BAD_READ_PCS_REG
    *lock = (1 == data);

    return GT_OK;
}

#ifndef CO_CPU_RUN
/**
* @internal mvHwsMMPcs28nmCheckGearBox function
* @endinternal
*
* @brief   Check Gear Box Status on related lanes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] numOfLanes               - number of lanes agregated in PCS
*
* @param[out] laneLock                 - true or false.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmCheckGearBox
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *laneLock
)
{
    GT_UREG_DATA data;
    GT_UREG_DATA mask;
    GT_U32 pcsNum, numOfLanes;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsNum = curPortParams.portPcsNumber;
    numOfLanes = curPortParams.numOfActLanes;

    /* Check Gear Box Status */
    /* gear box status can show lock in not used bits, so read only required bits */
    mask = ((1 << numOfLanes) - 1)<<1;
    genUnitRegisterGet(devNum, portGroup, MMPCS_UNIT, pcsNum, GEARBOX_STATUS, &data, mask);
    GM_WA_BAD_READ_PCS_REG

    *laneLock = (data == mask) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal mvHwsMMPcs28nmLoopBack function
* @endinternal
*
* @brief   Set PCS loop back.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
*                                      pcsType   - PCS type
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmLoopBack
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_CFG_SEQ *seq;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(lbType)
    {
    case DISABLE_LB:
      seq = &hwsMmPscSeqDb[MMPCS_LPBK_NORMAL_SEQ];
      break;
    case RX_2_TX_LB:
      seq = &hwsMmPscSeqDb[MMPCS_LPBK_RX2TX_SEQ];
      break;
    case TX_2_RX_LB:
      seq = &hwsMmPscSeqDb[MMPCS_LPBK_TX2RX_SEQ];
      break;
    default:
      return GT_NOT_SUPPORTED;
      break;
    }
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portPcsNumber, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsMMPcs28nmLoopBackGet function
* @endinternal
*
* @brief   Get the PCS loop back mode state.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmLoopBackGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_UREG_DATA    data;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MMPCS_UNIT, curPortParams.portPcsNumber, PCS40G_COMMON_CONTROL, &data, (0x3 << 3)));
    GM_WA_BAD_READ_PCS_REG

    switch(data>>3)
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
* @internal mvHwsMMPcs28nmTestGenCfg function
* @endinternal
*
* @brief   Set PCS internal test generator mechanisms.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pattern                  -  to generate
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmTestGenCfg
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    MV_HWS_PORT_TEST_GEN_PATTERN     pattern
)
{
    MV_CFG_SEQ *seq;

    switch (pattern)
    {
    case TEST_GEN_KRPAT:
      seq = &hwsMmPscSeqDb[MMPCS_GEN_KRPAT_SEQ];
      break;
    case TEST_GEN_Normal:
      seq = &hwsMmPscSeqDb[MMPCS_GEN_NORMAL_SEQ];
      break;
    default:
      return GT_NOT_SUPPORTED;
      break;
    }
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, pcsNum, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsMMPcs28nmTestGenStatus function
* @endinternal
*
* @brief   Get PCS internal test generator mechanisms error counters and status.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] pattern                  -  to generate
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmTestGenStatus
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    MV_HWS_PORT_TEST_GEN_PATTERN  pattern,
    MV_HWS_TEST_GEN_STATUS        *status
)
{
  GT_UREG_DATA data;

  pattern = pattern;

  if (status == 0)
  {
    return GT_BAD_PARAM;
  }

  CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MMPCS_UNIT, pcsNum, FEC_DEC_STATUS_REG, &data, 0));
  GM_WA_BAD_READ_PCS_REG
  status->checkerLock = (data >> 3) & 1; /* bit 3 */

  CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MMPCS_UNIT, pcsNum, TEST_PATTERN_ERROR_COUNT, &data, 0));
  GM_WA_BAD_READ_PCS_REG
  status->totalErrors = data;

  status->goodFrames = 0;

  return GT_OK;
}

/**
* @internal mvHwsMMPcs28nmFecConfig function
* @endinternal
*
* @brief   Configure FEC disable/enable on PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
*                                      fecEn     - if true, enable FEC
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmFecConfig
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    MV_HWS_PORT_FEC_MODE    portFecType
)
{
    GT_UREG_DATA data;
    GT_U8        countVal, i;
    GT_UOPT      activePcsNum;

    if (portFecType >= RS_FEC)
    {
        return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MMPCS_UNIT, pcsNum, PCS40G_COMMON_CONTROL, &data, 0));
    GM_WA_BAD_READ_PCS_REG
    countVal = (data >> 8) & 0x3;
    countVal = (countVal == 0) ? 1 : countVal * 2;

    data = (portFecType == FC_FEC) ? (1 << 10) : 0;
    for (i = 0; i < countVal; i++)
    {
        /* Return the current active PCS number */
        CHECK_STATUS(mvHwsMMPcs28nmPcsNumGet(devNum, pcsNum, i, &activePcsNum));

        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, MMPCS_UNIT, activePcsNum, PCS40G_COMMON_CONTROL, data, (1 << 10)));
    }

    return GT_OK;
}

/**
* @internal mvHwsMMPcs28nmFecConfigGet function
* @endinternal
*
* @brief   Return FEC disable/enable on PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmFecConfigGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_FEC_MODE    *portFecTypePtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 pcsNum;
    GT_UREG_DATA data;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsNum = curPortParams.portPcsNumber;

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MMPCS_UNIT, pcsNum, PCS40G_COMMON_CONTROL, &data, 0));
    GM_WA_BAD_READ_PCS_REG

    *portFecTypePtr = ((data >> 10) & 1) ? FC_FEC : FEC_OFF;

    return GT_OK;
}

/**
* @internal mvHwsMMPcs28nmActiveStatusGet function
* @endinternal
*
* @brief   Return number of PCS active lanes or 0, if current PCS isn't active.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
*
* @param[out] numOfLanes               - number of lanes agregated in PCS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMMPcs28nmActiveStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    GT_UOPT                 *numOfLanes
)
{
    GT_UREG_DATA data;

    if (numOfLanes == NULL)
    {
      return GT_BAD_PARAM;
    }

    *numOfLanes = 0;
    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MMPCS_UNIT, pcsNum, PCS_RESET_REG, &data, 0));
    GM_WA_BAD_READ_PCS_REG
    /* check unreset bit status */
    /*if (!((data >> 6) & 1))*/
    if (!(data & 0x7))
    {
        /* unit is under RESET */
        return GT_OK;
    }

    /* get number of active lanes */
    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MMPCS_UNIT, pcsNum, PCS40G_COMMON_CONTROL, &data, 0));
    GM_WA_BAD_READ_PCS_REG
    switch ((data >> 8) & 3)
    {
    case 0:
        *numOfLanes = 1;
        break;
    case 1:
        *numOfLanes = 2;
        break;
    case 2:
        *numOfLanes = 4;
        break;
    default:
        *numOfLanes = 0;
        break;
    }

    /* Check if 40G or 50G defined */
    if(hwsIsQuadCgMacSupported(devNum,portGroup, pcsNum/*assuming pcsNum=portNum*/))
    {
        if(*numOfLanes == 4)
        {
            /* read {cg_goto_sd} field */
            CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_UNIT, (pcsNum & HWS_2_LSB_MASK_CNS), CG_CONTROL_2, &data, 0xF << 13));
            if(data != 0)
            {
                *numOfLanes = 2;
            }
        }
    }

    return GT_OK;
}
#endif

GT_STATUS mvHwsMmPcs28nm40GConnectWa
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum
)
{
    devNum = devNum;
    portGroup = portGroup;
    pcsNum = pcsNum;

    return GT_OK;
}

GT_STATUS mvHwsMmPcs28nm40GBackWa
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    GT_U32 regAddr, data;
    GT_U32 unitAddr;
    GT_U32 unitIndex;
    GT_U32 countVal;
    GT_U32 gearVal;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 pcsNum;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsNum = curPortParams.portPcsNumber;

    mvUnitExtInfoGet(devNum, MMPCS_UNIT, pcsNum, &unitAddr, &unitIndex , &pcsNum);

    regAddr = FEC_DEC_DMA_WR_DATA + unitAddr + pcsNum * unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0));
    if (data != 0x8000)
    {
        /* unit not initalize or under TRx training, delete */
        DEBUG_PCS(("\nRun mvHwsMmPcs40GBackWa PORT NOT CREATED on device %d, pcs %d", devNum, pcsNum));
        return GT_OK;
    }
    hwsOsTimerWkFuncPtr(2);

    regAddr = PCS40G_COMMON_CONTROL + unitAddr + pcsNum * unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0));
    countVal = (data >> 8) & 3;
    countVal = (countVal == 0) ? 1 : countVal * 2;

    if (countVal != 4)
    {
        /* not 40G */
        DEBUG_PCS(("\nRun mvHwsMmPcs40GBackWa PORT NOT CREATED (4) on device %d, pcs %d", devNum, pcsNum));
        return GT_OK;
    }

    DEBUG_PCS(("\nRun mvHwsMmPcs40GBackWa on device %d, pcs %d", devNum, pcsNum));
    /* Check Gear Box Status */
    regAddr = unitAddr + pcsNum*unitIndex + GEARBOX_STATUS;
    hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0);
    gearVal = (data >> 1) & 0xF;
    if (gearVal != 0xF)
    {
        DEBUG_PCS(("\nCheck Gear Box Status on device %d, pcs %d (data 0x%x)", devNum, pcsNum, data));
        if (gearVal == 0)
        {
            /* no connector in port */
            return GT_OK;
        }
        else
        {
          return GT_FAIL;
        }
    }
    else
    {
        regAddr = PCS40G_COMMON_STATUS + unitAddr + pcsNum*unitIndex;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0));
        if (!(data & 1))
        {
            DEBUG_PCS(("\nRun RESET on device %d, pcs %d", devNum, pcsNum));
            /* one SD sq_detect = 1 perform pcs reset sequence */
            mvHwsMMPcs28nmReset(devNum,portGroup,phyPortNum,portMode,RESET);
            hwsOsTimerWkFuncPtr(1);
            mvHwsMMPcs28nmReset(devNum,portGroup,phyPortNum,portMode,UNRESET);
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsMMPcs28nmPcsNumGet function
* @endinternal
*
* @brief   Return the current active PCS number.
*
* @param[in] devNum                   - system device number
*                                      pcsNum    - physical PCS number
*                                      laneNum   - lane number in PCS
*
* @param[out] activePcsNum             - Current active PCS number
*                                       None.
*/
static GT_STATUS mvHwsMMPcs28nmPcsNumGet
(
    GT_U8       devNum,
    GT_U32      firstPcsNum,
    GT_U8       pcsIndex,
    GT_UOPT     *activePcsNum
)
{
    if((HWS_DEV_SILICON_TYPE(devNum) != Alleycat3A0) || (firstPcsNum != 29))
    {
        *activePcsNum = firstPcsNum + pcsIndex;
    }
    else
    {
        /* for Alleycat3 port #29, the first PCS number is 29 and second PCS number is 50 */
        *activePcsNum = (pcsIndex == 0) ? 29 : 50;
    }

    return GT_OK;
}

/**
* @internal mvHwsFcFecCorrectedError function
* @endinternal
*
* @brief   Return number of bits corrected by FC-FEC.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - port number
*
* @param[out] corecctedError          - number of corrected
*                                       errors
*
* @retval 0                        - on success
* @retval 1                        - on error
*/

GT_STATUS mvHwsFcFecCorrectedError
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                portNum,
    GT_U32                *correctedError
)
{
    GT_UREG_DATA data;

    if(correctedError == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    if (HWS_DEV_SILICON_TYPE(devNum) == Bobcat3 || HWS_DEV_SILICON_TYPE(devNum) == Aldrin || HWS_DEV_SILICON_TYPE(devNum) == Aldrin2)
    {
        /*Number of bits corrected*/
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MMPCS_UNIT, portNum, FDEC_RECEIVED_CERROR_BITS_REG_0, &data, 0));
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MMPCS_UNIT, portNum, FDEC_RECEIVED_CERROR_BITS_REG_1, correctedError, 0));

        *correctedError=(((*correctedError)<<16)|data);
    }
    else
    {
        /*support only Aldrin & BC3*/
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal mvHwsRsFecCorrectedError function
* @endinternal
*
* @brief   Return number of codewords corrected by RS-FEC.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - port number
*
* @param[out] corecctedError          - number of corrected
*                                       errors
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsRsFecCorrectedError
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                portNum,
    GT_U32                *correctedError
)
{
    GT_UREG_DATA data;
    GT_U32       offset;

    if(NULL == correctedError)
        return GT_BAD_PTR;

    if (HWS_DEV_SILICON_TYPE(devNum) == Bobcat3 || HWS_DEV_SILICON_TYPE(devNum) == Aldrin2)
    {
        if (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2 && portNum >=24)
        {
            /*Aldrin2 RS FEC support only 0-23 ports*/
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("illegal port num for Aldrin2 RS-FEC"));
        }

        if ((portNum % 4 == 0))
            offset = 0;
        else if ((portNum % 4) == 1)
            offset = 0x100;
        else if ((portNum % 4) == 2)
            offset = 0x120;
        else
            offset = 0x140;


        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_RS_FEC_UNIT, (portNum & HWS_2_LSB_MASK_CNS), CG_RS_FEC_CCW_LO + offset, &data, 0));
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_RS_FEC_UNIT, (portNum & HWS_2_LSB_MASK_CNS), CG_RS_FEC_CCW_HI + offset, correctedError, 0));
        *correctedError=((*correctedError)<<16)+data;

    }
    else
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ARG_STRING_MAC("silicon type not supported"));
    }
    return GT_OK;
}


