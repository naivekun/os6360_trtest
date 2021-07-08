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
* SersesRxOptVipsIf.c
*
* DESCRIPTION:
*       VIPS IF implementation needed by Serdes Optimizer system.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
******************************************************************************/

#include "SerdesRxOptAppIf.h"
#include <api/sysConf/gtSysConf.h>
#include <api/hwIf/gtHwIf.h>
#include <common/os/gtOs.h>

/**
* @internal mvDeviceInfoGetDevType function
* @endinternal
*
* @brief   Return device type for device number.
*         Supported device types are: LionB, Xcat.
* @param[in] devNum                   - device number in the system.
*                                       Current device type or Unknown.
*/
MV_SERDES_TEST_DEV_TYPE mvDeviceInfoGetDevType
(
    GT_U32    devNum
)
{
    return Unknown;
}

/**
* @internal genRegisterSet function
* @endinternal
*
* @brief   Implement write access to device registers.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int genRegisterSet(GT_U8 devNum, GT_U32 portGroup, GT_U32 address, GT_U32 data, GT_U32 mask)
{
    return 0;
}

/**
* @internal genRegisterGet function
* @endinternal
*
* @brief   Read access to device registers.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int genRegisterGet (GT_U8 devNum, GT_U32 portGroup, GT_U32 address, GT_U32 *data, GT_U32 mask)
{
        
    return 0;
}



