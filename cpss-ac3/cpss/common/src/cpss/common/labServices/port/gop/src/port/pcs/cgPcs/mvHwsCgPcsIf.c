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
* mvHwsCgPcsIf.h
*
* DESCRIPTION:
*       CGPCS interface API
*
* FILE REVISION NUMBER:
*       $Revision: 9 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/cgPcs/mvHwsCgPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/cgPcs/mvHwsCgPcsDb.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>

static char* mvHwsPcsTypeGetFunc(void)
{
  return "CGPCS";
}

/**
* @internal mvHwsCgPcsIfInit function
* @endinternal
*
* @brief   Init GPCS configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgPcsIfInit(MV_HWS_PCS_FUNC_PTRS *funcPtrArray)
{
    funcPtrArray[CGPCS].pcsResetFunc = NULL; /* all functionality moved to CG MAC */
    funcPtrArray[CGPCS].pcsModeCfgFunc = mvHwsCgPcsMode;
    funcPtrArray[CGPCS].pcsLbCfgFunc = mvHwsCgPcsLoopBack;
    funcPtrArray[CGPCS].pcsLbCfgGetFunc = mvHwsCgPcsLoopBackGet;
    funcPtrArray[CGPCS].pcsTypeGetFunc = mvHwsPcsTypeGetFunc;
    funcPtrArray[CGPCS].pcsSignalDetectMaskEn = mvHwsCgPcsSignalDetectMaskSet;
    funcPtrArray[CGPCS].pcsFecCfgFunc       = (MV_HWS_PCS_FEC_CFG_FUNC_PTR)     mvHwsCgPcsFecConfig;
    funcPtrArray[CGPCS].pcsFecCfgGetFunc    = (MV_HWS_PCS_FEC_CFG_GET_FUNC_PTR) mvHwsCgPcsFecConfigGet;
    funcPtrArray[CGPCS].pcsCheckGearBoxFunc = mvHwsCgPcsCheckGearBox;

    return GT_OK;
}

/**
* @internal mvHwsCgPcsMode function
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
GT_STATUS mvHwsCgPcsMode
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32                  numOfLanes;
    MV_CFG_SEQ *seq;
    GT_U32 data, mask;
    GT_U32 accessAddr;
    GT_U32 baseAddr;
    GT_U32 unitIndex;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    numOfLanes = (curPortParams.serdesMediaType == RXAUI_MEDIA) ? (2 * curPortParams.numOfActLanes) : (curPortParams.numOfActLanes);

    /* EXTERNAL_CONTROL + 0x1000 * 16;  access XLG MAC 16 */
    mvUnitInfoGet(devNum, XLGMAC_UNIT, &baseAddr, &unitIndex);
    if (curPortParams.portPcsNumber == 0)
    {
        /* network port */
        accessAddr = baseAddr + EXTERNAL_CONTROL;
    }
    else
    {
        /* fabric ports */
        accessAddr = baseAddr + unitIndex * 16 + EXTERNAL_CONTROL;
    }
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, 1, 3));

    seq = &hwsCgPcsPscSeqDb[CGPCS_MODE_MISC_SEQ];
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portPcsNumber, seq->cfgSeq, seq->cfgSeqSize));

    /* FEC config */
    data = 0;
    mask = (7 << 23);
    if (curPortParams.portFecMode == FC_FEC)
    {
        data = (7 << 23);
    }
    /* write data to 0x1800000 [25:23]*/
    mvUnitInfoGet(devNum, CG_UNIT, &baseAddr, &unitIndex);
    accessAddr  = baseAddr + curPortParams.portPcsNumber * unitIndex + CG_CONTROL_0;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, data, mask));

    switch(numOfLanes)
    {
    case 10:
      seq = &hwsCgPcsPscSeqDb[CGPCS_MODE_10_LANE_SEQ];
      break;
    case 12:
      seq = &hwsCgPcsPscSeqDb[CGPCS_MODE_12_LANE_SEQ];
      break;
    default:
      return GT_NOT_SUPPORTED;
      break;
    }
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portPcsNumber, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}


