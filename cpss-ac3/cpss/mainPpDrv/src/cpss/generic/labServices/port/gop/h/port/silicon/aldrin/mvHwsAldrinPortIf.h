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
*/
/**
********************************************************************************
* @file mvHwsAldrinPortIf.h
*
* @brief Aldrin port interface header file
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsAldrinPortIf_H
#define __mvHwsAldrinPortIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>


/**
* @internal hwsAldrinPortsElementsCfg function
* @endinternal
*
* @brief   Init Aldrin port modes
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS hwsAldrinPortsElementsCfg
(
    GT_U8              devNum,
    GT_U32             portGroup
);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsAldrinPortIf_H */



