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
* @file mvHwsCgMacRev3If.c
*
* @brief CG MAC Puma3 B0 interface
*
* @version   1
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

#include <cpss/common/labServices/port/gop/port/mac/cgMac/mvHwsCgMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/cgMac/mvHwsCgMacRev3If.h>
#include <cpss/common/labServices/port/gop/port/mac/cgMac/mvHwsCgMacDb.h>

static char* mvHwsMacTypeGet(void)
{
  return "CGMAC_Rev3";
}

/**
* @internal hwsCgMacRev3IfInit function
* @endinternal
*
* @brief   Init XLG MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsCgMacRev3IfInit(MV_HWS_MAC_FUNC_PTRS *funcPtrArray)
{
    funcPtrArray[CGMAC].macRestFunc = mvHwsCgMacReset;
    funcPtrArray[CGMAC].macModeCfgFunc = mvHwsCgMacModeCfg;
    funcPtrArray[CGMAC].macLinkGetFunc = mvHwsCgMacLinkStatus;
    funcPtrArray[CGMAC].macLbCfgFunc = mvHwsCgMacLoopbackCfg;
#ifndef CO_CPU_RUN
    funcPtrArray[CGMAC].macLbStatusGetFunc = mvHwsCgMacLoopbackStatusGet;
#endif
    funcPtrArray[CGMAC].macTypeGetFunc = mvHwsMacTypeGet;

    /* new functions */
    funcPtrArray[CGMAC].macClearChannelCfgFunc = mvHwsCgMacRev3ClearChannelCfg;
    funcPtrArray[CGMAC].macFcStateCfgFunc = NULL;

    return GT_OK;
}

/**
* @internal mvHwsCgMacRev3ClearChannelCfg function
* @endinternal
*
* @brief   Configures MAC advanced feature accordingly.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] txIpg                    - Number of octets in steps of 4. Valid values are 8, 12, 16, ..., 100.
* @param[in] txPreamble               - length, in bytes, of preamble transmitted by this port.
* @param[in] rxPreamble               - length, in bytes, of preamble supported for received packets.
* @param[in] txCrc                    - TX CRC
* @param[in] rxCrc                    - RX CRC
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCgMacRev3ClearChannelCfg
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
    GT_U32 data;

    portMode = portMode;
    CHECK_STATUS(mvUnitInfoGet(devNum, CG_UNIT, &baseAddr, &unitIndex));

    txCrc = txCrc;
    rxCrc = rxCrc;
    /* config TX IPG */
    accessAddr  = baseAddr + CG_TX_IPG_LENGTH + macNum * unitIndex;
    data = txIpg/4;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, data, 0));

    /* config preamble */
    if ((txPreamble == rxPreamble) && (txPreamble == 4))
    {
        data = (1 << 30) + (1 << 21);
        accessAddr  = baseAddr + CG_COMMAND_CONFIG + macNum * unitIndex;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, data, data));
    }
    return GT_OK;
}
