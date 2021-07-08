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
* @file mvHwsCgPcs28nmIf.c
*
* @brief CGPCS interface API
*
* @version   2
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/cgPcs/mvHwsCgPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/cgPcs/mvHwsCgPcs28nmIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/cgPcs/mvHwsCgPcs28nmDb.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mac/cgMac/mvHwsCgMac28nmIf.h>

static char* mvHwsPcsTypeGetFunc(void)
{
  return "CGPCS_28nm";
}

/**
* @internal mvHwsCgPcs28nmIfInit function
* @endinternal
*
* @brief   Init GPCS configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgPcs28nmIfInit(GT_U8 devNum, MV_HWS_PCS_FUNC_PTRS *funcPtrArray)
{
    CHECK_STATUS(hwsCgPcs28nmSeqInit(devNum));

    funcPtrArray[CGPCS].pcsResetFunc     = mvHwsCgPcs28nmReset;
    funcPtrArray[CGPCS].pcsModeCfgFunc   = mvHwsCgPcs28nmMode;
    funcPtrArray[CGPCS].pcsLbCfgFunc     = mvHwsCgPcsLoopBack;
    funcPtrArray[CGPCS].pcsLbCfgGetFunc = mvHwsCgPcsLoopBackGet;
    funcPtrArray[CGPCS].pcsTypeGetFunc   = mvHwsPcsTypeGetFunc;
    funcPtrArray[CGPCS].pcsFecCfgFunc    = mvHwsCgPcs28nmFecConfig;
    funcPtrArray[CGPCS].pcsFecCfgGetFunc = mvHwsCgPcs28nmFecConfigGet;
    funcPtrArray[CGPCS].pcsCheckGearBoxFunc = mvHwsCgPcs28nmCheckGearBox;
    funcPtrArray[CGPCS].pcsAlignLockGetFunc = mvHwsCgPcs28nmAlignLockGet;
    funcPtrArray[CGPCS].pcsActiveStatusGetFunc = mvHwsCgPcs28nmActiveStatusGet;
    funcPtrArray[CGPCS].pcsSendFaultSetFunc = mvHwsCgPcs28nmSendFaultSet;

    return GT_OK;
}

/**
* @internal mvHwsCgPcs28nmCheckGearBox function
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
/*
    In order to check Gear box we need to check the following registers:
    SPEED FEC register to check
    100G no fec block lock CG->IP_STATUS [19:0]
    rs fec amps_lock CG->IP_STATUS2 [5:2]
    50G	no fec block_lock CG->IP_STATUS2 Ch0 ->  [13:10] ; Ch2 -> [17:14]
    rs fec amps_lock CG->IP_STATUS2 Ch0 -> [3:2] ; Ch2 -> [5:4]
    fc fec Lane Locked MPCS->GEARBOX_STATUS[0]
    25G no fec Lane Locked MPCS->GEARBOX_STATUS[0]
    rs fec amps_lock IP_STATUS2 Ch0 -> [2] ; Ch1-> [3] ; Ch2 -> [4] ; Ch3 -> [5]
    fc fec Lane Locked MPCS->GEARBOX_STATUS[0]
*/
GT_STATUS mvHwsCgPcs28nmCheckGearBox
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *laneLock
)
{
    GT_U32 data, addr = 0xFFFFFFFF;
    GT_U32 mask;
    GT_U32 pcsNum, numOfLanes, localPcsNum;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsNum = curPortParams.portPcsNumber;
    numOfLanes = curPortParams.numOfActLanes;
    localPcsNum = (curPortParams.portPcsNumber % 4);

    *laneLock = GT_FALSE;

    switch (numOfLanes)
    {
        case 1: /* 25G */
            /* IP_STATUS2 Ch0 -> [2] ; Ch1-> [3] ; Ch2 -> [4] ; Ch3 -> [5] */
            addr = CG_IP_STATUS_2;
            mask = (1<<(localPcsNum + 2));
            break;

        case 2: /* 50G */
            if (curPortParams.portFecMode == FEC_OFF)
            { /* Ch0 ->  [13:10] ; Ch2 -> [17:14] */
                if (localPcsNum == 0)
                {
                    mask = 0x3C00;
                }
                else /* localPcsNum = 2*/
                {
                    mask = 0x3C000;
                }
                addr = CG_IP_STATUS_2;
            }
            else if (curPortParams.portFecMode == RS_FEC)
            { /* CG->IP_STATUS2 Ch0 -> [3:2] ; Ch2 -> [5:4] */
                if (localPcsNum == 0)
                {
                    mask = 0xC;
                }
                else /* localPcsNum = 2*/
                {
                    mask = 0x30;
                }
                addr = CG_IP_STATUS_2;
            }
            break;

        case 4:
            if (curPortParams.portFecMode == FEC_OFF)
            { /* CG->IP_STATUS [19:0] */
                mask = 0xFFFFF;
                addr = CG_IP_STATUS;
            }
            else if (curPortParams.portFecMode == RS_FEC)
            { /* CG->IP_STATUS2 [5:2] */
                mask = 0x3C;
                addr = CG_IP_STATUS_2;
            }
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (addr == 0xFFFFFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_UNIT, pcsNum & HWS_2_LSB_MASK_CNS, addr, &data, mask));
    if ((data & mask) == mask)
    {
        *laneLock = GT_TRUE;
    }

    return GT_OK;
}