/**
* @internal mvHwsCgPcsLoopBack function
* @endinternal
*
* @brief   Set PCS loop back.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgPcsLoopBack
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
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
      seq = &hwsCgPcsPscSeqDb[CGPCS_LPBK_NORMAL_SEQ];
      break;
    case TX_2_RX_LB:
      seq = &hwsCgPcsPscSeqDb[CGPCS_LPBK_TX2RX_SEQ];
      break;
    default:
      return GT_NOT_SUPPORTED;
      break;
    }
    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portPcsNumber, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsCgPcsLoopBackGet function
* @endinternal
*
* @brief   Get the PCS loop back mode state.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgPcsLoopBackGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams ;
    GT_UREG_DATA    data;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CGPCS_UNIT, curPortParams.portPcsNumber, CGPCS_CONTROL_1, &data, (0x1 << 14)));

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
* @internal mvHwsCgPcsSignalDetectMaskSet function
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
GT_STATUS mvHwsCgPcsSignalDetectMaskSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    GT_BOOL                 maskEn
)
{
    GT_U32 regAddr, data;
    GT_U32 unitAddr;
    GT_U32 unitIndex;

    mvUnitInfoGet(devNum, CG_UNIT, &unitAddr, &unitIndex);

    regAddr = CG_CONTROL_0 + unitAddr + pcsNum*unitIndex;
    data = (maskEn == GT_TRUE) ? (1 << 21) : 0;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, data, (1 << 21)));

    return GT_OK;
}

/**
* @internal mvHwsCgPcsFecConfig function
* @endinternal
*
* @brief   Configure FEC disable/enable on PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] fecEn                    - if true, enable FEC
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgPcsFecConfig
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  pcsNum,
    GT_BOOL                 fecEn
)
{
    GT_U32 regAddr, data;
    GT_U32 unitAddr;
    GT_U32 unitIndex;

    mvUnitInfoGet(devNum, CG_UNIT, &unitAddr, &unitIndex);

    regAddr = CG_CONTROL_0 + unitAddr + pcsNum*unitIndex;
    data = (fecEn == GT_TRUE) ? (3 << 23) : 0;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, data, (3 << 23)));

    return GT_OK;
}

/**
* @internal mvHwsCgPcsFecConfigGet function
* @endinternal
*
* @brief   Return FEC disable/enable on PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @param[out] fecEn                    - if true, enable FEC
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgPcsFecConfigGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *fecEn
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 regAddr, data;
    GT_U32 unitAddr;
    GT_U32 unitIndex;
    GT_U32 pcsNum;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsNum = curPortParams.portPcsNumber;

    mvUnitInfoGet(devNum, CG_UNIT, &unitAddr, &unitIndex);

    regAddr = CG_CONTROL_0 + unitAddr + pcsNum*unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0));

    *fecEn = ((data >> 23) & 3) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}


/**
* @internal mvHwsCgPcsCheckGearBox function
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
GT_STATUS mvHwsCgPcsCheckGearBox
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *laneLock
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 regAddr;
    GT_U32 data;
    GT_U32 mask;
    GT_U32 unitAddr;
    GT_U32 unitIndex;
    GT_U32 pcsNum, numOfLanes;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsNum = curPortParams.portPcsNumber;
    numOfLanes = curPortParams.numOfActLanes;

    mvUnitInfoGet(devNum, CG_UNIT, &unitAddr, &unitIndex);

    /* Check Gear Box Status */
    regAddr = unitAddr + pcsNum*unitIndex + CG_MULTI_LANE_ALIGN_STATUS1;
    hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0);
    /* gear box status can show lock in not used bits, so read only
       required bits */
    if (numOfLanes < 9)
    {
        mask = ((1 << numOfLanes) - 1)<<1;
        *laneLock = (data == mask) ? GT_TRUE : GT_FALSE;
    }
    else
    {
        if (data != 0xFF)
        {
            *laneLock = GT_FALSE;
        }
        else
        {
            regAddr = unitAddr + pcsNum*unitIndex + CG_MULTI_LANE_ALIGN_STATUS2;
            hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, 0);
            mask = ((1 << (numOfLanes-8)) - 1)<<1;
            *laneLock = (data == mask) ? GT_TRUE : GT_FALSE;
        }
    }

    return GT_OK;
}

