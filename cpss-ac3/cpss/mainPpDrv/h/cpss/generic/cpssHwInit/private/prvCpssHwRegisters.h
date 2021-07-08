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
* @file prvCpssHwRegisters.h
*
* @brief Private API definition for register access of the generic device.
*
* Those APIs gives next benefit :
* for multi-port group device
* - write of 'global config' into special units may need to we written only in
* some of the port groups (not all)
* - write of 'per port' may need to we written to other port group then
* specified according to 'dest port'
*
* still SOME functionality MUST NOT use those APIs , like:
* - interrupts handling --> needed per specific port group
* - prvCpssPortGroupsCounterSummary(...) , prvCpssPortGroupBusyWait(...)
* and maybe others...
* - maybe others
*
*
* $Revision: 1 $
*
* COMMENT:
* Intermediate file - includes path to real H in 'common' directory.
*
*/
#include <cpss/common/cpssHwInit/private/prvCpssHwRegisters.h>


