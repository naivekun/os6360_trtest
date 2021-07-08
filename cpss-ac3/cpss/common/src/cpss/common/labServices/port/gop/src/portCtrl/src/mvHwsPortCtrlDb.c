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
* @file mvHwsPortCtrlDb.c
*
* @brief Port Control Database
*
* @version   1
********************************************************************************
*/
#include <mvHwsPortCtrlInc.h>

/* Port Control Supervisor module (M1) table */
MV_HWS_PORT_CTRL_SUPERVISOR mvPortCtrlSpv;
#ifdef REG_PORT_TASK
/* Port Control Port management module (M2) table */
MV_HWS_PORT_CTRL_PORT_MNG mvPortCtrlPortMng;
/* Port Control Port State machine module (M3) table */
MV_HWS_PORT_CTRL_PORT_SM mvPortCtrlPortSm[MV_PORT_CTRL_MAX_PORT_NUM];
#endif
/* Port Control AP Port management module (M4) table */
MV_HWS_PORT_CTRL_AP_PORT_MNG mvHwsPortCtrlApPortMng;
/* Port Control AP Port State machine module (M5) table */
MV_HWS_PORT_CTRL_AP_DETECT mvHwsPortCtrlApPortDetect;
#ifdef AP_GENERAL_TASK
/* Port Control General module (M6) table */
MV_HWS_PORT_CTRL_GENERAL mvPortCtrlGen;
#endif

/**
* @internal mvPortCtrlDbInit function
* @endinternal
*
* @brief   Initialize Port control database
*         - Clear all tables
*         - Set default values
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlDbInit(void)
{
    GT_U32 state;
    GT_U32 portIndex;

    /* Clear tables */
    /* ============ */
    osMemSet(&mvPortCtrlSpv,             0, sizeof(MV_HWS_PORT_CTRL_SUPERVISOR));
    /* Code not relevant for BobK CM3 due to space limit */
#ifdef REG_PORT_TASK
    osMemSet(&mvPortCtrlPortMng,         0, sizeof(MV_HWS_PORT_CTRL_PORT_MNG));
    osMemSet(&mvPortCtrlPortSm[0],       0, sizeof(MV_HWS_PORT_CTRL_PORT_SM) * MV_PORT_CTRL_MAX_PORT_NUM);
#endif
    osMemSet(&mvHwsPortCtrlApPortMng,    0, sizeof(MV_HWS_PORT_CTRL_AP_PORT_MNG));
    osMemSet(&mvHwsPortCtrlApPortDetect, 0, sizeof(mvHwsPortCtrlApPortDetect));
#ifdef AP_GENERAL_TASK
    osMemSet(&mvPortCtrlGen,             0, sizeof(MV_HWS_PORT_CTRL_GENERAL));
#endif

    /* M1_SUPERVISOR */
    /* ============= */
    mvPortCtrlSpv.info.state         = SPV_DELAY_STATE;
    mvPortCtrlSpv.info.event         = SPV_HIGH_MSG_EVENT;
    mvPortCtrlSpv.info.delayDuration = MV_PROCESS_MSG_RX_DELAY;
    mvPortCtrlSpv.lowMsgThreshold    = MV_PORT_CTRL_SPV_MSG_EXEC_THRESHOLD;
    mvPortCtrlSpv.highMsgThreshold   = MV_PORT_CTRL_SPV_MSG_EXEC_THRESHOLD;

    /*
    ** State Machine Transitions Table
    ** +================+=============+=============+=============+
    ** + Event \ State  + High Msg O1 + Low Msg O2  + Delay O3    +
    ** +================+=============+=============+=============+
    ** + High Msg       +     O1      +   Invalid   +    ==> O1   +
    ** +================+=============+=============+=============+
    ** + Low Msg        +   ==> O2    +   02        +   Invalid   +
    ** +================+=============+=============+=============+
    ** + Delay          +   Invalid   +   ==> O3    +   Invalid   +
    ** +================+=============+=============+=============+
    */
    osMemSet(&mvPortCtrlSpv.funcTbl[0][0], 0,
             sizeof(PORT_CTRL_FUNCPTR) * (SPV_MAX_STATE * SPV_MAX_EVENT));

    state = SPV_HIGH_MSG_PROCESS_STATE;
    mvPortCtrlSpv.funcTbl[SPV_HIGH_MSG_EVENT][state] = (PORT_CTRL_FUNCPTR)mvPortCtrlSpvHighMsg;
    mvPortCtrlSpv.funcTbl[SPV_LOW_MSG_EVENT][state]  = (PORT_CTRL_FUNCPTR)mvPortCtrlSpvLowMsg;

    state = SPV_LOW_MSG_PROCESS_STATE;
    mvPortCtrlSpv.funcTbl[SPV_LOW_MSG_EVENT][state]  = (PORT_CTRL_FUNCPTR)mvPortCtrlSpvLowMsg;
    mvPortCtrlSpv.funcTbl[SPV_DELAY_EVENT][state]    = (PORT_CTRL_FUNCPTR)mvPortCtrlSpvDelay;

    state = SPV_DELAY_STATE;
    mvPortCtrlSpv.funcTbl[SPV_HIGH_MSG_EVENT][state] = (PORT_CTRL_FUNCPTR)mvPortCtrlSpvHighMsg;

