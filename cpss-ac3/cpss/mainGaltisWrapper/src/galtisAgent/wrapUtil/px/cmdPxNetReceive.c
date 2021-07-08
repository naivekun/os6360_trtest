/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cmdDxChNetReceive.c
*
* DESCRIPTION:
*       galtis agent interface for packet capture using networkIf -- PX
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*******************************************************************************/

#include <cmdShell/common/cmdCommon.h>
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <galtisAgent/wrapUtil/cmdCpssNetReceive.h>
#include <cpss/px/networkIf/cpssPxNetIfTypes.h>

/******************************** Locals *************************************/

/**
* @internal prvCpssGenNetIfRx2Tx function
* @endinternal
*
* @brief   This function returns packets from PP destined to the CPU port.
*         -- SDMA relate.
*
* @note   APPLICABLE DEVICES:      ALL PCI/SDMA PX Devices
*/
extern GT_STATUS prvCpssGenNetIfRx2Tx
(
    IN GT_U8  *buffList[],
    IN GT_U32 buffLenList[],
    IN GT_U32 numOfBufs
);

GT_STATUS cmdPxRxPktReceive
(
    IN GT_U8                                devNum,
    IN GT_U8                                queueIdx,
    IN GT_U32                               numOfBuff,
    IN GT_U8                               *packetBuffs[],
    IN GT_U32                               buffLen[]
)
{
    GT_STATUS rc;
    /* If Rx to Tx disabled the function prvCpssGenNetIfRx2Tx returns
    without sending the packet. */
    prvCpssGenNetIfRx2Tx(packetBuffs,buffLen,numOfBuff);


    if(GT_FALSE == cmdCpssRxPktIsCaptureReady())
    {
        return GT_OK;
    }

    rc = cmdCpssRxPktReceive(devNum,queueIdx,
                             NULL,NULL,NULL,
                             numOfBuff,packetBuffs,buffLen);

    return rc;
}


