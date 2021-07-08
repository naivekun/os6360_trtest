/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* mvHwsRavenPortIf.h
*
* DESCRIPTION:
*        Raven port interface header file
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
******************************************************************************/

#ifndef __mvHwsRavenPortIf_H
#define __mvHwsRavenPortIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>


/* number of ports in Raven */
#define HWS_RAVEN_PORTS_NUM_CNS 17

/**
* @internal hwsRavenPortsElementsCfg function
* @endinternal
*
* @brief    Init Raven port modes DB
*
* @param[in] devNum                   - system device number
*/
GT_STATUS hwsRavenPortsElementsCfg
(
    GT_U8              devNum
);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsRavenPortIf_H */


