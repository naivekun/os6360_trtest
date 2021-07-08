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
* mvHwsXglMacRev3If.c
*
* DESCRIPTION:
*       XGL MAC revision 3 interface (for puma3B0)
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMacDbRev3.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMacRev3If.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMacIf.h>

#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>

static char* mvHwsMacTypeGet(void)
{
  return "XLGMAC_Rev3";
}

/**
* @internal hwsXlgMacRev3IfInit function
* @endinternal
*
* @brief   Init XLG MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsXlgMacRev3IfInit(MV_HWS_MAC_FUNC_PTRS *funcPtrArray)
{
    funcPtrArray[XLGMAC].macRestFunc = mvHwsXlgMacReset;
    funcPtrArray[XLGMAC].macLinkGetFunc = mvHwsXlgMacLinkStatus;
    funcPtrArray[XLGMAC].macLbCfgFunc = mvHwsXlgMacLoopbackCfg;
    funcPtrArray[XLGMAC].macTypeGetFunc = mvHwsMacTypeGet;
#ifndef CO_CPU_RUN
    funcPtrArray[XLGMAC].macLbStatusGetFunc = mvHwsXlgMacLoopbackStatusGet;
    funcPtrArray[XLGMAC].macActiveStatusFunc = mvHwsXlgMacActiveStatusGet;
#endif

    /* Rev3 specific function */
    funcPtrArray[XLGMAC].macModeCfgFunc = mvHwsXlgMacRev3ModeCfg;
    funcPtrArray[XLGMAC].macClearChannelCfgFunc = mvHwsXlgMacRev3ClearChannelCfg;
    funcPtrArray[XLGMAC].macFcStateCfgFunc = NULL;

    return GT_OK;
}


/**
* @internal mvHwsXlgMacRev3ModeCfg function
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
GT_STATUS mvHwsXlgMacRev3ModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_CFG_SEQ *seq;
    GT_U32 accessAddr;
    GT_U32 baseAddr;
    GT_U32 unitIndex, i;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CHECK_STATUS(mvUnitInfoGet(devNum, XLGMAC_UNIT, &baseAddr, &unitIndex));

    switch(curPortParams.numOfActLanes)
    {
    case 1:
      seq = &hwsXlgMacSeqDbRev3[XLGMAC_MODE_1_Lane_SEQ];
      break;
    case 2:
      seq = &hwsXlgMacSeqDbRev3[XLGMAC_MODE_2_Lane_SEQ];
      break;
    case 4:
      seq = &hwsXlgMacSeqDbRev3[XLGMAC_MODE_4_Lane_SEQ];
      break;
    default:
        return GT_BAD_PARAM;
        break;
    }

    CHECK_STATUS(mvCfgSeqExec(devNum, portGroup, curPortParams.portMacNumber, seq->cfgSeq, seq->cfgSeqSize));

    /* keep in reset all unused MACs */
    for (i = 1; (i < curPortParams.numOfActLanes); i++)
    {
        if ((curPortParams.portMacNumber+i) < 12)
        {
            /* 0x010C0000 + 0x1000*p: where p (0-11) represents network port */
            accessAddr = baseAddr + MSM_PORT_MAC_CONTROL_REGISTER0 + (curPortParams.portMacNumber+i) * unitIndex;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, 0, (1 << 1)));
        }
    }
    return GT_OK;
}

/**
* @internal mvHwsXlgMacRev3ClearChannelCfg function
* @endinternal
*
* @brief   Configures MAC advanced feature accordingly.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] txIpg                    - IPG between 2 packets in LAN mode
* @param[in] txPreamble               - length, in bytes, of preamble transmitted by this port.
* @param[in] rxPreamble               - length, in bytes, of preamble supported for received packets.
* @param[in] txCrc                    - TX CRC
* @param[in] rxCrc                    - RX CRC
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXlgMacRev3ClearChannelCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  txIpg,
    GT_U32                  txPreamble,
    GT_U32                  rxPreamble,
    MV_HWS_PORT_CRC_MODE    txCrc,
    MV_HWS_PORT_CRC_MODE    rxCrc
)
{
    GT_U32 accessAddr;
    GT_U32 baseAddr;
    GT_U32 unitIndex;
    GT_U32 data, txCrcVal, rxCrcVal;

    portMode = portMode;
    CHECK_STATUS(mvUnitInfoGet(devNum, XLGMAC_UNIT, &baseAddr, &unitIndex));

    switch (txCrc)
    {
    case HWS_1Byte_CRC:
        txCrcVal = 1;
        break;
    case HWS_2Bytes_CRC:
        txCrcVal = 2;
        break;
    case HWS_3Bytes_CRC:
        txCrcVal = 3;
        break;
    case HWS_4Bytes_CRC:
        txCrcVal = 4;
        break;
    default:
        return GT_BAD_PARAM;
    }
    switch (rxCrc)
    {
    case HWS_1Byte_CRC:
        rxCrcVal = 1;
        break;
    case HWS_2Bytes_CRC:
        rxCrcVal = 2;
        break;
    case HWS_3Bytes_CRC:
        rxCrcVal = 3;
        break;
    case HWS_4Bytes_CRC:
        rxCrcVal = 4;
        break;
    default:
        return GT_BAD_PARAM;
    }

    data = (rxCrcVal << 13) + (txCrcVal << 10) + ((rxPreamble & 7) << 7) + ((txPreamble & 7) << 4) + (txIpg & 0xF);
    accessAddr  = baseAddr + MSM_PORT_MAC_CONTROL_REGISTER5 + macNum * unitIndex;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, data, 0));

    return GT_OK;
}