#ifdef REG_PORT_TASK
    /* M2_PORT_MNG */
    /* =========== */
/* Code not relevant for BobK CM3 due to space limit */

    mvPortCtrlPortMng.info.state         = PORT_MNG_DELAY_STATE;
    mvPortCtrlPortMng.info.event         = PORT_MNG_MSG_PROCESS_EVENT;
    mvPortCtrlPortMng.info.delayDuration = MV_PROCESS_MSG_RX_DELAY;

    /*
    ** State Machine Transitions Table
    ** +================+=============+=============+
    ** + Event \ State  +   Msg O1    +   Delay O2  +
    ** +================+=============+=============+
    ** + Msg            +     O1      +    ==> O1   +
    ** +================+=============+=============+
    ** + Delay          +   ==> O2    +   Invalid   +
    ** +================+=============+=============+
    */
    state = PORT_MNG_MSG_PROCESS_STATE;
    mvPortCtrlPortMng.funcTbl[PORT_MNG_MSG_PROCESS_EVENT][state] = (PORT_CTRL_FUNCPTR)mvPortCtrlPortMsg;
    mvPortCtrlPortMng.funcTbl[PORT_MNG_DELAY_EVENT][state]       = (PORT_CTRL_FUNCPTR)mvPortCtrlPortDelay;

    state = PORT_MNG_DELAY_STATE;
    mvPortCtrlPortMng.funcTbl[PORT_MNG_MSG_PROCESS_EVENT][state] = (PORT_CTRL_FUNCPTR)mvPortCtrlPortMsg;
    mvPortCtrlPortMng.funcTbl[PORT_MNG_DELAY_EVENT][state]       = NULL;


    /* M3_PORT_SM */
    /* ========== */
    for (portIndex = 0; portIndex < MV_PORT_CTRL_MAX_PORT_NUM; portIndex++)
    {
        mvPortCtrlPortSm[portIndex].state = PORT_SM_IDLE_STATE;
    }
