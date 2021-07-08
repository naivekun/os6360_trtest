/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplCpssDeviceInfo.c
*
* DESCRIPTION:
*       A lua wrapper for device dependant data and functions.
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 9 $
*******************************************************************************/

#include <cpss/common/cpssTypes.h>
#include <lua.h>


/***** declarations ********/

/***** declarations ********/


/**
* @internal prvWrlDevFilterNextDevGet function
* @endinternal
*
* @brief   Return the number of the next existing device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to start from. For the first one  should be 0xFF.
*
* @param[out] nextDevNumPtr            - number of next device after devNum.
*
* @retval GT_OK                    - on success.
* @retval GT_NO_MORE               - devNum is the last device. nextDevNumPtr will be set to 0xFF.
* @retval GT_BAD_PARAM             - devNum > max device number
*/
GT_STATUS prvWrlDevFilterNextDevGet
(
    IN  GT_U8 devNum,
    OUT GT_U8 *nextDevNumPtr
);

/**
* @internal prvWrlCpssDeviceFamilyGet function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Getting of divice family.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*                                       device family
*/
CPSS_PP_FAMILY_TYPE_ENT prvWrlCpssDeviceFamilyGet
(
    IN GT_U8                    devNum
);


/**
* @internal pvrCpssDeviceArrayFromLuaGet function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Pop dev array from lua stack.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] L                        - lua state
* @param[in] L_index                  - entry lua stack index
*
* @param[out] numDevices               - nuber of taking devices
* @param[out] deviceArray[]            - taking devices
* @param[out] error_message            - error message (relevant, if GT_OK != status)
*                                       1 if array was pushed to stack otherwise 0
*/
GT_STATUS pvrCpssDeviceArrayFromLuaGet
(
    IN  lua_State               *L,
    IN  GT_32                   L_index,
    OUT GT_U8_PTR               numDevices,
    OUT GT_U8                   deviceArray[],
    OUT GT_CHAR_PTR             *error_message
);


/**
* @internal prvCpssIsDevExists function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Checks of device existance.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*                                       GT_TRUE, if exists, otherwise GT_FALSE
*/
GT_BOOL prvCpssIsDevExists
(
    IN  GT_U8                   devNum
);


/**
* @internal prvCpssMaxDeviceNumberGet function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Getting of max device number.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  None.
*
*                                       max device number
*/
GT_U8 prvCpssMaxDeviceNumberGet
(
);


/**
* @internal prvCpssPortsDevConvertDataBack function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Provides specific device and port data conversion.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] hwDevNumPtr              - HW device number
* @param[in,out] portNumPtr               - port number
* @param[in] errorMessage             - error message
*                                       1; pushed to lua stack converted device and port
*/
GT_STATUS prvCpssPortsDevConvertDataBack
(
    INOUT GT_HW_DEV_NUM         *hwDevNumPtr,
    INOUT GT_PORT_NUM           *portNumPtr,
    OUT   GT_CHAR_PTR           *errorMessage
);


/**
* @internal prvCpssIsHwDevicePortExist function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Checks of hw dev/port pair existance.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] hwDevNum                 - hardware device number
* @param[in] hwPortNum                - hardware port number
*
* @param[out] isExistsPtr              - dev/port existence property
* @param[out] errorMessagePtrPtr       - error message
*                                       GT_TRUE, if exists, otherwise GT_FALSE
*/
GT_BOOL prvCpssIsHwDevicePortExist
(
    IN  GT_HW_DEV_NUM           hwDevNum,
    IN  GT_PHYSICAL_PORT_NUM    hwPortNum,
    OUT GT_BOOL                 *isExistsPtr,
    OUT GT_CHAR_PTR             *errorMessagePtrPtr
);


