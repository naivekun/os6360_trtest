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
* mvHwsXlgMacDb.c
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 13 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mac/hglMac/mvHwsHglMacDb.h>

static MV_CFG_ELEMENT modeSeqParams[] = {
    {WRITE_OP, {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3, (7 << 13), (7 << 13)}},
    {WRITE_OP, {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4, (1 << 8),  (1 << 8)}},
    {WRITE_OP, {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0,  0x0,      (1 << 11)}}
};

/* seqId to params array mapping */
MV_CFG_SEQ hwsHglMacSeqDb[] =
{
    {modeSeqParams, MV_SEQ_SIZE(modeSeqParams)} /* HGLMAC_MODE_SEQ */
};

GT_STATUS hwsHglMacSeqGet(MV_HWS_HGL_MAC_SUB_SEQ seqType, MV_CFG_ELEMENT *seqLine, GT_U32 lineNum)
{
    if ((seqType >= MV_MAC_HGL_LAST_SEQ) ||
        (hwsHglMacSeqDb[seqType].cfgSeq == NULL))
    {
        return GT_NO_SUCH;
    }

    *seqLine = hwsHglMacSeqDb[seqType].cfgSeq[lineNum];
    return GT_OK;
}
