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
* mvHwsMti400MacIf.c
*
* DESCRIPTION: MTI 400G MAC
*
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mac/mti400Mac/mvHwsMti400MacIf.h>
#include <cpss/common/labServices/port/gop/common/os/mvSemaphore.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mac/mti400Mac/mvHwsMti400MacDb.h>

#if 0
#include <cpss/common/labServices/port/gop/port/silicon/falcon/mvHwsFalconPortIf.h>
#endif


static char* mvHwsMacTypeGet(void)
{
  return "MTI400_MAC";
}

/**
* @internal mvHwsMti400MacIfInit function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] funcPtrArray             - (pointer to) func ptr array
*/
GT_STATUS mvHwsMti400MacIfInit(MV_HWS_MAC_FUNC_PTRS *funcPtrArray)
{
    funcPtrArray[MTI_MAC_400].macRestFunc    = mvHwsMti400MacReset;
    funcPtrArray[MTI_MAC_400].macModeCfgFunc = mvHwsMti400MacModeCfg;
    funcPtrArray[MTI_MAC_400].macLinkGetFunc = mvHwsMti400MacLinkStatus;
    funcPtrArray[MTI_MAC_400].macLbCfgFunc   = NULL;
#ifndef CO_CPU_RUN
    funcPtrArray[MTI_MAC_400].macLbStatusGetFunc = NULL;
#endif
    funcPtrArray[MTI_MAC_400].macActiveStatusFunc = NULL;
    funcPtrArray[MTI_MAC_400].macTypeGetFunc = mvHwsMacTypeGet;
    funcPtrArray[MTI_MAC_400].macFcStateCfgFunc = NULL;
    funcPtrArray[MTI_MAC_400].macPortEnableFunc = mvHwsMti400MacPortEnable;

    return GT_OK;
}

/**
* @internal mvHwsMti400MacModeCfg function
* @endinternal
*
* @brief   Set the MTI400 MAC in GOP.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMti400MacModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS *curPortParams = hwsPortModeParamsGet(devNum, portGroup, portMacNum, portMode);
    MV_CFG_SEQ *seq;
    GT_STATUS  st;

    if(curPortParams == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (curPortParams->portStandard)
    {
        case _200GBase_KR4:
        case _200GBase_KR8:
        case _400GBase_KR8:
            seq = &hwsMti400MacSeqDb[MTI400_MAC_SEGMENTED_4_LANES_MODE_SEQ];
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    st = mvCfgSeqExec(devNum, portGroup, curPortParams->portMacNumber, seq->cfgSeq, seq->cfgSeqSize);
    CHECK_STATUS_EXT(st, LOG_ARG_MAC_IDX_MAC(portMacNum), LOG_ARG_STRING_MAC("calling mvCfgSeqExec"));

    return GT_OK;
}

/**
* @internal mvHwsMti400MacReset function
* @endinternal
*
* @brief   Set MTI400 MAC RESET/UNRESET or FULL_RESET action.
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
GT_STATUS mvHwsMti400MacReset
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
    localMacIndex = localMacIndex / 4;  /* bit_24 for 200G_KR4 on port 0, bit_25 for 200G_KR4 on port 4 */

    if (FULL_RESET == action)
    {
        switch(portMode)
        {
            case _200GBase_KR4:
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, portGroup, address, (24+localMacIndex), 1, 0));
                seq = &hwsMti400MacSeqDb[MTI400_MAC_SEGMENTED_4_LANES_POWER_DOWN_SEQ];
                break;

            case _400GBase_KR8:
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, portGroup, address, 24, 1, 0));
                seq = &hwsMti400MacSeqDb[MTI400_MAC_SEGMENTED_8_LANES_POWER_DOWN_SEQ];
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "FULL_RESET not supported for portMode = %d \n", portMode);
        }
    }
    else if (RESET == action)
    {
        switch(portMode)
        {
            case _200GBase_KR4:
            case _400GBase_KR8:
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, portGroup, address, (24 + localMacIndex), 1, 0));
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "RESET not supported for portMode = %d \n", portMode);
        }
    }
    else if (UNRESET == action)
    {
        switch(portMode)
        {
            case _200GBase_KR4:
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, portGroup, address, (24 + localMacIndex), 1, 1));
                seq = &hwsMti400MacSeqDb[MTI400_MAC_SEGMENTED_4_LANES_UNRESET_SEQ];
                break;

            case _400GBase_KR8:
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, portGroup, address, 24, 1, 1));
                seq = &hwsMti400MacSeqDb[MTI400_MAC_SEGMENTED_8_LANES_UNRESET_SEQ];
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
* @internal mvHwsMti400MacLinkStatus function
* @endinternal
*
* @brief   Get CG MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMti400MacLinkStatus
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

    address = MTIP_EXT_SEG_PORT_STATUS + ((macNum %8) / 4) * 0x14 + unitAddr;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, address, &data, 1));

    *linkStatus = (data == 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal mvHwsMti400MacPortEnable function
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
GT_STATUS mvHwsMti400MacPortEnable
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

    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, RAVEN_UNIT_MTI_MAC400, macNum, MTI_MAC_COMMAND_CONFIG, data, 3));

    return GT_OK;
}


