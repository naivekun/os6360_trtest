/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*  mvHwsRavenPortModeElements.c
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>


/************************* definition *****************************************************/


/************************* Globals ********************************************************/
/* Last line of each SupModes array MUST be NON_SUP_MODE */
static MV_HWS_PORT_INIT_PARAMS hwsRavenPort0SupModes[] = {
    /* port mode,   MAC Type    Mac Num    PCS Type    PCS Num    FEC       Speed      FirstLane LanesList  LanesNum   Media         10Bit      fecSupported */
    { SGMII,         MAC_NA,    NA_NUM,     PCS_NA,    NA_NUM,   FEC_OFF,   SPEED_NA,       0,    {0,0,0,0},   0,      XAUI_MEDIA,   _10BIT_NA, FEC_NA},
    { NON_SUP_MODE,  MAC_NA,    NA_NUM,     PCS_NA,    NA_NUM,   FEC_OFF,   SPEED_NA,  NA_NUM,    {0,0,0,0},   0,      XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

/* map the right supported modes table for each port */
MV_HWS_PORT_INIT_PARAMS *hwsPortsRavenParamsSupModesMap[] =
{
    hwsRavenPort0SupModes,hwsRavenPort0SupModes,hwsRavenPort0SupModes,hwsRavenPort0SupModes,
    hwsRavenPort0SupModes,hwsRavenPort0SupModes,hwsRavenPort0SupModes,hwsRavenPort0SupModes,
    hwsRavenPort0SupModes,hwsRavenPort0SupModes,hwsRavenPort0SupModes,hwsRavenPort0SupModes,
    hwsRavenPort0SupModes,hwsRavenPort0SupModes,hwsRavenPort0SupModes,hwsRavenPort0SupModes
};

/************************* functions ******************************************************/