#endif

    /* M4_AP_PORT_MNG */
    /* ============== */
    mvHwsPortCtrlApPortMng.info.state         = AP_PORT_MNG_PORT_DELAY_STATE;
    mvHwsPortCtrlApPortMng.info.event         = AP_PORT_MNG_MSG_PROCESS_EVENT;
    mvHwsPortCtrlApPortMng.info.delayDuration = MV_PROCESS_MSG_RX_DELAY;

    for (portIndex = 0; portIndex < MV_PORT_CTRL_MAX_AP_PORT_NUM; portIndex++)
    {
        mvHwsPortCtrlApPortMng.tasks[portIndex].pendTasksCount     = 0;
        mvHwsPortCtrlApPortMng.tasks[portIndex].pendTasksAddIndex  = 0;
        mvHwsPortCtrlApPortMng.tasks[portIndex].pendTasksProcIndex = 0;

        mvHwsPortCtrlApPortMng.apPortTimer[portIndex].sysCfState        = PORT_SM_SERDES_SYSTEM_ENABLE;
        mvHwsPortCtrlApPortMng.apPortTimer[portIndex].sysCfStatus       = PORT_SM_SERDES_SYSTEM_NOT_VALID;
        mvHwsPortCtrlApPortMng.apPortTimer[portIndex].sysCfgThreshold   = PORT_CTRL_TIMER_DEFAULT;

        mvHwsPortCtrlApPortMng.apPortTimer[portIndex].trainingCount       = 0;
        mvHwsPortCtrlApPortMng.apPortTimer[portIndex].trainingMaxInterval = PORT_CTRL_TRAINING_INTERVAL;
        mvHwsPortCtrlApPortMng.apPortTimer[portIndex].trainingThreshold   = PORT_CTRL_TIMER_DEFAULT;
    }

    /*
    ** AP Port Mng State Machine Transitions Table
    ** +================+=============+=============+=============+
    ** + Event \ State  +  Active O1  +   Msg O2    +  Delay O3   +
    ** +================+=============+=============+=============+
    ** + Active         +     O1      +   ==> O1    +   Invalid   +
    ** +================+=============+=============+=============+
    ** + Msg            +   Invalid   +     O2      +   ==> O2    +
    ** +================+=============+=============+=============+
    ** + Delay          +   ==> O3    +   Invalid   +   Invalid   +
    ** +================+=============+=============+=============+
    */
    osMemSet(&mvHwsPortCtrlApPortMng.funcTbl[0][0], 0,
             sizeof(PORT_CTRL_FUNCPTR) * (AP_PORT_MNG_MAX_STATE * AP_PORT_MNG_MAX_EVENT));

    state = AP_PORT_MNG_ACTIVE_PORT_STATE;
    mvHwsPortCtrlApPortMng.funcTbl[AP_PORT_MNG_ACTIVE_PORT_EVENT][state] = (PORT_CTRL_FUNCPTR)mvPortCtrlApPortActive;
    mvHwsPortCtrlApPortMng.funcTbl[AP_PORT_MNG_DELAY_EVENT][state]       = (PORT_CTRL_FUNCPTR)mvPortCtrlApPortDelay;

    state = AP_PORT_MNG_MSG_PROCESS_STATE;
    mvHwsPortCtrlApPortMng.funcTbl[AP_PORT_MNG_MSG_PROCESS_EVENT][state] = (PORT_CTRL_FUNCPTR)mvPortCtrlApPortMsg;
    mvHwsPortCtrlApPortMng.funcTbl[AP_PORT_MNG_ACTIVE_PORT_EVENT][state] = (PORT_CTRL_FUNCPTR)mvPortCtrlApPortActive;

    state = AP_PORT_MNG_PORT_DELAY_STATE;
    mvHwsPortCtrlApPortMng.funcTbl[AP_PORT_MNG_MSG_PROCESS_EVENT][state] = (PORT_CTRL_FUNCPTR)mvPortCtrlApPortMsg;

    /* M5_AP_PORT_SM */
    /* ============= */
    mvHwsPortCtrlApPortDetect.info.state         = AP_PORT_DETECT_DELAY_STATE;
    mvHwsPortCtrlApPortDetect.info.event         = AP_PORT_DETECT_MSG_PROCESS_EVENT;
    mvHwsPortCtrlApPortDetect.info.delayDuration = MV_PROCESS_MSG_RX_DELAY;

    for (portIndex = 0; portIndex < MV_PORT_CTRL_MAX_AP_PORT_NUM; portIndex++)
    {
        mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info.state = AP_PORT_SM_IDLE_STATE;
    }

    mvHwsPortCtrlApPortDetect.introp.txDisDuration          = PORT_CTRL_AP_TX_DIS_DURATION;
    mvHwsPortCtrlApPortDetect.introp.abilityDuration        = PORT_CTRL_AP_ABILITY_DURATION;
    mvHwsPortCtrlApPortDetect.introp.abilityMaxInterval     = PORT_CTRL_AP_ABILITY_MAX_COUNT;
    mvHwsPortCtrlApPortDetect.introp.abilityFailMaxInterval = PORT_CTRL_AP_ABILITY_FAIL_MAX_COUNT;
    mvHwsPortCtrlApPortDetect.introp.apLinkDuration         = PORT_CTRL_AP_LINK_CHECK_DURATION;
    mvHwsPortCtrlApPortDetect.introp.apLinkMaxInterval      = PORT_CTRL_AP_LINK_CHECK_MAX_COUNT;
    mvHwsPortCtrlApPortDetect.introp.pdLinkDuration         = PORT_CTRL_PD_LINK_CHECK_DURATION;
    mvHwsPortCtrlApPortDetect.introp.pdLinkMaxInterval      = PORT_CTRL_PD_LINK_CHECK_MAX_COUNT;

    /*
    ** AP Detection State Machine Transitions Table
    ** +================+=============+=============+=============+
    ** + Event \ State  +  Active O1  +   Msg O2    +  Delay O3   +
    ** +================+=============+=============+=============+
    ** + Active         +    O1       +    ==> O1   +   Invalid   +
    ** +================+=============+=============+=============+
    ** + Msg            +    Invalid  +    O2       +   ==> O2    +
    ** +================+=============+=============+=============+
    ** + Delay          +    ==> O3   +    Invalid  +   Invalid   +
    ** +================+=============+=============+=============+
    */
    osMemSet(&mvHwsPortCtrlApPortDetect.funcTbl[0][0], 0,
             sizeof(PORT_CTRL_FUNCPTR) * (AP_PORT_DETECT_MAX_STATE * AP_PORT_DETECT_MAX_EVENT));

    state = AP_PORT_DETECT_ACTIVE_PORT_STATE;
    mvHwsPortCtrlApPortDetect.funcTbl[AP_PORT_DETECT_ACTIVE_PORT_EVENT][state] = (PORT_CTRL_FUNCPTR)mvPortCtrlApDetectActive;
    mvHwsPortCtrlApPortDetect.funcTbl[AP_PORT_DETECT_DELAY_EVENT][state]       = (PORT_CTRL_FUNCPTR)mvPortCtrlApDetectDelay;

    state = AP_PORT_DETECT_MSG_PROCESS_STATE;
    mvHwsPortCtrlApPortDetect.funcTbl[AP_PORT_DETECT_MSG_PROCESS_EVENT][state] = (PORT_CTRL_FUNCPTR)mvPortCtrlApDetectMsg;
    mvHwsPortCtrlApPortDetect.funcTbl[AP_PORT_DETECT_ACTIVE_PORT_EVENT][state] = (PORT_CTRL_FUNCPTR)mvPortCtrlApDetectActive;

    state = AP_PORT_DETECT_DELAY_STATE;
    mvHwsPortCtrlApPortDetect.funcTbl[AP_PORT_DETECT_MSG_PROCESS_EVENT][state] = (PORT_CTRL_FUNCPTR)mvPortCtrlApDetectMsg;
