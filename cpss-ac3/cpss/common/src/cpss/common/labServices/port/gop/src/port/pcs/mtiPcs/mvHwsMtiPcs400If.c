/******************************************************************************
*              Copyright (c) Marvell International Ltd. and its affiliates
*
* This software file (the "File") is owned and distributed by Marvell
* International Ltd. and/or its affiliates ("Marvell") under the following
* alternative licensing terms.
* If you received this File from Marvell, you may opt to use, redistribute
* and/or modify this File under the following licensing terms.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*  -   Redistributions of source code must retain the above copyright notice,
*       this list of conditions and the following disclaimer.
*  -   Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*  -    Neither the name of Marvell nor the names of its contributors may be
*       used to endorse or promote products derived from this software without
*       specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************
*/
/**
********************************************************************************
* @file mvHwsMtiPcs400If.c
*
* @brief MTI PCS400 interface API
*
* @version   1
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcsDb.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs400If.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>

static char* mvHwsMtiPcs400TypeGetFunc(void)
{
  return "MTI_PCS400";
}

/**
* @internal mvHwsMtiPcs400Reset function
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
GT_STATUS mvHwsMtiPcs400Reset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    MV_HWS_PORT_INIT_PARAMS *curPortParams = hwsPortModeParamsGet(devNum, portGroup, portMacNum, portMode);
    MV_CFG_SEQ              *seq;
    GT_STATUS               st;
    MV_HWS_MTI_PCS_SUB_SEQ  subSeq;
    GT_U32                  pcsNum;

    if(curPortParams == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsNum = curPortParams->portPcsNumber;

    if (FULL_RESET == action)
    {
        subSeq = MTI_PCS_POWER_DOWN_SEQ;
    }
    else if (RESET == action)
    {
        subSeq = MTI_PCS_RESET_SEQ;
    }
    else if (UNRESET == action)
    {
        subSeq = MTI_PCS_UNRESET_SEQ;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }
    seq = &hwsMtiPcs400SeqDb[subSeq];
    st = mvCfgSeqExec(devNum, portGroup, pcsNum, seq->cfgSeq, seq->cfgSeqSize);
    CHECK_STATUS_EXT(st, LOG_ARG_MAC_IDX_MAC(portMacNum), LOG_ARG_STRING_MAC("calling mvCfgSeqExec"))

    return GT_OK;
}


/**
* @internal mvHwsMtiPcs400Mode function
* @endinternal
*
* @brief   Set the MtiPcs400 mode
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiPcs400Mode
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_CFG_SEQ              *seq;
    GT_STATUS               st;
    MV_HWS_MTI_PCS_SUB_SEQ  subSeq;
    MV_HWS_PORT_INIT_PARAMS *curPortParams = hwsPortModeParamsGet(devNum, portGroup, portMacNum, portMode);

    if(curPortParams == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (portMode)
    {
        case _200GBase_KR4:
            subSeq = MTI_PCS_200R4_MODE_SEQ;
            break;
        case _400GBase_KR8:
            subSeq = MTI_PCS_400R8_MODE_SEQ;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    seq = &hwsMtiPcs400SeqDb[subSeq];

    st = mvCfgSeqExec(devNum, portGroup, curPortParams->portPcsNumber, seq->cfgSeq, seq->cfgSeqSize);
    CHECK_STATUS_EXT(st, LOG_ARG_MAC_IDX_MAC(portMacNum), LOG_ARG_STRING_MAC("calling mvCfgSeqExec"))

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs400LoopBack function
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
GT_STATUS mvHwsMtiPcs400LoopBack
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
)
{
    MV_HWS_PORT_INIT_PARAMS *curPortParams = hwsPortModeParamsGet(devNum, portGroup, phyPortNum, portMode);
    MV_CFG_SEQ *seq;
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr;

    if(curPortParams == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /*
        In MAC/PCS loopback there is no link indication from MTI to MPF.
        Without link up indication the MPF drain all the packets that received from Eagle.
        As a result, we will need to force those links up indication in MPF interface
    */
    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_UNIT_MPFS,( phyPortNum & 0xfffffff8), &baseAddr, &unitIndex, &unitNum));
    if (phyPortNum%8 == 0) { /*in seg mode port 0 is taken from mpf 8 */
        baseAddr += 8*HWS_MPF_CHANNEL_OFFSET;
    }
    else /*in seg mode port 4 is taken from mpf 9 */
    {
        baseAddr += 9*HWS_MPF_CHANNEL_OFFSET;
    }
    regAddr = baseAddr + MPFS_SAU_CONTROL;

    switch (lbType)
    {
        case DISABLE_LB:
            seq = &hwsMtiPcs400SeqDb[MTI_PCS_LPBK_NORMAL_SEQ];
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 30, 2, 0));
            break;
        case TX_2_RX_LB:
            seq = &hwsMtiPcs400SeqDb[MTI_PCS_LPBK_TX2RX_SEQ];
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 30, 2, 0x3));
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams->portPcsNumber, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs400LoopBackGet function
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
GT_STATUS mvHwsMtiPcs400LoopBackGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
)
{
    MV_HWS_PORT_INIT_PARAMS *curPortParams = hwsPortModeParamsGet(devNum, portGroup, phyPortNum, portMode);
    GT_UREG_DATA    data;

    if(curPortParams == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, RAVEN_UNIT_MTI_PCS400, curPortParams->portPcsNumber, MTI_PCS_CONTROL1, &data, (0x1 << 14)));

    switch(data>>14)
    {
        case 0:
            *lbType = DISABLE_LB;
            break;
        case 1:
            *lbType = TX_2_RX_LB;
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs400AlignLockGet function
* @endinternal
*
* @brief   Read align lock status of given MtiPcs100.
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
GT_STATUS mvHwsMtiPcs400AlignLockGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 pcsNum,
    GT_BOOL                 *lock
)
{
    GT_UREG_DATA    data;
    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, RAVEN_UNIT_MTI_EXT, pcsNum, MTIP_EXT_SEG_PORT_STATUS, &data, 0x2));

    *lock = (data != 0);

    return GT_OK;
}
/**
* @internal mvHwsMtiPcs400SendFaultSet function
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
GT_STATUS mvHwsMtiPcs400SendFaultSet
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               pcsNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              send
)
{
    MV_CFG_SEQ   *seq;

    portMode = portMode;
    if (send==GT_TRUE)
    {
        /*  start sending fault signals  */
        seq = &hwsMtiPcs400SeqDb[MTI_PCS_START_SEND_FAULT_SEQ];
        CHECK_STATUS_EXT(mvCfgSeqExec(devNum, portGroup, pcsNum, seq->cfgSeq, seq->cfgSeqSize), LOG_ARG_PCS_IDX_MAC(pcsNum));
    }
    else
    {
        /*  stop sending fault signals */
        seq = &hwsMtiPcs400SeqDb[MTI_PCS_STOP_SEND_FAULT_SEQ];
        CHECK_STATUS_EXT(mvCfgSeqExec(devNum, portGroup, pcsNum, seq->cfgSeq, seq->cfgSeqSize), LOG_ARG_PCS_IDX_MAC(pcsNum));
    }

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs400FecConfigGet function
* @endinternal
*
* @brief   Return the FEC mode  status on PCS.
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
GT_STATUS mvHwsMtiPcs400FecConfigGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_FEC_MODE    *portFecTypePtr
)
{
    return  mvHwsMtipExtFecTypeGet(devNum, portGroup, phyPortNum, portMode, portFecTypePtr);
}

