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
* @file mvHwsPortPrvIf.h
*
* @brief
*
* @version   19
********************************************************************************
*/

#ifndef __mvHwsPortPrvIf_H
#define __mvHwsPortPrvIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>

/**
* @internal hwsPortFixAlign90 function
* @endinternal
*
* @brief   Run fix Align90 process on current port.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortFixAlign90
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal hwsPortGetTuneMode function
* @endinternal
*
* @brief   returns the tune mode according to port mode..
*
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortGetTuneMode
(
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_MAN_TUNE_MODE   *tuneMode,
    GT_BOOL                     *isTrainingMode
);

/**
* @internal mvHwsRebuildActiveLaneList function
* @endinternal
*
* @brief   Get SD vector.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsRebuildActiveLaneList
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                portNum,
    MV_HWS_PORT_STANDARD  portMode,
    GT_U32                *laneList
);

/**
* @internal mvHwsBuildActiveLaneList function
* @endinternal
*
* @brief   Get SD vector.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsBuildActiveLaneList
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                portNum,
    MV_HWS_PORT_STANDARD  portMode
);

/**
* @internal mvHwsDestroyActiveLaneList function
* @endinternal
*
* @brief   Free lane list allocated memory.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsDestroyActiveLaneList
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                portNum,
    MV_HWS_PORT_STANDARD  portMode
);

/*******************************************************************************
* mvHwsBuildDeviceLaneList
*
* DESCRIPTION:
*       Build the device lane list per port and per mort mode.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsBuildDeviceLaneList
(
   GT_U8    devNum,
   GT_U32   portGroup
);

/*******************************************************************************
* mvHwsDestroyDeviceLaneList
*
* DESCRIPTION:
*       Free the device lane list allocated memory.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsDestroyDeviceLaneList
(
   GT_U8    devNum,
   GT_U32   portGroup
);

/**
* @internal mvHwsBuildDeviceLaneList function
* @endinternal
*
* @brief   Build the device lane list per port and per mort mode.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsBuildDeviceLaneList
(
   GT_U8    devNum,
   GT_U32   portGroup
);

/**
* @internal mvHwsDestroyDeviceLaneList function
* @endinternal
*
* @brief   Free the device lane list allocated memory.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsDestroyDeviceLaneList
(
   GT_U8    devNum,
   GT_U32   portGroup
);

/**
* @internal mvHwsMmPcs40GConnectWa function
* @endinternal
*
* @brief   40G_Link_WA that need to add to the Create Port command at 40G mode.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMmPcs40GConnectWa
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                number
);

GT_STATUS mvHwsMmPcs40GBackWa
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode
);

GT_STATUS mvHwsMmPcs28nm40GBackWa
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode
);

GT_STATUS mvHwsXPcsConnect
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portNum
);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsPortPrvIf_H */



