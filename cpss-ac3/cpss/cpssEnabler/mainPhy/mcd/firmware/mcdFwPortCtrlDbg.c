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
* mcdFwPortCtrlLog.c
*
* DESCRIPTION:
*       Port Control Debug
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#include <mcdFwPortCtrlInc.h>

/* Default values for Register dump filering */
GT_U32 apRegDumpActive   = PORT_CTRL_DBG_REG_DUMP_DISABLE;
GT_U32 apRegDumpEnhanced = PORT_CTRL_DBG_REG_INT_NONE;

#ifdef DISABLE_DBG
/* Default values for AP Device and Port Group */
static GT_U8  apDevNum    = 0;
static GT_U32 apPortGroup = 0;
#endif

extern void mvApResetStats(GT_U8 portIndex);

GT_U32 apDumpParam = LOG_ALL_PORT_DUMP;

/**
* @internal mvPortCtrlDebugParamSet function
* @endinternal
*
* @brief   Debug parameter selection function
*/
void mvPortCtrlDebugParamSet(GT_U32 param)
{
    apDumpParam = param;
#ifdef FREE_RTOS_HWS_ENHANCED_PRINT_MODE
    mvPortCtrlDbgUnlock();
#endif /* FREE_RTOS_HWS_ENHANCED_PRINT_MODE */
}

#ifdef FREE_RTOS_HWS_ENHANCED_PRINT_MODE
/**
* @internal mvPortCtrlDebugRoutine function
* @endinternal
*
* @brief   Debug process execution sequence
*/
void mvPortCtrlDebugRoutine(void* pvParameters)
{
    for( ;; )
    {
        if (mvPortCtrlDbgLock() == GT_OK)
        {
            mvPortCtrlLogDump(apDumpParam);
        }
    }
}
#endif /* FREE_RTOS_HWS_ENHANCED_PRINT_MODE */

/**
* @internal mvPortCtrlDbgCfgRegsDump function
* @endinternal
*
* @brief   Config AP register dump functionality
*/
void mvPortCtrlDbgCfgRegsDump(GT_U32 active, GT_U32 mode)
{
    apRegDumpActive   = active;
    apRegDumpEnhanced = mode;
}

/**
* @internal mvPortCtrlDbgOnDemandRegsDump function
* @endinternal
*
* @brief   Print AP control and status registers
*/
void mvPortCtrlDbgOnDemandRegsDump(GT_U8 portIndex, GT_U8 pcsNum, GT_U8 state)
{
#ifdef DISABLE_DBG
    GT_U32 stReg0;
    GT_U32 stReg1;
    GT_U32 apLpReg1;
    GT_U32 apLpReg2;
    GT_U32 apLpReg3;
    GT_U32 apAdvReg1;
    GT_U32 apAdvReg2;
    GT_U32 apAdvReg3;
    GT_U32 apSt;
    GT_U32 apBpEthSt;
    MCD_AP_SM_INFO  *apSm;

    apSm = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);

    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_REG_ST_0,  &stReg0,  0);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_REG_ST_1,  &stReg1,  0);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_LP_BASE_ABILITY_REG_1, &apLpReg1, 0);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_LP_BASE_ABILITY_REG_2, &apLpReg2, 0);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_LP_BASE_ABILITY_REG_3, &apLpReg3, 0);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_AP_ADV_REG_1, &apAdvReg1, 0);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_AP_ADV_REG_2, &apAdvReg2, 0);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_AP_ADV_REG_3, &apAdvReg3, 0);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_AP_ST, &apSt, 0);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_BP_ETH_ST_REG, &apBpEthSt, 0);

    mvPortCtrlLogAdd(REG_LOG(state, apSm->portNum, PORT_CTRL_AP_REG_ST_0, stReg0));
    mvPortCtrlLogAdd(REG_LOG(state, apSm->portNum, PORT_CTRL_AP_REG_ST_1, stReg1));
    mvPortCtrlLogAdd(REG_LOG(state, apSm->portNum, PORT_CTRL_AP_INT_REG_802_3_LP_BASE_ABILITY_REG_1, apLpReg1));
    mvPortCtrlLogAdd(REG_LOG(state, apSm->portNum, PORT_CTRL_AP_INT_REG_802_3_LP_BASE_ABILITY_REG_2, apLpReg2));
    mvPortCtrlLogAdd(REG_LOG(state, apSm->portNum, PORT_CTRL_AP_INT_REG_802_3_LP_BASE_ABILITY_REG_3, apLpReg3));
    mvPortCtrlLogAdd(REG_LOG(state, apSm->portNum, PORT_CTRL_AP_INT_REG_802_3_AP_ADV_REG_1, apAdvReg1));
    mvPortCtrlLogAdd(REG_LOG(state, apSm->portNum, PORT_CTRL_AP_INT_REG_802_3_AP_ADV_REG_2, apAdvReg2));
    mvPortCtrlLogAdd(REG_LOG(state, apSm->portNum, PORT_CTRL_AP_INT_REG_802_3_AP_ADV_REG_3, apAdvReg3));
    mvPortCtrlLogAdd(REG_LOG(state, apSm->portNum, PORT_CTRL_AP_INT_REG_802_3_AP_ST, apSt));
    mvPortCtrlLogAdd(REG_LOG(state, apSm->portNum, PORT_CTRL_AP_INT_REG_802_3_BP_ETH_ST_REG, apBpEthSt));