#ifdef AP_GENERAL_TASK
    /* M6_GENERAL */
    /* ========== */
    mvPortCtrlGen.info.state         = PORT_GEN_DELAY_STATE;
    mvPortCtrlGen.info.event         = PORT_GEN_MSG_EVENT;
    mvPortCtrlGen.info.delayDuration = MV_PROCESS_MSG_RX_DELAY;

    /*
    ** State Machine Transitions Table
    ** +================+=============+=============+
    ** + Event \ State  +   Msg O1    +   Delay O2  +
    ** +================+=============+=============+
    ** + Msg            +     O1      +    ==> O1   +
    ** +================+=============+=============+
    ** + Delay          +   ==> O2    +   Invalid   +
    ** +================+=============+=============+
    */
    state = PORT_GEN_MSG_PROCESS_STATE;
    mvPortCtrlGen.funcTbl[PORT_GEN_MSG_EVENT][state]   = (PORT_CTRL_FUNCPTR)mvPortCtrlGenMsgProcess;
    mvPortCtrlGen.funcTbl[PORT_GEN_DELAY_EVENT][state] = (PORT_CTRL_FUNCPTR)mvPortCtrlGenDelay;

    state = PORT_GEN_DELAY_STATE;
    mvPortCtrlGen.funcTbl[PORT_GEN_MSG_EVENT][state]   = (PORT_CTRL_FUNCPTR)mvPortCtrlGenMsgProcess;
    mvPortCtrlGen.funcTbl[PORT_GEN_DELAY_EVENT][state] = NULL;
#endif
    return GT_OK;
}

/**
* @internal mvPortCtrlDevInit function
* @endinternal
*
* @brief   Initialize Port control device
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlDevInit(void)
{
    return GT_OK;
}


