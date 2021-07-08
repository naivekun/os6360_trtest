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
* @file mvHwsCgMacRev3If.h
*
* @brief CG MAC interface
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsCgMacRev3If_H
#define __mvHwsCgMacRev3If_H

#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>

/**
* @internal hwsCgMacRev3IfInit function
* @endinternal
*
* @brief   Init XLG MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsCgMacRev3IfInit(MV_HWS_MAC_FUNC_PTRS *funcPtrArray);

/**
* @internal mvHwsCgMacRev3ClearChannelCfg function
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
);

/**
* @internal hwsCgMacRev3IfClose function
* @endinternal
*
* @brief   Release all system resources allocated by MAC IF functions.
*/
void hwsCgMacRev3IfClose(void);

#endif /* __mvHwsCgMacIf_H */