#endif
}

/**
* @internal mvPortCtrlDbgCtrlRegsDump function
* @endinternal
*
* @brief   Print AP control and status registers
*/
void mvPortCtrlDbgCtrlRegsDump(GT_U8 portIndex, GT_U8 pcsNum, GT_U8 state)
{
#ifdef DISABLE_DBG
    GT_U32 cfgReg0;
    GT_U32 cfgReg1;
    GT_U32 stReg0;

    if (apRegDumpActive != PORT_CTRL_DBG_REG_DUMP_DISABLE)
    {
        genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_REG_CFG_0, &cfgReg0, 0);
        genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_REG_CFG_1, &cfgReg1, 0);
        genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_REG_ST_0,  &stReg0,  0);

        mvPortCtrlLogAdd(REG_LOG(state, portIndex, PORT_CTRL_AP_REG_CFG_0, cfgReg0));
        mvPortCtrlLogAdd(REG_LOG(state, portIndex, PORT_CTRL_AP_REG_CFG_1, cfgReg1));
        mvPortCtrlLogAdd(REG_LOG(state, portIndex, PORT_CTRL_AP_REG_ST_0, stReg0));
    }
#endif
}

/**
* @internal mvPortCtrlDbgIntRegsDump function
* @endinternal
*
* @brief   Print AP internal registers
*/
void mvPortCtrlDbgIntRegsDump(GT_U8 port, GT_U8 pcsNum, GT_U8 state)
{
#ifdef DISABLE_DBG
    GT_U32 apCtrl;
    GT_U32 apSt;
    GT_U32 apAdvReg1;
    GT_U32 apAdvReg2;
    GT_U32 apAdvReg3;
    GT_U32 apLpReg1;
    GT_U32 apLpReg2;
    GT_U32 apLpReg3;
    GT_U32 apAnegCtrl0;
    GT_U32 apAnegCtrl1;

    if ((apRegDumpActive   != PORT_CTRL_DBG_REG_DUMP_DISABLE) &&
        (apRegDumpEnhanced  & PORT_CTRL_DBG_REG_INT_REDUCE))
    {
        genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_AP_CTRL, &apCtrl, 0);
        genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_AP_ST, &apSt, 0);
        genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_AP_ADV_REG_1, &apAdvReg1, 0);
        genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_AP_ADV_REG_2, &apAdvReg2, 0);
        genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_AP_ADV_REG_3, &apAdvReg3, 0);
        genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_LP_BASE_ABILITY_REG_1, &apLpReg1, 0);
        genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_LP_BASE_ABILITY_REG_2, &apLpReg2, 0);
        genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_LP_BASE_ABILITY_REG_3, &apLpReg3, 0);
        genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_ANEG_CTRL_0, &apAnegCtrl0, 0);
        genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_ANEG_CTRL_1, &apAnegCtrl1, 0);

        mvPortCtrlLogAdd(REG_LOG(state, port, PORT_CTRL_AP_INT_REG_802_3_AP_CTRL, apCtrl));
        mvPortCtrlLogAdd(REG_LOG(state, port, PORT_CTRL_AP_INT_REG_802_3_AP_ST, apSt));
        mvPortCtrlLogAdd(REG_LOG(state, port, PORT_CTRL_AP_INT_REG_802_3_AP_ADV_REG_1, apAdvReg1));
        mvPortCtrlLogAdd(REG_LOG(state, port, PORT_CTRL_AP_INT_REG_802_3_AP_ADV_REG_2, apAdvReg2));
        mvPortCtrlLogAdd(REG_LOG(state, port, PORT_CTRL_AP_INT_REG_802_3_AP_ADV_REG_3, apAdvReg3));
        mvPortCtrlLogAdd(REG_LOG(state, port, PORT_CTRL_AP_INT_REG_802_3_LP_BASE_ABILITY_REG_1, apLpReg1));
        mvPortCtrlLogAdd(REG_LOG(state, port, PORT_CTRL_AP_INT_REG_802_3_LP_BASE_ABILITY_REG_2, apLpReg2));
        mvPortCtrlLogAdd(REG_LOG(state, port, PORT_CTRL_AP_INT_REG_802_3_LP_BASE_ABILITY_REG_3, apLpReg3));
        mvPortCtrlLogAdd(REG_LOG(state, port, PORT_CTRL_AP_INT_REG_ANEG_CTRL_0, apAnegCtrl0));
        mvPortCtrlLogAdd(REG_LOG(state, port, PORT_CTRL_AP_INT_REG_ANEG_CTRL_1, apAnegCtrl1));
    }
