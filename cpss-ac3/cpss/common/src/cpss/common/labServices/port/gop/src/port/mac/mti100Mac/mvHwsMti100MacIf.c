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
* mvHwsMti100MacIf.c
*
* DESCRIPTION: MTI100 1G-100G MAC
*
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mac/mti100Mac/mvHwsMti100MacIf.h>
#include <cpss/common/labServices/port/gop/common/os/mvSemaphore.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mac/mti100Mac/mvHwsMti100MacDb.h>

#if 0
#include <cpss/common/labServices/port/gop/port/silicon/falcon/mvHwsFalconPortIf.h>
#endif


static char* mvHwsMacTypeGet(void)
{
  return "MTI100_MAC";
}

/**
* @internal mvHwsMti100MacIfInit function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] funcPtrArray             - (pointer to) func ptr array
*/
GT_STATUS mvHwsMti100MacIfInit(MV_HWS_MAC_FUNC_PTRS *funcPtrArray)
{
    funcPtrArray[MTI_MAC_100].macRestFunc    = mvHwsMti100MacReset;
    funcPtrArray[MTI_MAC_100].macModeCfgFunc = mvHwsMti100MacModeCfg;
    funcPtrArray[MTI_MAC_100].macLinkGetFunc = mvHwsMti100MacLinkStatus;
    funcPtrArray[MTI_MAC_100].macLbCfgFunc   = NULL;
#ifndef CO_CPU_RUN
    funcPtrArray[MTI_MAC_100].macLbStatusGetFunc = NULL;
#endif
    funcPtrArray[MTI_MAC_100].macActiveStatusFunc = NULL;
    funcPtrArray[MTI_MAC_100].macTypeGetFunc = mvHwsMacTypeGet;
    funcPtrArray[MTI_MAC_100].macFcStateCfgFunc = NULL;
    funcPtrArray[MTI_MAC_100].macPortEnableFunc = mvHwsMti100MacPortEnable;

    return GT_OK;
}

/**
* @internal mvHwsMti100MacModeCfg function
* @endinternal
*
* @brief   Set the MTI100 MAC in GOP.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMti100MacModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_CFG_SEQ *seq;
    GT_STATUS  st;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (curPortParams.portStandard)
    {
        case _50GBase_KR:
        case _100GBase_KR2:
            seq = &hwsMti100MacSeqDb[MTI100_MAC_XLG_MODE_SEQ];
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

#if 0
    mvHwsCgMac28nmAccessLock(devNum, portMacNum);
#endif

    st = mvCfgSeqExec(devNum, portGroup, curPortParams.portMacNumber, seq->cfgSeq, seq->cfgSeqSize);

#if 0
    mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
#endif

    /* Fine tuning for TX_FIFO_THRESHOLD for 100G_R2
        Port rate [Gb/s]    TH[entries]
              25                10
              50                10
              100               11
              200               7
              400               7
    */
    if(curPortParams.portStandard == _100GBase_KR2)
    {
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, RAVEN_UNIT_MTI_MAC100, curPortParams.portMacNumber, MTI_MAC_TX_FIFO_SECTIONS, 0xB, 0xFFFF));
    }

    CHECK_STATUS_EXT(st, LOG_ARG_MAC_IDX_MAC(portMacNum), LOG_ARG_STRING_MAC("calling mvCfgSeqExec"));

    return GT_OK;
}

/**
* @internal mvHwsMti100MacReset function
* @endinternal
*
* @brief   Set MTI100 MAC RESET/UNRESET or FULL_RESET action.
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
GT_STATUS mvHwsMti100MacReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    MV_CFG_SEQ  *seq = NULL;
    GT_STATUS st;
    GT_U32  localMacIndex;
    GT_U32  unitAddr, unitIndex, unitNum, address;

    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_UNIT_MTI_EXT, portMacNum, &unitAddr, &unitIndex,&unitNum));
    if (unitAddr == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    address = MTIP_EXT_GLOBAL_RESET_CONTROL + unitAddr;
    localMacIndex = portMacNum % 8;

    if (FULL_RESET == action)
    {
        switch(portMode)
        {
            case _50GBase_KR:
            case _100GBase_KR2:
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, portGroup, address, (16 + localMacIndex), 1, 0));

                seq = &hwsMti100MacSeqDb[MTI100_MAC_XLG_POWER_DOWN_SEQ];
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "FULL_RESET not supported for portMode = %d \n", portMode);
        }
    }
    else if (RESET == action)
    {
        switch(portMode)
        {
            case _50GBase_KR:
            case _100GBase_KR2:
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, portGroup, address, (16 + localMacIndex), 1, 0));
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "RESET not supported for portMode = %d \n", portMode);
        }
    }
    else if (UNRESET == action)
    {
        switch(portMode)
        {
            case _50GBase_KR:
            case _100GBase_KR2:
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, portGroup, address, (16 + localMacIndex), 1, 1));

                seq = &hwsMti100MacSeqDb[MTI100_MAC_XLG_UNRESET_SEQ];
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "UNRESET not supported for portMode = %d \n", portMode);
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    if(seq != NULL)
    {
        st = mvCfgSeqExec(devNum, portGroup, portMacNum, seq->cfgSeq, seq->cfgSeqSize);
        CHECK_STATUS_EXT(st, LOG_ARG_MAC_IDX_MAC(portMacNum), LOG_ARG_STRING_MAC("calling mvCfgSeqExec"))
    }

    return GT_OK;
}

/**
* @internal mvHwsMti100MacLinkStatus function
* @endinternal
*
* @brief   Get MTI100 MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMti100MacLinkStatus
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL     *linkStatus
)
{
    GT_U32  data;
    GT_U32  unitAddr, unitIndex, unitNum, address;
    portMode = portMode;

    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_UNIT_MTI_EXT, macNum, &unitAddr, &unitIndex,&unitNum));

    if (unitAddr == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    address = MTIP_EXT_PORT_STATUS + (macNum %8) * 0x18 + unitAddr;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, address, &data, 1));

    *linkStatus = (data == 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal mvHwsMti100MacPortEnable function
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
GT_STATUS mvHwsMti100MacPortEnable
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 enable
)
{
    GT_U32 data = (enable==GT_TRUE) ? 3 : 0;
    portMode = portMode;

    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, RAVEN_UNIT_MTI_MAC100, macNum, MTI_MAC_COMMAND_CONFIG, data, 3));

    return GT_OK;
}