/**
* @internal mvHwsMtiPcs400IfInit function
* @endinternal
*
* @brief   Init MtiPcs400 configuration sequences and IF
*          functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiPcs400IfInit(MV_HWS_PCS_FUNC_PTRS *funcPtrArray)
{
    funcPtrArray[MTI_PCS_400].pcsResetFunc     = mvHwsMtiPcs400Reset;
    funcPtrArray[MTI_PCS_400].pcsModeCfgFunc   = mvHwsMtiPcs400Mode;
    funcPtrArray[MTI_PCS_400].pcsLbCfgFunc     = mvHwsMtiPcs400LoopBack;
    funcPtrArray[MTI_PCS_400].pcsLbCfgGetFunc  = mvHwsMtiPcs400LoopBackGet;
    funcPtrArray[MTI_PCS_400].pcsTypeGetFunc   = mvHwsMtiPcs400TypeGetFunc;
    funcPtrArray[MTI_PCS_400].pcsFecCfgFunc    = NULL;
    funcPtrArray[MTI_PCS_400].pcsFecCfgGetFunc = mvHwsMtiPcs400FecConfigGet;
    funcPtrArray[MTI_PCS_400].pcsCheckGearBoxFunc = NULL;
    funcPtrArray[MTI_PCS_400].pcsAlignLockGetFunc = mvHwsMtiPcs400AlignLockGet;
    funcPtrArray[MTI_PCS_400].pcsActiveStatusGetFunc = NULL;
    funcPtrArray[MTI_PCS_400].pcsSendFaultSetFunc = mvHwsMtiPcs400SendFaultSet;

    return GT_OK;
}