#endif
}

/**
* @internal mvPortCtrlDbgAllIntRegsDump function
* @endinternal
*
* @brief   Print All AP internal registers
*/
void mvPortCtrlDbgAllIntRegsDump(GT_U8 port, GT_U8 pcsNum, GT_U8 state)
{
#ifdef DISABLE_DBG
    GT_U32 apNextPageTx;
    GT_U32 apNextPageCode_0_15;
    GT_U32 apNextPageCode_16_31;
    GT_U32 apLpNextPageAbilityTx;
    GT_U32 apLpNextPageAbilityCode_0_15;
    GT_U32 apLpNextPageAbilityCode_16_31;
    GT_U32 apBpEthSt;
    GT_U32 apLpAdv;

    if ((apRegDumpActive   != PORT_CTRL_DBG_REG_DUMP_DISABLE) &&
        (apRegDumpEnhanced  & PORT_CTRL_DBG_REG_INT_FULL))
    {
        genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_NEXT_PG_TX_REG, &apNextPageTx, 0);
        genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15, &apNextPageCode_0_15, 0);
        genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31, &apNextPageCode_16_31, 0);
        genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_REG, &apLpNextPageAbilityTx, 0);
        genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_CODE_0_15, &apLpNextPageAbilityCode_0_15, 0);
        genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_CODE_16_31, &apLpNextPageAbilityCode_16_31, 0);
        genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_BP_ETH_ST_REG, &apBpEthSt, 0);
        genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_LP_ADV_REG, &apLpAdv, 0);

        mvPortCtrlLogAdd(REG_LOG(state, port, PORT_CTRL_AP_INT_REG_802_3_NEXT_PG_TX_REG, apNextPageTx));
        mvPortCtrlLogAdd(REG_LOG(state, port, PORT_CTRL_AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15, apNextPageCode_0_15));
        mvPortCtrlLogAdd(REG_LOG(state, port, PORT_CTRL_AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31, apNextPageCode_16_31));
        mvPortCtrlLogAdd(REG_LOG(state, port, PORT_CTRL_AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_REG, apLpNextPageAbilityTx));
        mvPortCtrlLogAdd(REG_LOG(state, port, PORT_CTRL_AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_CODE_0_15, apLpNextPageAbilityCode_0_15));
        mvPortCtrlLogAdd(REG_LOG(state, port, PORT_CTRL_AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_CODE_16_31, apLpNextPageAbilityCode_16_31));
        mvPortCtrlLogAdd(REG_LOG(state, port, PORT_CTRL_AP_INT_REG_802_3_BP_ETH_ST_REG, apBpEthSt));
        mvPortCtrlLogAdd(REG_LOG(state, port, PORT_CTRL_AP_INT_REG_802_3_LP_ADV_REG, apLpAdv));
    }
#endif
}

/**
* @internal mvPortCtrlDbgAllRegsDump function
* @endinternal
*
* @brief   Print All AP internal registers
*/
void mvPortCtrlDbgAllRegsDump(GT_U8 port, GT_U8 pcsNum, GT_U8 state)
{
#ifdef DISABLE_DBG
    mvPortCtrlDbgCtrlRegsDump(portIndex, pcsNum, AP_PORT_SM_INIT_STATE);
    mvPortCtrlDbgIntRegsDump(portIndex, pcsNum, AP_PORT_SM_INIT_STATE);
    mvPortCtrlDbgAllIntRegsDump(portIndex, pcsNum, AP_PORT_SM_INIT_STATE);
#endif
}

