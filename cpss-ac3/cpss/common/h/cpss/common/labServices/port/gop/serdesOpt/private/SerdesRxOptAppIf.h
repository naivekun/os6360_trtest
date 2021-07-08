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
* SersesRxOptAppIf.h
*
* DESCRIPTION:
*       Application IF defintion.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
******************************************************************************/

#ifndef __serdesRxOptAppIf_H
#define __serdesRxOptAppIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtOs/gtGenTypes.h>

typedef enum
{
    Unknown,
    LionB,
    Xcat,
    BullsEye,
    Milos4

}MV_SERDES_TEST_DEV_TYPE;

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
);

#ifndef __siliconIf_H

/**
* @internal genRegisterSet function
* @endinternal
*
* @brief   Implement write access to device registers.
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - port group (core) number
* @param[in] address                  -  to access
* @param[in] data                     -  to write
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int genRegisterSet
(
    GT_U8  devNum, 
    GT_U32 portGroup,
    GT_U32 address, 
    GT_U32 data, 
    GT_U32 mask
);

/**
* @internal genRegisterGet function
* @endinternal
*
* @brief   Read access to device registers.
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - port group (core) number
* @param[in] address                  -  to access
*
* @param[out] data                     - read data
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int genRegisterGet 
(
    GT_U8 devNum, 
    GT_U32 portGroup,
    GT_U32 address, 
    GT_U32 *data, 
    GT_U32 mask
);

#endif 

#ifdef __cplusplus
}
#endif

#endif /* __serdesRxOptDevIf_H */



