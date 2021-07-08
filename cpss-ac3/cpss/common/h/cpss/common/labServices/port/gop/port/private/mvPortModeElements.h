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
* @file mvPortModeElements.h
*
* @brief Ln2CpuPortunit registers and tables classes declaration.
*
* @version   9
********************************************************************************
*/

#ifndef __mvPortModeElements_H
#define __mvPortModeElements_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcDefs.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

#define NA_NUM          (0xfffe)
#define PRINT_ALL_DEVS  255

typedef struct
{
    MV_HWS_PORT_INIT_PARAMS         *supModesCatalog;
    MV_HWS_PORT_FEC_MODE            *perModeFecList;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
}MV_HWS_SINGLE_PORT_STC;

typedef MV_HWS_SINGLE_PORT_STC* MV_HWS_DEVICE_PORTS_ELEMENTS;

extern MV_HWS_DEVICE_PORTS_ELEMENTS hwsDevicesPortsElementsArray[HWS_MAX_DEVICE_NUM];
extern MV_HWS_PORT_INIT_PARAMS hwsSingleEntryInitParams;

MV_HWS_PORT_INIT_PARAMS *hwsPortsParamsArrayGet
(
    GT_U8            devNum,
    GT_U32           portGroup,
    GT_U32           portNum
);

MV_HWS_PORT_INIT_PARAMS *hwsPortModeParamsGet
(
   GT_U8                    devNum,
   GT_U32                   portGroup,
   GT_U32                   portNum,
   MV_HWS_PORT_STANDARD     portMode
);

GT_STATUS hwsPortModeParamsGetToBuffer
(
   GT_U8                    devNum,
   GT_U32                   portGroup,
   GT_U32                   portNum,
   MV_HWS_PORT_STANDARD     portMode,
   MV_HWS_PORT_INIT_PARAMS  *portParamsBuffer
);

GT_STATUS hwsPortModeParamsSet
(
   GT_U8                    devNum,
   GT_U32                   portGroup,
   GT_U32                   portNum,
   MV_HWS_PORT_INIT_PARAMS  *portParams
);

GT_STATUS hwsPortsParamsCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  macNum,
    GT_U32                  pcsNum,
    GT_U8                   sdVecSize,
    GT_U8                   *sdVector
);

GT_STATUS hwsPortsElementsClose
(
    GT_U8                   devNum
);

GT_STATUS hwsInitPortsModesParam
(
    GT_U8                   devNum,
    MV_HWS_PORT_INIT_PARAMS *portModes[]
);

GT_STATUS hwsPortModeParamsSetMode
(
   GT_U8                    devNum,
   GT_U32                   portGroup,
   GT_U32                   portNum,
   MV_HWS_PORT_STANDARD     portMode
);

GT_STATUS hwsPortModeParamsSetFec
(
   GT_U8                    devNum,
   GT_U32                   portGroup,
   GT_U32                   portNum,
   MV_HWS_PORT_STANDARD     portMode,
   MV_HWS_PORT_FEC_MODE     fecMode
);

GT_BOOL hwsIsPortModeSupported
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode
);

GT_STATUS hwsPortParamsGetLanes
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      portNum,
    MV_HWS_PORT_INIT_PARAMS*    portParams
);

GT_BOOL hwsIsFecModeSupported
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_FEC_MODE    fecMode
);

GT_BOOL hwsIsQuadCgMacSupported
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portNum
);

/*============================== NOT DEFINED: MV_HWS_REDUCED_BUILD ==============================*/
#ifndef MV_HWS_REDUCED_BUILD

GT_STATUS hwsDevicePortsElementsCfg
(
    GT_U8       devNum
);

MV_HWS_PORT_INIT_PARAMS *hwsPortsElementsFindMode
(
    MV_HWS_PORT_INIT_PARAMS hwsPortSupModes[],
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  *portModeIndex
);

GT_STATUS debugPrvHwsPrintElementsDb
(
    GT_U8                   devNum,
    GT_U32                  portNum
);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __mvPortModeElements_H */