/**
* @internal mvPortCtrlDbgLinkRegsDump function
* @endinternal
*
* @brief   Print SD, CDR, and PCS Link status registers
*/
void mvPortCtrlDbgLinkRegsDump(GT_U8 port, GT_U8 laneNum, GT_U8 state)
{
#ifdef DISABLE_DBG
    GT_U32 data;
    GT_U32 sd  = 0;
    GT_U32 cdr = 0;

    if (apRegDumpActive != PORT_CTRL_DBG_REG_DUMP_DISABLE)
    {
        /* CDR lock dectection enable */
        genUnitRegisterSet(apDevNum, apPortGroup, SERDES_PHY_UNIT, laneNum, 0x8C, 0x100, 0x100);
        genUnitRegisterGet(apDevNum, apPortGroup, SERDES_PHY_UNIT, laneNum, 0x8C, &data, 0);
        if (data & 0x80)
        {
            cdr = 1;
        }

        genUnitRegisterGet(apDevNum, apPortGroup, SERDES_UNIT, laneNum, 0x18, &data, 0);
        if (!(data & 0x2))
        {
            sd = 1;
        }

        mvPcPrintf("Port %d State O%d: Signal Detect %s, CDR Lock %s\n",
                   port, state,
                   (sd == 1) ? ("On") : ("Off"),
                   (cdr == 1) ? ("On") : ("Off"));
    }
#endif
}

#ifndef DISABLE_CLI
/**
* @internal mvPortCtrlDbgStatsDump function
* @endinternal
*
* @brief   Print AP protocol statistics counter and timers
*/
void mvPortCtrlDbgStatsDump(GT_U8 port, GT_U8 peer)
{
#ifdef MV_PORT_CTRL_DEBUG
    GT_U8 portIndex = 0xff;
    GT_U8 peerPortIndex = 0xff;

    MCD_AP_SM_INFO  *apSm;
    MCD_AP_SM_STATS *apStats;
    MCD_AP_SM_INFO  *peerApSm;
    MCD_AP_SM_STATS *peerApStats;
    GT_U8 portNumBase = 0;

#ifdef BC2_DEV_SUPPORT
    if(BobcatA0 == hwsDeviceSpecInfo[0].devType)
    {
        portNumBase = MV_PORT_CTRL_BC2_AP_PORT_NUM_BASE;
    }
    else if(Alleycat3A0 == hwsDeviceSpecInfo[0].devType)
    {
        portNumBase = MV_PORT_CTRL_AC3_AP_PORT_NUM_BASE;
    }
    else
    {
        portNumBase = 0;
    }
#else
    portNumBase = MV_PORT_CTRL_AP_PORT_NUM_BASE;
#endif

    if ((port != 0xff) && (port < portNumBase + MV_PORT_CTRL_MAX_AP_PORT_NUM))
    {
        portIndex = MV_HWS_PORT_CTRL_AP_LOGICAL_PORT_GET(port, portNumBase);
        if (portIndex >= MV_PORT_CTRL_MAX_AP_PORT_NUM) {
            printf("Invalid port\n");
            return;
        }
        apSm      = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
        apStats   = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);
    }

    if ((peer != 0xff) && (peer < portNumBase + MV_PORT_CTRL_MAX_AP_PORT_NUM))
    {
        peerPortIndex = MV_HWS_PORT_CTRL_AP_LOGICAL_PORT_GET(peer, portNumBase);
        if (peerPortIndex >= MV_PORT_CTRL_MAX_AP_PORT_NUM) {
            printf("Invalid port\n");
            return;
        }
        peerApSm      = &(mvHwsPortCtrlApPortDetect.apPortSm[peerPortIndex].info);
        peerApStats   = &(mvHwsPortCtrlApPortDetect.apPortSm[peerPortIndex].stats);
    }

    if ((peerPortIndex != 0xff) && (portIndex != 0xff))
    {
        mvPcPrintf("Port              %8d  %8d\n"
                   "Tx Disable cnt    %08d  %08d\n"
                   "Ability check cnt %08d  %08d\n"
                   "Ability succ cnt  %08d  %08d\n"
                   "Link Fail cnt     %08d  %08d\n"
                   "Link succ cnt     %08d  %08d\n",
                   apSm->portNum,              peerApSm->portNum,
                   apStats->txDisCnt,          peerApStats->txDisCnt,
                   apStats->abilityCnt,        peerApStats->abilityCnt,
                   apStats->abilitySuccessCnt, peerApStats->abilitySuccessCnt,
                   apStats->linkFailCnt,       peerApStats->linkFailCnt,
                   apStats->linkSuccessCnt,    peerApStats->linkSuccessCnt);

        mvPcPrintf("HCD Time          %08d  %08d\n"
                   "link Up Time      %08d  %08d\n"
                   "Total Exec Time   %08d  %08d\n",
                   apStats->hcdResoultionTime,                       peerApStats->hcdResoultionTime,
                   apStats->linkUpTime,                              peerApStats->linkUpTime,
                   apStats->hcdResoultionTime + apStats->linkUpTime, peerApStats->hcdResoultionTime + peerApStats->linkUpTime);
    }
    else if (portIndex != 0xff)
    {
        mvPcPrintf("Port              %8d\n"
                   "Tx Disable cnt    %08d\n"
                   "Ability check cnt %08d\n"
                   "Ability succ cnt  %08d\n"
                   "Link Fail cnt     %08d\n"
                   "Link succ cnt     %08d\n",
                   apSm->portNum,
                   apStats->txDisCnt,
                   apStats->abilityCnt,
                   apStats->abilitySuccessCnt,
                   apStats->linkFailCnt,
                   apStats->linkSuccessCnt);

        mvPcPrintf("HCD Time          %08d\n"
                   "link Up Time      %08d\n"
                   "Total Exec Time   %08d\n",
                   apStats->hcdResoultionTime,
                   apStats->linkUpTime,
                   apStats->hcdResoultionTime + apStats->linkUpTime);
    }
    if (portIndex != 0xff)
    {
        mvPcPrintf("portEnableTime    %08d\n"
                   "hcdClbkStartTime  %08d\n"
                   "hcdClbkStopTime   %08d\n"
                   "trxClbkStartTime  %08d\n"
                   "trxClbkStopTime   %08d\n"
                   "linkClbkStartTime %08d\n"
                   "linkClbkStopTime  %08d\n",
                   apStats->portEnableTime,
                   apStats->hcdClbkStartTime, apStats->hcdClbkStopTime,
                   apStats->trxClbkStartTime, apStats->trxClbkStopTime,
                   apStats->linkClbkStartTime, apStats->linkClbkStopTime);
    }
