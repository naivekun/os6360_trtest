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
* @file prvCpssTmShaping.c
*
* @brief Private utilities for TM Shaping Configuration verification
*
* @version   1
********************************************************************************
*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/extServices/os/gtOs/gtGenTypes.h>
/*
#include <cpss/generic/tm/cpssTmServices.h>
#include <cpss/generic/tm/cpssTmCtl.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/tm/cpssTmPublicDefs.h>
#include <tm_ctl.h>
#include <tm_nodes_read.h>
#include <tm_drop.h>
#include <platform/tm_regs.h>
#include <platform/cpss_tm_rw_registers_proc.h>
*/
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpss/generic/cpssHwInit/private/prvCpssHwRegisters.h>
#include <cpss/generic/tm/cpssTmShaping.h>
#include <cpssCommon/cpssPresteraDefs.h>


/* private functions , for usage in command string */
/**
* @internal prvCpssTmShapingProfileValidate function
* @endinternal
*
* @brief   Validates shaping profile can be configured and prints updated bursts values if possible
*/

GT_STATUS prvCpssTmShapingProfileValidate
(
    IN GT_U8                    devNum,
    IN GT_U32                   level,
    IN GT_U32                   cirBw,
    IN GT_U32                   eirBw,
    IN GT_U32                   cbs,
    IN GT_U32                   ebs
)
{
    GT_STATUS rc;
	CPSS_TM_SHAPING_PROFILE_PARAMS_STC      profileStr;
	profileStr.cirBw = cirBw;
	profileStr.cbs = cbs;
	profileStr.eirBw = eirBw;
	profileStr.ebs = ebs;
    rc = cpssTmShapingProfileValidate(devNum,level,&profileStr);
	cpssOsPrintf("result : %d\n", rc);
	if (GT_BAD_SIZE == rc)
	{
		cpssOsPrintf("cbs/ebs  are too small for  desired B/W\n");
		if (profileStr.cirBw) cpssOsPrintf("   cbs should be : %d\n",profileStr.cbs);
		if (profileStr.eirBw) cpssOsPrintf("   ebs should be : %d\n",profileStr.ebs);
	}
    return rc;
}

/**
* @internal prvCpssTmPortShapingValidate function
* @endinternal
*
* @brief   Validates shaping profile can be configured and prints updated bursts values if possible
*/

GT_STATUS prvCpssTmPortShapingValidate
(
    IN GT_U8                    devNum,
    IN GT_U32                   cirBw,
    IN GT_U32                   eirBw,
    IN GT_U32                   cbs,
    IN GT_U32                   ebs
)
{
    GT_STATUS rc;
    rc = cpssTmPortShapingValidate(devNum,cirBw, eirBw, &cbs , &ebs);
	cpssOsPrintf("result : %d\n", rc);
	if (GT_BAD_SIZE == rc)
	{
		cpssOsPrintf("cbs/ebs  are too small for  desired B/W\n");
		if (cirBw) cpssOsPrintf("   cbs should be : %d\n",cbs);
		if (eirBw) cpssOsPrintf("   ebs should be : %d\n",ebs);
	}
     return rc;
}