/**
* @internal mvHwsCgPcs28nmAlignLockGet function
* @endinternal
*
* @brief   Read align lock status of given CG PCS.
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
GT_STATUS mvHwsCgPcs28nmAlignLockGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    GT_BOOL                 *lock
)
{
    GT_U32 data;

    /* Get align_lock - Alignment Marker Lock indication.
        When asserted (1), the alignment marker lock state machines are locked. */
    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_UNIT, pcsNum, CG_IP_STATUS_2, &data, 0x2));

    *lock = (data != 0);

    return GT_OK;
}

/**
* @internal mvHwsCgPcs28nmMode function
* @endinternal
*
* @brief   Set the CG mode with RS_FEC mode
*         In CG PCS:
*         - 25G mode must work with RS_FEC
*         - 50G/100G modes can work with/without RS_FEC
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgPcs28nmMode
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32                  numOfLanes;
    MV_CFG_SEQ *seqMode=NULL, *seqFec=NULL;
    GT_U32  laneNum;
    GT_STATUS st;
    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    numOfLanes = (curPortParams.serdesMediaType == RXAUI_MEDIA) ? 2 * curPortParams.numOfActLanes : curPortParams.numOfActLanes;

    if (curPortParams.portFecMode == FC_FEC)
    {
        /* FC_FEC is not supported in CG unit */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* for 25G/50G: get the related lane number of corresponded PCS number */
    laneNum = (curPortParams.portPcsNumber % 4);

    switch (numOfLanes)
    {
        case 1: /* 25G */
            seqMode = &hwsCgPcs28nmSeqDb[CGPCS_MODE_1_LANE_0_SEQ + laneNum];
            break;
        case 2: /* 50G */
            if (laneNum == 0)
            {
                seqMode = &hwsCgPcs28nmSeqDb[CGPCS_MODE_2_LANE_0_SEQ];

                if (curPortParams.portFecMode == RS_FEC)
                {
                    seqFec = &hwsCgPcs28nmSeqDb[CGPCS_RS_FEC_2_LANE_0_SEQ];
                }
            }
            else if (laneNum == 2)
            {
                seqMode = &hwsCgPcs28nmSeqDb[CGPCS_MODE_2_LANE_2_SEQ];

                if (curPortParams.portFecMode == RS_FEC)
                {
                    seqFec = &hwsCgPcs28nmSeqDb[CGPCS_RS_FEC_2_LANE_2_SEQ];
                }
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case 4: /* 100G/102G */
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
            if (portMode == _100GBase_MLG)
            {
                seqMode = &hwsCgPcs28nmSeqDb[CGPCS_MODE_4_LANE_MLG_SEQ];
            }
            else
#endif
            {
                seqMode = &hwsCgPcs28nmSeqDb[CGPCS_MODE_4_LANE_SEQ];
            }
            if (curPortParams.portFecMode == RS_FEC)
            {
                seqFec = &hwsCgPcs28nmSeqDb[CGPCS_RS_FEC_4_LANE_SEQ];
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    mvHwsCgMac28nmAccessLock(devNum, portMacNum);
    st = mvCfgSeqExec(devNum, portGroup, curPortParams.portPcsNumber, seqMode->cfgSeq, seqMode->cfgSeqSize);
    if ((seqFec != NULL) && (st == GT_OK))
    {
        st = mvCfgSeqExec(devNum, portGroup, curPortParams.portPcsNumber, seqFec->cfgSeq, seqFec->cfgSeqSize);
    }
    mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
    CHECK_STATUS_EXT(st, LOG_ARG_MAC_IDX_MAC(portMacNum), LOG_ARG_STRING_MAC("calling mvCfgSeqExec"));

    return GT_OK;
}

/**
* @internal mvHwsCgPcs28nmFecConfig function
* @endinternal
*
* @brief   Configure the RS_FEC mode on CG PCS.
*         In CG PCS:
*         - 25G mode must work with RS_FEC
*         - 50G/100G modes can work with/without RS_FEC
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] portFecType              -   FEC_ON (FC_FEC)
*                                      FEC_OFF
*                                      RS_FEC
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgPcs28nmFecConfig
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    MV_HWS_PORT_FEC_MODE    portFecType
)
{
    MV_CFG_SEQ  *seq=NULL;
    GT_U32      data, laneNum;
    GT_U32      activeLanes;

    /* for 25G/50G: get the related lane number of corresponded PCS number */
    laneNum = (pcsNum%4);

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_UNIT, (pcsNum & HWS_2_LSB_MASK_CNS), CG_CONTROL_2, &data, (0xF << 17)));

    /* get the number of active lanes on CG PCS: 1=25G, 2=50G, 4=100G */
    CHECK_STATUS(mvHwsCgPcs28nmActiveLanesNumGet(devNum, portGroup, pcsNum, &activeLanes));

    switch (activeLanes)
    {
        case 1: /* 25G */
            /* TBD - update for 25G_S (only FC or none) and 25G KR (FC/RS/none) */
            break;
        case 2: /* 50G */
            if (portFecType == RS_FEC)
            {
                seq = (laneNum == 0) ? &hwsCgPcs28nmSeqDb[CGPCS_RS_FEC_2_LANE_0_SEQ] :
                                       &hwsCgPcs28nmSeqDb[CGPCS_RS_FEC_2_LANE_2_SEQ];
                CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, pcsNum, seq->cfgSeq, seq->cfgSeqSize));
            }
            else
            {
                data = (laneNum == 0) ? (3 << 17) : (3 << 19);
                CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, CG_UNIT, (pcsNum & HWS_2_LSB_MASK_CNS), CG_CONTROL_2, 0, data));
            }
            break;
        case 4: /* 100G */
            if (portFecType == RS_FEC)
            {
                seq = &hwsCgPcsPscSeqDb[CGPCS_RS_FEC_4_LANE_SEQ];
                CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, pcsNum, seq->cfgSeq, seq->cfgSeqSize));
            }
            else
            {
                CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, CG_UNIT, pcsNum, CG_CONTROL_2, 0, (0xF << 17)));
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal mvHwsCgPcs28nmFecConfigGet function
* @endinternal
*
* @brief   Return the RS_FEC disable/enable status on PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @param[out] portFecTypePtr           - pointer to fec mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgPcs28nmFecConfigGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_FEC_MODE    *portFecTypePtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  data, laneNum;
    GT_U32  activeLanes;
    GT_U32  pcsNum;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsNum = curPortParams.portPcsNumber;
    activeLanes = curPortParams.numOfActLanes;

    /* for 25G/50G: get the related lane number of corresponded PCS number */
    laneNum = (pcsNum%4);

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_UNIT, (pcsNum & HWS_2_LSB_MASK_CNS), CG_CONTROL_2, &data, (0xF << 17)));

    switch (activeLanes)
    {
        case 1: /* 25G */
            if ((data >> (17+laneNum)) & 1)
            {
                *portFecTypePtr = RS_FEC;
            }
            else
            {   /* 25G mode must work with RS_FEC */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case 2: /* 50G */
            if (laneNum == 0)
            {
                *portFecTypePtr = (((data >> 17) & 3) == 3) ? RS_FEC : FEC_OFF;
            }
            else if (laneNum == 2)
            {
                *portFecTypePtr = (((data >> 19) & 3) == 3) ? RS_FEC : FEC_OFF;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case 4: /* 100G */
            *portFecTypePtr = (((data >> 17) & 0xF) == 0xF) ? RS_FEC : FEC_OFF;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal mvHwsCgPcs28nmReset function
* @endinternal
*
* @brief   Set CG PCS RESET/UNRESET or FULL_RESET action.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] action                   - RESET/UNRESET or FULL_RESET
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgPcs28nmReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_CFG_SEQ  *seq;
    GT_U32      laneNum;
    GT_U32      cgEnabledLanesBitmap;
    GT_BOOL     otherLineInUse;
    GT_U32      mlgMode=0;
    GT_U32      pcsNum;
    MV_HWS_CGPCS_SUB_SEQ seqId;
    GT_STATUS st;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsNum = curPortParams.portPcsNumber;
    /* for 25G/50G: get the related lane number of corresponded PCS number */
    laneNum = (pcsNum%4);

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_UNIT, (pcsNum & HWS_2_LSB_MASK_CNS), CG_CONTROL_2, &cgEnabledLanesBitmap, (0xF << 13)));
    cgEnabledLanesBitmap = (cgEnabledLanesBitmap >> 13) & 0xF;

    /* check if MLG mode is enabled */
    if ((pcsNum % 4) == 0)
    {
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_UNIT, pcsNum, CG_CONTROL_2, &mlgMode, (1 << 6)));
    }

    if (FULL_RESET == action)
    {
        switch (curPortParams.numOfActLanes)
        {
            case 1: /* 25G */

                otherLineInUse = (((cgEnabledLanesBitmap & ~(1<<laneNum)) & 0xF) == 0) ? GT_FALSE : GT_TRUE;

                if(otherLineInUse)
                {
                    seq = &hwsCgPcs28nmSeqDb[CGPCS_REDUCED_POWER_DOWN_1_LANE_0_SEQ + laneNum];
                }
                else
                {
                    seq = &hwsCgPcs28nmSeqDb[CGPCS_POWER_DOWN_1_LANE_0_SEQ + laneNum];
                }
                break;
            case 2: /* 50G */

                otherLineInUse = ((((cgEnabledLanesBitmap & 0x5) & ~(1<<laneNum)) & 0xF) == 0) ? GT_FALSE : GT_TRUE;

                if (laneNum == 0)
                {
                    seqId = (otherLineInUse == GT_FALSE) ? CGPCS_POWER_DOWN_2_LANE_0_SEQ : CGPCS_REDUCED_POWER_DOWN_2_LANE_0_SEQ;
                }
                else if (laneNum == 2)
                {
                    seqId = (otherLineInUse == GT_FALSE) ? CGPCS_POWER_DOWN_2_LANE_2_SEQ : CGPCS_REDUCED_POWER_DOWN_2_LANE_2_SEQ;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                seq = &hwsCgPcs28nmSeqDb[seqId];
                break;
            case 4:
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
                /* 100G_MLG */
                if ((mlgMode >> 6) == 0x1)
                {
                    seq = &hwsCgPcs28nmSeqDb[CGPCS_POWER_DOWN_4_LANE_MLG_SEQ];
                }
                else
#endif
                {/* 100G */
                    seq = &hwsCgPcs28nmSeqDb[CGPCS_POWER_DOWN_4_LANE_SEQ];
                }
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
                seq = &hwsCgPcs28nmSeqDb[CGPCS_RESET_1_LANE_0_SEQ + laneNum];
                break;
            case 2: /* 50G */
                if (laneNum == 0)
                {
                    seq = &hwsCgPcs28nmSeqDb[CGPCS_RESET_2_LANE_0_SEQ];
                }
                else if (laneNum == 2)
                {
                    seq = &hwsCgPcs28nmSeqDb[CGPCS_RESET_2_LANE_2_SEQ];
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                break;
            case 4:
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
                /* 100G_MLG */
                if ((mlgMode >> 6) == 0x1)
                {
                    seq = &hwsCgPcs28nmSeqDb[CGPCS_RESET_4_LANE_MLG_SEQ];
                }
                else
#endif
                {/* 100G */
                    seq = &hwsCgPcs28nmSeqDb[CGPCS_RESET_4_LANE_SEQ];
                }
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
                seq = &hwsCgPcs28nmSeqDb[CGPCS_UNRESET_1_LANE_0_SEQ + laneNum];
                break;
            case 2: /* 50G */
                if (laneNum == 0)
                {
                    seq = &hwsCgPcs28nmSeqDb[CGPCS_UNRESET_2_LANE_0_SEQ];
                }
                else if (laneNum == 2)
                {
                    seq = &hwsCgPcs28nmSeqDb[CGPCS_UNRESET_2_LANE_2_SEQ];
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                break;
            case 4:
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
                /* 100G_MLG */
                if ((mlgMode >> 6) == 0x1)
                {
                    seq = &hwsCgPcs28nmSeqDb[CGPCS_UNRESET_4_LANE_MLG_SEQ];
                }
                else
#endif
                {/* 100G */
                    seq = &hwsCgPcs28nmSeqDb[CGPCS_UNRESET_4_LANE_SEQ];
                }
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
    st = mvCfgSeqExec(devNum, portGroup, pcsNum, seq->cfgSeq, seq->cfgSeqSize);
    mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
    CHECK_STATUS_EXT(st, LOG_ARG_MAC_IDX_MAC(portMacNum), LOG_ARG_STRING_MAC("calling mvCfgSeqExec"));

    return GT_OK;
}

/**
* @internal mvHwsCgPcs28nmActiveLanesNumGet function
* @endinternal
*
* @brief   Return the number of active lanes of CG PCS number.
*         25G - 1 lane, 50G - 2 lanes, 100G - 4 lanes
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - PCS number
*
* @param[out] activeLanes              - Number of activeLanes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgPcs28nmActiveLanesNumGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      pcsNum,
    GT_U32      *activeLanes
)
{
    GT_U32  data, laneNum;

    /* for 25G/50G: get the related lane number of corresponded PCS number */
    laneNum = (pcsNum % 4);

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_UNIT, (pcsNum & HWS_2_LSB_MASK_CNS), CG_CONTROL_2, &data, (7 << 2)));

    data = data >> 2;

    switch (laneNum)
    {
        case 0: /* 25G - lane 0, 50G - lane 0, 100G */
            switch (data & 5)
            {
                case 0:
                    *activeLanes = 1; /* 25G */
                    break;
                case 1:
                    *activeLanes = 2; /* 50G */
                    break;
                case 4:
                    *activeLanes = 4; /* 100G */
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case 1: /* 25G - lane 1 */
            if ((data & 1) == 0)
            {
                *activeLanes = 1; /* 25G */
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case 2: /* 25G - lane 2, 50G - lane 2 */
            switch (data & 2)
            {
                case 0:
                    *activeLanes = 1; /* 25G */
                    break;
                case 2:
                    *activeLanes = 2; /* 50G */
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case 3: /* 25G - lane 3 */
            if ((data & 2) == 0)
            {
                *activeLanes = 1; /* 25G */
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal mvHwsCgPcs28nmActiveStatusGet function
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
GT_STATUS mvHwsCgPcs28nmActiveStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    GT_UOPT                 *numOfLanes
)
{
    GT_U32 data;


    if (numOfLanes == NULL)
    {
      return GT_BAD_PARAM;
    }

    *numOfLanes = 0;

    if(pcsNum % 4 != 0)
    {
        return GT_OK;
    }

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_UNIT, pcsNum, CG_RESETS, &data, 0));
    /* check unreset bit status */
    if (!(data & (0x5 << 26)))
    {
        /* unit is under RESET */
        return GT_OK;
    }



    /* get number of active lanes */
     CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_UNIT, pcsNum , CG_CONTROL_2, &data, 0xFF << 13));
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
* @internal mvHwsCgPcs28nmSendFaultSet function
* @endinternal
*
* @brief   Configure the PCS to start or stop sending fault signals to partner.
*         on single lane, the result will be local-fault on the sender and remote-fault on the receiver,
*         on multi-lane there will be local-fault on both sides, and there won't be align lock
*         at either side.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - pcs number
* @param[in  portMode                 - port mode
* @param[in] send                     - start/ stop send faults
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgPcs28nmSendFaultSet
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               pcsNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              send
)
{
    MV_CFG_SEQ   *seq;

    /* to prevent errors, needed in cases of RS_FEC for MMPCS */
    portMode = portMode;

    if (send==GT_TRUE)
    {
        /*  start sending fault signals - multi lane - full lanes sequence */
        seq = &hwsCgPcs28nmSeqDb[CGPCS_START_SEND_FAULT_SEQ];
        CHECK_STATUS_EXT(mvCfgSeqExec(devNum, portGroup, pcsNum, seq->cfgSeq, seq->cfgSeqSize), LOG_ARG_PCS_IDX_MAC(pcsNum));
    }
    else
    {
        /*  stop sending fault signals - multi lane - full lanes sequence */
        seq = &hwsCgPcs28nmSeqDb[CGPCS_STOP_SEND_FAULT_SEQ];
        CHECK_STATUS_EXT(mvCfgSeqExec(devNum, portGroup, pcsNum, seq->cfgSeq, seq->cfgSeqSize), LOG_ARG_PCS_IDX_MAC(pcsNum));
    }

    return GT_OK;
}