#endif /* MV_PORT_CTRL_DEBUG */
}
#endif /* DISABLE_CLI */

/**
* @internal mvPortCtrlDbgReg function
* @endinternal
*
* @brief   Configure AP register
*/
void mvPortCtrlDbgReg(GT_U8 reg, GT_U8 value)
{
    mvPcPrintf("AP Debug: Reg 0x%x, Value 0x%x\n", reg, value);
}

/**
* @internal mvPortCtrlDbgStatsReset function
* @endinternal
*
* @brief   Reset AP protocol statistics counter
*/
void mvPortCtrlDbgStatsReset(GT_U8 port)
{
      GT_U8 portIndex;
      GT_U8 portNumBase = 0;

#ifdef BC2_DEV_SUPPORT
    if(BobcatA0 == hwsDeviceSpecInfo[0].devType)
    {
        portNumBase = MV_PORT_CTRL_BC2_AP_PORT_NUM_BASE;
    }
    else if(Alleycat3A0 == hwsDeviceSpecInfo[0].devType)
    {
        portNumBase = MV_PORT_CTRL_AC3_AP_PORT_NUM_BASE;
    }
    else
    {
        portNumBase = 0;
    }
#else
      portNumBase = MV_PORT_CTRL_AP_PORT_NUM_BASE;
#endif

    portIndex = MV_HWS_PORT_CTRL_AP_LOGICAL_PORT_GET(port, portNumBase);

    mvApResetStats(portIndex);
}

#ifndef DISABLE_CLI
/**
* @internal mvPortCtrlDbgIntropDump function
* @endinternal
*
* @brief   Print AP protocol interop parameters
*/
void mvPortCtrlDbgIntropDump(void)
{
#ifdef MV_PORT_CTRL_DEBUG
    MCD_AP_DETECT_ITEROP *apIntrop = &(mvHwsPortCtrlApPortDetect.introp);

    mvPcPrintf("Tx Disable duration        %08d msec\n"
               "Ability duration           %08d msec\n"
               "Ability max interval       %08d intervals\n"
               "Ability fail max interval  %08d intervals\n"
               "AP Link Duration           %08d msec\n"
               "AP Link Max interval check %08d intervals\n"
               "PD Link Duration           %08d msec\n"
               "PD Link Max interval check %08d intervals\n",
               apIntrop->txDisDuration,
               apIntrop->abilityDuration,
               apIntrop->abilityMaxInterval,
               apIntrop->abilityFailMaxInterval,
               apIntrop->apLinkDuration,
               apIntrop->apLinkMaxInterval,
               apIntrop->pdLinkDuration,
               apIntrop->pdLinkMaxInterval);
#endif
}
#endif /* DISABLE_CLI */

