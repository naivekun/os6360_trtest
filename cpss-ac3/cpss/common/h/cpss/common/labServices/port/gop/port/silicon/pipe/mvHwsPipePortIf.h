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
* @file mvHwsPipePortIf.h
*
* @brief Bobcat3 port interface header file
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsPipePortIf_H
#define __mvHwsPipePortIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>


/* number of ports in Pipe */
#define HWS_PIPE_PORTS_NUM_CNS 16

/**
* @internal hwsPipePortsElementsCfg function
* @endinternal
*
* @brief   Init Pipe port modes
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS hwsPipePortsElementsCfg
(
    GT_U8              devNum,
    GT_U32             portGroup
);

/**
* @internal hwsPipeSerdesAddrCalc function
* @endinternal
*
* @brief   Calculate serdes register address for Pipe
*
* @param[in] serdesNum                - serdes lane number
* @param[in] regAddr                  - offset of required register in SD Unit
*
* @param[out] addressPtr               - (ptr to) register address
*                                       None
*/
GT_VOID hwsPipeSerdesAddrCalc
(
    GT_UOPT         serdesNum,
    GT_UREG_DATA    regAddr,
    GT_U32          *addressPtr
);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsPipePortIf_H */