/**
* @internal mvPortCtrlDbgIntropCfgParam function
* @endinternal
*
* @brief   Config AP protocol interop parameters
*/
void mvPortCtrlDbgIntropCfgParam(GT_U8 param, GT_U8 value)
{
    MCD_AP_DETECT_ITEROP *apIntrop = &(mvHwsPortCtrlApPortDetect.introp);
    GT_U8 paramIndex = 1 << param;

    if (paramIndex == AP_PORT_INTROP_TX_DIS)
    {
        apIntrop->txDisDuration = value;
    }
    else if (paramIndex ==  AP_PORT_INTROP_ABILITY_DUR)
    {
        apIntrop->abilityDuration = value;
    }
    else if (paramIndex == AP_PORT_INTROP_ABILITY_MAX_INT)
    {
        apIntrop->abilityMaxInterval = value;
    }
    else if (paramIndex == AP_PORT_INTROP_ABILITY_MAX_FAIL_INT)
    {
        apIntrop->abilityFailMaxInterval = value;
    }
    else if (paramIndex == AP_PORT_INTROP_AP_LINK_DUR)
    {
        apIntrop->apLinkDuration = value;
    }
    else if (paramIndex == AP_PORT_INTROP_AP_LINK_MAX_INT)
    {
        apIntrop->apLinkMaxInterval = value;
    }
    else if (paramIndex == PD_PORT_INTROP_AP_LINK_DUR)
    {
        apIntrop->pdLinkDuration = value;
    }
    else if (paramIndex == PD_PORT_INTROP_AP_LINK_MAX_INT)
    {
        apIntrop->pdLinkMaxInterval = value;
    }
}

/**
* @internal mvPortCtrlDbgIntropCfg function
* @endinternal
*
* @brief   Config AP protocol interop parameters
*/
void mvPortCtrlDbgIntropCfg(MCD_IPC_PORT_AP_INTROP_STRUCT *apIntropParams)
{
    MCD_AP_DETECT_ITEROP *apIntrop = &(mvHwsPortCtrlApPortDetect.introp);

    if (apIntropParams->attrBitMask & AP_PORT_INTROP_TX_DIS)
    {
        apIntrop->txDisDuration = apIntropParams->txDisDuration;
    }
    if (apIntropParams->attrBitMask & AP_PORT_INTROP_ABILITY_DUR)
    {
        apIntrop->abilityDuration = apIntropParams->abilityDuration;
    }
    if (apIntropParams->attrBitMask & AP_PORT_INTROP_ABILITY_MAX_INT)
    {
        apIntrop->abilityMaxInterval = apIntropParams->abilityMaxInterval;
    }
    if (apIntropParams->attrBitMask & AP_PORT_INTROP_ABILITY_MAX_FAIL_INT)
    {
        apIntrop->abilityFailMaxInterval = apIntropParams->abilityFailMaxInterval;
    }
    if (apIntropParams->attrBitMask & AP_PORT_INTROP_AP_LINK_DUR)
    {
        apIntrop->apLinkDuration = apIntropParams->apLinkDuration;
    }
    if (apIntropParams->attrBitMask & AP_PORT_INTROP_AP_LINK_MAX_INT)
    {
        apIntrop->apLinkMaxInterval = apIntropParams->apLinkMaxInterval;
    }
    if (apIntropParams->attrBitMask & PD_PORT_INTROP_AP_LINK_DUR)
    {
        apIntrop->pdLinkDuration = apIntropParams->pdLinkDuration;
    }
    if (apIntropParams->attrBitMask & PD_PORT_INTROP_AP_LINK_MAX_INT)
    {
        apIntrop->pdLinkMaxInterval = apIntropParams->pdLinkMaxInterval;
    }
#ifndef DISABLE_CLI
    /* Print AP protocol interop parameters */
    mvPortCtrlDbgIntropDump();
#endif /* DISABLE_CLI */
}



